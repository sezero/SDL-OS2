EXENAME = playsound
VERSION = 1.0.3

!include ..\..\..\packages.mif

CFLAGS  = $(CFLAGS_EXE) -ei -wcd=136 -DHAVE_SIGNAL_H -DHAVE_SETBUF -DHAVE_ASSERT_H

LIBS = sdl12.lib sdlsound.lib

$(EXEFILE): playsound.obj
  @echo * Link: $@
  @wlink SYS os2v2 libpath $(LIBPATH) lib {$(LIBS)} op q op el file {playsound} name $*

.c.obj:
  @wcc386 $(CFLAGS) $<

clean: .SYMBOLIC
  @echo * Clean: $(TITLENAME)
  @if exist *.obj @del *.obj
  @if exist *.err @del *.err
  @if exist $(EXEFILE) @del $(EXEFILE)