#define INCL_OS2MM
#define INCL_GPI
#include "SDL_os2alt.h"
#include <os2.h>
#define  _MEERROR_H_
#include <mmioos2.h>
#include <os2me.h>
#define INCL_MM_OS2
#include <dive.h>
#include <fourcc.h>

struct VIDEOBUFHANDLE {
  PVOID		pVideoBuf;
  ULONG		ulDiveBufNum;
};

static HDIVE		hDive;
static BOOL		fSetupBlitter;

BOOL DIVEInit(SDL_PrivateVideoData *pPVData)
{
  DIVE_CAPS		sDiveCaps = { 0 };
  FOURCC		fccFormats[100]	= { 0 };
  PVIDEOMODE		pVideoMode;

  fSetupBlitter = FALSE;

  sDiveCaps.pFormatData    = fccFormats;
  sDiveCaps.ulFormatLength = 100;
  sDiveCaps.ulStructLen    = sizeof(DIVE_CAPS);

  if ( DiveQueryCaps( &sDiveCaps, DIVE_BUFFER_SCREEN ) )
  {
    debug( "DIVE routines cannot function in this system environment." );
    return FALSE;
  }

  if ( sDiveCaps.ulDepth < 8 )
  {
    debug( "Not enough screen colors to run DIVE. "
           "Must be at least 256 colors." );
    return FALSE;
  }

  if ( DiveOpen( &hDive, FALSE, NULL ) )
  {
    SDL_SetError( "Unable to open an instance of DIVE." );
    return FALSE;
  }

  if ( pPVData == NULL )
  {
    // It only check DIVE availability...
    DiveClose( hDive );
    return TRUE;
  }

  pVideoMode = SDL_malloc( sizeof(VIDEOMODE) );
  if ( pVideoMode == NULL )
  {
    debug( "Not enough memory" );
    DiveClose( hDive );
    return FALSE;
  }

  pVideoMode->pNext = NULL;	// Only one video mode - current
  pVideoMode->uiModeID = 0;	// Not used with DIVE
  pVideoMode->sSDLRect.x = 0;
  pVideoMode->sSDLRect.y = 0;
  pVideoMode->sSDLRect.w = sDiveCaps.ulHorizontalResolution;
  pVideoMode->sSDLRect.h = sDiveCaps.ulVerticalResolution;
  pVideoMode->uiBPP = sDiveCaps.ulDepth;
  pVideoMode->uiScanLineSize = sDiveCaps.ulScanLineBytes;
  pVideoMode->fccColorEncoding = sDiveCaps.fccColorEncoding;

  pPVData->pVideoModes = pVideoMode;	   // Only one video mode - current
  pPVData->pDesktopVideoMode = pVideoMode;

  return TRUE;
}

VOID DIVEDone(SDL_PrivateVideoData *pPVData)
{
  DiveClose( hDive );
}

BOOL DIVEQueryVideoModes(SDL_PrivateVideoData *pPVData)
{
  // Video mode is already detected in DIVEInit(not-NULL).
  // Only one (current) video mode listed in pPVData->pVideoModes.

  ULONG		cFSModes = pPVData->pVideoModes->uiBPP >> 3;
  ULONG		ulIdx;
  SDL_Rect	**ppSDLRectList;

  // Make lists of resolutions for all BPP <= current mode. Lists for each BPP
  // contains only one item - pointer to current resolution.

  ppSDLRectList = SDL_malloc( 2 * sizeof(SDL_Rect*) );
  if ( ppSDLRectList == NULL )
  {
    debug( "Not enough memory" );
    return FALSE;
  }

  ppSDLRectList[0] = &pPVData->pVideoModes->sSDLRect;
  ppSDLRectList[1] = NULL;                            // "end of list"

  for( ulIdx = 0; ulIdx < cFSModes; ulIdx++ )
    pPVData->pListVideoModes[ ulIdx + 1 ] = ppSDLRectList;

  return TRUE;
}

VOID DIVEFreeVideoModes(SDL_PrivateVideoData *pPVData)
{
  // We have only one video mode with DIVE
  SDL_free( pPVData->pVideoModes );
  // All resolutions sorted list contains same pointers.
  SDL_free( pPVData->pListVideoModes[0] );
}

VOID DIVEUpdateRects(SDL_PrivateVideoData *pPVData, SDL_Rect *pSDLRectWin,
                     SDL_Rect *pSDLRectFrame, int cRects, SDL_Rect *pRects)
{
  ULONG			ulRC;
  PBYTE			pbLineMask;
  ULONG			ulY, ulH;
  BOOL			fFullUpdate = cRects == 0;

  if ( pPVData->pSDLSurface == NULL )
  {
    debug( "No surface" );
    return;
  }

  if ( !fSetupBlitter )
  {
//    debug( "No blitter" );
    return;
  }

  if ( !fFullUpdate )
  {
    ULONG		ulSurfaceHeight = pPVData->pSDLSurface->h;

    pbLineMask = alloca( ulSurfaceHeight );
    if ( pbLineMask == NULL )
    {
      debug( "Not enough stack size" );
      return;
    }
    memset( pbLineMask, 0, ulSurfaceHeight );

    for( ; cRects > 0; cRects--, pRects++ )
    {
      ulY = pRects->y;
      ulH = pRects->h;
      if ( ulY == 0 && ulH == 0 && pRects->x == 0 && pRects->w == 0 )
      {
        fFullUpdate = TRUE;
        break;
      }

      if ( ulY >= ulSurfaceHeight )
        continue;

      if ( (ulY + ulH) > ulSurfaceHeight )
        ulH = pRects->h - ulY;

      if ( ulH == 0 )
        continue;

      if ( ulH >= ulSurfaceHeight )
      {
        fFullUpdate = TRUE;
        break;
      }

      memset( &pbLineMask[ulY], 1, ulH );
    }
  }

  if ( fFullUpdate )
  {
    ulRC = DiveBlitImage( hDive, pPVData->pVideoBufHdl->ulDiveBufNum,
                          DIVE_BUFFER_SCREEN );
    if ( ulRC != DIVE_SUCCESS )
      debug( "DiveBlitImage(), rc = %u", ulRC );
  }
  else
  {
    ulRC = DiveBlitImageLines( hDive, pPVData->pVideoBufHdl->ulDiveBufNum,
                               DIVE_BUFFER_SCREEN, pbLineMask );
    if ( ulRC != DIVE_SUCCESS )
      debug( "DiveBlitImageLines(), rc = %u", ulRC );
  }
}

BOOL DIVESetPalette(SDL_PrivateVideoData *pPVData, ULONG ulStart,
                    ULONG cColors, SDL_Color *pColors)
{
  ULONG			ulRC;

  ulRC = DiveSetDestinationPalette( hDive, ulStart, cColors, (PBYTE)pColors );
  if ( ulRC != DIVE_SUCCESS )
  {
    SDL_SetError( "DiveSetDestinationPalette(), rc = %u", ulRC );
    return FALSE;
  }

  return FALSE;
}

VOID DIVEVisibleRegion(SDL_PrivateVideoData *pPVData, ULONG ulFlag)
{
  ULONG			ulRC;
  SDL_Rect		sSDLRectFrame;
  SIZEL			szlWndSize;
  POINTL		ptlWndPos;
  HPS			hps;
  HRGN			hrgn;
  RGNRECT		rgnCtl;
  RECTL			rcls[50];
  SETUP_BLITTER		sSetupBlitter;

  if ( ulFlag == VRN_DISABLED )
  {
    if ( pPVData->pDesktopVideoMode->uiBPP == 8 )
      DiveSetDestinationPalette( hDive, 0, 0, 0 );

    fSetupBlitter = FALSE;
    return;
  }

  if ( pPVData->pSDLSurface == NULL )
  {
    debug( "No surface" );
    fSetupBlitter = FALSE;
    return;
  }

  // Detect sizes and positions of window and frame
  ptlWndPos.x = 0;
  ptlWndPos.y = 0;
  WinMapWindowPoints( pPVData->hwndClient, HWND_DESKTOP, &ptlWndPos, 1 );
  os2alt_getFrame( pPVData, &sSDLRectFrame, &szlWndSize );

  // Query visible rectangles

  hps = WinGetPS( pPVData->hwndClient );
  hrgn = GpiCreateRegion( hps, 0, NULL );
  if ( !hrgn )
  {
    WinReleasePS( hps );
    debug( "GpiCreateRegion() fail" );
    return;
  }

  WinQueryVisibleRegion( pPVData->hwndClient, hrgn );
  rgnCtl.ircStart     = 0;
  rgnCtl.crc          = sizeof(rcls) / sizeof(RECTL);
  rgnCtl.ulDirection  = 1;
  GpiQueryRegionRects( hps, hrgn, NULL, &rgnCtl, rcls );
  GpiDestroyRegion( hps, hrgn );
  WinReleasePS( hps );

  if ( rgnCtl.crcReturned > rgnCtl.crc )
    debug( "Not enough length of the rectangles list" );

  // Setup blitter

  sSetupBlitter.ulStructLen       = sizeof(SETUP_BLITTER);
  sSetupBlitter.fccSrcColorFormat = pPVData->pDesktopVideoMode->fccColorEncoding;
  sSetupBlitter.ulSrcPosX         = 0;
  sSetupBlitter.ulSrcPosY         = 0;
  sSetupBlitter.ulSrcWidth        = pPVData->pSDLSurface->w;
  sSetupBlitter.ulSrcHeight       = pPVData->pSDLSurface->h;
  sSetupBlitter.fccDstColorFormat = FOURCC_SCRN;
  sSetupBlitter.lDstPosX          = sSDLRectFrame.x;
  sSetupBlitter.lDstPosY          = szlWndSize.cy - sSDLRectFrame.h - sSDLRectFrame.y;
  sSetupBlitter.ulDstWidth        = sSDLRectFrame.w;
  sSetupBlitter.ulDstHeight       = sSDLRectFrame.h;
  sSetupBlitter.fInvert           = FALSE;
  sSetupBlitter.ulDitherType      = 0;
  sSetupBlitter.lScreenPosX       = ptlWndPos.x;
  sSetupBlitter.lScreenPosY       = ptlWndPos.y;
  sSetupBlitter.ulNumDstRects     = rgnCtl.crcReturned;
  sSetupBlitter.pVisDstRects      = &rcls;

  ulRC = DiveSetupBlitter( hDive, &sSetupBlitter );
  if ( ulRC != DIVE_SUCCESS )
  {
    debug( "DiveSetupBlitter(), rc = %u", ulRC );
    fSetupBlitter = FALSE;
  }
  else
  {
    fSetupBlitter = TRUE;
    WinInvalidateRect( pPVData->hwndClient, NULL, TRUE );
  }
}

PVOID DIVEVideoBufAlloc(struct VIDEOBUFHANDLE **ppVideoBufHdl,
                        PULONG pcbLineSize, PVIDEOMODE pVideoMode,
                        ULONG ulWidth, ULONG ulHeight)
{
  ULONG		ulRC;
  ULONG		cbLineSize = ulWidth * (pVideoMode->uiBPP >> 3);
  ULONG		ulDiveBufNum = 0;
  PVOID		pVideoBuf = NULL;

  debug( "Enter" );

  // SDL-like calculation (see SDL_CalculatePitch())
  switch( cbLineSize )
  {
    case 1:
      cbLineSize = ( cbLineSize + 7 ) / 8;
      break;
    case 4:
      cbLineSize = ( cbLineSize + 1 ) / 2;
      break;
  }
  cbLineSize = ( cbLineSize + 3 ) & ~3;	/* 4-byte aligning */

  ulRC = DosAllocMem( &pVideoBuf, ulHeight * cbLineSize,
                      PAG_COMMIT | PAG_EXECUTE | PAG_READ | PAG_WRITE );
  if ( ulRC != NO_ERROR )
  {
    SDL_SetError( "DosAllocMem(), rc = %u", ulRC );
    return NULL;
  }

  ulRC = DiveAllocImageBuffer( hDive, &ulDiveBufNum,
                               pVideoMode->fccColorEncoding,
                               ulWidth, ulHeight, cbLineSize, pVideoBuf );
  if ( ulRC != DIVE_SUCCESS )
  {
    debug( "DiveAllocImageBuffer(), rc = 0x%X", ulRC );
    DosFreeMem( pVideoBuf );
    return NULL;
  }

  *pcbLineSize = cbLineSize;

  *ppVideoBufHdl = SDL_malloc( sizeof(struct VIDEOBUFHANDLE) );
  (*ppVideoBufHdl)->pVideoBuf = pVideoBuf;
  (*ppVideoBufHdl)->ulDiveBufNum = ulDiveBufNum;
  debug( "pVideoBuf = %p, ulDiveBufNum = %u, pVideoBufHdl = %p",
         pVideoBuf, ulDiveBufNum, *ppVideoBufHdl );
  return pVideoBuf;
}

VOID DIVEVideoBufFree(struct VIDEOBUFHANDLE *pVideoBufHdl)
{
  debug( "Enter" );
  DiveFreeImageBuffer( hDive, pVideoBufHdl->ulDiveBufNum );
  DosFreeMem( pVideoBufHdl->pVideoBuf );
  SDL_free( pVideoBufHdl );
}


VIDEOSYS	sVSDIVE =
{
  TRUE,
  DIVEInit,		// vsInit
  DIVEDone,		// vsDone
  DIVEQueryVideoModes,	// vsQueryVideoModes
  DIVEFreeVideoModes,	// vsFreeVideoModes
  DIVEUpdateRects,	// vsUpdateRects
  NULL,			// vsSetFullscreen
  DIVESetPalette,	// vsSetPalette
  DIVEVisibleRegion,	// vsVisibleRegion
  DIVEVideoBufAlloc,	// vsVideoBufAlloc
  DIVEVideoBufFree	// vsVideoBufFree
};
