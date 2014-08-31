LIBNAME = vorbis
VERSION = 1.3.4
INCPATH =..\include

!include ..\..\..\packages.mif

SRCS = analysis.c barkmel.c bitrate.c block.c codebook.c envelope.c floor0.c &
       floor1.c info.c lookup.c lpc.c lsp.c mapping0.c mdct.c psy.c &
       registry.c res0.c sharedbook.c smallft.c synthesis.c &
       vorbisenc.c vorbisfile.c window.c
# psytune.c misc.c tone.c

#CFLAGS  = -dHAVE_$(PLATFORM) -bt=$(PLATFORM) -dHAVE_ALLOCA_H -d0 -q -i$(INCPATH) -fi=direct.h
CFLAGS  = $(CFLAGS_STATIC) -j -DHAVE_ALLOCA_H -DUSE_MEMORY_H


OBJS = $(SRCS:.c=.obj)

$(LIBFILE_STATIC): $(OBJS)
  @echo * Create library: $(LIBFILE_STATIC)...
  @for %f in ($(OBJS)) do @wlib -b -q $* +%f >nul

.c.obj:
  @wcc386 $(CFLAGS) $<

clean: .SYMBOLIC
  @echo * Clean: $(TITLENAME)
  @if exist *.obj @del *.obj
  @if exist *.err @del *.err
  @if exist $(LIBFILE) @del $(LIBFILE)
  @if exist $(LIBFILE_STATIC) @del $(LIBFILE_STATIC)
