LIBNAME = ogg
VERSION = 1.3.2
INCPATH =.\include

!include ..\..\..\packages.mif

SRCS = bitwise.c framing.c

OBJS = $(SRCS:.c=.obj)

CFLAGS = $(CFLAGS_STATIC)

$(LIBFILE_STATIC): $(OBJS)
  @echo * Create library: $@...
  @for %f in ($(OBJS)) do @wlib -b -q $* +%f >nul

.c.obj:
  @wcc386 $(CFLAGS) $<

clean: .SYMBOLIC
  @echo * Clean: $(TITLENAME)
  @if exist *.obj @del *.obj
  @if exist *.err @del *.err
  @if exist $(LIBFILE_STATIC) @del $(LIBFILE_STATIC)
