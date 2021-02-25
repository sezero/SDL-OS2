/* The optimized version of SDL_audio.c for OS/2. It does not used an
 * additional thread.
 *
 * Digi, 2014
 */

#include "SDL_config.h"

#if SDL_AUDIO_DRIVER_DARTALT

#include "SDL.h"
#define INCL_DOSSEMAPHORES
#define INCL_ERRORS
#define INCL_DOS
#define INCL_ERRORS
#define INCL_OS2MM
#include <os2.h>
#include <os2me.h>

#define AUDIO_NUM_SOUND_BUFFERS	4

Uint16 SDL_AudioFmt = AUDIO_S16; /* global for SDL_mixer.c. */

static HMTX			hmtxLock = NULLHANDLE;
static BOOL			fPause = FALSE;
static USHORT			usAudioDeviceId = 0;
static void (SDLCALL *fnCallback)(void *userdata, Uint8 *stream, int len);
static void			*pUserData;
static MCI_MIXSETUP_PARMS	sMCIMixSetup = { 0 };
static MCI_BUFFER_PARMS		sMCIBuffer = { 0 };
static SDL_AudioCVT		sOnFlyCVT = { 0 };
static BYTE			bSilence = 0; /* silence value for real HW mode */

#ifdef DEBUG_BUILD

#define debug(s,...) do { printf(__func__"(): "##s"\n", ##__VA_ARGS__); fflush(stdout); } while(0)

#define SDL_SetError(s,...) do {                                   \
  printf(__func__ "() Set error: " ##s "\n", ##__VA_ARGS__);       \
  SDL_SetError(s, ##__VA_ARGS__);                                  \
} while(0)

#define SDL_NotEnoughMemory() do {                                 \
  puts(__func__ "() Not enough memory"); SDL_OutOfMemory();        \
} while(0)

#define _MCIDebugError(fn, rc) do { \
  if (rc != MCIERR_SUCCESS) { \
      char _MCIErr_acBuf[128]; \
      mciGetErrorString(rc, _MCIErr_acBuf, sizeof(_MCIErr_acBuf)); \
      debug("[%s] %s", fn, _MCIErr_acBuf);                         \
  } } while(FALSE)

#else

#define debug(s,...)            do {} while (0)
#define SDL_NotEnoughMemory() SDL_OutOfMemory()
#define _MCIDebugError(fn, rc)  do {} while (0)

#endif /* DEBUG_BUILD */

static LONG APIENTRY AudioEvent(ULONG ulStatus, PMCI_MIX_BUFFER pBuffer, ULONG ulFlags);


static ULONG _getEnvULong(const char *name, ULONG ulMax, ULONG ulDefault)
{
    ULONG   ulValue;
    char*   end;
    char*   envval = SDL_getenv(name);

    if (envval == NULL)
        return ulDefault;

    ulValue = SDL_strtoul(envval, &end, 10);
    return (end == envval) || (ulValue > ulMax)? ulDefault : ulMax;
}

static VOID _MCIError(PCSZ pszFunc, ULONG ulResult)
{
    CHAR acBuf[128];

    mciGetErrorString(ulResult, acBuf, sizeof(acBuf));
    SDL_SetError("[%s] %s", pszFunc, acBuf);
}

static ULONG _mixSetup(ULONG ulBPS, ULONG ulFreq, ULONG ulChannels)
{
    ULONG ulRC;

    sMCIMixSetup.ulFormatTag     = MCI_WAVE_FORMAT_PCM;
    sMCIMixSetup.ulBitsPerSample = ulBPS;
    sMCIMixSetup.ulSamplesPerSec = ulFreq;
    sMCIMixSetup.ulChannels      = ulChannels;
    sMCIMixSetup.ulFormatMode    = MCI_PLAY;
    sMCIMixSetup.ulDeviceType    = MCI_DEVTYPE_WAVEFORM_AUDIO;
    sMCIMixSetup.pmixEvent       = AudioEvent;

    ulRC = mciSendCommand(usAudioDeviceId, MCI_MIXSETUP,
                          MCI_WAIT | MCI_MIXSETUP_INIT, &sMCIMixSetup, 0);
    debug("Setup mixer [BPS: %u, Freq.: %u, Channels: %u]: %s",
          sMCIMixSetup.ulBitsPerSample, sMCIMixSetup.ulSamplesPerSec,
          sMCIMixSetup.ulChannels, ulRC == MCIERR_SUCCESS ? "SUCCESS" : "FAIL");
    return ulRC;
}

static LONG APIENTRY AudioEvent(ULONG ulStatus, PMCI_MIX_BUFFER pBuffer, ULONG ulFlags)
{
    ULONG ulRC;

    if (ulFlags != MIX_WRITE_COMPLETE) {
        debug("flags = 0x%X", ulFlags);
        return 0; /* It seems, return value not matter */
    }

    ulRC = DosRequestMutexSem(hmtxLock, 2000);
    if (ulRC != NO_ERROR) {
        debug("DosRequestMutexSem(), rc = %u", ulRC);
    }

    if (fPause || ulRC != NO_ERROR) {
        DosReleaseMutexSem(hmtxLock);
        memset(pBuffer->pBuffer, bSilence, pBuffer->ulBufferLength);
    } else if (sOnFlyCVT.needed) {
        if (sOnFlyCVT.buf == NULL) {
            debug("Converting buffer not allocated");
            DosReleaseMutexSem(hmtxLock);
            return 0;
        }

        memset(sOnFlyCVT.buf, bSilence, sOnFlyCVT.len);
        fnCallback(pUserData, sOnFlyCVT.buf, sOnFlyCVT.len);
        SDL_ConvertAudio(&sOnFlyCVT);

        DosReleaseMutexSem(hmtxLock);

        /* sOnFlyCVT.len_cvt must be less or equal cbBuffer */
        if (sOnFlyCVT.len_cvt > pBuffer->ulBufferLength) {
            debug("Converted data too big");
            sOnFlyCVT.len_cvt = pBuffer->ulBufferLength;
        }
        memcpy(pBuffer->pBuffer, sOnFlyCVT.buf, sOnFlyCVT.len_cvt);
    } else {
        memset(pBuffer->pBuffer, bSilence, pBuffer->ulBufferLength);
        fnCallback(pUserData, (PUCHAR)pBuffer->pBuffer, pBuffer->ulBufferLength);

        DosReleaseMutexSem(hmtxLock);
    }

    ulRC = sMCIMixSetup.pmixWrite(sMCIMixSetup.ulMixHandle, pBuffer, 1);
    if (ulRC != MCIERR_SUCCESS) {
        debug("pmixWrite() failed");
    }

    return 0;
}

int SDLCALL SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    ULONG ulRC;
    PSZ   pszEnv;
    ULONG ulBPS;
    ULONG ulIdx;
    MCI_AMP_OPEN_PARMS sMCIAmpOpen = { 0 };
    BOOL  fSharedDevice = _getEnvULong("SDL_AUDIO_SHARE", 1, 0);

    debug("Requested: Freq.: %u, Channels: %u, format: 0x%X", desired->freq,
          desired->channels, desired->format);

    if (hmtxLock != NULLHANDLE) {
        SDL_SetError("Audio device is already opened");
        return  -1;
    }

    /* Verify some parameters */
    if (desired->callback == NULL) {
        SDL_SetError("SDL_OpenAudio() passed a NULL callback");
        return -1;
    }

    if (desired->freq == 0) {
        pszEnv = SDL_getenv("SDL_AUDIO_FREQUENCY");
        if (pszEnv != NULL)
            desired->freq = SDL_atoi(pszEnv);
    }
    if (desired->freq == 0)
        desired->freq = 22050;

    if (desired->format == 0)
        desired->format = AUDIO_U16;

    if (desired->channels == 0) {
        pszEnv = SDL_getenv("SDL_AUDIO_CHANNELS");
        if (pszEnv)
            desired->channels = (Uint8)SDL_atoi(pszEnv);
    }

    switch (desired->channels) {
    case 0:
        desired->channels = 2;
    case 1:	/* Mono */
    case 2:	/* Stereo */
    case 4:	/* surround */
    case 6:	/* surround with center and lfe */
      break;
    default:
        SDL_SetError("1 (mono) and 2 (stereo) channels supported");
        return -1 ;
    }

    if (desired->samples == 0) {
        pszEnv = SDL_getenv("SDL_AUDIO_SAMPLES");
        if (pszEnv)
            desired->samples = (Uint16)SDL_atoi(pszEnv);
    }
    if (desired->samples == 0) {
        /* Pick a default of ~46 ms at desired frequency */
        ULONG ulSamples = (desired->freq / 1000) * 46;
        ULONG ulPower2 = 1;

        while (ulPower2 < ulSamples) {
            ulPower2 <<= 1;
        }
        desired->samples = ulPower2;
    }

    desired->size = ((desired->format & 0xFF) / 8) * desired->channels *
                                                     desired->samples;
    desired->silence = (desired->format == AUDIO_U8)? 0x80 : 0;

    /* Store callback user function and user data */
    fnCallback = desired->callback;
    pUserData = desired->userdata;

    /* Initialization */
    ulRC = DosCreateMutexSem(NULL, &hmtxLock, 0, FALSE);
    if (ulRC != NO_ERROR) {
        SDL_SetError("DosRequestMutexSem(), rc = %u", ulRC);
        return -1;
    }

    /* Open audio device */
    sMCIAmpOpen.usDeviceID = 0;
    sMCIAmpOpen.pszDeviceType = (PSZ)MCI_DEVTYPE_AUDIO_AMPMIX;
    ulRC = mciSendCommand(0, MCI_OPEN,
          (fSharedDevice)? MCI_WAIT | MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE :
                           MCI_WAIT | MCI_OPEN_TYPE_ID,
           &sMCIAmpOpen,  0);
    if (ulRC != MCIERR_SUCCESS) {
        _MCIError("MCI_OPEN", ulRC);
        DosCloseMutexSem(hmtxLock);
        hmtxLock = NULLHANDLE;
        usAudioDeviceId = 0;
        return -1;
    }
    usAudioDeviceId = sMCIAmpOpen.usDeviceID;

    /* Setup mixer */
    ulBPS = desired->format & 0xFF; /* SDL audio format -> bits per sample */
    ulRC = _mixSetup(ulBPS, desired->freq, desired->channels);

    /* If requested audio format not supported - reduce parameters */
    if (ulRC != MCIERR_SUCCESS && desired->freq > 44100) {
        ulRC = _mixSetup(ulBPS, 44100, desired->channels);
        if (ulRC != MCIERR_SUCCESS && desired->channels > 2) {
            ulRC = _mixSetup(ulBPS, desired->freq, 2);
            if (ulRC != MCIERR_SUCCESS && desired->freq > 44100)
                ulRC = _mixSetup(ulBPS, 44100, 2);
        }
    }

    if (ulRC != MCIERR_SUCCESS) {
        _MCIError("MCI_MIXSETUP, MCI_MIXSETUP_INIT", ulRC);
        sMCIMixSetup.ulBitsPerSample = 0;
        SDL_AudioQuit();
        return -1;
    }

    if (sMCIMixSetup.ulBitsPerSample == 8) {
        SDL_AudioFmt = AUDIO_U8;
        bSilence = 0x80;
    } else {
        SDL_AudioFmt = AUDIO_S16;
        bSilence = 0x0;
    }

    /* Store information to convert audio data */
    sOnFlyCVT.buf = NULL;
    sOnFlyCVT.needed = FALSE;
    sMCIBuffer.ulBufferSize = desired->size;

    if (obtained != NULL) {
        obtained->freq = sMCIMixSetup.ulSamplesPerSec;
        obtained->format = SDL_AudioFmt;
        obtained->channels = sMCIMixSetup.ulChannels;
        obtained->silence = bSilence;
        obtained->samples = desired->samples;
        obtained->padding = 0;
        obtained->size = desired->size;
    } else {
        if (desired->freq != sMCIMixSetup.ulSamplesPerSec ||
            desired->format != SDL_AudioFmt ||
            desired->channels != sMCIMixSetup.ulChannels) {
            if (SDL_BuildAudioCVT(&sOnFlyCVT,
                                  desired->format, desired->channels,
                                  desired->freq, SDL_AudioFmt,
                                  sMCIMixSetup.ulChannels,
                                  sMCIMixSetup.ulSamplesPerSec) < 0) {
                debug("SDL_BuildAudioCVT() fail");
                SDL_AudioQuit();
                return -1 ;
            }

            if (sOnFlyCVT.needed) {
                sMCIBuffer.ulBufferSize = sOnFlyCVT.len * sOnFlyCVT.len_mult;

                sOnFlyCVT.len = desired->size;
                sOnFlyCVT.buf = (Uint8 *) SDL_malloc(sMCIBuffer.ulBufferSize);
                if (sOnFlyCVT.buf == NULL) {
                    SDL_OutOfMemory();
                    SDL_AudioQuit();
                    return -1;
                }
                SDL_AudioFmt = sOnFlyCVT.src_format;
                debug("On-fly audio converting prepared: "
                      "Freq.: %u, Channels: %u, format: 0x%X",
                      sMCIMixSetup.ulSamplesPerSec, sMCIMixSetup.ulChannels,
                      SDL_AudioFmt);
            }
        }
    }

    /* Allocate memory buffers */
    sMCIBuffer.ulNumBuffers = AUDIO_NUM_SOUND_BUFFERS;
    sMCIBuffer.pBufList = (PMCI_MIX_BUFFER) SDL_malloc(
                           AUDIO_NUM_SOUND_BUFFERS * sizeof(MCI_MIX_BUFFER));
    if (sMCIBuffer.pBufList == NULL) {
        SDL_OutOfMemory();
        SDL_AudioQuit();
        return -1 ;
    }

    ulRC = mciSendCommand(usAudioDeviceId, MCI_BUFFER,
                          MCI_WAIT | MCI_ALLOCATE_MEMORY, &sMCIBuffer, 0);
    if (ulRC != MCIERR_SUCCESS) {
        _MCIError("MCI_BUFFER", ulRC);
        SDL_AudioQuit();
        return -1;
    }

    /* Fill all device buffers with data */
    for (ulIdx = 0; ulIdx < sMCIBuffer.ulNumBuffers; ulIdx++) {
        ((PMCI_MIX_BUFFER)sMCIBuffer.pBufList)[ulIdx].ulFlags        = 0;
        ((PMCI_MIX_BUFFER)sMCIBuffer.pBufList)[ulIdx].ulBufferLength = sMCIBuffer.ulBufferSize;
        ((PMCI_MIX_BUFFER)sMCIBuffer.pBufList)[ulIdx].ulUserParm     = sMCIBuffer.ulBufferSize;

        memset(((PMCI_MIX_BUFFER)sMCIBuffer.pBufList)[ulIdx].pBuffer,
               bSilence, sMCIBuffer.ulBufferSize);
    }

    fPause = TRUE;
    /* Write buffers to kick off the amp mixer */
    ulRC = sMCIMixSetup.pmixWrite(sMCIMixSetup.ulMixHandle,
                                  (PMCI_MIX_BUFFER)sMCIBuffer.pBufList,
                                  sMCIBuffer.ulNumBuffers);
    _MCIDebugError("MCI_BUFFER", ulRC);

    debug("Done");
    return 0;
}

void SDLCALL SDL_AudioQuit(void)
{
    MCI_GENERIC_PARMS sMCIGenericParms;
    ULONG ulRC;

    debug("Enter");

    if (hmtxLock == NULLHANDLE)
        return;

    fPause = TRUE;

    if (sMCIMixSetup.ulBitsPerSample != 0) {
        ulRC = mciSendCommand(usAudioDeviceId, MCI_MIXSETUP,
                              MCI_WAIT | MCI_MIXSETUP_DEINIT, &sMCIMixSetup, 0);
        _MCIDebugError("MCI_MIXSETUP, MCI_MIXSETUP_DEINIT", ulRC);
        sMCIMixSetup.ulBitsPerSample = 0;
    }

    if (sMCIBuffer.pBufList != NULL) {
        ulRC = mciSendCommand(usAudioDeviceId, MCI_BUFFER,
                              MCI_WAIT | MCI_DEALLOCATE_MEMORY, &sMCIBuffer, 0);
        _MCIDebugError("MCI_BUFFER", ulRC);
        SDL_free(sMCIBuffer.pBufList);
        sMCIBuffer.pBufList = NULL;
    }

    if (usAudioDeviceId != 0) {
        ulRC = mciSendCommand(usAudioDeviceId, MCI_CLOSE, MCI_WAIT,
                              &sMCIGenericParms, 0);
        _MCIDebugError("MCI_CLOSE", ulRC);
        usAudioDeviceId = 0;
    }

    ulRC = DosRequestMutexSem(hmtxLock, SEM_INDEFINITE_WAIT);
    if (ulRC != NO_ERROR) {
        debug("DosRequestMutexSem(), rc = %u", ulRC);
    }
    else if (sOnFlyCVT.buf != NULL) {
        SDL_free(sOnFlyCVT.buf);
        sOnFlyCVT.buf = NULL;
    }

    DosCloseMutexSem(hmtxLock);
    fPause = FALSE;
    hmtxLock = NULLHANDLE;
}

int SDLCALL SDL_AudioInit(const char *driver_name)
{
    /* Not used. All initialisation/finalisation work at
     * SDL_OpenAudio() / SDL_AudioQuit() */
    return 0; /* 0 - success, -1 - fail */
}

void SDLCALL SDL_CloseAudio(void)
{
    /* Warp to SDL_AudioQuit() */
    debug("Enter");
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    SDL_AudioQuit();
    debug("Done");
}

char * SDLCALL SDL_AudioDriverName(char *namebuf, int maxlen)
{
    SDL_strlcpy(namebuf, "OS/2 DART", maxlen);
    return namebuf;
}

SDL_audiostatus SDLCALL SDL_GetAudioStatus(void)
{
    if (hmtxLock == NULLHANDLE)
        return SDL_AUDIO_STOPPED;

    return (fPause)? SDL_AUDIO_PAUSED : SDL_AUDIO_PLAYING;
}

void SDLCALL SDL_PauseAudio(int pause_on)
{
    DosRequestMutexSem(hmtxLock, 2000); /* SDL_LockAudio() */
    fPause = (pause_on != 0);
    DosReleaseMutexSem(hmtxLock);       /* SDL_UnlockAudio() */
}

void SDLCALL SDL_LockAudio(void)
{
    ULONG ulRC = DosRequestMutexSem(hmtxLock, 2000);

    if (ulRC != NO_ERROR) {
        debug("DosRequestMutexSem(), rc = %u", ulRC);
    }
}

void SDLCALL SDL_UnlockAudio(void)
{
    DosReleaseMutexSem(hmtxLock);
}

void SDL_Audio_SetCaption(const char *caption)
{
}

#endif
