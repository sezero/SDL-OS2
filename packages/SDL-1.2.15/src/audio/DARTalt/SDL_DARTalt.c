#define INCL_DOSSEMAPHORES
#define INCL_ERRORS
#define INCL_DOS
#define INCL_ERRORS
#define INCL_DOSPROCESS
#define INCL_OS2MM
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <os2me.h>
#include <SDL_events.h>

#include "SDL_config.h"

/* Output audio to nowhere... */

#include "SDL_rwops.h"
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audiomem.h"
#include "../SDL_audio_c.h"
#include "../SDL_audiodev_c.h"
#include "SDL_DARTalt.h"

#ifdef DEBUG_BUILD

#define debug(s,...) do { printf( __func__"(): "##s"\n", ##__VA_ARGS__ ); fflush(stdout); } while(0)

#define SDL_SetError(s,...) do {\
  printf( __func__"() Set error: "##s"\n", ##__VA_ARGS__ );\
  SDL_SetError( s, ##__VA_ARGS__ ); \
} while(0)

#define SDL_NotEnoughMemory() do {\
  puts( __func__"() Not enough memory" ); SDL_OutOfMemory(); \
} while(0)

#define _MCIDebugError(fn, rc) do { \
  if ( rc != MCIERR_SUCCESS ) { \
    CHAR	_MCIErr_acBuf[128]; \
    mciGetErrorString( rc, (PCHAR)&_MCIErr_acBuf, sizeof(_MCIErr_acBuf) ); \
    debug( "[%s] %s", fn, &_MCIErr_acBuf ); \
  } } while( FALSE )

#else

#define debug(s,...)
#define SDL_NotEnoughMemory() SDL_OutOfMemory()
#define _MCIDebugError(fn, rc)

#endif // DEBUG_BUILD

#define _NUM_SOUND_BUFFERS	4


LONG APIENTRY AudioEvent(ULONG ulStatus, PMCI_MIX_BUFFER pBuffer, ULONG ulFlags)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pBuffer->ulUserParm;
  ULONG			ulRC;

  if ( ulFlags != MIX_WRITE_COMPLETE )
    return 0; // It seems, return value not matter.

  ulRC = DosRequestMutexSem( pPAData->hmtxLock, SEM_INDEFINITE_WAIT );
  if ( ulRC == NO_ERROR )
  {
    if ( pPAData->ulBufInUseCnt > 0 )
      pPAData->ulBufInUseCnt--;
    DosPostEventSem( pPAData->hevBufReady );
    DosReleaseMutexSem( pPAData->hmtxLock );
  }

  return 1;
}


static VOID _MCIError(PSZ pszFunc, ULONG ulResult)
{
  CHAR			acBuf[128];

  mciGetErrorString( ulResult, (PCHAR)&acBuf, sizeof(acBuf) );
  SDL_SetError( "[%s] %s", pszFunc, &acBuf );
}

static ULONG _mixSetup(SDL_PrivateAudioData *pPAData, ULONG ulBPS, ULONG ulFreq,
                      ULONG ulChannels)
{
  ULONG			ulRC;

  pPAData->sMCIMixSetup.ulFormatTag     = MCI_WAVE_FORMAT_PCM;
  pPAData->sMCIMixSetup.ulBitsPerSample = ulBPS;
  pPAData->sMCIMixSetup.ulSamplesPerSec = ulFreq;
  pPAData->sMCIMixSetup.ulChannels      = ulChannels;
  pPAData->sMCIMixSetup.ulFormatMode    = MCI_PLAY;
  pPAData->sMCIMixSetup.ulDeviceType    = MCI_DEVTYPE_WAVEFORM_AUDIO;
  pPAData->sMCIMixSetup.pmixEvent       = AudioEvent;

  ulRC = mciSendCommand( pPAData->lAudioDeviceId, MCI_MIXSETUP,
                     MCI_WAIT | MCI_MIXSETUP_INIT, &pPAData->sMCIMixSetup, 0 );
  debug( "Setup mixer [BPS: %u, Freq.: %u, Channels: %u]: %s",
         pPAData->sMCIMixSetup.ulBitsPerSample,
         pPAData->sMCIMixSetup.ulSamplesPerSec,
         pPAData->sMCIMixSetup.ulChannels,
         ulRC == MCIERR_SUCCESS ? "SUCCESS" : "FAIL" );
  return ulRC;
}


/* This function waits until it is possible to write a full sound buffer */

static void DART_WaitAudio(SDL_AudioDevice *pDevice)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pDevice->hidden;
  HEV			hevBufReady;
  ULONG			ulRC;

  ulRC = DosRequestMutexSem( pPAData->hmtxLock, SEM_INDEFINITE_WAIT );
  if ( ulRC != NO_ERROR )
  {
    debug( "DosRequestMutexSem(), rc = %u", ulRC );
    return;
  }

  while( pPAData->ulBufInUseCnt == pPAData->sMCIBuffer.ulNumBuffers )
  {
    hevBufReady = pPAData->hevBufReady;
    DosResetEventSem( hevBufReady, &ulRC );
    DosReleaseMutexSem( pPAData->hmtxLock );

    ulRC = DosWaitEventSem( hevBufReady, SEM_INDEFINITE_WAIT );
    if ( ulRC != NO_ERROR )
    {
      debug( "DosWaitEventSem(), rc = %u", ulRC );
      return;
    }

    ulRC = DosRequestMutexSem( pPAData->hmtxLock, SEM_INDEFINITE_WAIT );
    if ( ulRC != NO_ERROR )
    {
      debug( "DosRequestMutexSem(), rc = %u", ulRC );
      return;
    }
  }

  DosReleaseMutexSem( pPAData->hmtxLock );
}

static void DART_PlayAudio(SDL_AudioDevice *pDevice)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pDevice->hidden;
  PMCI_MIX_BUFFER	pBufList;
  ULONG			ulRC;

  ulRC = DosRequestMutexSem( pPAData->hmtxLock, SEM_INDEFINITE_WAIT );
  if ( ulRC != NO_ERROR )
  {
    debug( "DosRequestMutexSem(), rc = %u", ulRC );
    return;
  }

  pBufList = pPAData->sMCIBuffer.pBufList;
  ulRC = pPAData->sMCIMixSetup.pmixWrite( pPAData->sMCIMixSetup.ulMixHandle,
                                          &pBufList[pPAData->ulNextBufIdx], 1 );
  if ( ulRC != MCIERR_SUCCESS )
    _MCIError( "pmixWrite()", ulRC );

  pPAData->ulNextBufIdx = ( pPAData->ulNextBufIdx + 1 ) %
                          pPAData->sMCIBuffer.ulNumBuffers;
  pPAData->ulBufInUseCnt++;

  DosReleaseMutexSem( pPAData->hmtxLock );
}

static Uint8 *DART_GetAudioBuf(SDL_AudioDevice *pDevice)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pDevice->hidden;
  ULONG			ulRC;
  Uint8			*puBuf;

  ulRC = DosRequestMutexSem( pPAData->hmtxLock, SEM_INDEFINITE_WAIT );
  if ( ulRC != NO_ERROR )
  {
    debug( "DosRequestMutexSem(), rc = %u", ulRC );
    return NULL;
  }

  if ( pPAData->ulBufInUseCnt == pPAData->sMCIBuffer.ulNumBuffers )
  {
    debug( "No buffers available" );
    puBuf = NULL;
  }
  else
  {
    PMCI_MIX_BUFFER	pBufList = pPAData->sMCIBuffer.pBufList;
    puBuf = (Uint8 *)pBufList[pPAData->ulNextBufIdx].pBuffer;
  }

  DosReleaseMutexSem( pPAData->hmtxLock );
  return puBuf;
}

static void DART_WaitDone(SDL_AudioDevice *pDevice)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pDevice->hidden;
  HEV			hevBufReady;
  ULONG			ulRC;
  ULONG			ulIdx;

  debug( "Enter" );
  ulRC = DosRequestMutexSem( pPAData->hmtxLock, SEM_INDEFINITE_WAIT );
  if ( ulRC != NO_ERROR )
  {
    debug( "DosRequestMutexSem(), rc = %u", ulRC );
    return;
  }

  for( ulIdx = 0;
       ulIdx < pPAData->sMCIBuffer.ulNumBuffers && pPAData->ulBufInUseCnt > 0;
       ulIdx++ )
  {
    hevBufReady = pPAData->hevBufReady;
    DosResetEventSem( hevBufReady, &ulRC );
    DosReleaseMutexSem( pPAData->hmtxLock );

    ulRC = DosWaitEventSem( hevBufReady, 1000 );
    if ( ulRC != NO_ERROR )
    {
      debug( "DosWaitEventSem(), rc = %u", ulRC );
      return;
    }

    ulRC = DosRequestMutexSem( pPAData->hmtxLock, SEM_INDEFINITE_WAIT );
    if ( ulRC != NO_ERROR )
    {
      debug( "DosRequestMutexSem(), rc = %u", ulRC );
      return;
    }
  }

  DosReleaseMutexSem( pPAData->hmtxLock );
  debug( "Done" );
}

static int DART_OpenAudio(SDL_AudioDevice *pDevice, SDL_AudioSpec *pSDLSpec)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pDevice->hidden;
  MCI_AMP_OPEN_PARMS	sMCIAmpOpen = { 0 };
  ULONG			ulRC;
  PMCI_MIX_BUFFER	pBufList;
  ULONG			ulIdx;
  Uint16		uiFormat;
  MCI_GENERIC_PARMS	sMCIGeneric = { 0 };

  for( uiFormat = SDL_FirstAudioFormat( pSDLSpec->format );
       uiFormat != 0; uiFormat = SDL_NextAudioFormat() )
  {
    if ( uiFormat == AUDIO_U8 || uiFormat == AUDIO_S16LSB )
      break;
  }

  if ( uiFormat == 0 ) {
    SDL_SetError( "Unsupported audio format" );
    return -1;
  }
  pSDLSpec->format = uiFormat;

  ulRC = DosCreateMutexSem( NULL, &pPAData->hmtxLock, 0, FALSE );
  if ( ulRC != NO_ERROR )
  {
    SDL_SetError( "DosCreateMutexSem(), rc = %u", ulRC );
    return -1;
  }

  ulRC = DosCreateEventSem( NULL, &pPAData->hevBufReady, 0, FALSE );
  if ( ulRC != NO_ERROR )
  {
    SDL_SetError( "DosCreateEventSem(), rc = %u", ulRC );
    DART_CloseAudio( pDevice );
    return -1;
  }

  memset( &sMCIAmpOpen, 0, sizeof(MCI_AMP_OPEN_PARMS) );
  sMCIAmpOpen.usDeviceID = 0;
  sMCIAmpOpen.pszDeviceType = (PSZ)MCI_DEVTYPE_AUDIO_AMPMIX;
  ulRC = mciSendCommand( 0, MCI_OPEN, MCI_WAIT | MCI_OPEN_TYPE_ID,
                         &sMCIAmpOpen,  0 );
  if ( ulRC != MCIERR_SUCCESS )
  {
    _MCIError( "MCI_OPEN", ulRC );
    DART_CloseAudio( pDevice );
    return -1;
  }
  pPAData->lAudioDeviceId = sMCIAmpOpen.usDeviceID;

  uiFormat &= 0xFF; // SDL audio format -> bits per sample
  ulRC = _mixSetup( pPAData, uiFormat, pSDLSpec->freq, pSDLSpec->channels );
  // If requested audio format not supported - reduce parameters
  if ( ( ulRC != MCIERR_SUCCESS ) && ( pSDLSpec->freq > 44100 ) )
  {
    ulRC = _mixSetup( pPAData, uiFormat, 44100, pSDLSpec->channels );
    if ( ( ulRC != MCIERR_SUCCESS ) && ( pSDLSpec->channels > 2 ) )
    {
      ulRC = _mixSetup( pPAData, uiFormat, pSDLSpec->freq, 2 );
      if ( ( ulRC != MCIERR_SUCCESS ) && ( pSDLSpec->freq > 44100 ) )
        ulRC = _mixSetup( pPAData, uiFormat, 44100, 2 );
    }
  }

  if ( ulRC != MCIERR_SUCCESS )
  {
    _MCIError( "MCI_MIXSETUP, MCI_MIXSETUP_INIT", ulRC );
    pPAData->sMCIMixSetup.ulBitsPerSample = 0;
    DART_CloseAudio( pDevice );
    return -1;
  }

  pSDLSpec->freq = pPAData->sMCIMixSetup.ulSamplesPerSec;
  pSDLSpec->channels = pPAData->sMCIMixSetup.ulChannels;
  /* Update the fragment size as size in bytes and "silence" value */
  SDL_CalculateAudioSpec( pSDLSpec );

  pBufList = SDL_malloc( _NUM_SOUND_BUFFERS * sizeof(MCI_MIX_BUFFER) );
  if ( pBufList == NULL )
  {
    SDL_NotEnoughMemory();
    DART_CloseAudio( pDevice );
    return NULL;
  }

  pPAData->sMCIBuffer.ulBufferSize = pSDLSpec->size;
  pPAData->sMCIBuffer.ulNumBuffers = _NUM_SOUND_BUFFERS;
  pPAData->sMCIBuffer.pBufList = pBufList;
  debug( "Buffer size: %u, Number: %u",
         pPAData->sMCIBuffer.ulBufferSize, pPAData->sMCIBuffer.ulNumBuffers );
  ulRC = mciSendCommand( sMCIAmpOpen.usDeviceID, MCI_BUFFER,
                     MCI_WAIT | MCI_ALLOCATE_MEMORY, &pPAData->sMCIBuffer, 0 );
  if ( ulRC != MCIERR_SUCCESS )
  {
    _MCIError( "MCI_BUFFER", ulRC );
    SDL_free( pBufList );
    pPAData->sMCIBuffer.pBufList = NULL;
    DART_CloseAudio( pDevice );
    return -1;
  }

  // Fill all device buffers with data
  for( ulIdx = 0; ulIdx < pPAData->sMCIBuffer.ulNumBuffers; ulIdx++ )
  {
    pBufList[ulIdx].ulFlags        = 0;
    pBufList[ulIdx].ulBufferLength = pPAData->sMCIBuffer.ulBufferSize;
    pBufList[ulIdx].ulUserParm     = (ULONG)pPAData;

    memset( pBufList[ulIdx].pBuffer, pSDLSpec->silence,
            pPAData->sMCIBuffer.ulBufferSize );
  }

  /* grab exclusive rights to device instance (not entire device) */
  ulRC = mciSendCommand( sMCIAmpOpen.usDeviceID, MCI_ACQUIREDEVICE,
                         MCI_EXCLUSIVE_INSTANCE, &sMCIGeneric, 0 );
  if ( ulRC != MCIERR_SUCCESS )
    _MCIError( "MCI_ACQUIREDEVICE", ulRC );

  return 0;
}

static void DART_CloseAudio(SDL_AudioDevice *pDevice)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pDevice->hidden;
  MCI_GENERIC_PARMS	sMCIGeneric = { 0 };
  ULONG			ulRC;

  if ( pPAData->lAudioDeviceId != -1 )
  {
    ulRC = mciSendCommand( pPAData->lAudioDeviceId, MCI_STOP, MCI_WAIT,
                           &sMCIGeneric, 0);
    _MCIDebugError( "MCI_STOP", ulRC );

    ulRC = mciSendCommand( pPAData->lAudioDeviceId, MCI_RELEASEDEVICE,
                           MCI_RETURN_RESOURCE, &sMCIGeneric, 0 );
    _MCIDebugError( "MCI_RELEASEDEVICE", ulRC );
  }

  if ( pPAData->sMCIBuffer.pBufList != NULL )
  {
    ulRC = mciSendCommand( pPAData->lAudioDeviceId, MCI_BUFFER,
                   MCI_WAIT | MCI_DEALLOCATE_MEMORY, &pPAData->sMCIBuffer, 0 );
    _MCIDebugError( "MCI_BUFFER, MCI_DEALLOCATE_MEMORY", ulRC );
    SDL_free( pPAData->sMCIBuffer.pBufList );
    pPAData->sMCIBuffer.pBufList = NULL;
  }

  if ( pPAData->sMCIMixSetup.ulBitsPerSample != 0 )
  {
    ulRC = mciSendCommand( pPAData->lAudioDeviceId, MCI_MIXSETUP,
                     MCI_WAIT | MCI_MIXSETUP_DEINIT, &pPAData->sMCIMixSetup, 0 );
    _MCIDebugError( "MCI_MIXSETUP, MCI_MIXSETUP_DEINIT", ulRC );
    pPAData->sMCIMixSetup.ulBitsPerSample = 0; // We use it as mixer initialized sine
  }

  if ( pPAData->lAudioDeviceId != -1 )
  {
    ulRC = mciSendCommand( pPAData->lAudioDeviceId, MCI_CLOSE, MCI_WAIT,
                           &sMCIGeneric, 0 );
    _MCIDebugError( "MCI_CLOSE", ulRC );
    pPAData->lAudioDeviceId = -1;
  }

  if ( pPAData->hmtxLock != NULLHANDLE )
  {
    DosCloseEventSem( pPAData->hevBufReady );
    pPAData->hevBufReady = NULLHANDLE;
  }

  if ( pPAData->hmtxLock != NULLHANDLE )
  {
    DosCloseMutexSem( pPAData->hmtxLock );
    pPAData->hmtxLock = NULLHANDLE;
  }

  pPAData->ulNextBufIdx = 0;
  pPAData->ulBufInUseCnt = 0;
}


/* Audio driver bootstrap functions */

static int Audio_Available(void)
{
  return 1;
}

static void Audio_DeleteDevice(SDL_AudioDevice *pDevice)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pDevice->hidden;

  DART_CloseAudio( pDevice );
  SDL_free( pPAData );
  SDL_free( pDevice );
}

static SDL_AudioDevice *Audio_CreateDevice(int iDevIndex)
{
  SDL_AudioDevice	*pDevice;
  SDL_PrivateAudioData	*pPAData;

  // Create private audio data

  pPAData = SDL_calloc( 1, sizeof(SDL_PrivateAudioData) );
  if ( pPAData == NULL )
  {
    SDL_NotEnoughMemory();
    return NULL;
  }
  pPAData->lAudioDeviceId = -1;

  // Create SDL audio device

  pDevice = (SDL_AudioDevice *)SDL_calloc( 1, sizeof(SDL_AudioDevice) );
  if ( pDevice == NULL )
  {
    SDL_NotEnoughMemory();
    DosCloseMutexSem( pPAData->hmtxLock );
    SDL_free( pPAData );
    return NULL;
  }
  pDevice->hidden = pPAData;

  /* Set the function pointers */
  pDevice->OpenAudio   = DART_OpenAudio;
  pDevice->WaitAudio   = DART_WaitAudio;
  pDevice->PlayAudio   = DART_PlayAudio;
  pDevice->GetAudioBuf = DART_GetAudioBuf;
  pDevice->WaitDone    = DART_WaitDone;
  pDevice->CloseAudio  = DART_CloseAudio;
  pDevice->free        = Audio_DeleteDevice;

  return pDevice;
}

AudioBootStrap DARTALT_bootstrap =
  { "DART", "OS/2 DART", Audio_Available, Audio_CreateDevice };
