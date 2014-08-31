LIBNAME = zlib
VERSION = 1.2.8

!include ..\..\packages.mif

SRCS = adler32.c compress.c crc32.c deflate.c gzclose.c gzlib.c gzread.c &
       gzwrite.c infback.c inffast.c inflate.c inftrees.c trees.c uncompr.c &
       zutil.c

OBJS = $(SRCS:.c=.obj)

.extensions:
.extensions: .lib .dll .obj .c

.c: .\test

CFLAGS = $(CFLAGS_STATIC) -fp3 -s -oilrtfm -wx -w1

all: $(LIBFILE_STATIC) $(EXEPATH)\minigzip.exe

$(LIBFILE_STATIC): $(OBJS)
  @echo * Create library: $(LIBFILE_STATIC)...
  @for %f in ($(OBJS)) do @wlib -b -q $* +%f >nul

$(EXEPATH)\minigzip.exe: $(LIBFILE_STATIC) minigzip.obj
  @echo * Link: $@
  @wlink SYS os2v2 libpath $(LIBPATH) lib {$(LIBFILE_STATIC)} op q op el file {minigzip} name $*

.c.obj:
  @wcc386 $(CFLAGS) $<

minigzip.c:
  wcc386 $(CFLAGS) -DOS2 -DSTDC $@

clean: .SYMBOLIC
  @echo * Clean: $(TITLENAME)
  @if exist *.obj @del *.obj
  @if exist *.err @del *.err
  @if exist $(LIBFILE_STATIC) @del $(LIBFILE_STATIC)
  @if exist $(EXEPATH)\minigzip.exe @del $(EXEPATH)\minigzip.exe
