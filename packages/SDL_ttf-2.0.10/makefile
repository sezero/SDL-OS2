DLLNAME = sdlttf
VERSION = 2.0.10

!include ..\..\packages.mif

SRCS = SDL_ttf.c

LIBS = freetype_static.lib sdl12.lib

CFLAGS  = $(CFLAGS_DLL) -6r -s -ei -j -fp6 -fpi87 -DBUILD_SDL

OBJS = $(SRCS:.c=.obj)

$(LIBFILE): $(DLLFILE)
  @echo * Create library: $@...
  @wlib -b -n -q $@ $(DLLFILE)

$(DLLFILE): $(OBJS) $(LNKFILE)
  @echo * Link: $@
  @wlink @$(LNKFILE)

$(LNKFILE):
  @%create $@
  @%append $@ SYSTEM os2v2_dll INITINSTANCE TERMINSTANCE
  @%append $@ NAME $(DLLPATH)\$(LIBNAME)
  @for %i in ($(OBJS)) do @%append $@ FILE %i
  @%append $@ OPTION QUIET
!ifdef %osdir
  @$(%osdir)\KLIBC\BIN\date +"OPTION DESCRIPTION '@$#libsdl org:$(VERSION)$#@$#$#1$#$# %F               $(%HOSTNAME)::::::@@Simple DirectMedia Layer truetype font loading library'" >>$^@
!else
  @%append $@ OPTION DESCRIPTION '@$#libsdl org:$(VERSION)$#@Simple DirectMedia Layer truetype font loading library'
!endif
  @%append $@ LIBPATH $(LIBPATH)
  @for %i in ($(LIBS)) do @%append $@ LIB %i
  @%append $@ OPTION MAP=$*
  @%append $@ OPTION ELIMINATE
  @%append $@ OPTION MANYAUTODATA
  @%append $@ OPTION OSNAME='OS/2 and eComStation'
  @%append $@ OPTION SHOWDEAD

.c.obj:
  @wcc386 $(CFLAGS) $<

clean: .SYMBOLIC
  @echo * Clean: $(TITLENAME)
  @if exist *.obj @del *.obj
  @if exist *.err @del *.err
  @if exist *.map @del *.map
  @if exist $(LNKFILE) @del $(LNKFILE)
  @if exist $(DLLFILE) @del $(DLLFILE)
  @if exist $(LIBFILE) @del $(LIBFILE)
