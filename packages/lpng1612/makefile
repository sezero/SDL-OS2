LIBNAME = libpng
VERSION = 1.6.12

!include ..\..\packages.mif

SRCS = png.c pngerror.c pngget.c pngmem.c pngpread.c pngread.c pngrio.c &
       pngrtran.c pngrutil.c pngset.c pngtrans.c pngwio.c pngwrite.c &
       pngwtran.c pngwutil.c

OBJS = $(SRCS:.c=.obj)

CFLAGS = $(CFLAGS_STATIC) -wcd=124

all: $(LIBFILE_STATIC) $(EXEPATH)\pngtest.exe

$(LIBFILE_STATIC): $(OBJS)
  @echo * Create library: $@...
  @for %f in ($(OBJS)) do @wlib -b -q $* +%f >nul

$(EXEPATH)\pngtest.exe: $(LIBFILE_STATIC) pngtest.obj
  @echo * Link: $@
  @wlink SYS os2v2 libpath $(LIBPATH) lib {$(LIBFILE_STATIC) zlib_static} op q op el file {pngtest} name $*

.c.obj:
  @wcc386 $(CFLAGS) $<

clean: .SYMBOLIC
  @echo * Clean: $(TITLENAME)
  @if exist *.obj @del *.obj
  @if exist *.err @del *.err
  @if exist $(LIBFILE_STATIC) @del $(LIBFILE_STATIC)
  @if exist $(EXEPATH)\minigzip.exe @del $(EXEPATH)\minigzip.exe
  @if exist $(EXEPATH)\pngtest.exe @del $(EXEPATH)\pngtest.exe
