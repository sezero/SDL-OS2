/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#ifndef _SDL_os2vman_h
#define _SDL_os2vman_h

#include "SDL_config.h"
#include "SDL_video.h"

#define _ULS_CALLCONV_
#define CALLCONV _System
#include <unidef.h>                    // Unicode API
#include <uconv.h>                     // Unicode API (codepage conversion)

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>

/* Fullscreen modes, bpp defines */
#define OS2VMAN_FSMODELIST_BPP15       0
#define OS2VMAN_FSMODELIST_BPP8        1
#define OS2VMAN_FSMODELIST_BPP16       2
#define OS2VMAN_FSMODELIST_BPP24       3
#define OS2VMAN_FSMODELIST_BPP32       4
#define OS2VMAN_FSMODELIST_SIZE        5

#define VS_WINDOW			0
#define VS_FULLSCREEN			1
#define VS_HIDDEN			2

#define VRN_DISABLED			0
#define VRN_ENABLED			1
#define VRN_CHANGED			2

typedef struct SDL_PrivateVideoData SDL_PrivateVideoData;

typedef struct _VideoMode {
  struct _VideoMode		*pNext;

  unsigned int			uiModeID;
  SDL_Rect			sSDLRect;
  unsigned int			uiBPP;
  unsigned int			uiScanLineSize;
  ULONG				fccColorEncoding;
} VIDEOMODE, *PVIDEOMODE;

typedef struct _VIDEOSYS {
  BOOL		fFreeWinSize;
  BOOL (* vsInit)(SDL_PrivateVideoData *pPVData);
  VOID (* vsDone)();
  BOOL (* vsQueryVideoModes)(SDL_PrivateVideoData *pPVData);
  VOID (* vsFreeVideoModes)(SDL_PrivateVideoData *pPVData);
  VOID (* vsUpdateRects)(SDL_PrivateVideoData *pPVData,
                         SDL_Rect *pSDLRectWin, SDL_Rect *pSDLRectFrame,
                         int cRects, SDL_Rect *pRects);
  VOID (* vsSetFullscreen)(SDL_PrivateVideoData *pPVData,
                           PVIDEOMODE pFSVideoMode); // NULL - return to desktop
  BOOL (* vsSetPalette)(SDL_PrivateVideoData *pPVData, ULONG ulStart,
                        ULONG cColors, SDL_Color *pColors);
  VOID (* vsVisibleRegion)(SDL_PrivateVideoData *pPVData, ULONG ulFlag);
  PVOID (* vsVideoBufAlloc)(struct VIDEOBUFHANDLE **ppVideoBufHdl,
                            PULONG pcbLineSize,
                            PVIDEOMODE pVideoMode,
                            ULONG ulWidth, ULONG ulHeight);
  VOID ( *vsVideoBufFree)(struct VIDEOBUFHANDLE *pVideoBufHdl);
} VIDEOSYS, *PVIDEOSYS;

struct VIDEOBUFHANDLE;

/* Private display data */
struct SDL_PrivateVideoData {
  PVIDEOSYS		pVideoSys;
  BOOL			fVIOSession;

  // List of supported video modes
  PVIDEOMODE		pVideoModes;
  // Current video mode
  PVIDEOMODE		pDesktopVideoMode;
  // Sorted by size video modes for different BPP
  SDL_Rect		**pListVideoModes[OS2VMAN_FSMODELIST_SIZE];
  HMTX			hmtxLock;
  // Current window title
  PSZ			pszTitle;
  HAB			hab;
  // Object - convert characters in unicodde
  UconvObject		ucoUnicode;
  // Mouse captured by SDL
  BOOL			fMouseCapturable;
  // Mouse captured by window
  BOOL			fMouseCaptured;
  // Mouse visible state asked by SDL
  BOOL			fMouseVisible;
  ULONG			ulMouseSkipMove;
  HPOINTER		hptrMousePointer;
  SWP			swpSave;
  BOOL			fWndActive;
  // Fullscreen/desktop current mode flag
  BOOL			fFullscreen;
  // Visible state of window - window/fullscreen/hidden (not actived fullscreen)
  ULONG			ulVisibleState; // VS_*
  // Window size changed flag for informing system in PumpEvents()
  BOOL			fWndResized;
  ULONG			ulPumpEventsTimestamp;
  // Current video mode surface
  SDL_Surface		*pSDLSurface;
  // Fullscreen video mode for current surface
  PVIDEOMODE		pFSVideoMode;
  // Sign that SDL_RESIZABLE flag specifed for video mode
  BOOL			fResizable;
  HEV			hevWindowReady;
  // PM window thread ID
  TID			tidWnd;
  HWND                  hwndFrame;
  HWND                  hwndClient;

  // DIVE video buffer
  struct VIDEOBUFHANDLE *pVideoBufHdl;

  // Allowed hot-keys (from ini-files)
  BOOL			fKeyAltHome;
  BOOL			fKeyAltEnd;
  BOOL			fKeyAltGrEnter;
};

struct WMcursor
{
  HBITMAP		hbm;
  HPOINTER		hptr;
  PCHAR			pchData;
};

extern VIDEOSYS		sVSVMan;
extern VIDEOSYS		sVSDIVE;

extern VOID os2alt_getFrame(SDL_PrivateVideoData *pPVData,
                            SDL_Rect *pSDLRectFrame,
                            PSIZEL pszlWndSize);


#ifdef DEBUG_BUILD
#define debug(s,...) do { printf( __func__"(): "##s"\n", ##__VA_ARGS__ ); fflush(stdout); } while(0)
#define SDL_SetError(s,...) do {\
  printf( __func__"() Set error: "##s"\n", ##__VA_ARGS__ );\
  SDL_SetError( s, ##__VA_ARGS__ ); \
} while(0)
#define SDL_NotEnoughMemory() do {\
  puts( __func__"() Not enough memory" ); SDL_OutOfMemory(); \
} while(0)
#else
#define debug(s,...)
#define SDL_NotEnoughMemory() SDL_OutOfMemory()
#endif

#endif
