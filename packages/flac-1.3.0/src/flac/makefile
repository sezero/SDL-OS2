EXENAME = flac

INCPATH = ..\..\include

!include ..\..\..\..\packages.mif

SRCS = analyze.c decode.c encode.c foreign_metadata.c  &
       local_string_utils.c main.c utils.c vorbiscomment.c
# iffscan.c

CFLAGS = $(CFLAGS_EXE) -DHAVE_INTTYPES_H

OBJS = $(SRCS:.c=.obj)

LIBPATH +=;..\share

LIBS = flac_static.lib share.lib

$(EXEFILE): $(OBJS)
  @echo * Link: $@
  @wlink SYS os2v2 libpath $(LIBPATH) lib {$(LIBS)} op q op el file {$(OBJS:.obj=)} name $*

.c.obj:
  @wcc386 $(CFLAGS) -fo=$@ $<

clean: .SYMBOLIC
  @echo * Clean: $(TITLENAME)
  @if exist *.obj @del *.obj
  @if exist *.err @del *.err
  @if exist $(EXEFILE) @del $(EXEFILE)
