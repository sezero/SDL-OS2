TITLENAME=Test programs for the SDL library

!include ..\..\..\packages.mif

TARGETS = checkkeys.exe graywin.exe loopwave.exe testalpha.exe testbitmap.exe &
          testblitspeed.exe testcdrom.exe testcursor.exe testdyngl.exe &
          testerror.exe testfile.exe testgamma.exe testgl.exe testhread.exe &
          testjoystick.exe testkeys.exe testlock.exe &
          testoverlay2.exe testoverlay.exe testpalette.exe testplatform.exe &
          testsem.exe testsprite.exe testtimer.exe testver.exe testvidinfo.exe &
          testwin.exe testwm.exe threadwin.exe torturethread.exe testloadso.exe
# testiconv.exe

OBJS = $(TARGETS:.exe=.obj)

BINPATH = $(LIBHOME)\packages\SDL-1.2.15\test

all: $(TARGETS) show_info

CFLAGS = $(CFLAGS_EXE) -ei -5s

LIBS = sdl12.lib sdlimage.lib sdlmixer.lib sdlnet.lib sdlttf.lib

.obj.exe:
  @rem echo * Link: $@
  @wlink SYS os2v2 libpath $(LIBPATH) lib {$(LIBS)} op q file {$<} name $*

.c.obj:
  @rem echo * Compilation: $<
  @wcc386 $(CFLAGS) $<

show_info: .SYMBOLIC
  @echo * Tests compiled in $(BINPATH)

clean: .SYMBOLIC
  @echo * Clean tests in $(BINPATH)
  @for %i in ($(OBJS)) do @if exist %i @del %i
  @for %i in ($(TARGETS:.exe=.err)) do @if exist %i @del %i
  @for %i in ($(TARGETS)) do @if exist %i @del %i
