#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define INCL_DOSPROCESS
#define INCL_GPI
#define INCL_DOSMISC
#define INCL_WIN
#include <os2.h>
#include "grop.h"
#include "debug.h"

// CALC_FPS - If defined - sample will work up to 10 sec. and show FPS on exit.
// #define CALC_FPS

static BOOL fStop = FALSE;


static VOID cbMouseMove(PGROPDATA pGrop, BOOL fRelative, LONG lX, LONG lY)
{
  printf( "fnMouseMove( %s, %d, %d )\n", fRelative ? "TRUE" : "FALSE", lX, lY );
}

static VOID cbMouseButton(PGROPDATA pGrop, ULONG ulButton, BOOL fDown)
{
  printf( "cbMouseButton( %u, %u )\n", ulButton, fDown );
}

static VOID cbKeyboard(PGROPDATA pGrop, ULONG ulScanCode, ULONG ulChar,
                       ULONG ulFlags)
{
  printf( "fnKeyboard( %u, '%c', 0x%X )\t- %s\n",
          ulScanCode, ( ulChar == 0 ? ' ' : (CHAR)ulChar ), ulFlags,
          (ulFlags & KC_KEYUP) != 0 ? "Released" : "Pressed" );

  fStop = ulScanCode == 1; // <ESC>

  switch( ulChar )
  {
    case ' ':
      gropSetFullscreen( pGrop, GROP_SET_FS_SWITCH );
      break;

    case 'z':
      if ( !gropMinimize( pGrop ) )
        puts( "gropMinimize() failed." );
      break;
  }
}

static VOID cbActive(PGROPDATA pGrop, ULONG ulType, BOOL fSet)
{
  printf( "cbActive( %u, %u )\n", ulType, fSet );
}

static VOID cbSize(PGROPDATA pGrop, ULONG ulWidth, ULONG ulHeight)
{
  printf( "cbSize( %u, %u )\n", ulWidth, ulHeight );
}

static BOOL cbQuit(PGROPDATA pGrop)
{
  printf( "cbQuit()\n" );
  fStop = TRUE;
  return FALSE;
}


static VOID _setPixel(PGROPSETMODE pUserMode, ULONG ulX, ULONG ulY)
{
  ULONG      ulOffs;
  PCHAR      pPixel;

  if ( ulX >= pUserMode->ulWidth || ulY >= pUserMode->ulHeight )
    return;

  ulOffs = (ulY * pUserMode->ulScanLineSize) +
                         (ulX * (pUserMode->ulBPP >> 3));
  pPixel = &((PCHAR)pUserMode->pBuffer)[ulOffs];

  switch( pUserMode->ulBPP )
  {
    case 8:
      *((PBYTE)pPixel) = 0xFF;
      break;

    case 16:
      *((PUSHORT)pPixel) = 0xFFFF;
      break;

    case 24:
      *((PUSHORT)pPixel) = 0xFFFF;
      pPixel++;
      *((PBYTE)pPixel) = 0xFF;
      break;

    case 32:
      *((PULONG)pPixel) = 0xFFFFFF;
      break;
  }
}

VOID sigBreak(int sinno)
{
  debug( "The system BREAK signal received." );
  fStop = TRUE;
}

void main()
{
  PGROPDATA            pGrop;
  GROPSETMODE          stSetMode;
  GROPCALLBACK         stCallback = { cbMouseMove, cbMouseButton, cbKeyboard,
                                      cbActive, cbSize, cbQuit };

  // GROP module initialization.
  if ( !gropInit() )
  {
    puts( "gropInit() failed" );
    return;
  }

  puts( "\n<SPACE>\tSwitch fullscreen/window mode.\n<ESC>\tExit program.\n" );

  // Create a new GROP object.
  pGrop = gropNew( GROP_VS_DIVE, &stCallback, NULL );
  if ( pGrop == NULL )
    return;

  // Set the title-bar window text.
  gropSetWindowTitle( pGrop, "GROP Sample" );

  // Set video mode.
  stSetMode.ulFlags = 0;//GROP_MODEFL_FULLSCREEN;         // input: GROP_MODEFL_xxxxx
  stSetMode.ulWidth = 640;          // input: requested width
  stSetMode.ulHeight = 480;         // input: requested height
  stSetMode.ulBPP = 16;            // input: recommended / output: used
  if ( !gropSetMode( pGrop, &stSetMode ) )
  {
    puts( "Can not set requested mode." );
    exit( 1 );
  }

  // Set new pointer for the view area.
  gropSetPointer( pGrop,
                  WinQuerySysPointer( HWND_DESKTOP, SPTR_MOVE, FALSE ) );

  signal( SIGINT, sigBreak );
  signal( SIGTERM, sigBreak );
  
  // Simple animation.
  {
    ULONG    ulLine, ulLPos;
    PBYTE    pBufferTopLine = debugMAlloc( stSetMode.ulScanLineSize );
#ifdef CALC_FPS
    ULONG    ulFrameCount = 0;
    ULONG    ulTimeStart, ulTime;
#endif

    // Buffer cleaning.
    memset( stSetMode.pBuffer, 0,
            stSetMode.ulScanLineSize * stSetMode.ulHeight );

    // Draw simple picture in the obtained buffer.
    for( ulLine = 0; ulLine < stSetMode.ulHeight; ulLine++ )
    {
      for( ulLPos = 0; ulLPos < stSetMode.ulWidth;
           ulLPos += ( (ulLine % 10) == 0 ? 1 : 10 ) )
        _setPixel( &stSetMode, ulLPos, ulLine );
    }

#ifdef CALC_FPS
    DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT, &ulTimeStart, sizeof(ULONG) );
#endif

    // Animation...

    while( !fStop )
    {
      // Show our picture.
      gropUpdate( pGrop, 0, NULL );

      // Scroll image in the buffer.
      memcpy( pBufferTopLine, stSetMode.pBuffer, stSetMode.ulScanLineSize );
      memcpy( stSetMode.pBuffer,
              &((PCHAR)stSetMode.pBuffer)[stSetMode.ulScanLineSize],
              stSetMode.ulScanLineSize * (stSetMode.ulHeight - 1) );
      memcpy( &((PCHAR)stSetMode.pBuffer)[stSetMode.ulScanLineSize *
                                          (stSetMode.ulHeight - 1)],
              pBufferTopLine, stSetMode.ulScanLineSize );

#ifndef CALC_FPS
      DosSleep( 1 );
#else
      ulFrameCount++;
      DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT, &ulTime, sizeof(ULONG) );
      ulTime -= ulTimeStart;
      if ( ulTime >= 10000 ) // End in 10 seconds.
        break;
#endif
    }
    debugFree( pBufferTopLine );

#ifdef CALC_FPS
    printf( "* FPS: %0.3f (%u frames, %0.3f seconds)\n",
            (double)(ulFrameCount * 1000) / ulTime,
            ulFrameCount, (double)ulTime / 1000 );
#endif
  }

  gropFree( pGrop );
  gropDone();
  puts( "Done." );
}
