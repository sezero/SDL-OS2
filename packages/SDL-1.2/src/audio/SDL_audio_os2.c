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

Uint16 OS2DART_AudioFmt = AUDIO_S16; /* global for SDL_mixer.c. */

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

static LONG APIENTRY AudioEvent(ULONG status, PMCI_MIX_BUFFER pBuffer, ULONG flags);


static ULONG _getEnvULong(const char *name, ULONG maxvalue, ULONG defvalue)
{
    ULONG   value;
    char*   end;
    char*   envval = SDL_getenv(name);

    if (envval == NULL)
        return defvalue;

    value = SDL_strtoul(envval, &end, 10);
    return (end == envval) || (value > maxvalue)? defvalue : maxvalue;
}

static void _MCIError(const char *fn, ULONG result)
{
    CHAR buf[128];

    mciGetErrorString(result, buf, sizeof(buf));
    SDL_SetError("[%s] %s", fn, buf);
}

static ULONG _mixSetup(ULONG bps, ULONG freq, ULONG channels)
{
    ULONG rc;

    sMCIMixSetup.ulFormatTag     = MCI_WAVE_FORMAT_PCM;
    sMCIMixSetup.ulBitsPerSample = bps;
    sMCIMixSetup.ulSamplesPerSec = freq;
    sMCIMixSetup.ulChannels      = channels;
    sMCIMixSetup.ulFormatMode    = MCI_PLAY;
    sMCIMixSetup.ulDeviceType    = MCI_DEVTYPE_WAVEFORM_AUDIO;
    sMCIMixSetup.pmixEvent       = AudioEvent;

    rc = mciSendCommand(usAudioDeviceId, MCI_MIXSETUP,
                        MCI_WAIT | MCI_MIXSETUP_INIT, &sMCIMixSetup, 0);
    debug("Setup mixer [BPS: %u, Freq.: %u, Channels: %u]: %s",
          sMCIMixSetup.ulBitsPerSample, sMCIMixSetup.ulSamplesPerSec,
          sMCIMixSetup.ulChannels, rc == MCIERR_SUCCESS ? "SUCCESS" : "FAIL");
    return rc;
}

static LONG APIENTRY AudioEvent(ULONG status, PMCI_MIX_BUFFER pBuffer, ULONG flags)
{
    ULONG rc;

    (void) status;

    if (flags != MIX_WRITE_COMPLETE) {
        debug("flags = 0x%X", ulFlags);
        return 0; /* It seems, return value not matter */
    }

    rc = DosRequestMutexSem(hmtxLock, 2000);
    if (rc != NO_ERROR) {
        debug("DosRequestMutexSem(), rc = %u", rc);
    }

    if (fPause || rc != NO_ERROR) {
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

    rc = sMCIMixSetup.pmixWrite(sMCIMixSetup.ulMixHandle, pBuffer, 1);
    if (rc != MCIERR_SUCCESS) {
        debug("pmixWrite() failed");
    }

    return 0;
}

int SDLCALL SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    ULONG rc;
    const char* env;
    ULONG bps;
    ULONG idx;
    MCI_AMP_OPEN_PARMS sMCIAmpOpen;
    BOOL  shared;

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
        env = SDL_getenv("SDL_AUDIO_FREQUENCY");
        if (env != NULL)
            desired->freq = SDL_atoi(env);
    }
    if (desired->freq == 0)
        desired->freq = 22050;

    if (desired->format == 0)
        desired->format = AUDIO_U16;

    if (desired->channels == 0) {
        env = SDL_getenv("SDL_AUDIO_CHANNELS");
        if (env)
            desired->channels = (Uint8)SDL_atoi(env);
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
        env = SDL_getenv("SDL_AUDIO_SAMPLES");
        if (env)
            desired->samples = (Uint16)SDL_atoi(env);
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
    rc = DosCreateMutexSem(NULL, &hmtxLock, 0, FALSE);
    if (rc != NO_ERROR) {
        SDL_SetError("DosRequestMutexSem(), rc = %u", rc);
        return -1;
    }

    /* Open audio device */
    memset(&sMCIAmpOpen, 0, sizeof(sMCIAmpOpen));
    shared = _getEnvULong("SDL_AUDIO_SHARE", 1, 0);
    sMCIAmpOpen.usDeviceID = 0;
    sMCIAmpOpen.pszDeviceType = (PSZ)MCI_DEVTYPE_AUDIO_AMPMIX;
    rc = mciSendCommand(0, MCI_OPEN,
                 (shared)? MCI_WAIT | MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE :
                           MCI_WAIT | MCI_OPEN_TYPE_ID,
           &sMCIAmpOpen,  0);
    if (rc != MCIERR_SUCCESS) {
        _MCIError("MCI_OPEN", rc);
        DosCloseMutexSem(hmtxLock);
        hmtxLock = NULLHANDLE;
        usAudioDeviceId = 0;
        return -1;
    }
    usAudioDeviceId = sMCIAmpOpen.usDeviceID;

    /* Setup mixer */
    bps = desired->format & 0xFF; /* SDL audio format -> bits per sample */
    rc = _mixSetup(bps, desired->freq, desired->channels);

    /* If requested audio format not supported - reduce parameters */
    if (rc != MCIERR_SUCCESS && desired->freq > 44100) {
        rc = _mixSetup(bps, 44100, desired->channels);
        if (rc != MCIERR_SUCCESS && desired->channels > 2) {
            rc = _mixSetup(bps, desired->freq, 2);
            if (rc != MCIERR_SUCCESS && desired->freq > 44100)
                rc = _mixSetup(bps, 44100, 2);
        }
    }

    if (rc != MCIERR_SUCCESS) {
        _MCIError("MCI_MIXSETUP, MCI_MIXSETUP_INIT", rc);
        sMCIMixSetup.ulBitsPerSample = 0;
        SDL_AudioQuit();
        return -1;
    }

    if (sMCIMixSetup.ulBitsPerSample == 8) {
        OS2DART_AudioFmt = AUDIO_U8;
        bSilence = 0x80;
    } else {
        OS2DART_AudioFmt = AUDIO_S16;
        bSilence = 0x0;
    }

    /* Store information to convert audio data */
    sOnFlyCVT.buf = NULL;
    sOnFlyCVT.needed = FALSE;
    sMCIBuffer.ulBufferSize = desired->size;

    if (obtained != NULL) {
        obtained->freq = sMCIMixSetup.ulSamplesPerSec;
        obtained->format = OS2DART_AudioFmt;
        obtained->channels = sMCIMixSetup.ulChannels;
        obtained->silence = bSilence;
        obtained->samples = desired->samples;
        obtained->padding = 0;
        obtained->size = desired->size;
    } else {
        if (desired->freq != sMCIMixSetup.ulSamplesPerSec ||
            desired->format != OS2DART_AudioFmt ||
            desired->channels != sMCIMixSetup.ulChannels) {
            if (SDL_BuildAudioCVT(&sOnFlyCVT,
                                  desired->format, desired->channels,
                                  desired->freq, OS2DART_AudioFmt,
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
                OS2DART_AudioFmt = sOnFlyCVT.src_format;
                debug("On-fly audio converting prepared: "
                      "Freq.: %u, Channels: %u, format: 0x%X",
                      sMCIMixSetup.ulSamplesPerSec, sMCIMixSetup.ulChannels,
                      OS2DART_AudioFmt);
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

    rc = mciSendCommand(usAudioDeviceId, MCI_BUFFER,
                        MCI_WAIT | MCI_ALLOCATE_MEMORY, &sMCIBuffer, 0);
    if (rc != MCIERR_SUCCESS) {
        _MCIError("MCI_BUFFER", rc);
        SDL_AudioQuit();
        return -1;
    }

    /* Fill all device buffers with data */
    for (idx = 0; idx < sMCIBuffer.ulNumBuffers; idx++) {
        ((PMCI_MIX_BUFFER)sMCIBuffer.pBufList)[idx].ulFlags        = 0;
        ((PMCI_MIX_BUFFER)sMCIBuffer.pBufList)[idx].ulBufferLength = sMCIBuffer.ulBufferSize;
        ((PMCI_MIX_BUFFER)sMCIBuffer.pBufList)[idx].ulUserParm     = sMCIBuffer.ulBufferSize;

        memset(((PMCI_MIX_BUFFER)sMCIBuffer.pBufList)[idx].pBuffer,
               bSilence, sMCIBuffer.ulBufferSize);
    }

    fPause = TRUE;
    /* Write buffers to kick off the amp mixer */
    rc = sMCIMixSetup.pmixWrite(sMCIMixSetup.ulMixHandle,
                                (PMCI_MIX_BUFFER)sMCIBuffer.pBufList,
                                sMCIBuffer.ulNumBuffers);
    _MCIDebugError("MCI_BUFFER", rc);

    debug("Done");
    return 0;
}

void SDLCALL SDL_AudioQuit(void)
{
    MCI_GENERIC_PARMS sMCIGenericParms;
    ULONG rc;

    debug("Enter");

    if (hmtxLock == NULLHANDLE)
        return;

    fPause = TRUE;

    if (sMCIMixSetup.ulBitsPerSample != 0) {
        rc = mciSendCommand(usAudioDeviceId, MCI_MIXSETUP,
                            MCI_WAIT | MCI_MIXSETUP_DEINIT, &sMCIMixSetup, 0);
        _MCIDebugError("MCI_MIXSETUP, MCI_MIXSETUP_DEINIT", rc);
        sMCIMixSetup.ulBitsPerSample = 0;
    }

    if (sMCIBuffer.pBufList != NULL) {
        rc = mciSendCommand(usAudioDeviceId, MCI_BUFFER,
                              MCI_WAIT | MCI_DEALLOCATE_MEMORY, &sMCIBuffer, 0);
        _MCIDebugError("MCI_BUFFER", rc);
        SDL_free(sMCIBuffer.pBufList);
        sMCIBuffer.pBufList = NULL;
    }

    if (usAudioDeviceId != 0) {
        rc = mciSendCommand(usAudioDeviceId, MCI_CLOSE, MCI_WAIT,
                            &sMCIGenericParms, 0);
        _MCIDebugError("MCI_CLOSE", rc);
        usAudioDeviceId = 0;
    }

    rc = DosRequestMutexSem(hmtxLock, SEM_INDEFINITE_WAIT);
    if (rc != NO_ERROR) {
        debug("DosRequestMutexSem(), rc = %u", rc);
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
    ULONG rc = DosRequestMutexSem(hmtxLock, 2000);

    if (rc != NO_ERROR) {
        debug("DosRequestMutexSem(), rc = %u", rc);
    }
}

void SDLCALL SDL_UnlockAudio(void)
{
    DosReleaseMutexSem(hmtxLock);
}

void SDL_Audio_SetCaption(const char *caption)
{
}

#endif /* SDL_AUDIO_DRIVER_DARTALT */
