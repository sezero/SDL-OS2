#include "SDL_config.h"

#include <process.h>
#include <time.h>

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_os2alt.h"

#include "gradd.h"
#define INCL_GRE_DEVICE
#define INCL_GRE_DEVMISC
#include <pmddi.h>
#include "os2ini.h"

#define WIN_CLIENT_CLASS	"OS2SDL"
#define ID_APP_WINDOW		1
#define WM_CREATEWINDOW		WM_USER + 1
#define WM_FULLSCREEN		WM_USER + 2

#pragma pack(1)
typedef struct BMPINFO
{
   BITMAPINFO;
   RGB  clr;
} BMPINFO, *PBMPINFO;
#pragma pack()

static SDLKey			aScanSDLKeyMap[0xFF] = { SDLK_UNKNOWN };

struct SCAN2SDLKEY {
  ULONG		ulScan;
  SDLKey	enSDLKey;
};

const static struct SCAN2SDLKEY	aScan2SDLKey[] = {
  // First line of keyboard:
  {0x1, SDLK_ESCAPE}, {0x3b, SDLK_F1}, {0x3c, SDLK_F2}, {0x3d, SDLK_F3},
  {0x3e, SDLK_F4}, {0x3f, SDLK_F5}, {0x40, SDLK_F6}, {0x41, SDLK_F7},
  {0x42, SDLK_F8}, {0x43, SDLK_F9}, {0x44, SDLK_F10}, {0x57, SDLK_F11},
  {0x58, SDLK_F12}, {0x5d, SDLK_PRINT}, {0x46, SDLK_SCROLLOCK},
  {0x5f, SDLK_PAUSE},
  // Second line of keyboard:
  {0x29, SDLK_BACKQUOTE}, {0x2, SDLK_1}, {0x3, SDLK_2}, {0x4, SDLK_3},
  {0x5, SDLK_4}, {0x6, SDLK_5}, {0x7, SDLK_6}, {0x8, SDLK_7}, {0x9, SDLK_8},
  {0xa, SDLK_9}, {0xb, SDLK_0}, {0xc, SDLK_MINUS}, {0xd, SDLK_EQUALS},
  {0xe, SDLK_BACKSPACE}, {0x68, SDLK_INSERT}, {0x60, SDLK_HOME},
  {0x62, SDLK_PAGEUP}, {0x45, SDLK_NUMLOCK}, {0x5c, SDLK_KP_DIVIDE},
  {0x37, SDLK_KP_MULTIPLY}, {0x4a, SDLK_KP_MINUS},
  // Third line of keyboard:
  {0xf, SDLK_TAB}, {0x10, SDLK_q}, {0x11, SDLK_w}, {0x12, SDLK_e},
  {0x13, SDLK_r}, {0x14, SDLK_t}, {0x15, SDLK_y}, {0x16, SDLK_u},
  {0x17, SDLK_i}, {0x18, SDLK_o}, {0x19, SDLK_p}, {0x1a, SDLK_LEFTBRACKET},
  {0x1b, SDLK_RIGHTBRACKET}, {0x1c, SDLK_RETURN}, {0x69, SDLK_DELETE},
  {0x65, SDLK_END}, {0x67, SDLK_PAGEDOWN}, {0x47, SDLK_KP7}, {0x48, SDLK_KP8},
  {0x49, SDLK_KP9}, {0x4e, SDLK_KP_PLUS},
  // Fourth line of keyboard:
  {0x3a, SDLK_CAPSLOCK}, {0x1e, SDLK_a}, {0x1f, SDLK_s}, {0x20, SDLK_d},
  {0x21, SDLK_f}, {0x22, SDLK_g}, {0x23, SDLK_h}, {0x24, SDLK_j},
  {0x25, SDLK_k}, {0x26, SDLK_l}, {0x27, SDLK_SEMICOLON}, {0x28, SDLK_QUOTE},
  {0x2b, SDLK_BACKSLASH}, {0x4b, SDLK_KP4}, {0x4c, SDLK_KP5}, {0x4d, SDLK_KP6},
  // Fifth line of keyboard:
  {0x2a, SDLK_LSHIFT}, {0x56, SDLK_WORLD_1}, // Code 161, letter i' on hungarian keyboard
  {0x2c, SDLK_z}, {0x2d, SDLK_x}, {0x2e, SDLK_c}, {0x2f, SDLK_v},
  {0x30, SDLK_b}, {0x31, SDLK_n}, {0x32, SDLK_m}, {0x33, SDLK_COMMA},
  {0x34, SDLK_PERIOD}, {0x35, SDLK_SLASH}, {0x36, SDLK_RSHIFT}, {0x61, SDLK_UP},
  {0x4f, SDLK_KP1}, {0x50, SDLK_KP2}, {0x51, SDLK_KP3}, {0x5a, SDLK_KP_ENTER},
  // Sixth line of keyboard:
  {0x1d, SDLK_LCTRL}, {0x7e, SDLK_LSUPER}, // Windows key
  {0x38, SDLK_LALT}, {0x39, SDLK_SPACE},
  {0x5e, SDLK_RALT}, {0x7f, SDLK_RSUPER}, {0x7c, SDLK_MENU}, {0x5b, SDLK_RCTRL},
  {0x63, SDLK_LEFT}, {0x66, SDLK_DOWN}, {0x64, SDLK_RIGHT}, {0x52, SDLK_KP0},
  {0x53, SDLK_KP_PERIOD} };

struct UNICODESHIFTKEY {
  SDLKey	enSDLKey;
  SDLKey	enSDLShiftKey;
  ULONG ulSDLKey;
};

const static struct UNICODESHIFTKEY	aUnicodeShiftKey[] = {
  {SDLK_BACKQUOTE, '~'}, {SDLK_1, SDLK_EXCLAIM}, {SDLK_2, SDLK_AT},
  {SDLK_3, SDLK_HASH}, {SDLK_4, SDLK_DOLLAR}, {SDLK_5, '%'},
  {SDLK_6, SDLK_CARET}, {SDLK_7, SDLK_AMPERSAND}, {SDLK_8, SDLK_ASTERISK},
  {SDLK_9, SDLK_LEFTPAREN}, {SDLK_0, SDLK_RIGHTPAREN},
  {SDLK_MINUS, SDLK_UNDERSCORE}, {SDLK_PLUS, SDLK_EQUALS},
  {SDLK_LEFTBRACKET, '{'}, {SDLK_RIGHTBRACKET, '}'},
  {SDLK_SEMICOLON, SDLK_COLON}, {SDLK_QUOTE, SDLK_QUOTEDBL},
  {SDLK_BACKSLASH, '|'}, {SDLK_COMMA, SDLK_LESS}, {SDLK_PERIOD, SDLK_GREATER},
  {SDLK_SLASH, SDLK_QUESTION} };


static VOID _memnot(PBYTE pDst, PBYTE pSrc, ULONG ulLen)
{
  while( ulLen-- > 0 )
    *pDst++ = ~*pSrc++;
}
static VOID _memxor(PBYTE pDst, PBYTE pSrc1, PBYTE pSrc2, ULONG ulLen)
{
  while( ulLen-- > 0 )
    *pDst++ = (*pSrc1++)^(*pSrc2++);
}

static VOID _getSDLPixelFormat(SDL_PixelFormat *pSDLPixelFormat,
                               PVIDEOMODE pVideoMode)
{
  ULONG		ulRshift, ulGshift, ulBshift;
  ULONG		ulRmask, ulGmask, ulBmask;
  ULONG		ulRloss, ulGloss, ulBloss;

  pSDLPixelFormat->BitsPerPixel = pVideoMode->uiBPP;
  pSDLPixelFormat->BytesPerPixel = ( pSDLPixelFormat->BitsPerPixel + 7 ) / 8;

  switch( pVideoMode->fccColorEncoding )
  {
    case FOURCC_LUT8:
      ulRshift = 0; ulGshift = 0; ulBshift = 0;
      ulRmask = 0; ulGmask = 0; ulBmask = 0;
      ulRloss = 8; ulGloss = 8; ulBloss = 8;
      break;

    case FOURCC_R555:
      ulRshift = 10; ulGshift = 5; ulBshift = 0;
      ulRmask = 0x7C00; ulGmask = 0x03E0; ulBmask = 0x001F;
      ulRloss = 3; ulGloss = 3; ulBloss = 3;
      break;

    case FOURCC_R565:
      ulRshift = 11; ulGshift = 5; ulBshift = 0;
      ulRmask = 0xF800; ulGmask = 0x07E0; ulBmask = 0x001F;
      ulRloss = 3; ulGloss = 2; ulBloss = 3;
      break;

    case FOURCC_R664:
      ulRshift = 10; ulGshift = 4; ulBshift = 0;
      ulRmask = 0xFC00; ulGmask = 0x03F0; ulBmask = 0x000F;
      ulRloss = 2; ulGloss = 4; ulBloss = 3;
      break;

    case FOURCC_R666:
      ulRshift = 12; ulGshift = 6; ulBshift = 0;
      ulRmask = 0x03F000; ulGmask = 0x000FC0; ulBmask = 0x00003F;
      ulRloss = 2; ulGloss = 2; ulBloss = 2;
      break;

    case FOURCC_RGB3:
    case FOURCC_RGB4:
      ulRshift = 0; ulGshift = 8; ulBshift = 16;
      ulRmask = 0x0000FF; ulGmask = 0x00FF00; ulBmask = 0xFF0000;
      ulRloss = 0x00; ulGloss = 0x00; ulBloss = 0x00;
      break;

    default:
      debug( "Unknown color encoding: %.4s", pVideoMode->fccColorEncoding );
    case FOURCC_BGR3:
    case FOURCC_BGR4:
      ulRshift = 16; ulGshift = 8; ulBshift = 0;
      ulRmask = 0xFF0000; ulGmask = 0x00FF00; ulBmask = 0x0000FF;
      ulRloss = 0; ulGloss = 0; ulBloss = 0;
      break;
  }

  pSDLPixelFormat->Rshift = ulRshift;
  pSDLPixelFormat->Gshift = ulGshift;
  pSDLPixelFormat->Bshift = ulBshift;
  pSDLPixelFormat->Rmask  = ulRmask;
  pSDLPixelFormat->Gmask  = ulGmask;
  pSDLPixelFormat->Bmask  = ulBmask;
  pSDLPixelFormat->Rloss  = ulRloss;
  pSDLPixelFormat->Gloss  = ulGloss;
  pSDLPixelFormat->Bloss  = ulBloss;

  pSDLPixelFormat->Ashift = 0x00;
  pSDLPixelFormat->Amask  = 0x00;
  pSDLPixelFormat->Aloss  = 0x00;
}

// os2alt_getFrame() return surface frame on window or fullscreen and (optional)
// size of the window or fullscreen
VOID os2alt_getFrame(SDL_PrivateVideoData *pPVData, SDL_Rect *pSDLRectFrame,
                     PSIZEL pszlWndSize)
{
  SWP		swpWndSize;

  if ( pPVData->fFullscreen )
  {
    swpWndSize.cx = pPVData->pFSVideoMode->sSDLRect.w;
    swpWndSize.cy = pPVData->pFSVideoMode->sSDLRect.h;
  }
  else
    WinQueryWindowPos( pPVData->hwndClient, &swpWndSize );

  // Calc surface frame size and position at center of window or screen

  if ( pPVData->pVideoSys->fFreeWinSize )
  {
    // Fit output frame to window, keep proportional.
    if ( swpWndSize.cx * pPVData->pSDLSurface->h > swpWndSize.cy * pPVData->pSDLSurface->w )
    {
      pSDLRectFrame->h = swpWndSize.cy;
      pSDLRectFrame->w = ( swpWndSize.cy * pPVData->pSDLSurface->w ) / pPVData->pSDLSurface->h;
    }
    else
    {
      pSDLRectFrame->w = swpWndSize.cx;
      pSDLRectFrame->h = ( swpWndSize.cx * pPVData->pSDLSurface->h ) / pPVData->pSDLSurface->w;
    }
  }
  else
  {
    pSDLRectFrame->w = pPVData->pSDLSurface->w;
    pSDLRectFrame->h = pPVData->pSDLSurface->h;
  }

  pSDLRectFrame->x = ( swpWndSize.cx - pSDLRectFrame->w ) / 2;
  pSDLRectFrame->y = ( swpWndSize.cy - pSDLRectFrame->h ) / 2;

  if ( pszlWndSize != NULL )
  {
    pszlWndSize->cx = swpWndSize.cx;
    pszlWndSize->cy = swpWndSize.cy;
  }
}

static VOID _mapWndPoint2Surface(SDL_PrivateVideoData *pPVData,
                                 PPOINTL pptlPoint)
{
  SDL_Rect		sSDLRectFrame;
  SIZEL			szlWndSize;
  LONG			lFrameX, lFrameY;

  os2alt_getFrame( pPVData, &sSDLRectFrame, &szlWndSize );
  lFrameX = pptlPoint->x - sSDLRectFrame.x;
  lFrameY = ( szlWndSize.cy - pptlPoint->y ) - sSDLRectFrame.y;
  // Scaling frame -> surface - possible future cases when surface size
  // not equal frame size (surface scale with DIVE?).
  pptlPoint->x = ( lFrameX * pPVData->pSDLSurface->w ) / sSDLRectFrame.w;
  pptlPoint->y = ( lFrameY * pPVData->pSDLSurface->h ) / sSDLRectFrame.h;
}

static VOID _mapSurfacePoint2Wnd(SDL_PrivateVideoData *pPVData,
                                 LONG lX, LONG lY, PPOINTL pptlPoint)
{
  SDL_Rect		sSDLRectFrame;
  SIZEL			szlWndSize;
  LONG			lFrameX, lFrameY;

  os2alt_getFrame( pPVData, &sSDLRectFrame, &szlWndSize );
  // Scaling surface -> frame - for cases when surface size
  // not equal frame size (surface scaled with DIVE?).
  lFrameX = ( lX * sSDLRectFrame.w ) / pPVData->pSDLSurface->w;
  lFrameY = ( lY * sSDLRectFrame.h ) / pPVData->pSDLSurface->h;
  lFrameX += sSDLRectFrame.x;
  lFrameY = sSDLRectFrame.h - lFrameY;
  lFrameY += ( szlWndSize.cy - sSDLRectFrame.h - sSDLRectFrame.y );
  pptlPoint->x = lFrameX;
  pptlPoint->y = lFrameY;
}

static VOID _updateRects(SDL_PrivateVideoData *pPVData, int cRects,
                         SDL_Rect *pRects)
{
  SDL_Rect		sSDLRectWin;   // Window (client) rectangle on screen
  SDL_Rect		sSDLRectFrame; // Surface frame rectangle on window
  SIZEL			szlWndSize;
  POINTL		ptlWndPos = { 0 };
  SDL_Rect		sSDLRectTemp;

  if ( pPVData->ulVisibleState == VS_HIDDEN )
    return;

  if ( cRects == 0 )
  {
    if ( pPVData->pSDLSurface == NULL )
    {
      debug( "Full surface area update requested - no surface" );
      return;
    }

    sSDLRectTemp.x = 0;
    sSDLRectTemp.y = 0;
    sSDLRectTemp.w = pPVData->pSDLSurface->w;
    sSDLRectTemp.h = pPVData->pSDLSurface->h;
    pRects = &sSDLRectTemp;
    cRects = 1;
  }

  os2alt_getFrame( pPVData, &sSDLRectFrame, &szlWndSize );
  sSDLRectWin.w = szlWndSize.cx;
  sSDLRectWin.h = szlWndSize.cy;

  if ( pPVData->fFullscreen )
  {
    sSDLRectWin.x = 0;
    sSDLRectWin.y = 0;
  }
  else
  {
    PRECTL		prectlRequest;
    ULONG		ulIdx, ulCount = 0;
    HPS			hps;
    HRGN		hrgnVisible, hrgnRequest, hrgnUpdate;
    RGNRECT		rgnCtl;
    int			cUpdRects = 0;
    SDL_Rect		*pUpdRectsNew, *pUpdRects = NULL;
    RECTL		arectlUpdate[10];
    BOOL		fFinish;
    ULONG		ulFrameBottomOffs;

    // Get PM window position
    WinMapWindowPoints( pPVData->hwndClient, HWND_DESKTOP, &ptlWndPos, 1 );
    sSDLRectWin.x = ptlWndPos.x;
    sSDLRectWin.y = pPVData->pDesktopVideoMode->sSDLRect.h - ptlWndPos.y -
                    szlWndSize.cy;

    // Intersect window's visible region with requested SDL-rectangles and
    // get result as list of SDL-rectangles for update.

    ulFrameBottomOffs = sSDLRectWin.h - sSDLRectFrame.h - sSDLRectFrame.y;
    if ( abs( ulFrameBottomOffs - sSDLRectFrame.y ) > 1 )
      debug( "Frame vertical window offset from bottom is %u, from top is %u. "
             "Must be equal!", ulFrameBottomOffs, sSDLRectFrame.y );

    // Convert SDL-rectangles list to the list of PRECTLs

    prectlRequest = SDL_malloc( cRects * sizeof(RECTL) );
    if ( prectlRequest == NULL )
    {
      DosReleaseMutexSem( pPVData->hmtxLock );
      debug( "Not enough memory" );
      return;
    }

    for( ulIdx = 0; ulIdx < cRects; ulIdx++ )
    {
      if ( pRects[ulIdx].w == 0 || pRects[ulIdx].h == 0 )
        continue;

      prectlRequest[ulCount].xLeft = sSDLRectFrame.x + pRects[ulIdx].x;
      prectlRequest[ulCount].xRight = prectlRequest[ulCount].xLeft + pRects[ulIdx].w;
      prectlRequest[ulCount].yTop = ulFrameBottomOffs +
                                  ( sSDLRectFrame.h - pRects[ulIdx].y );
      prectlRequest[ulCount].yBottom = prectlRequest[ulCount].yTop - pRects[ulIdx].h;
      ulCount++;
    }

    if ( ulCount == 0 )
    {
      SDL_free( prectlRequest );
      return;
    }

    // Create regions objects of visible, requested and intersection result
    // areas
    hps = WinGetPS( pPVData->hwndClient );
    hrgnVisible = GpiCreateRegion( hps, 0, NULL );
    WinQueryVisibleRegion( pPVData->hwndClient, hrgnVisible );
    hrgnRequest = GpiCreateRegion( hps, ulCount, prectlRequest );
    SDL_free( prectlRequest );
    hrgnUpdate = GpiCreateRegion( hps, 0, NULL ); // "AND" result region

    // "AND" on visible and requested regions
    GpiCombineRegion( hps, hrgnUpdate, hrgnRequest, hrgnVisible, CRGN_AND );

    // Read result to the list of SDL-rectangles
    rgnCtl.ulDirection = RECTDIR_LFRT_BOTTOP;
    rgnCtl.ircStart = 1;
    do
    {
      rgnCtl.crcReturned = 0;
      rgnCtl.crc = sizeof(arectlUpdate) / sizeof(RECTL);
      rgnCtl.ircStart = cUpdRects + 1;
      GpiQueryRegionRects( hps, hrgnUpdate, NULL, &rgnCtl, &arectlUpdate );
      fFinish = rgnCtl.crcReturned < rgnCtl.crc;
      ulCount = fFinish ? rgnCtl.crcReturned : rgnCtl.crc;

      if ( ulCount == 0 )
        break;

      // Allocate/expand result list of SDL-rectangles
      pUpdRectsNew = SDL_realloc( pUpdRects,
                               	  (cUpdRects + ulCount) * sizeof(SDL_Rect) );
      if ( pUpdRectsNew == NULL )
      {
        debug( "Not enough memory" );
        SDL_free( pUpdRects );
        pUpdRects = NULL;
        break;
      }
      pUpdRects = pUpdRectsNew;

      // Fill new SDL-rectangles records
      for( ulIdx = 0; ulIdx < ulCount; ulIdx++, cUpdRects++ )
      {
        pUpdRects[cUpdRects].x = arectlUpdate[ulIdx].xLeft - sSDLRectFrame.x;
        pUpdRects[cUpdRects].y = (sSDLRectWin.h - arectlUpdate[ulIdx].yTop) -
                                 sSDLRectFrame.y;
        pUpdRects[cUpdRects].w = arectlUpdate[ulIdx].xRight - arectlUpdate[ulIdx].xLeft;
        pUpdRects[cUpdRects].h = arectlUpdate[ulIdx].yTop - arectlUpdate[ulIdx].yBottom;
      }
    }
    while( !fFinish );
    GpiDestroyRegion( hps, hrgnRequest );
    GpiDestroyRegion( hps, hrgnVisible );
    GpiDestroyRegion( hps, hrgnUpdate );
    WinReleasePS( hps );

    // Update found rectangles
    cRects = cUpdRects;
    pRects = pUpdRects;
  }

  if ( cRects != 0 )
    pPVData->pVideoSys->vsUpdateRects( pPVData,
                      &sSDLRectWin,   // window rectangle on screen
                      &sSDLRectFrame, // surface rectangle on window
                      cRects,         // number of rectangles to update
                      pRects );       // rectangles on surface to update

  if ( !pPVData->fFullscreen && (pRects != NULL) )
    SDL_free( pRects );
}

static VOID _captureMouse(SDL_PrivateVideoData *pPVData)
{
  SWP		swpWnd;
  POINTL	ptlPos;

  WinSetCapture( HWND_DESKTOP, pPVData->hwndClient );
  pPVData->fMouseCaptured = TRUE;

  WinQueryWindowPos( pPVData->hwndClient, &swpWnd );
  ptlPos.x = swpWnd.cx / 2;
  ptlPos.y = swpWnd.cy / 2;
  WinMapWindowPoints( pPVData->hwndClient, HWND_DESKTOP, &ptlPos, 1 );
  pPVData->ulMouseSkipMove++;
  WinSetPointerPos( HWND_DESKTOP, ptlPos.x, ptlPos.y );
}

static VOID _mouseButtonDown(SDL_PrivateVideoData *pPVData, ULONG ulSDLButton)
{
  SDL_PrivateMouseButton( SDL_PRESSED, ulSDLButton, 0, 0 );

  DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );

  if ( pPVData->fMouseCapturable && !pPVData->fMouseCaptured )
  {
    WinSetPointer( HWND_DESKTOP, NULL );
    _captureMouse( pPVData );
  }

  DosReleaseMutexSem( pPVData->hmtxLock );
}

static VOID _checkFullscreen(SDL_PrivateVideoData *pPVData)
{
  ULONG			ulNeedState;

  if ( !pPVData->fFullscreen )
    ulNeedState = VS_WINDOW;
  else if ( pPVData->fFullscreen && !pPVData->fWndActive )
    ulNeedState = VS_HIDDEN;
  else if ( pPVData->fFullscreen && pPVData->fWndActive )
    ulNeedState = VS_FULLSCREEN;

  if ( pPVData->ulVisibleState == ulNeedState )
  {
//    debug( "Visible state not changed (%u)", ulNeedState );
    return;
  }

  pPVData->ulVisibleState = ulNeedState;
  debug( "New state: %u (%s)", ulNeedState,
         ulNeedState == VS_WINDOW ? "VS_WINDOW" :
           ulNeedState == VS_FULLSCREEN ? "VS_FULLSCREEN" : "VS_HIDDEN" );

  switch( ulNeedState )
  {
    case VS_HIDDEN:
    case VS_WINDOW:
      // Return to the desktop

      if ( pPVData->pVideoSys->vsSetFullscreen != NULL )
      {
        pPVData->pVideoSys->vsSetFullscreen( pPVData, NULL );
        // Turn VRN on after return to desktop
        WinSetVisibleRegionNotify( pPVData->hwndClient, TRUE );
        WinSendMsg( pPVData->hwndClient, WM_VRNENABLED, 0, 0 );
      }

      // Let others write to the screen again...
      WinLockWindowUpdate( HWND_DESKTOP, 0 );
      WinSetCapture( HWND_DESKTOP, NULLHANDLE );
      WinShowPointer( HWND_DESKTOP, TRUE );

      if ( ulNeedState == VS_HIDDEN )
        WinShowWindow( pPVData->hwndFrame, FALSE );
      else
        // Restore window's size and position
        WinSetWindowPos( pPVData->hwndFrame,
                         pPVData->swpSave.hwndInsertBehind,
                         pPVData->swpSave.x, pPVData->swpSave.y,
                         pPVData->swpSave.cx, pPVData->swpSave.cy,
                         pPVData->swpSave.fl );

      WinInvalidateRect( WinQueryDesktopWindow( pPVData->hab, 0 ), NULL, TRUE );
      break;

    case VS_FULLSCREEN:
      // Go to fullscreen

      if ( pPVData->pVideoSys->vsSetFullscreen != NULL )
      {
        // Turn VRN off before switch video mode
        WinSetVisibleRegionNotify( pPVData->hwndClient, FALSE );
        WinSendMsg( pPVData->hwndClient, WM_VRNDISABLED, 0, 0 );
      }

      // Save window position before fullscreen
      WinQueryWindowPos( pPVData->hwndFrame, &pPVData->swpSave );
#if 1
      // Move window to upper left corner, set window's size equal surface size
      {
        RECTL		rctl;

        rctl.xLeft = 0;
        rctl.yBottom = 0;
        rctl.xRight = pPVData->pFSVideoMode->sSDLRect.w;
        rctl.yTop = pPVData->pFSVideoMode->sSDLRect.h;
        WinCalcFrameRect( pPVData->hwndFrame, &rctl, FALSE );
        WinSetWindowPos( pPVData->hwndFrame, HWND_TOP,
                         rctl.xLeft,
                         pPVData->pDesktopVideoMode->sSDLRect.h -
                           pPVData->pFSVideoMode->sSDLRect.h + rctl.yBottom,
                         rctl.xRight - rctl.xLeft, rctl.yTop - rctl.yBottom,
                         SWP_SIZE | SWP_MOVE | SWP_ZORDER | SWP_SHOW );
      }
#endif
      // Don't let other applications write to screen anymore
      WinLockWindowUpdate( HWND_DESKTOP, HWND_DESKTOP );
      WinSetCapture( HWND_DESKTOP, pPVData->hwndClient );
      // Hide mouse pointer
      WinShowPointer( HWND_DESKTOP, FALSE );

      if ( pPVData->pVideoSys->vsSetFullscreen != NULL )
        pPVData->pVideoSys->vsSetFullscreen( pPVData, pPVData->pFSVideoMode );
      _updateRects( pPVData, 0, NULL );
      break;
  }

  WinSetVisibleRegionNotify( pPVData->hwndClient, TRUE );
  debug( "Done" );
}


//  Presentation manager (window) service
//  -------------------------------------

// For some Win*() functions we must have message queue in any thread
#define WIN_MSG_QUEUE_BEGIN \
  { HAB		_pm_hab = WinInitialize( 0 ); \
    HMQ		_pm_hmq = WinCreateMsgQueue( _pm_hab, 0 ); \
    ERRORID	_pm_error = WinGetLastError( _pm_hab );
#define WIN_MSG_QUEUE_END \
  if ( ERRORIDERROR( _pm_error ) == 0 ) WinDestroyMsgQueue( _pm_hmq ); }

MRESULT EXPENTRY wndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SDL_PrivateVideoData	*pPVData =
                       (SDL_PrivateVideoData *)WinQueryWindowULong( hwnd, 0 );
  HPS			hps;
  RECTL			rctl;
  ULONG			ulRC;

  switch( msg )
  {
    case WM_ACTIVATE:
      DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
      pPVData->fWndActive = (BOOL)mp1;
      debug( "WM_ACTIVATE: %u", (BOOL)mp1 );
      _checkFullscreen( pPVData );
      if ( pPVData->fWndActive )
      {
        if ( pPVData->fMouseVisible && !pPVData->fMouseCaptured )
          WinSetPointer( HWND_DESKTOP,
                         WinQuerySysPointer( HWND_DESKTOP, SPTR_ARROW, FALSE ) );
        else
          WinSetPointer( HWND_DESKTOP, NULL);

        if ( pPVData->fMouseCapturable )
          _captureMouse( pPVData );
      }
      else
      {
        WinSetPointer( HWND_DESKTOP,
                       WinQuerySysPointer( HWND_DESKTOP, SPTR_ARROW, FALSE ) );

        if ( pPVData->fMouseCaptured )
        {
          WinSetCapture( HWND_DESKTOP, hwnd );
          pPVData->fMouseCaptured = FALSE;
        }
      }
      DosReleaseMutexSem( pPVData->hmtxLock );
      SDL_PrivateAppActive( (Uint8)mp1, SDL_APPACTIVE );
      break;

    case WM_SETFOCUS:
      SDL_PrivateAppActive( SHORT1FROMMP( mp2 ) != 0, SDL_APPINPUTFOCUS );
      break;

    case WM_FULLSCREEN:
      debug( "WM_FULLSCREEN" );
      DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
      if ( (SDL_Surface *)mp1 != pPVData->pSDLSurface )
        debug( "Surface specified in command not same as current" );
      else if ( (pPVData->pSDLSurface->flags & SDL_RESIZABLE) != 0 )
        debug( "Ignore WM_FULLSCREEN with resizable surface" );
      else
      {
        pPVData->fFullscreen = (int)mp2 != 0;
        debug( "Switch to new mode: %s", pPVData->fFullscreen ? "fullscreen" : "window" );
        _checkFullscreen( pPVData );
      }
      DosReleaseMutexSem( pPVData->hmtxLock );
      break;

    case WM_DESTROY:
      debug( "WM_DESTROY" );
      DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
      pPVData->fMouseCaptured = FALSE;
//    Deactivation message WM_ACTIVATE already received, we shout be in desktop
//    now. No need switch to desktop again here.
      pPVData->fFullscreen = FALSE;
      _checkFullscreen( pPVData );
      DosReleaseMutexSem( pPVData->hmtxLock );
      WinSetCapture( HWND_DESKTOP, NULLHANDLE );
      WinSetPointer( HWND_DESKTOP, WinQuerySysPointer( hwnd, SPTR_ARROW, FALSE ) );
      debug( "Done" );
      break;

    case WM_PAINT:
      hps = WinBeginPaint( hwnd, 0, &rctl );
      WinFillRect( hps, &rctl, CLR_BLACK );

      DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
      _updateRects( pPVData, 0, NULL );
      DosReleaseMutexSem( pPVData->hmtxLock );
      WinEndPaint( hps );
      return (MRESULT)FALSE;

    case WM_CLOSE:
      debug( "WM_CLOSE" );
      SDL_PrivateQuit();
      return (MRESULT)FALSE;

    case WM_SIZE:
      DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
      pPVData->fWndResized = pPVData->fResizable;
      DosReleaseMutexSem( pPVData->hmtxLock );
      break;

    case WM_VRNDISABLED:
      if ( pPVData->pVideoSys->vsVisibleRegion != NULL )
      {
        DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
        pPVData->pVideoSys->vsVisibleRegion( pPVData, VRN_DISABLED );
        DosReleaseMutexSem( pPVData->hmtxLock );
      }
      break;

    case WM_VRNENABLED:
      if ( pPVData->pVideoSys->vsVisibleRegion != NULL )
      {
        DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
        pPVData->pVideoSys->vsVisibleRegion( pPVData,
                                     ((BOOL)mp1) ? VRN_CHANGED : VRN_ENABLED );
        DosReleaseMutexSem( pPVData->hmtxLock );
      }
      break;

    case WM_CHAR:
      {
        SDL_keysym	sSDLKeysym;
        ULONG		ulFlags = SHORT1FROMMP(mp1);      // WM_CHAR flags
        ULONG		ulVirtualKey = SHORT2FROMMP(mp2); // Virtual key code VK_*
        ULONG		ulCharCode = SHORT1FROMMP(mp2);   // Character code
        ULONG		ulScanCode = CHAR4FROMMP(mp1);    // Scan code
//        BOOL		fKeyUp = (ulFlags & KC_KEYUP) != 0;
        SDLKey		enSDLKey = aScanSDLKeyMap[ulScanCode];
        ULONG		ulUnicode = 0;
        ULONG		ulIdx;

        if ( (ulFlags & (KC_KEYUP | KC_ALT)) == KC_ALT ) // Key pressed with ALT
        {
          // Check for fastkeys: ALT+HOME to toggle FS mode
          //                     ALT+END to close app
          switch( ulVirtualKey )
          {
            case VK_HOME:
              if ( !pPVData->fKeyAltHome )
              {
                debug( "Alt+Home tuned off by ini-record" );
                break;
              }
              debug( "Alt+Home pressed - switch to %s",
                     pPVData->fFullscreen ? "desktop" : "fullscreen" );
              WinSendMsg( pPVData->hwndClient, WM_FULLSCREEN,
                          (MPARAM)pPVData->pSDLSurface,
                          (MPARAM)(pPVData->fFullscreen ? 0 : 1) );
              return (MRESULT)TRUE;

            case VK_END:
              if ( !pPVData->fKeyAltEnd )
              {
                debug( "Alt+End tuned off by ini-record" );
                break;
              }
              debug( "Alt+End pressed - quit" );
              if ( pPVData->fFullscreen )
                WinSendMsg( pPVData->hwndClient, WM_FULLSCREEN,
                            (MPARAM)pPVData->pSDLSurface, 0 );
              SDL_PrivateQuit();
              return (MRESULT)TRUE;
          }
        }

        if ( ( (ulFlags & KC_KEYUP) == 0 ) &&
             ( (ulVirtualKey == VK_NEWLINE) || (ulVirtualKey == VK_ENTER) ) &&
             ( (SDL_GetModState() & (KMOD_RALT | KMOD_MODE)) != 0 ) )
        {
          if ( !pPVData->fKeyAltGrEnter )
            debug( "AltGr+Enter tuned off by ini-record" );
          else
          {
            debug( "AltGr+Enter pressed - switch to %s",
                   pPVData->fFullscreen ? "desktop" : "fullscreen" );
            WinSendMsg( pPVData->hwndClient, WM_FULLSCREEN,
                        (MPARAM)pPVData->pSDLSurface,
                        (MPARAM)(pPVData->fFullscreen ? 0 : 1) );
            return (MRESULT)TRUE;
          }
        }

        if ( SDL_TranslateUNICODE )
        {
          if ( (ulFlags & (KC_KEYUP | KC_CHAR)) == KC_CHAR // Pressed, have ch. code
               && pPVData->ucoUnicode != NULL )	// Conv. object created
          {
            // Detect unicode value for key

            CHAR	achInput[2];
            PCHAR	pchInput = &achInput;
            size_t	cInput = sizeof(achInput);
            UniChar	auchOutput[4];
            UniChar	*puchOutput = &auchOutput;
            size_t	cOutput = sizeof(auchOutput);
            size_t	cSubst;

            achInput[0] = ulCharCode;
            achInput[1] = 0;
            ulRC = UniUconvToUcs( pPVData->ucoUnicode, (PVOID)&pchInput,
                                  &cInput, &puchOutput, &cOutput, &cSubst );

            if ( ulRC != ULS_SUCCESS )
              debug( "UniUconvToUcs(), rc = %u", ulRC );
            else
              ulUnicode = auchOutput[0];
          }

          // SDL-style convert key codes for unicode when Shift pressed
          if ( (ulFlags & KC_SHIFT) != 0 )
            for( ulIdx = 0;
                 ulIdx < sizeof(aUnicodeShiftKey) / sizeof(struct UNICODESHIFTKEY);
                 ulIdx++ )
              if ( aUnicodeShiftKey[ulIdx].enSDLKey == enSDLKey )
              {
                enSDLKey = aUnicodeShiftKey[ulIdx].enSDLShiftKey;
                break;
              }
        }

        sSDLKeysym.scancode = ulScanCode;
        sSDLKeysym.mod = KMOD_NONE;
        sSDLKeysym.sym = enSDLKey;
        sSDLKeysym.unicode = ulUnicode;

        SDL_PrivateKeyboard( (ulFlags & KC_KEYUP) == 0 ?
                               SDL_PRESSED : SDL_RELEASED, &sSDLKeysym );
      }
      return (MRESULT)TRUE;

    case WM_TRANSLATEACCEL:
      // ALT and acceleration keys not allowed (must be processed in WM_CHAR)
      if ( mp1 == NULL || ((PQMSG)mp1)->msg != WM_CHAR )
        break;
      return (MRESULT)FALSE;

    case WM_MOUSEMOVE:
      DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
      if ( pPVData->pSDLSurface == NULL )
      {
        DosReleaseMutexSem( pPVData->hmtxLock );
        break;
      }

      if ( pPVData->ulMouseSkipMove > 0 )
      {
        pPVData->ulMouseSkipMove--;
        DosReleaseMutexSem( pPVData->hmtxLock );
      }
      else
      {
        POINTL		ptlPos;

        ptlPos.x = (LONG)SHORT1FROMMP(mp1);
        ptlPos.y = (LONG)SHORT2FROMMP(mp1);
        _mapWndPoint2Surface( pPVData, &ptlPos );
        if ( pPVData->fMouseCaptured && pPVData->fWndActive )
        {
          POINTL	ptlCenter;
          SWP		swpWndSize;

          if ( pPVData->fFullscreen )
          {
            swpWndSize.cx = pPVData->pFSVideoMode->sSDLRect.w;
            swpWndSize.cy = pPVData->pFSVideoMode->sSDLRect.h;
          }
          else
            WinQueryWindowPos( pPVData->hwndClient, &swpWndSize );

          pPVData->ulMouseSkipMove++;
          DosReleaseMutexSem( pPVData->hmtxLock );

          ptlCenter.x = swpWndSize.cx / 2;
          ptlCenter.y = swpWndSize.cy / 2;
          // Mouse at center of window
          WinMapWindowPoints( hwnd, HWND_DESKTOP, &ptlCenter, 1 );
          WinSetPointerPos( HWND_DESKTOP, ptlCenter.x, ptlCenter.y );

          // Move mouse to the center of surface
          SDL_PrivateMouseMotion( 0, 1,
                                  ptlPos.x - (pPVData->pSDLSurface->w / 2),
                                  ptlPos.y - (pPVData->pSDLSurface->h / 2) );
        }
        else
        {
          // Send position on surface to SDL
          DosReleaseMutexSem( pPVData->hmtxLock );
          SDL_PrivateMouseMotion( 0, 0, ptlPos.x, ptlPos.y );
        }
      }

      if ( pPVData->fMouseVisible && !pPVData->fMouseCaptured )
      {
        if ( pPVData->hptrMousePointer )
          WinSetPointer( HWND_DESKTOP, pPVData->hptrMousePointer );
        else
          WinSetPointer( HWND_DESKTOP,
                         WinQuerySysPointer( HWND_DESKTOP, SPTR_ARROW, FALSE ) );
      }
      else
        WinSetPointer( HWND_DESKTOP, NULL );

      return (MRESULT)FALSE;

    case WM_BUTTON1DOWN:
      _mouseButtonDown( pPVData, SDL_BUTTON_LEFT );
      break;

    case WM_BUTTON1UP:
      SDL_PrivateMouseButton( SDL_RELEASED, SDL_BUTTON_LEFT, 0, 0 );
      break;

    case WM_BUTTON2DOWN:
      _mouseButtonDown( pPVData, SDL_BUTTON_RIGHT );
      break;

    case WM_BUTTON2UP:
      SDL_PrivateMouseButton( SDL_RELEASED, SDL_BUTTON_RIGHT, 0, 0 );
      break;

    case WM_BUTTON3DOWN:
      _mouseButtonDown( pPVData, SDL_BUTTON_MIDDLE );
      break;

    case WM_BUTTON3UP:
      SDL_PrivateMouseButton( SDL_RELEASED, SDL_BUTTON_MIDDLE, 0, 0 );
      break;
  }

  return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

static BOOL _wndCreateWindow(SDL_PrivateVideoData *pPVData)
{
  RECTL		rectlWnd;
  ULONG		ulFrameFlags = FCF_TASKLIST | FCF_DLGBORDER | FCF_TITLEBAR |
                               FCF_SYSMENU | FCF_MINBUTTON | FCF_SHELLPOSITION;
  SWP		swpWnd;

  debug( "Enter" );
  DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );

  if ( pPVData->pSDLSurface == NULL )
  {
    debug( "Surface not exits" );
    DosReleaseMutexSem( pPVData->hmtxLock );
    DosPostEventSem( pPVData->hevWindowReady );
    return FALSE;
  }

  do
  {
    if ( pPVData->fResizable )
    {
      // Do not recreate resizable window - change size if need
      if ( ( pPVData->hwndClient != NULLHANDLE ) &&
           (BOOL)WinQueryWindowULong( pPVData->hwndClient, 4 ) ) // Win. resizable
      {
        // Resizable window can't have fullscreen surface but can be switched
        // to fullscreen mode by hot-keys. Now we must return to desktop.
        WinSendMsg( pPVData->hwndClient, WM_FULLSCREEN,
                    (MPARAM)pPVData->pSDLSurface, (MPARAM)0 );

        WinQueryWindowPos( pPVData->hwndClient, &swpWnd );

        if ( ( pPVData->pSDLSurface->w != swpWnd.cx ) ||
             ( pPVData->pSDLSurface->h != swpWnd.cy ) )
        {
          debug( "Resizable surface - change window size" );
          break;
        }

        debug( "Resizable surface, window size equal surface size" );
        DosReleaseMutexSem( pPVData->hmtxLock );
        DosPostEventSem( pPVData->hevWindowReady );
        return TRUE;
      }

      ulFrameFlags |= FCF_SIZEBORDER | FCF_MINBUTTON | FCF_MAXBUTTON;
    }

    // Window exists - switch to desktop and destroy old window
    if ( pPVData->hwndFrame != NULLHANDLE )
    {
      debug( "Send command to switch in window mode" );
      WinSendMsg( pPVData->hwndClient, WM_FULLSCREEN,
                  (MPARAM)pPVData->pSDLSurface, (MPARAM)0 );
      debug( "Destroy previous window" );
      WinDestroyWindow( pPVData->hwndFrame );
      pPVData->hwndFrame = NULLHANDLE;
    }

    debug( "Create a new window" );
    pPVData->hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0, &ulFrameFlags,
                                  WIN_CLIENT_CLASS, pPVData->pszTitle, 0, 0,
                                  ID_APP_WINDOW, &pPVData->hwndClient );
    if ( pPVData->hwndFrame == NULLHANDLE )
    {
      pPVData->hwndClient = NULLHANDLE;
      debug( "WinCreateStdWindow() failed" );
      DosReleaseMutexSem( pPVData->hmtxLock );
      DosPostEventSem( pPVData->hevWindowReady );
      return FALSE;
    }

    WinSetWindowULong( pPVData->hwndClient, 0, (ULONG)pPVData );
    // Resizable flag for window
    WinSetWindowULong( pPVData->hwndClient, 4, (ULONG)pPVData->fResizable );
    WinSetVisibleRegionNotify( pPVData->hwndClient, TRUE );
  }
  while( FALSE );

  // Set window size equal surface size
  rectlWnd.xLeft = 0;
  rectlWnd.xRight = pPVData->pSDLSurface->w;
  rectlWnd.yBottom = 0;
  rectlWnd.yTop = pPVData->pSDLSurface->h;
  WinCalcFrameRect( pPVData->hwndFrame, &rectlWnd, FALSE );
  WinSetWindowPos( pPVData->hwndFrame, HWND_TOP, 0, 0,
                   rectlWnd.xRight - rectlWnd.xLeft,
                   rectlWnd.yTop - rectlWnd.yBottom,
                   SWP_SIZE | SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER );

  // Make sure that the title and the right edge of the window is visible
  WinQueryWindowPos( pPVData->hwndFrame, &swpWnd );
  if ( (swpWnd.x + swpWnd.cx) > pPVData->pDesktopVideoMode->sSDLRect.w )
    swpWnd.x = swpWnd.cx > pPVData->pDesktopVideoMode->sSDLRect.w ?
                 0 : pPVData->pDesktopVideoMode->sSDLRect.w - swpWnd.cx;
  if ( (swpWnd.y + swpWnd.cy) > pPVData->pDesktopVideoMode->sSDLRect.h )
    swpWnd.y = pPVData->pDesktopVideoMode->sSDLRect.h - swpWnd.cy;
  WinSetWindowPos( pPVData->hwndFrame, HWND_TOP, swpWnd.x, swpWnd.y,
                   swpWnd.cx, swpWnd.cy, SWP_MOVE );

  pPVData->fWndResized = FALSE;
  DosReleaseMutexSem( pPVData->hmtxLock );
  DosPostEventSem( pPVData->hevWindowReady );
  return TRUE;
}

void FAR wndThread(void FAR *pData)
{
  SDL_PrivateVideoData	*pPVData = (SDL_PrivateVideoData *)pData;
  HAB			hab = NULLHANDLE;
  HMQ			hmq = NULLHANDLE;
  QMSG			qmsg;

  do
  {
    hab = WinInitialize( 0 );
    if ( hab == NULLHANDLE )
    {
      debug( "WinInitialize() failed" );
      break;
    }

    hmq = WinCreateMsgQueue( hab, 0 );
    if ( hmq == NULLHANDLE )
    {
      debug( "WinCreateMsgQueue() failed" );
      break;
    }

    pPVData->hab = hab;
    WinRegisterClass( hab, WIN_CLIENT_CLASS, wndProc,
                      CS_SIZEREDRAW | CS_MOVENOTIFY | CS_SYNCPAINT,
                      sizeof(SDL_PrivateVideoData*) + sizeof(ULONG) );
    if ( !_wndCreateWindow( pPVData ) )
      break;

    while( WinGetMsg( hab, &qmsg, (HWND)NULLHANDLE, 0L, 0L ) )
    {
      if ( qmsg.msg == WM_CREATEWINDOW )
      {
        debug( "Message WM_CREATEWINDOW received" );
        if ( !_wndCreateWindow( pPVData ) )
          break;

        continue;
      }

      WinDispatchMsg( hab, &qmsg );
    }
    debug( "Destroy window..." );

    DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
    WinDestroyWindow( pPVData->hwndFrame );
    pPVData->hwndFrame = NULLHANDLE;
    pPVData->hwndClient = NULLHANDLE;
    DosReleaseMutexSem( pPVData->hmtxLock );
  }
  while( FALSE );

  if ( hmq != NULLHANDLE )
    WinDestroyMsgQueue( hmq );
  if ( hab != NULLHANDLE )
    WinTerminate( hab );
  pPVData->tidWnd = (TID)(-1);

  WIN_MSG_QUEUE_BEGIN
  WinInvalidateRect( WinQueryDesktopWindow( _pm_hab, 0 ), NULL, TRUE );
  WinUpdateWindow( WinQueryDesktopWindow( _pm_hab, 0 ) );
  WIN_MSG_QUEUE_END

  DosPostEventSem( pPVData->hevWindowReady );
  debug( "Exit thread" );
  _endthread();
}

static BOOL wndInit(SDL_PrivateVideoData *pPVData)
{
  ULONG		ulCount;
  ULONG		ulRC;
  BOOL		fReady = FALSE;

  debug( "Enter" );
  ulRC = DosResetEventSem( pPVData->hevWindowReady, &ulCount );
  if ( ulRC != NO_ERROR && ulRC != ERROR_ALREADY_RESET  )
  {
    debug( "DosResetEventSem(), rc = %u", ulRC );
    return FALSE;
  }

  DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
  if ( pPVData->pSDLSurface == NULL )
  {
    DosReleaseMutexSem( pPVData->hmtxLock );
    debug( "Surface not exits." );
    return FALSE;
  }

  if ( pPVData->tidWnd == -1 )
  {
    // Thread not runned. Start thread, PM window will be created in the thread
    pPVData->tidWnd = _beginthread( wndThread, NULL, 65536, (PVOID)pPVData );
    if ( pPVData->tidWnd == -1 )
      debug( "Cannot start thread" );
    else
      fReady = TRUE;
  }
  else if ( pPVData->hwndFrame != NULLHANDLE )
  {
    // Thread exits, send command to destroy old window and create new
    WinPostMsg( pPVData->hwndFrame, WM_CREATEWINDOW, 0L, 0L );
    fReady = TRUE;
  }
  else
    debug( "Thread exists but have no window's handle" );

  DosReleaseMutexSem( pPVData->hmtxLock );

  if ( !fReady )
    return FALSE;

  ulRC = DosWaitEventSem( pPVData->hevWindowReady, 2000 );
  if ( ulRC != NO_ERROR )
  {
    debug( "DosWaitEventSem(), rc = %u", ulRC );
    return FALSE;
  }

  DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
  fReady = pPVData->hwndFrame != NULLHANDLE;
  DosReleaseMutexSem( pPVData->hmtxLock );

  debug( "Done" );
  return fReady;
}

static VOID wndDone(SDL_PrivateVideoData *pPVData)
{
  ULONG			ulRC;
  ULONG			tidWnd = pPVData->tidWnd;

  if ( tidWnd != (TID)(-1) )
  {
    if ( pPVData->hwndFrame != NULLHANDLE )
    {
      WinPostMsg( pPVData->hwndFrame, WM_QUIT, 0, 0 );
      debug( "Wait thread..." );
      ulRC = DosWaitThread( &tidWnd, DCWW_WAIT );
      if ( ulRC == NO_ERROR )
      {
        pPVData->tidWnd = (TID)(-1);
        debug( "Thread ended" );
      }
      else
        debug( "DosWaitThread(), rc = %u", ulRC );
    }

    if ( pPVData->tidWnd != (TID)(-1) )
    {
      ulRC = DosKillThread( pPVData->tidWnd );
      debug( "DosKillThread(), rc = %u", ulRC );
    }
  }
}


//  SDL interface functions
//  -----------------------

static int os2_SetColors(SDL_VideoDevice *pDevice, int iFirst, int iCount,
                         SDL_Color *pColors)
{	
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;
  BOOL			fGotAll;

  DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
  fGotAll = pPVData->pVideoSys->vsSetPalette( pPVData, iFirst, iCount, pColors );
  DosReleaseMutexSem( pPVData->hmtxLock );

  return fGotAll ? 1 : 0;
}

static int os2_AllocHWSurface(SDL_VideoDevice *pDevice,
                              SDL_Surface *pSDLSurface)
{
  return -1;
}

static void os2_FreeHWSurface(SDL_VideoDevice *pDevice,
                              SDL_Surface *pSDLSurface)
{ }

static int os2_LockHWSurface(SDL_VideoDevice *pDevice, SDL_Surface *pSDLSurface)
{
  return 0;
}

static void os2_UnlockHWSurface(SDL_VideoDevice *pDevice,
                                SDL_Surface *pSDLSurface)
{ }

static WMcursor *os2_CreateWMCursor(SDL_VideoDevice *pDevice,
                                    Uint8 *puiData, Uint8 *puiMask,
                                    int iW, int iH, int iHotX, int iHotY)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;
  HPOINTER		hptr;
  HBITMAP		hbm;
  BITMAPINFOHEADER	bmih = { 0 };
  BMPINFO		bmi;
  HPS              	hps;
  PCHAR			pchTemp;
  PCHAR			xptr, aptr;
  ULONG			ulMaxX, ulMaxY, ulMaxXBRnd;
  LONG			i, run, pad;
  WMcursor		*pResult = SDL_malloc( sizeof(WMcursor) );

  debug( "Enter" );

  if ( pResult == NULL )
  {
    SDL_NotEnoughMemory();
    return NULL;
  }

  WIN_MSG_QUEUE_BEGIN

  ulMaxX = WinQuerySysValue( HWND_DESKTOP, SV_CXPOINTER );
  ulMaxXBRnd = ( ulMaxX + 7 ) / 8;
  ulMaxY = WinQuerySysValue( HWND_DESKTOP, SV_CYPOINTER );

  do
  {
    if ( ( iW > ulMaxX ) || ( iH > ulMaxY ) )
    {
      SDL_free( pResult );
      pResult = NULL;
      break;
    }

    pchTemp = SDL_calloc( 1, ulMaxXBRnd * ulMaxY * 2 );
    if ( pchTemp == NULL )
    {
      SDL_NotEnoughMemory();
      SDL_free( pResult );
      pResult = NULL;
      break;
    }

    hps = WinGetPS( pPVData->hwndClient );

    bmi.cbFix = sizeof(BITMAPINFOHEADER);
    bmi.cx = ulMaxX;
    bmi.cy = ulMaxY * 2;
    bmi.cPlanes = 1;
    bmi.cBitCount = 1;
    bmi.argbColor[0].bBlue = 0x00;
    bmi.argbColor[0].bGreen = 0x00;
    bmi.argbColor[0].bRed = 0x00;
    bmi.argbColor[1].bBlue = 0x00;
    bmi.argbColor[1].bGreen = 0x00;
    bmi.argbColor[1].bRed = 0xff;

    bmih.cbFix = sizeof(BITMAPINFOHEADER);
    bmih.cx = ulMaxX;
    bmih.cy = ulMaxY * 2;
    bmih.cPlanes = 1;
    bmih.cBitCount = 1;

    run = ( iW + 7 ) / 8;
    pad = ulMaxXBRnd - run;

    for( i = 0; i < iH; i++ )
    {
      xptr = pchTemp + ulMaxXBRnd * ( ulMaxY - 1 - i );
      aptr = pchTemp + ulMaxXBRnd * ( 2 * ulMaxY - 1 - i );
      _memxor( xptr, puiData, puiMask, run );
      xptr += run;
      puiData += run;
      _memnot( aptr, puiMask, run );
      puiMask += run;
      aptr += run;
      SDL_memset( xptr,  0, pad );
      xptr += pad;
      SDL_memset( aptr, 0xFF, pad );
      aptr += pad;
    }

    pad += run;

    for( i = iH; i < ulMaxY; i++ )
    {
      xptr = pchTemp + ulMaxXBRnd * ( ulMaxY - 1 - i );
      aptr = pchTemp + ulMaxXBRnd * ( 2 * ulMaxY - 1 - i );

      SDL_memset( xptr,  0, ulMaxXBRnd );
      xptr += ulMaxXBRnd;
      SDL_memset( aptr, 0xFF, ulMaxXBRnd );
      aptr += ulMaxXBRnd;
    }

    hbm = GpiCreateBitmap( hps, (PBITMAPINFOHEADER2)&bmih, CBM_INIT,
                           pchTemp, (PBITMAPINFO2)&bmi );
    hptr = WinCreatePointer( HWND_DESKTOP, hbm, TRUE, iHotX, ulMaxY - iHotY - 1 );

    WinReleasePS( hps );

    pResult->hptr = hptr;
    pResult->hbm = hbm;
    pResult->pchData = pchTemp;
    debug( "Done %p", pResult );
  }
  while( FALSE );

  WIN_MSG_QUEUE_END
  return (WMcursor *)pResult;
}

static void os2_FreeWMCursor(SDL_VideoDevice *pDevice, WMcursor *pCursor)
{
  debug( "Enter %p", pCursor );
  if ( pCursor != NULL )
  {
    GpiDeleteBitmap( pCursor->hbm );
    WinDestroyPointer( pCursor->hptr );
    SDL_free( pCursor->pchData );
    SDL_free( pCursor );
  }
}

static int os2_ShowWMCursor(SDL_VideoDevice *pDevice, WMcursor *pCursor)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;

  debug( "Enter" );
  if ( pPVData->fFullscreen )
  {
    WinSetPointer( HWND_DESKTOP, FALSE );
    pPVData->hptrMousePointer = NULL;
    pPVData->fMouseVisible = FALSE;
    return 0;
  }

  if ( pCursor != NULL )
  {
//    WIN_MSG_QUEUE_BEGIN
    WinSetPointer( HWND_DESKTOP, pCursor->hptr );
//    WIN_MSG_QUEUE_END
    pPVData->hptrMousePointer = pCursor->hptr;
    pPVData->fMouseVisible = TRUE;
  }
  else
  {
    WinSetPointer( HWND_DESKTOP, FALSE );
    pPVData->hptrMousePointer = NULL;
    pPVData->fMouseVisible = FALSE;
  }

  return 1;
}

static void os2_WarpWMCursor(SDL_VideoDevice *pDevice, Uint16 uiX, Uint16 uiY)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;
  POINTL		ptlPoint;

  DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
  if ( pPVData->hwndClient == NULLHANDLE || pPVData->pSDLSurface == NULL )
  {
    DosReleaseMutexSem( pPVData->hmtxLock );
    return;
  }

  _mapSurfacePoint2Wnd( pPVData, uiX, uiY, &ptlPoint );
/*if ( pPVData->fFullscreen )
{
  ptlPoint.y += pPVData->pDesktopVideoMode->sSDLRect.h - pPVData->pFSVideoMode->sSDLRect.h;
}*/
//  WIN_MSG_QUEUE_BEGIN
  WinMapWindowPoints( pPVData->hwndClient, HWND_DESKTOP, &ptlPoint, 1 );
  DosReleaseMutexSem( pPVData->hmtxLock );
  WinSetPointerPos( HWND_DESKTOP, ptlPoint.x, ptlPoint.y );
//  WIN_MSG_QUEUE_END
}

static void os2_MoveWMCursor(SDL_VideoDevice *pDevice, int iX, int iY)
{
}

static void os2_SetCaption(SDL_VideoDevice *pDevice, const char *pcTitle,
                           const char *pcIcon)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;

  debug( "New window title: \"%s\"", pcTitle == NULL ? "" : pcTitle );
  DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );

  if ( pPVData->pszTitle != NULL )
    SDL_free( pPVData->pszTitle );

  pPVData->pszTitle = pcTitle != NULL ? SDL_strdup( pcTitle ) : NULL;

  if ( pPVData->hwndFrame != NULLHANDLE )
  {
    debug( "Window exists - set title" );
    WIN_MSG_QUEUE_BEGIN

    if ( !WinSetWindowText( pPVData->hwndFrame, pPVData->pszTitle ) )
      debug( "WinSetWindowText() failed" );

    WIN_MSG_QUEUE_END
  }

  DosReleaseMutexSem( pPVData->hmtxLock );
}

static void os2_SetIcon(SDL_VideoDevice *pDevice, SDL_Surface *pSDLSurfaceIcon,
                        Uint8 *puiMask)
{
  debug( "Enter" );
}

static int os2_IconifyWindow(SDL_VideoDevice *pDevice)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;
  HWND			hwndFrame;

  debug( "Enter" );
  DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
  hwndFrame = pPVData->pFSVideoMode ? NULLHANDLE : pPVData->hwndFrame;
  DosReleaseMutexSem( pPVData->hmtxLock );

  return hwndFrame != NULLHANDLE &&
         WinSetWindowPos( hwndFrame, HWND_TOP, 0, 0, 0, 0, SWP_MINIMIZE ) ?
           1 : 0;
}

static SDL_GrabMode os2_GrabInput(SDL_VideoDevice *pDevice, SDL_GrabMode mode)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;

  debug( "Enter, mode: %s", mode == SDL_GRAB_ON ? "ON" : "OFF" );
  DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );

  pPVData->fMouseCapturable = mode == SDL_GRAB_ON;

  WIN_MSG_QUEUE_BEGIN

  if ( pPVData->fMouseCapturable )
  {
    if ( pPVData->hwndClient == NULLHANDLE )
    {
      DosReleaseMutexSem( pPVData->hmtxLock );
      debug( "Have not windows handle" );
    }
    else if ( WinQueryFocus( HWND_DESKTOP ) == pPVData->hwndClient )
    {
      _captureMouse( pPVData );
      WinSetPointer( HWND_DESKTOP, NULL );
    }
  }
  else if ( pPVData->fMouseCaptured )
  {
    WinSetCapture( HWND_DESKTOP, NULLHANDLE );
    WinSetPointer( HWND_DESKTOP,
                   WinQuerySysPointer( HWND_DESKTOP, SPTR_ARROW, FALSE ) );
    pPVData->fMouseCaptured = FALSE;
  }

  WIN_MSG_QUEUE_END

  DosReleaseMutexSem( pPVData->hmtxLock );
  debug( "Done" );
  return mode;
}

static void os2_UpdateMouse(SDL_VideoDevice *pDevice)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;
  POINTL		ptlMousePos;
  SWP			swpClient;

  debug( "Enter" );
  DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
  if ( pPVData->hwndClient == NULLHANDLE )
  {
    debug( "Have not window handle" );
    DosReleaseMutexSem( pPVData->hmtxLock );
    return;
  }

  if ( pPVData->pSDLSurface == NULL )
  {
    debug( "Have not surface" );
    DosReleaseMutexSem( pPVData->hmtxLock );
    return;
  }

//  WIN_MSG_QUEUE_BEGIN

  WinQueryPointerPos( HWND_DESKTOP, &ptlMousePos );
  WinMapWindowPoints( HWND_DESKTOP, pPVData->hwndClient, &ptlMousePos, 1 );
  WinQueryWindowPos( pPVData->hwndClient, &swpClient );

//  WIN_MSG_QUEUE_END

  ptlMousePos.x = ptlMousePos.x * pPVData->pSDLSurface->w / swpClient.cx;
  ptlMousePos.y = pPVData->pSDLSurface->h -
                  ( ptlMousePos.y * pPVData->pSDLSurface->h / swpClient.cy );

  DosReleaseMutexSem( pPVData->hmtxLock );

  SDL_PrivateAppActive( 1, SDL_APPMOUSEFOCUS );
  SDL_PrivateAppActive( 1, SDL_APPINPUTFOCUS );
  SDL_PrivateAppActive( 1, SDL_APPACTIVE );
  SDL_PrivateMouseMotion( 0, 0, ptlMousePos.x, ptlMousePos.y );
}

static int os2_ToggleFullScreen(SDL_VideoDevice *pDevice, int iOn)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;
  int			iResult = 0;

  debug( "Enter" );
  DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
  if ( pPVData->hwndClient == NULLHANDLE )
    debug( "Have not window handle" );
  else
  {
    WinPostMsg( pPVData->hwndClient, WM_FULLSCREEN,
                (MPARAM)pPVData->pSDLSurface, (MPARAM)iOn );
    iResult = 1;
  }
  DosReleaseMutexSem( pPVData->hmtxLock );

  debug( "Done" );
  return iResult;
}

static void os2_PumpEvents(SDL_VideoDevice *pDevice)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;
  SWP			swpWndSize;
  ULONG			ulRC;
  ULONG			ulTime;
  BOOL			fResized = FALSE;

  ulRC = DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
  if ( ulRC != NO_ERROR )
  {
    debug( "DosRequestMutexSem(), rc = %u", ulRC );
    if ( ulRC == ERROR_SEM_OWNER_DIED )
      SDL_PrivateQuit();
    return;
  }

  if ( !pPVData->pVideoSys->fFreeWinSize )
  {
    // Inform SDL about size changes every second
    DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT, &ulTime, sizeof(ULONG) );
    if ( (ULONG)(ulTime - pPVData->ulPumpEventsTimestamp) > 1000 )
    {
      pPVData->ulPumpEventsTimestamp = ulTime;

      if ( ( pPVData->pSDLSurface != NULL ) && pPVData->fResizable &&
           ( pPVData->hwndClient != NULLHANDLE ) && pPVData->fWndResized )
      {
        WinQueryWindowPos( pPVData->hwndClient, &swpWndSize );

        fResized = ( (swpWndSize.fl & SWP_MINIMIZE) == 0 ) &&
                   ( ( pPVData->pSDLSurface->w != swpWndSize.cx ) ||
                     ( pPVData->pSDLSurface->h != swpWndSize.cy ) );
        pPVData->fWndResized = FALSE;
      }
    }
  } // !pPVData->pVideoSys->fFreeWinSize

  DosReleaseMutexSem( pPVData->hmtxLock );

  if ( fResized )
    SDL_PrivateResize( swpWndSize.cx, swpWndSize.cy );
}

static void os2_UpdateRects(SDL_VideoDevice *pDevice, int cRects,
                            SDL_Rect *pRects)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;
  ULONG			ulRC;

  if ( cRects == 0 )
    return;

  ulRC = DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
  if ( ulRC != NO_ERROR )
  {
    debug( "DosRequestMutexSem(), rc = %u", ulRC );
    return;
  }

  if ( pPVData->pSDLSurface == NULL )
    debug( "Have not surface" );
  else if ( pPVData->hwndClient == NULL )
    debug( "Have not window handle" );
  else
    _updateRects( pPVData, cRects, pRects );

  DosReleaseMutexSem( pPVData->hmtxLock );
}

static void os2_InitOSKeymap(SDL_VideoDevice *pDevice)
{
  ULONG		ulIdx;

  for( ulIdx = 0; ulIdx < sizeof(aScan2SDLKey) / sizeof(struct SCAN2SDLKEY);
       ulIdx++ )
    aScanSDLKeyMap[ aScan2SDLKey[ulIdx].ulScan ] = aScan2SDLKey[ulIdx].enSDLKey;
}

static SDL_Surface *os2_SetVideoMode(SDL_VideoDevice *pDevice,
                                     SDL_Surface *pSDLSurfaceCurrent,
                                     int iWidth, int iHeight, int iBPP,
                                     Uint32 uiFlags)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;
  ULONG			ulRC;
  PVIDEOMODE		pVideoMode;
  PVIDEOMODE		pFSVideoMode = NULL;
  SDL_PixelFormat       sSDLPixelFormat;
  SDL_Surface		*pSDLSurface;
  struct VIDEOBUFHANDLE	*pVideoBufHdl, *pPrevVideoBufHdl;

  debug( "Requested: %dx%d, bpp: %d", iWidth, iHeight, iBPP );

  // No palette modes
  // if ( iBPP == 8 )
  //   iBPP = 32;

  if ( (uiFlags & SDL_RESIZABLE) != 0 )
  {
    debug( "SDL_RESIZABLE" );
    uiFlags &= ~SDL_FULLSCREEN;
  }

  // No hardware and double buffered support modes
  if ( (uiFlags & SDL_HWSURFACE) != 0 )
    uiFlags |= SDL_SWSURFACE;
  uiFlags &= ~(SDL_DOUBLEBUF | SDL_HWSURFACE);

  // Select video mode - algorithm from SDL/2 1.2 ported by Doodle 

  // Windowed mode. Select FS mode with desktop BPP and best resolution.
  if ( (uiFlags & SDL_FULLSCREEN) == 0 )
    iBPP = pPVData->pDesktopVideoMode->uiBPP;

  for( pVideoMode = pPVData->pVideoModes; pVideoMode != NULL;
       pVideoMode = pVideoMode->pNext )
  {
    // Check all available fullscreen modes for this resolution
    if ( ( pVideoMode->sSDLRect.w == iWidth ) &&
         ( pVideoMode->sSDLRect.h == iHeight ) )
    {
      // If good resolution, try to find the exact BPP, or something similar...
      if ( (uiFlags & SDL_FULLSCREEN) == 0 )
      {
        // For resizable (non-fullscreen) modes, we have to have the very same BPP as desktop!
        if ( pVideoMode->uiBPP == iBPP )
          pFSVideoMode = pVideoMode;
      }
      // For non-resizable modes, we don't have to force to desktop BPP
      else if ( ( pFSVideoMode == NULL ) ||
                ( ( pFSVideoMode->uiBPP != iBPP ) &&
                  ( pFSVideoMode->uiBPP < pVideoMode->uiBPP ) ) )
        pFSVideoMode = pVideoMode;
    }
  }

  // If we did not find a good fullscreen mode, then try a similar
  if ( pFSVideoMode == NULL )
  {
    // Go through the video modes again, and find a similar resolution!
    for( pVideoMode = pPVData->pVideoModes; pVideoMode != NULL;
         pVideoMode = pVideoMode->pNext )
    {
      // Check all available fullscreen modes for this resolution
      if ( ( pVideoMode->sSDLRect.w >= iWidth ) &&
           ( pVideoMode->sSDLRect.h >= iHeight ) &&
           ( pVideoMode->uiBPP == iBPP ) )
      {
        if ( ( pFSVideoMode == NULL ) ||
             (((pFSVideoMode->sSDLRect.w - iWidth) * (pFSVideoMode->sSDLRect.h - iHeight)) >
              ((pVideoMode->sSDLRect.w - iWidth) * (pVideoMode->sSDLRect.h - iHeight))) )
        {
          // Found a mode which is closer than the current one
          pFSVideoMode = pVideoMode;
        }
      }
    }
  }

  if ( pFSVideoMode == NULL )
  {
/*    SDL_SetError( "Appropriate video mode for %ux%u bpp %u not found",
                  iWidth, iHeight, iBPP );
    return NULL;*/
    debug( "Appropriate video mode for %ux%u bpp %u not found. "
           "Last chance - use desktop video mode.",
           iWidth, iHeight, iBPP );
    pFSVideoMode = pPVData->pDesktopVideoMode;
  }

  debug( "Found fullscreen mode: %dx%d, bpp: %d (mode id: 0x%X)",
         pFSVideoMode->sSDLRect.w, pFSVideoMode->sSDLRect.h,
         pFSVideoMode->uiBPP, pFSVideoMode->uiModeID );

  _getSDLPixelFormat( &sSDLPixelFormat, pFSVideoMode );
  if ( pPVData->pVideoSys->vsVideoBufAlloc == NULL )
  {
    pSDLSurface = SDL_CreateRGBSurface( SDL_SWSURFACE, iWidth, iHeight,
                    pFSVideoMode->uiBPP, sSDLPixelFormat.Rmask,
                    sSDLPixelFormat.Gmask, sSDLPixelFormat.Bmask,
                    sSDLPixelFormat.Amask );
  }
  else
  {
    ULONG cbLineSize;
    PVOID pVideoBuf = pPVData->pVideoSys->vsVideoBufAlloc( &pVideoBufHdl,
                         &cbLineSize, pFSVideoMode, iWidth, iHeight );

    if ( pVideoBuf == NULL )
    {
      SDL_NotEnoughMemory();
      return NULL;
    }

    pSDLSurface = SDL_CreateRGBSurfaceFrom( pVideoBuf, iWidth, iHeight,
                     pFSVideoMode->uiBPP, cbLineSize, sSDLPixelFormat.Rmask,
                     sSDLPixelFormat.Gmask, sSDLPixelFormat.Bmask,
                     sSDLPixelFormat.Amask );
  }

  if ( pSDLSurface == NULL )
  {
    SDL_NotEnoughMemory();
    if ( pPVData->pVideoSys->vsVideoBufFree != NULL )
      pPVData->pVideoSys->vsVideoBufFree( pVideoBufHdl );
    return NULL;
  }

  pSDLSurface->flags |= ( uiFlags & (SDL_FULLSCREEN | SDL_RESIZABLE) );
  debug( "Surface: %dx%d, bpp: %d, color mask: R:0x%X G:0x%X B:0x%X",
         pSDLSurface->w, pSDLSurface->h, pSDLSurface->format->BitsPerPixel,
         pSDLSurface->format->Rmask, pSDLSurface->format->Gmask,
         pSDLSurface->format->Bmask );

  ulRC = DosRequestMutexSem( pPVData->hmtxLock, SEM_INDEFINITE_WAIT );
  if ( ulRC != NO_ERROR )
  {
    SDL_SetError( "Cannot lock a mutex semaphore, rc = %u", ulRC );
    SDL_FreeSurface( pSDLSurface );
    if ( pPVData->pVideoSys->vsVideoBufFree != NULL )
      pPVData->pVideoSys->vsVideoBufFree( pVideoBufHdl );
    return NULL;
  }

  pPVData->pSDLSurface = pSDLSurface;
  pPVData->pFSVideoMode = pFSVideoMode;
  pPVData->fResizable = ( (uiFlags & SDL_RESIZABLE) != 0 ) ||
                        pPVData->pVideoSys->fFreeWinSize;
  pPrevVideoBufHdl = pPVData->pVideoBufHdl;
  pPVData->pVideoBufHdl = pVideoBufHdl;
  DosReleaseMutexSem( pPVData->hmtxLock );

  if ( !wndInit( pPVData ) )
  {
    SDL_FreeSurface( pSDLSurface );
    pPVData->pSDLSurface = NULL;
    if ( pPVData->pVideoSys->vsVideoBufFree != NULL )
      pPVData->pVideoSys->vsVideoBufFree( pVideoBufHdl );
    return NULL;
  }

  if ( (uiFlags & SDL_FULLSCREEN) != 0 )
    os2_ToggleFullScreen( pDevice, 1 );

  SDL_FreeSurface( pSDLSurfaceCurrent );
  if ( pPVData->pVideoSys->vsVideoBufFree != NULL && pPrevVideoBufHdl != NULL )
    pPVData->pVideoSys->vsVideoBufFree( pPrevVideoBufHdl );

  return pSDLSurface;
}

static int os2_VideoInit(SDL_VideoDevice *pDevice,
                         SDL_PixelFormat *pSDLPixelFormat)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;
  ULONG			ulRC;
  PTIB			tib;
  PPIB			pib;
  PSZ			pszFile;

  pDevice->info.current_w = pPVData->pDesktopVideoMode->sSDLRect.w;
  pDevice->info.current_h = pPVData->pDesktopVideoMode->sSDLRect.h;
  _getSDLPixelFormat( pSDLPixelFormat, pPVData->pDesktopVideoMode );

  ulRC = DosCreateMutexSem( NULL, &pPVData->hmtxLock, 0, FALSE );
  if ( ulRC != NO_ERROR )
  {
    SDL_SetError( "Cannot create a mutex semaphore, rc = %u", ulRC );
    return -1;
  }

  ulRC = DosCreateEventSem( NULL, &pPVData->hevWindowReady, 0, FALSE );
  if ( ulRC != NO_ERROR )
  {
    SDL_SetError( "Cannot create an event semaphore, rc = %u", ulRC );
    return -1;
  }

  ulRC = UniCreateUconvObject( L"", &pPVData->ucoUnicode );
  if ( ulRC != ULS_SUCCESS )
  {
    debug( "UniCreateUconvObject(), rc = %u", ulRC );
    pPVData->ucoUnicode = NULL;
  }

  pPVData->tidWnd = (TID)(-1);

  DosGetInfoBlocks( &tib, &pib );
  pszFile = SDL_strrchr( pib->pib_pchcmd, '\\' );
  pPVData->pszTitle = SDL_strdup( pszFile != NULL ?
                                    (pszFile + 1) : pib->pib_pchcmd );
  debug( "Program file: %s", pPVData->pszTitle );

  // Change process type code for use Win* API from VIO session 
  pPVData->fVIOSession = pib->pib_ultype == 2; // VIO windowable protect-mode session
  if ( pPVData->fVIOSession )
  {
    debug( "Change process type code to PM session." );
    pib->pib_ultype = 3; // Presentation Manager protect-mode session
  }

  return 0;
}

static void os2_VideoQuit(SDL_VideoDevice *pDevice)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;

  debug( "Enter" );
  wndDone( pPVData );

  if ( pPVData->pVideoSys->vsVideoBufFree != NULL &&
       pPVData->pVideoBufHdl != NULL )
  {
    debug( "vsVideoBufFree()..." );
    pPVData->pVideoSys->vsVideoBufFree( pPVData->pVideoBufHdl );
    debug( "vsVideoBufFree() - done" );
  }

  if ( pPVData->ucoUnicode != NULL )
  {
    debug( "UniFreeUconvObject()..." );
    UniFreeUconvObject( pPVData->ucoUnicode );
    debug( "UniFreeUconvObject() - done" );
  }

  if ( pPVData->pszTitle != NULL )
    SDL_free( pPVData->pszTitle );

  DosCloseEventSem( pPVData->hevWindowReady );
  DosCloseMutexSem( pPVData->hmtxLock );

  debug( "Done" );
}

static SDL_Rect **os2_ListModes(SDL_VideoDevice *pDevice,
                                SDL_PixelFormat *pSDLPixelFormat,
                                Uint32 uiFlags)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;
  ULONG			ulBPP = pSDLPixelFormat->BitsPerPixel;
  SDL_Rect		**ppSDLRect;

  if (
       // Any size for not fullscreen modes
       ( (uiFlags & SDL_FULLSCREEN) == 0 ) ||
       // VideoSys engine does not fill sorted list - allow any size
       ( pPVData->pListVideoModes == NULL ) )
  {
    ppSDLRect = (SDL_Rect **)(-1); // Any size, no fullscreen.
  }
  else if ( ( (ulBPP & 0x07) != 0 && ulBPP != 15 ) || (ulBPP > 32) )
  {
    debug( "Unsupported BPP: %u", ulBPP );
    ppSDLRect = NULL;
  }
  else
    ppSDLRect = pPVData->pListVideoModes[ ulBPP == 15 ? 0 : (ulBPP >> 3) ];

  return ppSDLRect;
}

static void os2_DeleteDevice(SDL_VideoDevice *pDevice)
{
  SDL_PrivateVideoData	*pPVData = pDevice->hidden;

  debug( "Enter" );
  pPVData->pVideoSys->vsFreeVideoModes( pPVData );
  pPVData->pVideoSys->vsDone(pPVData);
  SDL_free( pPVData );
  SDL_free( pDevice );

  os2iniClose();
  debug( "Done" );
}

static SDL_VideoDevice *_createDevice(int devindex, PVIDEOSYS pVideoSys)
{
  SDL_VideoDevice	*pDevice;
  SDL_PrivateVideoData	*pPVData;

  pDevice = SDL_calloc( 1, sizeof(SDL_VideoDevice) );
  if ( pDevice == NULL )
  {
    SDL_NotEnoughMemory();
    return NULL;
  }

  pPVData = SDL_calloc( 1, sizeof(struct SDL_PrivateVideoData) );
  if ( pDevice == NULL )
  {
    SDL_NotEnoughMemory();
    SDL_free( pDevice );
    return NULL;
  }
  pDevice->hidden = pPVData;

  pPVData->pVideoSys = pVideoSys;
  pPVData->fKeyAltHome = os2iniGetBool( "alt+home", TRUE );
  pPVData->fKeyAltEnd = os2iniGetBool( "alt+end", TRUE );
  pPVData->fKeyAltGrEnter = os2iniGetBool( "altgr+enter", FALSE );

  if ( !pPVData->pVideoSys->vsInit( pPVData ) ||
       !pPVData->pVideoSys->vsQueryVideoModes( pPVData ) )
  {
    SDL_free( pDevice->hidden );
    SDL_free( pDevice );
    pPVData->pVideoSys->vsDone();
    return NULL;
  }

  /* Initialization/Query functions */
  pDevice->VideoInit = os2_VideoInit;
  pDevice->ListModes = os2_ListModes;
  pDevice->SetVideoMode = os2_SetVideoMode;
  pDevice->ToggleFullScreen = os2_ToggleFullScreen;
  pDevice->UpdateMouse = os2_UpdateMouse;
//  pDevice->CreateYUVOverlay = NULL;
  pDevice->SetColors = os2_SetColors;
  pDevice->UpdateRects = os2_UpdateRects;
  pDevice->VideoQuit = os2_VideoQuit;
  /* Hardware acceleration functions */
  pDevice->AllocHWSurface = os2_AllocHWSurface;
//  pDevice->CheckHWBlit = NULL;
//  pDevice->FillHWRect = NULL;
//  pDevice->SetHWColorKey = NULL;
//  pDevice->SetHWAlpha = NULL;
  pDevice->LockHWSurface = os2_LockHWSurface;
  pDevice->UnlockHWSurface = os2_UnlockHWSurface;
//  pDevice->FlipHWSurface = NULL;
  pDevice->FreeHWSurface = os2_FreeHWSurface;
  /* Window manager functions */
  pDevice->SetCaption = os2_SetCaption;
  pDevice->SetIcon = os2_SetIcon;
  pDevice->IconifyWindow = os2_IconifyWindow;
  pDevice->GrabInput = os2_GrabInput;
//  pDevice->GetWMInfo = NULL;
  /* Cursor manager functions */
  pDevice->FreeWMCursor = os2_FreeWMCursor;
  pDevice->CreateWMCursor = os2_CreateWMCursor;
  pDevice->ShowWMCursor = os2_ShowWMCursor;
  pDevice->WarpWMCursor = os2_WarpWMCursor;
  pDevice->MoveWMCursor = os2_MoveWMCursor;
//  pDevice->CheckMouseMode = NULL;
  /* Event manager functions */
  pDevice->InitOSKeymap = os2_InitOSKeymap;
  pDevice->PumpEvents = os2_PumpEvents;
  /* The function used to dispose of this structure */
  pDevice->free = os2_DeleteDevice;
  /* Driver can center a smaller surface to simulate fullscreen */
  pDevice->handles_any_size = 1;

  return pDevice;
}


static SDL_VideoDevice *os2VMan_CreateDevice(int devindex)
{
  debug( "Enter" );
  return _createDevice( devindex, &sVSVMan );
}

static int os2VMan_Available()
{
  debug( "Enter" );
  os2iniOpen();

  if ( os2iniIsValue( "video", "dive" ) )
  {
    debug( "Switch \"video\" in ini-file is DIVE - ignore VMAN" );
    return 0;
  }

  // Check VMAN and clean up VMAN subsystem after initialization
  if ( !sVSVMan.vsInit( NULL ) )
  {
    debug( "VMAN initialization failed" );
    return 0;
  }

  return 1;
}


static SDL_VideoDevice *os2DIVE_CreateDevice(int devindex)
{
  debug( "Enter" );
  return _createDevice( devindex, &sVSDIVE );
}

static int os2DIVE_Available()
{
  debug( "Enter" );
  os2iniOpen();

  if ( os2iniIsValue( "video", "vman" ) )
  {
    debug( "Switch \"video\" in ini-file is VMAN - ignore DIVE" );
    return 0;
  }

  // Check DIVE
  if ( !sVSDIVE.vsInit( NULL ) )
  {
    debug( "DIVE initialization failed" );
    return 0;
  }

  return 1;
}


VideoBootStrap OS2VMAN_bootstrap =
  { "OS2", "OS/2 VMAN", os2VMan_Available, os2VMan_CreateDevice };

VideoBootStrap OS2DIVE_bootstrap =
  { "OS2", "OS/2 DIVE", os2DIVE_Available, os2DIVE_CreateDevice };
