#ifndef _SDL_os2grop_h
#define _SDL_os2grop_h

#include <SDL_config.h>
#include <SDL_video.h>
#include <grop.h>
#define CALLCONV _System
#include <unidef.h>                    // Unicode API
#include <uconv.h>                     // Unicode API (codepage conversion)

//           Private display data.

typedef struct SDL_PrivateVideoData SDL_PrivateVideoData;

// Video mode bpp/size.
typedef struct _BPPSIZE {
  ULONG                ulBPP;
  SDL_Rect             stSDLRect;
} BPPSIZE, *PBPPSIZE;

typedef struct _BPPSIZELIST {
  struct _BPPSIZELIST  *pNext;
  ULONG                ulBPP;
  SDL_Rect             *apSDLRect[1];
} BPPSIZELIST, *PBPPSIZELIST;

struct SDL_PrivateVideoData {
  BOOL                 fKeyAltHome;
  BOOL                 fKeyAltEnd;
  BOOL                 fKeyAltGrEnter;

  PGROPDATA            pGrop;
  PBPPSIZE             paBPPSize;
  PBPPSIZELIST         pBPPSizeList;
  PRECTL               prectlReserved;
  ULONG                crectlReserved;
  UconvObject          ucoUnicode;
  HPOINTER             hptrWndIcon;

  // Window size changes tracking.
  ULONG                ulWinWidth;
  ULONG                ulWinHeight;
  BOOL                 fWinResized;
  ULONG                ulResizedReportTime;
};


//           Mouse pointer structure for OS/2 related code.

#define VBOX_HACK_SUPPORT 1
// w/o VBOX_HACK_SUPPORT:  We use type cast for (WMcursor *) as HPOINTER.
//                         Content of structure is not interested for us.
// with VBOX_HACK_SUPPORT: We use only one field - hptr.
struct WMcursor
{
  HBITMAP    hbm;
  HPOINTER   hptr;
  char       *pchData;
};

#endif // _SDL_os2grop_h
