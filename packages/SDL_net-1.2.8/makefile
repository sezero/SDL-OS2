DLLNAME = sdlnet
VERSION = 1.2.8

!include ..\..\packages.mif

SRCS = SDLnet.c SDLnetselect.c SDLnetTCP.c SDLnetUDP.c

CFLAGS  = $(CFLAGS_DLL) -5s -fpi -sg -otexan -wx -ei -DBUILD_SDL

LIBS = sdl12.lib

.extensions:
.extensions: .lib .dll .obj .c

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
    @$(%osdir)\KLIBC\BIN\date +"OPTION DESCRIPTION '@$#libsdl org:$(VERSION)$#@$#$#1$#$# %F               $(%HOSTNAME)::::::@@Simple DirectMedia Layer Network Library'" >>$^@
!else
    @%append $@ OPTION DESCRIPTION '@$#libsdl org:$(VERSION)$#@Simple DirectMedia Layer Network Library'
!endif
    @%append $@ LIBPATH $(LIBPATH);..\libmikmod-3.3.6\os2
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
