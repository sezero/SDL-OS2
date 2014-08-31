#include "SDL_config.h"

#ifndef _SDL_DARTalt_h
#define _SDL_DARTalt_h

#include "../SDL_sysaudio.h"

/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_AudioDevice *this

typedef struct SDL_PrivateAudioData {
  MCI_MIXSETUP_PARMS		sMCIMixSetup;
  MCI_BUFFER_PARMS		sMCIBuffer;
  HMTX				hmtxLock;
  LONG				lAudioDeviceId;
  ULONG				ulNextBufIdx;
  ULONG				ulBufInUseCnt;
  HEV				hevBufReady;
} SDL_PrivateAudioData;

#endif /* _SDL_DARTalt_h */
