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

int os2iniGetBool(char *pszSwitch, int iDefault);
void os2iniOpen();

LONG APIENTRY AudioEvent(ULONG ulStatus, PMCI_MIX_BUFFER pMixBuffer,
                         ULONG ulFlags)
{
  SDL_PrivateAudioData	*pPAData =
                           (SDL_PrivateAudioData *)pMixBuffer->ulUserParm;
  ULONG			ulRC;
  PINPUTBUF		pInputBuf;

  ulRC = DosRequestMutexSem( pPAData->hmtxLock, 1000/*SEM_INDEFINITE_WAIT*/ );
  if ( ulRC != NO_ERROR )
    debug( "DosRequestMutexSem(), rc = %u", ulRC );
  else
  {
    HEV			hevBufReady = pPAData->hevBufReady;
 
    if ( ulFlags != MIX_WRITE_COMPLETE )
      debug( "flags is 0x%X", ulFlags );

    if ( pPAData->lAudioDeviceId != -1 ) // Prevent work in close state
    {
      pInputBuf = pPAData->pReadyBuf;
      if ( pInputBuf != NULL )
      {
        // Copy data from input buffer to the mixer's buffer.
        memcpy( pMixBuffer->pBuffer, &pInputBuf->acData,
                pPAData->sMCIBuffer.ulBufferSize );

        // Cut readed input buffer from the list "ready"
        if ( pInputBuf->pNext == NULL )
          pPAData->ppLastReadyBuf = &pPAData->pReadyBuf;
        pPAData->pReadyBuf = pInputBuf->pNext;

        // Add it to the list "free"
        pInputBuf->pNext = NULL;
        *pPAData->ppLastFreeBuf = pInputBuf;
        pPAData->ppLastFreeBuf = &pInputBuf->pNext;
      }
      else
        debug( "Have no input buffers" );

      // Return mixer's buffer to play.
      ulRC = pPAData->sMCIMixSetup.pmixWrite(
                  pPAData->sMCIMixSetup.ulMixHandle, pMixBuffer, 1 );
      if ( ulRC != MCIERR_SUCCESS )
        debug( "pmixWrite() failed" );

      hevBufReady = pPAData->hevBufReady;
      DosReleaseMutexSem( pPAData->hmtxLock );
    }

    ulRC = DosPostEventSem( hevBufReady );
    if ( ulRC != NO_ERROR && ulRC != ERROR_ALREADY_POSTED )
      debug( "DosPostEventSem(%u), rc = %u", hevBufReady, ulRC );
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
  ULONG			ulRC;
  HEV			hevBufReady;
 
  ulRC = DosRequestMutexSem( pPAData->hmtxLock, 1000/*SEM_INDEFINITE_WAIT*/ );
  if ( ulRC != NO_ERROR )
  {
    debug( "#1 DosRequestMutexSem(), rc = %u", ulRC );
    return;
  }
  hevBufReady = pPAData->hevBufReady;

  // Wait until no buffers in list "free".
  while( pPAData->hevBufReady != NULLHANDLE && pPAData->pFreeBuf == NULL )
  {
    DosReleaseMutexSem( pPAData->hmtxLock );

    ulRC = DosWaitEventSem( hevBufReady, 2000/*SEM_INDEFINITE_WAIT*/ );
    if ( ulRC != NO_ERROR )
    {
      debug( "DosWaitEventSem(), rc = %u", ulRC );
      return;
    }

    ulRC = DosRequestMutexSem( pPAData->hmtxLock, 1000/*SEM_INDEFINITE_WAIT*/ );
    if ( ulRC != NO_ERROR )
    {
      debug( "#2 DosRequestMutexSem(), rc = %u", ulRC );
      return;
    }
  }

  DosReleaseMutexSem( pPAData->hmtxLock );
}

static Uint8 *DART_GetAudioBuf(SDL_AudioDevice *pDevice)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pDevice->hidden;
  ULONG			ulRC;
  Uint8			*puBuf = NULL;
 
  ulRC = DosRequestMutexSem( pPAData->hmtxLock, 1000/*SEM_INDEFINITE_WAIT*/ );
  if ( ulRC != NO_ERROR )
    debug( "DosRequestMutexSem(), rc = %u", ulRC );
  else
  {
    // Return pointer to the data of first free buffer (if we have it)
    if ( pPAData->pFreeBuf != NULL )
      puBuf = (Uint8 *)&pPAData->pFreeBuf->acData;

    DosReleaseMutexSem( pPAData->hmtxLock );
  }

  return puBuf;
}

static void DART_PlayAudio(SDL_AudioDevice *pDevice)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pDevice->hidden;
  ULONG			ulRC;
  PINPUTBUF		pInputBuf;

  ulRC = DosRequestMutexSem( pPAData->hmtxLock, 1000/*SEM_INDEFINITE_WAIT*/ );
  if ( ulRC != NO_ERROR )
    debug( "DosRequestMutexSem(), rc = %u", ulRC );
  else
  {
    pInputBuf = pPAData->pFreeBuf;
    if ( pInputBuf == NULL )
      debug( "Have no free buffers" );
    else
    {
      // Cut readed input buffer from the list "free"
      if ( pInputBuf->pNext == NULL )
        pPAData->ppLastFreeBuf = &pPAData->pFreeBuf;
      pPAData->pFreeBuf = pInputBuf->pNext;

      // Add it to the list "ready"
      pInputBuf->pNext = NULL;
      *pPAData->ppLastReadyBuf = pInputBuf;
      pPAData->ppLastReadyBuf = &pInputBuf->pNext;
    }

    DosReleaseMutexSem( pPAData->hmtxLock );
  }
}

static void DART_WaitDone(SDL_AudioDevice *pDevice)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pDevice->hidden;
  ULONG			ulRC;
  HEV			hevBufReady;
 
  debug( "Enter" );
  ulRC = DosRequestMutexSem( pPAData->hmtxLock, 1000/*SEM_INDEFINITE_WAIT*/ );
  if ( ulRC != NO_ERROR )
  {
    debug( "#1 DosRequestMutexSem(), rc = %u", ulRC );
    return;
  }
  hevBufReady = pPAData->hevBufReady;

  // Look forward to when all the input buffers have been readed
  while( pPAData->pReadyBuf != NULL )
  {
    DosReleaseMutexSem( pPAData->hmtxLock );

    ulRC = DosWaitEventSem( hevBufReady, 2000/*SEM_INDEFINITE_WAIT*/ );
    if ( ulRC != NO_ERROR )
    {
      debug( "DosWaitEventSem(), rc = %u", ulRC );
      return;
    }

    ulRC = DosRequestMutexSem( pPAData->hmtxLock, 1000/*SEM_INDEFINITE_WAIT*/ );
    if ( ulRC != NO_ERROR )
    {
      debug( "#2 DosRequestMutexSem(), rc = %u", ulRC );
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
  BOOL			fSharedDevice = os2iniGetBool( "AudioShared", FALSE );
  PINPUTBUF		pInputBuf;

  debug( "Sound device is %s", fSharedDevice ? "shared" : "not shared" );

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

  ulRC = DosCreateEventSem( NULL, &pPAData->hevBufReady, DCE_AUTORESET, FALSE );
  if ( ulRC != NO_ERROR )
  {
    SDL_SetError( "DosCreateEventSem(), rc = %u", ulRC );
    DART_CloseAudio( pDevice );
    return -1;
  }

  memset( &sMCIAmpOpen, 0, sizeof(MCI_AMP_OPEN_PARMS) );
  sMCIAmpOpen.usDeviceID = 0;
  sMCIAmpOpen.pszDeviceType = (PSZ)MCI_DEVTYPE_AUDIO_AMPMIX;
  ulRC = mciSendCommand(
           0, MCI_OPEN,
           fSharedDevice ? MCI_WAIT | MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE :
                           MCI_WAIT | MCI_OPEN_TYPE_ID,
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

  // Allocate input buffers
  pPAData->ppLastFreeBuf = &pPAData->pFreeBuf;
  for( ulIdx = 0; ulIdx < DART_INPUT_BUFFERS; ulIdx++ )
  {
    pInputBuf = SDL_malloc( sizeof(INPUTBUF) - 1 + pSDLSpec->size );
    if ( pInputBuf == NULL )
    {
      SDL_NotEnoughMemory();
      DART_CloseAudio( pDevice );
      return NULL;
    }
    pInputBuf->pNext = NULL;
    *pPAData->ppLastFreeBuf = pInputBuf;
    pPAData->ppLastFreeBuf = &pInputBuf->pNext;
  }
  pPAData->ppLastReadyBuf = &pPAData->pReadyBuf;

  pBufList = SDL_malloc( DART_AUDIO_BUFFERS * sizeof(MCI_MIX_BUFFER) );
  if ( pBufList == NULL )
  {
    SDL_NotEnoughMemory();
    DART_CloseAudio( pDevice );
    return NULL;
  }

  pPAData->sMCIBuffer.ulBufferSize = pSDLSpec->size;
  pPAData->sMCIBuffer.ulNumBuffers = DART_AUDIO_BUFFERS;
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
  for( ulIdx = 0; ulIdx < DART_AUDIO_BUFFERS; ulIdx++ )
  {
    pBufList[ulIdx].ulFlags        = 0;
    pBufList[ulIdx].ulBufferLength = pPAData->sMCIBuffer.ulBufferSize;
    pBufList[ulIdx].ulUserParm     = (ULONG)pPAData;

    memset( pBufList[ulIdx].pBuffer, pSDLSpec->silence,
            pPAData->sMCIBuffer.ulBufferSize );
  }

/*
  if ( !fSharedDevice )
  {
    MCI_GENERIC_PARMS	sMCIGeneric = { 0 };

    // Grab exclusive rights to device instance (not entire device)
    ulRC = mciSendCommand( sMCIAmpOpen.usDeviceID, MCI_ACQUIREDEVICE,
                           MCI_EXCLUSIVE_INSTANCE, &sMCIGeneric, 0 );
    if ( ulRC != MCIERR_SUCCESS )
      _MCIError( "MCI_ACQUIREDEVICE", ulRC );
  }
*/

  // Write buffers to start
  ulRC = pPAData->sMCIMixSetup.pmixWrite( pPAData->sMCIMixSetup.ulMixHandle,
                                          pBufList, DART_AUDIO_BUFFERS );
  if ( ulRC != MCIERR_SUCCESS )
    _MCIError( "pmixWrite()", ulRC );

  debug( "Done" );
  return 0;
}

static void DART_CloseAudio(SDL_AudioDevice *pDevice)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pDevice->hidden;
  MCI_GENERIC_PARMS	sMCIGeneric = { 0 };
  ULONG			ulRC;
  PINPUTBUF		pInputBuf;

  if ( pPAData->lAudioDeviceId == -1 )
    debug( "Audio was not opened or already closed" );
  else
  {
    LONG	lAudioDeviceId = pPAData->lAudioDeviceId;

    debug( "Close device" );

    pPAData->lAudioDeviceId = -1;
    ulRC = mciSendCommand( lAudioDeviceId, MCI_STOP, MCI_WAIT,
                           &sMCIGeneric, 0);
    _MCIDebugError( "MCI_STOP", ulRC );

    ulRC = mciSendCommand( lAudioDeviceId, MCI_RELEASEDEVICE,
                           MCI_RETURN_RESOURCE, &sMCIGeneric, 0 );
    _MCIDebugError( "MCI_RELEASEDEVICE", ulRC );

    if ( pPAData->sMCIMixSetup.ulBitsPerSample != 0 )
    {
      ulRC = mciSendCommand( lAudioDeviceId, MCI_MIXSETUP,
                       MCI_WAIT | MCI_MIXSETUP_DEINIT, &pPAData->sMCIMixSetup, 0 );
      _MCIDebugError( "MCI_MIXSETUP, MCI_MIXSETUP_DEINIT", ulRC );
      pPAData->sMCIMixSetup.ulBitsPerSample = 0; // We use it as mixer initialized sine
    }

    if ( pPAData->sMCIBuffer.pBufList != NULL )
    {
      ulRC = mciSendCommand( lAudioDeviceId, MCI_BUFFER,
                     MCI_WAIT | MCI_DEALLOCATE_MEMORY, &pPAData->sMCIBuffer, 0 );
      _MCIDebugError( "MCI_BUFFER, MCI_DEALLOCATE_MEMORY", ulRC );
      SDL_free( pPAData->sMCIBuffer.pBufList );
      pPAData->sMCIBuffer.pBufList = NULL;
    }

    ulRC = mciSendCommand( lAudioDeviceId, MCI_CLOSE, MCI_WAIT,
                           &sMCIGeneric, 0 );
    _MCIDebugError( "MCI_CLOSE", ulRC );
    debug( "Done" );
  }

  if ( pPAData->hmtxLock != NULLHANDLE )
  {
    HEV		hevBufReady = pPAData->hevBufReady;

    pPAData->hevBufReady = NULLHANDLE;
    DosPostEventSem( hevBufReady ); // Prevent lock DART_WaitAudio()
    DosCloseEventSem( hevBufReady );
  }

  if ( pPAData->hmtxLock != NULLHANDLE )
  {
    DosCloseMutexSem( pPAData->hmtxLock );
    pPAData->hmtxLock = NULLHANDLE;
  }

  while( pPAData->pReadyBuf != NULL )
  {
    pInputBuf = pPAData->pReadyBuf->pNext;
    SDL_free( pPAData->pReadyBuf );
    pPAData->pReadyBuf = pInputBuf;
  }

  while( pPAData->pFreeBuf != NULL )
  {
    pInputBuf = pPAData->pFreeBuf->pNext;
    SDL_free( pPAData->pFreeBuf );
    pPAData->pFreeBuf = pInputBuf;
  }
}


/* Audio driver bootstrap functions */

static int Audio_Available(void)
{
  return 1;
}

static void Audio_DeleteDevice(SDL_AudioDevice *pDevice)
{
  SDL_PrivateAudioData	*pPAData = (SDL_PrivateAudioData *)pDevice->hidden;

  debug( "Enter" );
  DART_CloseAudio( pDevice );
  SDL_free( pPAData );
  SDL_free( pDevice );
  debug( "Done" );
}

static SDL_AudioDevice *Audio_CreateDevice(int iDevIndex)
{
  SDL_AudioDevice	*pDevice;
  SDL_PrivateAudioData	*pPAData;

  os2iniOpen();

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
