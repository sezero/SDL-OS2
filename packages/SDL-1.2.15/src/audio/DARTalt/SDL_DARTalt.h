#include "SDL_config.h"

#ifndef _SDL_DARTalt_h
#define _SDL_DARTalt_h

#include "../SDL_sysaudio.h"

/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_AudioDevice *this

#define DART_AUDIO_BUFFERS	4
#define DART_INPUT_BUFFERS	3

typedef struct _INPUTBUF {
  struct _INPUTBUF		*pNext;
  CHAR				acData[1];
} INPUTBUF, *PINPUTBUF;

typedef struct SDL_PrivateAudioData {
  MCI_MIXSETUP_PARMS		sMCIMixSetup;
  MCI_BUFFER_PARMS		sMCIBuffer;
  HMTX				hmtxLock;
  LONG				lAudioDeviceId;
  HEV				hevBufReady;
  PINPUTBUF			pFreeBuf;
  PINPUTBUF			*ppLastFreeBuf;
  PINPUTBUF			pReadyBuf;
  PINPUTBUF			*ppLastReadyBuf;
} SDL_PrivateAudioData;

#endif /* _SDL_DARTalt_h */
