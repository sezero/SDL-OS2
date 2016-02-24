/*
  GrOp graphics library allows applications to create framebuffers, producing
  graphics output and provides access to keyboard and mouse. The output can be
  displayed in windows or fullscreen.

  Vasilkin Andrey, 2016.
*/

#ifndef GROP_H
#define GROP_H

#include "linkseq.h"

// GROP_VS_xxxx for gropNew(ulVideoSys,)
#define GROP_VS_DIVE             0
// GROP_VS_VMAN_COMPATIBLE - only current (desktop) mode will be used.
#define GROP_VS_VMAN_COMPATIBLE  1
#define GROP_VS_VMAN             2

// GROP_SET_FS_xxxxx for gropSetFullscreen(,ulSetFullscreen)
#define GROP_SET_FS_OFF            0
#define GROP_SET_FS_ON             1
#define GROP_SET_FS_SWITCH         2

// GROP_MODEFL_xxxxx for GROPSETMODE.ulFlags
#define GROP_MODEFL_FULLSCREEN   1
// GROP_MODEFL_RESIZABLE
// Call GROPCALLBACK.fnSize on window resize.
#define GROP_MODEFL_RESIZABLE    2

// GROUP_ACTIVATE_xxxxx for GROPCALLBACK.fnActive(ulType,)
#define GROUP_ACTIVATE_WINDOW    0
#define GROUP_ACTIVATE_FOCUS     1

typedef struct _GROPDATA *PGROPDATA;

// User's functions for different events. See function gropNew(,pCallback,).

typedef struct _GROPCALLBACK {

  // This function called when the mouse pointer moves.
  VOID (*fnMouseMove)(PGROPDATA pGrop, BOOL fRelative, LONG lX, LONG lY);

  // This function called when the user presses or releases mouse button.
  // ulButton: 0 - left button, 1 - right button, 2 - middle button.
  VOID (*fnMouseButton)(PGROPDATA pGrop, ULONG ulButton, BOOL fDown);

  // This function called when an operator presses a key.
  // ulFlags - keyboard control codes as defined for WM_CHAR (flags KC_xxxxx).
  VOID (*fnKeyboard)(PGROPDATA pGrop, ULONG ulScanCode, ULONG ulChar,
                     ULONG ulFlags);

  // This function called when the window is activated/deactivated or window
  // receiving/losing the focus. ulType - GROUP_ACTIVATE_xxxxx,
  VOID (*fnActive)(PGROPDATA pGrop, ULONG ulType, BOOL fSet);

  // This function called when current mode have flag GROP_MODEFL_RESIZABLE and
  // the window changes its size.
  VOID (*fnSize)(PGROPDATA pGrop, ULONG ulWidth, ULONG ulHeight);

  // This function called to terminate the application (message WM_QUIT is
  // received).
  BOOL (*fnQuit)(PGROPDATA pGrop);

} GROPCALLBACK, *PGROPCALLBACK;

// User "work space". See function gropSetMode(,pData).

typedef struct _GROPSETMODE {
  ULONG                ulFlags;          // input: GROP_MODEFL_xxxxx.
  ULONG                ulWidth;          // input: requested width.
  ULONG                ulHeight;         // input: requested height.
  ULONG                ulBPP;            // input: recommended / output: used.
  ULONG                ulScanLineSize;   // output: size of line in bytes.
  ULONG                fccColorEncoding; // output: eq. current mode value.
  PVOID                pBuffer;          // output: graphic output buffer.
} GROPSETMODE, *PGROPSETMODE;


//           Internal data.

typedef struct _VIDEOMODE {
  ULONG                ulId;
  ULONG                ulBPP;
  ULONG                ulWidth;
  ULONG                ulHeight;
  ULONG                ulScanLineSize;
  ULONG                fccColorEncoding;
} VIDEOMODE, *PVIDEOMODE;

typedef struct _VIDEOMODESLIST {
  ULONG                ulCount;
  PVIDEOMODE           pList;
  ULONG                ulDesktopMode;      // Desktop mode index of pList.
} VIDEOMODESLIST, *PVIDEOMODESLIST;

typedef struct _VIDEOSYS {
  BOOL                 fFreeWinSize;
  BOOL                 fBPPConvSup;
  BOOL (*fnInit)(PVIDEOMODESLIST pModes, PVOID *ppVSData);
  VOID (*fnDone)(PVOID pVSData, PVIDEOMODESLIST pModes);
  BOOL (*fnSetMode)(PVOID pVSData, PVIDEOMODE pMode, BOOL fFullscreen,
                    HWND hwndDT, HDC hdcDT);
  PVOID (*fnVideoBufAlloc)(PVOID pVSData, ULONG ulWidth, ULONG ulHeight,
                           ULONG ulBPP, ULONG fccColorEncoding,
                           PULONG pulScanLineSize);
  VOID (*fnVideoBufFree)(PVOID pVSData, PVOID pBuffer);
  VOID (*fnSetVisibleRegion)(PVOID pVSData, HWND hwnd, PGROPSETMODE pUserMode,
                             PRECTL prectlWin, PRECTL prectlVA, BOOL fVisible);
  BOOL (*fnUpdate)(PVOID pVSData, PGROPDATA pGrop, ULONG cRect, PRECTL prectl);
  BOOL (*fnSetPalette)(PVOID pVSData, ULONG ulFirst, ULONG ulNumber,
                       PRGB2 pColors);
} VIDEOSYS, *PVIDEOSYS;

typedef struct _GROPDATA {
  SEQOBJ               sObj;

  GROPCALLBACK         stCallback;         // User callback functions.
  TID                  tid;                // GrOp object (window) thread Id.
  HMTX                 hmtxData;           // This data structure lock mutex.
  HEV                  hevReady;
  HAB                  hab;
  HMQ                  hmq;
  HWND                 hwndDT;             // Desktop window handle.
  HDC                  hdcDT;              // Desktop device-context handle.
  ULONG                ulDTWidth;       
  ULONG                ulDTHeight;
  PFNWP                fnOldFrameWinProc;
  HWND                 hwndFrame;
  HWND                 hwnd;
  ULONG                ulModeIdx;          // Current mode index.
  GROPSETMODE          stUserMode;         // User area data.
  BOOL                 fInternalResize;    // Internal using - for frame proc.
  BOOL                 fFullscreen;
  BOOL                 fActive;
  SWP                  swpOld;
  BOOL                 fCapture;
  HPOINTER             hptrPointer;
  RECTL                rectlWinArea;       // Client window or fullscreen
                                           // rectanle on the desktop.
  RECTL                rectlViewArea;      // View (user space) rectanle on
                                           // client window.
  BOOL                 fMouseInViewArea;   // Mouse pointer in user space flag.
  ULONG                ulMouseX;           // Mouse pointer location in the
  ULONG                ulMouseY;           // user space coordinates.
  LONG                 lSkipMouseMove;
  LONG                 lSkipWinResize;

  // Output video system data.
  ULONG                ulVideoSysIdx;   // Index of array aVideoSys[] (grop.c)
  PVIDEOSYS            pVideoSys;       // Video system routines.
  PVOID                pVSData;         // Video system internal data pointer.
  VIDEOMODESLIST       stModes;         // Fills by video system module.
  PVOID                pUser;
} GROPDATA;


//           Public routines.
//           ----------------

// BOOL gropInit();
// GrOp module initialization. Returns FALSE if an error occurred. 
BOOL gropInit();

// VOID gropDone();
// Shuts down GrOp module and frees the resources allocated to it. This should
// always be called before exit.
VOID gropDone();

// Macro is used to obtain user-defined pointer of GrOp object. 
#define gropGetUserPtr(grop) ((grop)->pUser)

// PGROPDATA gropNew(ULONG ulVideoSys, PGROPCALLBACK pCallback, PVOID pUser);
// Creates a GrOp object with specified callback functions.
// pUser is user-defined pointer for the new GrOp object. 
PGROPDATA gropNew(ULONG ulVideoSys, PGROPCALLBACK pCallback, PVOID pUser);

// VOID gropFree(PGROPDATA pGrop);
// Destroys the GrOp object created by gropNew(). This function should not be
// called from any callback function.
VOID gropFree(PGROPDATA pGrop);

// BOOL gropSetMode(PGROPDATA pGrop, PGROPSETMODE pData);
// Allocates video buffer to use in program and sets optimal video mode or
// creates a window.
BOOL gropSetMode(PGROPDATA pGrop, PGROPSETMODE pData);

// BOOL gropSetFullscreen(PGROPDATA pGrop, ULONG ulSetFullscreen);
// Switches between fulscreen and windowed mode. Switch from the fullscreen
// to desktop mode will only be possible if color depth values (BPP) are the
// same for both modes.
BOOL gropSetFullscreen(PGROPDATA pGrop, ULONG ulSetFullscreen);

// BOOL gropSetPointer(PGROPDATA pGrop, HPOINTER hptrPointer);
// Sets the mouse pointer handle or removes pointer from the screen if
// hptrPointer is NULL. 
BOOL gropSetPointer(PGROPDATA pGrop, HPOINTER hptrPointer);

// BOOL gropCapture(PGROPDATA pGrop, BOOL fCapture);
// Captures mouse pointer moves. Limits mouse pointer movements range by part
// of the screen that displays user buffer content or keeps unvisible pointer
// at center and sends movements offset (with GROPCALLBACK.fnMouseMove()).
BOOL gropCapture(PGROPDATA pGrop, BOOL fCapture);

// BOOL gropMinimize(PGROPDATA pGrop);
// Minimizes the window.    
BOOL gropMinimize(PGROPDATA pGrop);

// BOOL gropMouseMove(PGROPDATA pGrop, LONG lX, LONG lY);
// Sets the pointer position. lX, lY is position of pointer in user space
// coordinates.
BOOL gropMouseMove(PGROPDATA pGrop, LONG lX, LONG lY);

// BOOL gropGetMousePos(PGROPDATA pGrop, PULONG pulX, PULONG pulY);
// Returns the pointer position in user space coordinates.
BOOL gropGetMousePos(PGROPDATA pGrop, PULONG pulX, PULONG pulY);

// BOOL gropUpdate(PGROPDATA pGrop, ULONG cRect, PRECTL prectl);
// Makes sure the given list of rectangles is updated on the screen.
BOOL gropUpdate(PGROPDATA pGrop, ULONG cRect, PRECTL prectl);

// BOOL gropSetPaletter(PGROPDATA pGrop, ULONG ulFirst, ULONG ulNumber,
//                      PRGB2 pColors);
// Sets the palette for user buffer with 8-bit color depth (when BPP is 8).
BOOL gropSetPaletter(PGROPDATA pGrop, ULONG ulFirst, ULONG ulNumber,
                     PRGB2 pColors);

// BOOL gropSetWindowTitle(PGROPDATA pGrop, PSZ pszText);
// Sets title-bar window text.
BOOL gropSetWindowTitle(PGROPDATA pGrop, PSZ pszText);

// BOOL gropSetWindowTitle(PGROPDATA pGrop, PSZ pszText);
// Sets title-bar window icon.
BOOL gropSetWindowIcon(PGROPDATA pGrop, HPOINTER hptrIcon);

// BOOL gropClose(PGROPDATA pGrop);
// Send close signal to the GrOp object (WM_CLOSE to the window) to call user
// function GROPCALLBACK.fnClose(,TRUE), return to the desktop and close window. 
BOOL gropClose(PGROPDATA pGrop);

// Lock/unlock GROP. It will block GROP window and stop all callbacks calls.
BOOL gropLock(PGROPDATA pGrop);
VOID gropUnlock(PGROPDATA pGrop);

#endif // GROP_H
