#include <malloc.h>
#include <string.h>
#define INCL_OS2MM
#define INCL_GPI
#define INCL_DOSMEMMGR
#include <os2.h>
#define  _MEERROR_H_
#include <mmioos2.h>
#include <os2me.h>
#define INCL_MM_OS2
#include <dive.h>
#include <fourcc.h>
#include "debug.h"
#include "grop.h"

static BOOL vsInit(PVIDEOMODESLIST pModes, PVOID *ppVSData);
static VOID vsDone(PVOID pVSData, PVIDEOMODESLIST pModes);
static BOOL vsSetMode(PVOID pVSData, PVIDEOMODE pMode, BOOL fFullscreen, HWND hwndDT,
                      HDC hdcDT);
static PVOID vsVideoBufAlloc(PVOID pVSData, ULONG ulWidth, ULONG ulHeight,
                  ULONG ulBPP, ULONG fccColorEncoding, PULONG pulScanLineSize);
static VOID vsVideoBufFree(PVOID pVSData, PVOID pBuffer);
static VOID vsSetVisibleRegion(PVOID pVSData, HWND hwnd, PGROPSETMODE pUserMode,
                             PRECTL prectlWin, PRECTL prectlVA, BOOL fVisible);
static BOOL vsUpdate(PVOID pVSData, PGROPDATA pGrop, ULONG cRect, PRECTL rectl);
static BOOL vsSetPalette(PVOID pVSData, ULONG ulFirst, ULONG ulNumber,
                         PRGB2 pColors);

VIDEOSYS stVideoSysDIVE =
{
  TRUE,                // fFreeWinSize
  TRUE,                // fBPPConvSup
  vsInit,
  vsDone,
  vsSetMode,
  vsVideoBufAlloc,
  vsVideoBufFree,
  vsSetVisibleRegion,
  vsUpdate,
  vsSetPalette
};


typedef struct _DIVEData {
  HDIVE      hDive;
  BOOL       fBlitterReady;
} DIVEData, *PDIVEData;

// BOOL vsInit(PVIDEOMODESLIST pModes, PVOID *ppVSData)
//
// Output video system initialization. This function should fill VIDEOMODESLIST
// structure pointed by pModes.
// Returns TRUE on success.

static BOOL vsInit(PVIDEOMODESLIST pModes, PVOID *ppVSData)
{
  PDIVEData  pDIVEData;
  DIVE_CAPS  sDiveCaps = { 0 };
  FOURCC     fccFormats[100] = { 0 };
  PVIDEOMODE pMode;
  ULONG      ulIdx, cModes;

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

  pDIVEData = debugMAlloc( sizeof(DIVEData) );
  if ( pDIVEData == NULL )
  {
    debug( "Not enough memory" );
    return FALSE;
  }

  if ( DiveOpen( &pDIVEData->hDive, FALSE, NULL ) )
  {
    debug( "Unable to open an instance of DIVE." );
    debugFree( pDIVEData );
    return FALSE;
  }
  pDIVEData->fBlitterReady = FALSE;
  *ppVSData = pDIVEData;

  // Fake video modes for DIVE - all with desktop resolution and BPP from 8 to
  // current (desktop) mode BPP.

  cModes = sDiveCaps.ulDepth >> 3;
  pMode = debugMAlloc( sizeof(VIDEOMODE) * cModes );
  if ( pMode == NULL )
  {
    debug( "Not enough memory" );
    DiveClose( pDIVEData->hDive );
    debugFree( pDIVEData );
    return FALSE;
  }

  pModes->pList = pMode;
  pModes->ulCount = cModes;
  pModes->ulDesktopMode = cModes - 1;

  // Fill modes list.

  for( ulIdx = 1; ulIdx < cModes; ulIdx++, pMode++ )
  {
    pMode->ulId                    = ulIdx;      // Not used with DIVE
    pMode->ulWidth                 = sDiveCaps.ulHorizontalResolution;
    pMode->ulHeight                = sDiveCaps.ulVerticalResolution;
      // DiveSetupBlitter() returns DIVE_ERR_INVALID_CONVERSION on my
      // system for 24bit buffer. Just dirty skip it here...
    pMode->ulBPP                   = ulIdx == 3 ? 16 : (ulIdx << 3);

    pMode->ulScanLineSize = sDiveCaps.ulHorizontalResolution * ulIdx;
    pMode->ulScanLineSize = ( pMode->ulScanLineSize + 3 ) & ~3;

    switch( ulIdx )
    {
      case 1: // 8 bit
        pMode->fccColorEncoding        = FOURCC_LUT8;
        break;

      case 2: // 16 bit
        pMode->fccColorEncoding        = FOURCC_R565;
        break;

      case 3: // 24 bit
        pMode->fccColorEncoding        = FOURCC_R664;//FOURCC_RGB3;
        break;

      case 4: // 32 bit
        pMode->fccColorEncoding        = FOURCC_RGB4;
        break;
    }
  }

  // Last mode is a desktop mode.
  pMode->ulId                    = ulIdx;
  pMode->ulWidth                 = sDiveCaps.ulHorizontalResolution;
  pMode->ulHeight                = sDiveCaps.ulVerticalResolution;
  pMode->fccColorEncoding        = sDiveCaps.fccColorEncoding;
  pMode->ulBPP                   = sDiveCaps.ulDepth;
  pMode->ulScanLineSize          = sDiveCaps.ulScanLineBytes;

  debug( "Success" );
  return TRUE;
}

// VOID vsDone(PVOID pVSData, PVIDEOMODESLIST pModes)
//
// Output video system finalization. This function should destroy
// VIDEOMODESLIST structure pointed by pModes.

static VOID vsDone(PVOID pVSData, PVIDEOMODESLIST pModes)
{
  PDIVEData  pDIVEData = (PDIVEData)pVSData;

  if ( pDIVEData->fBlitterReady )
    DiveSetupBlitter( pDIVEData->hDive, 0 );

  debugFree( pModes->pList );
  pModes->pList = NULL;

  DiveClose( pDIVEData->hDive );
  debugFree( pDIVEData );
}

// BOOL vsSetMode(PVOID pVSData, PVIDEOMODE pMode, BOOL fFullscreen,
//                HWND hwndDT, HDC hdcDT)
//
// Sets video mode pMode - one of listed in vsInit(). Flag fFullscreen may be
// FALSE only if pMode is the desktop mode defined in vsInit().
// hwndDT, hdcDT - window handle & device context handle of the desktop window.
// Returns TRUE on success.

static BOOL vsSetMode(PVOID pVSData, PVIDEOMODE pMode, BOOL fFullscreen,
                      HWND hwndDT, HDC hdcDT)
{
  // We don't real changes video mode with DIVE.
  return TRUE;
}

static PVOID vsVideoBufAlloc(PVOID pVSData, ULONG ulWidth, ULONG ulHeight,
                             ULONG ulBPP, ULONG fccColorEncoding,
                             PULONG pulScanLineSize)
{
  PDIVEData  pDIVEData = (PDIVEData)pVSData;
  ULONG      ulRC;
  ULONG      ulScanLineSize = ulWidth * (ulBPP >> 3);
  PVOID      pBuffer, pVideoBuffer;

  // Bytes per line.
  ulScanLineSize = ( ulScanLineSize + 3 ) & ~3;	/* 4-byte aligning */
  *pulScanLineSize = ulScanLineSize;

  debug( "Allocate %u bytes...", (ulHeight * ulScanLineSize) + sizeof(ULONG) );
  ulRC = DosAllocMem( &pBuffer, (ulHeight * ulScanLineSize) + sizeof(ULONG),
                      PAG_COMMIT | PAG_EXECUTE | PAG_READ | PAG_WRITE );
  if ( ulRC != NO_ERROR )
  {
    debug( "DosAllocMem(), rc = %u", ulRC );
    return NULL;
  }

  // First 4 bytes of allocated memory space will be used to save DIVE buffer
  // number. 5'th byte of allocated buffer is a begining of the user buffer.
  pVideoBuffer = &((PULONG)pBuffer)[1];

  debug( "Memory allocated: 0x%P, video buffer: 0x%P", pBuffer, pVideoBuffer );
  ulRC = DiveAllocImageBuffer( pDIVEData->hDive, (PULONG)pBuffer,
                               fccColorEncoding, ulWidth, ulHeight,
                               ulScanLineSize, pVideoBuffer );
  if ( ulRC != DIVE_SUCCESS )
  {
    debug( "DiveAllocImageBuffer(), rc = 0x%X", ulRC );
    DosFreeMem( pBuffer );
    return NULL;
  }

  debug( "buffer: 0x%P, DIVE buffer number: %u",
         pVideoBuffer, *(PULONG)pBuffer );

  return pVideoBuffer;
}

static VOID vsVideoBufFree(PVOID pVSData, PVOID pBuffer)
{
  PDIVEData  pDIVEData = (PDIVEData)pVSData;
  ULONG      ulRC;

  // Move pointer pBuffer to the allocated memory space (-4 bytes).
  pBuffer = (PVOID)( ((PULONG)pBuffer) - 1 );
  // First 4 bytes of pBuffer now is the DIVE buffer number.
  ulRC = DiveFreeImageBuffer( pDIVEData->hDive, *((PULONG)pBuffer) );
  if ( ulRC != DIVE_SUCCESS )
    debug( "DiveFreeImageBuffer(,%u), rc = %u", *((PULONG)pBuffer), ulRC );

  ulRC = DosFreeMem( pBuffer );
  if ( ulRC != NO_ERROR )
    debug( "DosFreeMem(), rc = %u", ulRC );
}

static VOID vsSetVisibleRegion(PVOID pVSData, HWND hwnd, PGROPSETMODE pUserMode,
                               PRECTL prectlWin, PRECTL prectlVA, BOOL fVisible)
{
  PDIVEData  pDIVEData = (PDIVEData)pVSData;
  HPS        hps;
  HRGN       hrgn;
  RGNRECT    rgnCtl;
  PRECTL     prectl = NULL;
  ULONG      ulRC;

  if ( !fVisible )
  {
    if ( pDIVEData->fBlitterReady )
    {
      pDIVEData->fBlitterReady = FALSE;
      DiveSetupBlitter( pDIVEData->hDive, 0 );
    }
    return;
  }

  // Query visible rectangles

  hps = WinGetPS( hwnd );
  hrgn = GpiCreateRegion( hps, 0, NULL );
  if ( !hrgn )
  {
    WinReleasePS( hps );
    debug( "GpiCreateRegion() failed" );
    return;
  }

  WinQueryVisibleRegion( hwnd, hrgn );
  rgnCtl.ircStart     = 1;
  rgnCtl.crc          = 0;
  rgnCtl.ulDirection  = 1;
  GpiQueryRegionRects( hps, hrgn, NULL, &rgnCtl, NULL );
  if ( rgnCtl.crcReturned != 0 )
  {
    prectl = debugMAlloc( rgnCtl.crcReturned * sizeof(RECTL) );
    if ( prectl != NULL )
    {
      rgnCtl.ircStart     = 1;
      rgnCtl.crc          = rgnCtl.crcReturned;
      rgnCtl.ulDirection  = 1;
      GpiQueryRegionRects( hps, hrgn, NULL, &rgnCtl, prectl );
    }
    else
      debug( "Not enough memory" );
  }
  GpiDestroyRegion( hps, hrgn );
  WinReleasePS( hps );

  if ( prectl != NULL )
  {
    // Setup DIVE blitter.
    SETUP_BLITTER      sSetupBlitter;

    sSetupBlitter.ulStructLen       = sizeof(SETUP_BLITTER);
    sSetupBlitter.fccSrcColorFormat = pUserMode->fccColorEncoding;
    sSetupBlitter.fInvert           = FALSE;
    sSetupBlitter.ulSrcWidth        = pUserMode->ulWidth;
    sSetupBlitter.ulSrcHeight       = pUserMode->ulHeight;
    sSetupBlitter.ulSrcPosX         = 0;
    sSetupBlitter.ulSrcPosY         = 0;
    sSetupBlitter.ulDitherType      = 0;
    sSetupBlitter.fccDstColorFormat = FOURCC_SCRN;
    sSetupBlitter.ulDstWidth        = prectlVA->xRight - prectlVA->xLeft;
    sSetupBlitter.ulDstHeight       = prectlVA->yTop - prectlVA->yBottom;
    sSetupBlitter.lDstPosX          = prectlVA->xLeft;
    sSetupBlitter.lDstPosY          = prectlVA->yBottom;
    sSetupBlitter.lScreenPosX       = prectlWin->xLeft;
    sSetupBlitter.lScreenPosY       = prectlWin->yBottom;
    sSetupBlitter.ulNumDstRects     = rgnCtl.crcReturned;
    sSetupBlitter.pVisDstRects      = prectl;

    ulRC = DiveSetupBlitter( pDIVEData->hDive, &sSetupBlitter );
    if ( ulRC != DIVE_SUCCESS )
    {
      debug( "DiveSetupBlitter(), rc = 0x%X", ulRC );
      pDIVEData->fBlitterReady = FALSE;
      DiveSetupBlitter( pDIVEData->hDive, 0 );
    }
    else
    {
      pDIVEData->fBlitterReady = TRUE;
      WinInvalidateRect( hwnd, NULL, TRUE );
    }

    debugFree( prectl );
  }
}

static BOOL vsUpdate(PVOID pVSData, PGROPDATA pGrop, ULONG cRect, PRECTL prectl)
{
  PDIVEData  pDIVEData = (PDIVEData)pVSData;
  ULONG      ulRC;
  BOOL       fFullUpdate = cRect == 0;
  ULONG      ulDiveBufNum;
  ULONG      ulSrcHeight = pGrop->stUserMode.ulHeight;

  if ( ( pGrop->stUserMode.pBuffer == NULL ) || ( !pDIVEData->fBlitterReady ) )
    return FALSE;
  ulDiveBufNum = *( ((PULONG)pGrop->stUserMode.pBuffer) - 1 );

  if ( !fFullUpdate )
  {
    LONG     lTop, lBottom;
    PBYTE    pbLineMask;

    pbLineMask = alloca( ulSrcHeight );
    if ( pbLineMask == NULL )
    {
      debug( "Not enough stack size" );
      return FALSE;
    }
    memset( pbLineMask, 0, ulSrcHeight );

    for( ; (LONG)cRect > 0; cRect--, prectl++ )
    {
      lTop = prectl->yTop > ulSrcHeight ? ulSrcHeight : prectl->yTop;
      lBottom = prectl->yBottom < 0 ? 0 : prectl->yBottom;

      if ( lTop > lBottom )
        // pbLineMask - up-to-down lines mask.
        memset( &pbLineMask[ulSrcHeight - lTop], 1, lTop - lBottom );
    }

    ulRC = DiveBlitImageLines( pDIVEData->hDive, ulDiveBufNum,
                               DIVE_BUFFER_SCREEN, pbLineMask );
    if ( ulRC != DIVE_SUCCESS )
      debug( "DiveBlitImageLines(,%u,,), rc = %u", ulRC, ulDiveBufNum );
  }
  else
  {
    ulRC = DiveBlitImage( pDIVEData->hDive, ulDiveBufNum, DIVE_BUFFER_SCREEN );
    if ( ulRC != DIVE_SUCCESS )
      debug( "DiveBlitImage(,%u,), rc = %u", ulDiveBufNum, ulRC );
  }

  return TRUE;
}

static BOOL vsSetPalette(PVOID pVSData, ULONG ulFirst, ULONG ulNumber,
                         PRGB2 pColors)
{
/*
    http://www.edm2.com/index.php/Gearing_Up_For_Games_-_Part_2
*/
  PDIVEData  pDIVEData = (PDIVEData)pVSData;
  ULONG      ulRC;

  ulRC = DiveSetSourcePalette/*DiveSetDestinationPalette*/( pDIVEData->hDive, ulFirst, ulNumber,
                               (PBYTE)pColors );
  if ( ulRC != DIVE_SUCCESS )
  {
    debug( "DiveSetDestinationPalette(), rc = %u", ulRC );
    return FALSE;
  }

  return TRUE;
}
