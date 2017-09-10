#=============================================================================
#
#  This is an OpenWatcom makefile to build alternative version SDL for OS/2
#
#  * A new VMAN and DIVE support
#  * A new DART support
#
# Digi 2014.
#=============================================================================

DLLNAME = sdl12
VERSION = 1.2.15
INCPATH = .\include;.\src;.\src\audio;.\src\audio\DARTalt;.\src\audio\disk;.\src\cdrom;.\src\cdrom\os2;.\src\cpuinfo;.\src\events;.\src\file;.\src\joystick;.\src\joystick\os2;.\src\loadso\os2;.\src\stdlib;.\src\thread;.\src\thread\os2;.\src\timer;.\src\timer\os2;.\src\video;.\src\video\os2grop;.\src\video\os2grop\grop

!include ..\..\packages.mif

# Create debug build or not?
#DEBUG_BUILD=1

# Special flags for building SDL
CFLAGS = $(CFLAGS_DLL) -otexan -wx -ei -fo=.\obj\
CFLAGS += -DBUILD_SDL
# -DCHECK_LEAKS
# USE_ASM_MIXER_VC has no effect since SDL-1.2.10.
# -DUSE_ASM_MIXER_VC
CFLAGS += -DUSE_DOSSETPRIORITY

DESCRIPTION = Simple DirectMedia Layer (alternative port)

!ifdef DEBUG_BUILD
CFLAGS += -d2 -DDEBUG_BUILD
CFLAGS += -DDEBUG_FILE="SDL.dbg"
DESCRIPTION += DEBUG
!endif

PMGRE_LIB = $(LIBPATH)\pmgre.lib

audioobjs = SDL_audiocvt.obj SDL_mixer.obj SDL_mixer_MMX_VC.obj SDL_wave.obj &
            SDL_diskaudio.obj SDL_audio_os2.obj
# SDL_audio.obj SDL_DARTalt.obj

cdromobjs = SDL_cdrom.obj SDL_syscdrom.obj
cpuinfoobjs = SDL_cpuinfo.obj
eventsobjs = SDL_active.obj SDL_events.obj SDL_expose.obj SDL_keyboard.obj &
             SDL_mouse.obj SDL_quit.obj SDL_resize.obj
fileobjs = SDL_rwops.obj
joystickobjs = SDL_joystick.obj SDL_sysjoystick.obj
loadsoobjs = SDL_sysloadso.obj
threadobjs = SDL_thread.obj SDL_sysmutex.obj SDL_syssem.obj SDL_systhread.obj &
             SDL_syscond.obj
timerobjs = SDL_timer.obj SDL_systimer.obj
videoobjs = SDL_blit.obj SDL_blit_0.obj SDL_blit_1.obj SDL_blit_A.obj &
            SDL_blit_N.obj SDL_bmp.obj SDL_cursor.obj SDL_gamma.obj &
            SDL_pixels.obj SDL_RLEaccel.obj SDL_stretch.obj SDL_surface.obj &
            SDL_video.obj SDL_yuv.obj SDL_yuv_mmx.obj SDL_yuv_sw.obj &
            SDL_os2grop.obj os2ini.obj debug.obj grop.obj vs_dive.obj &
            vs_vman.obj

stdlibobjs = SDL_string.obj
# SDL_iconv.obj

object_files= SDL.obj SDL_error.obj SDL_fatal.obj &
              $(stdlibobjs) $(audioobjs) $(cpuinfoobjs) $(eventsobjs) &
              $(fileobjs) $(joystickobjs) $(loadsoobjs) $(threadobjs) &
              $(timerobjs) $(videoobjs) $(cdromobjs)

.extensions:
.extensions: .lib .dll .obj .c .asm

.c: .\src;.\src\audio;.\src\audio\DARTalt;.\src\audio\disk;.\src\cdrom;.\src\cdrom\os2;.\src\cpuinfo;.\src\events;.\src\file;.\src\joystick;.\src\joystick\os2;.\src\loadso\os2;.\src\stdlib;.\src\thread;.\src\thread\os2;.\src\timer;.\src\timer\os2;.\src\video;.\src\video\os2grop;.\src\video\os2grop\grop
.obj: .\obj

.c.obj : .AUTODEPEND
    @wcc386 $[* $(CFLAGS)

all: $(DLLFILE) $(LIBFILE) .symbolic

$(DLLFILE): $(LNKFILE) object_dir $(PMGRE_LIB) compiling_info $(object_files)
    @echo * Linking: $@
    @wlink @$(LNKFILE)

$(LIBFILE): $(DLLFILE)
    @echo * Creating LIB file: $@
    @wlib -q -b -n -c -pa -s -t -zld -ii -io $* $(DLLFILE)


SDL_RLEaccel.obj : object_dir
    @wcc386 .\src\video\SDL_RLEaccel.c $(cflags) -w1

object_dir : .symbolic
    @if not exist obj @mkdir obj

compiling_info : .symbolic
    @echo * Compiling...

$(PMGRE_LIB): $(LIBPATH)\def2lbc.sed $(LIBPATH)\pmgre.def
	@echo * Creating pmgre.lib
	sed -f $(LIBPATH)\def2lbc.sed $(LIBPATH)\pmgre.def > $(LIBPATH)\pmgre.exp
	wlib -q -b -n -c -pa -s -t -zld -ii -io $(PMGRE_LIB) @$(LIBPATH)\pmgre.exp

$(LNKFILE):
    @echo * Creating linker file: $@
    @%create $@
    @%append $@ SYSTEM os2v2_dll INITINSTANCE TERMINSTANCE
    @%append $@ NAME $(DLLPATH)\$(LIBNAME)
#    @%append $@ NAME $^&
    @for %i in ($(object_files)) do @%append $@ FILE obj\%i
    @%append $@ LIBPATH $(LIBPATH)
    @%append $@ LIB os2386.lib
    @%append $@ LIB pmgre.lib
    @%append $@ LIB mmpm2.lib
    @%append $@ LIB libuls.lib
    @%append $@ LIB libconv.lib
#    @%append $@ LIB os2iconv_static.lib
    @%append $@ OPTION QUIET
    @%append $@ OPTION MAP=$^&.map
!ifdef %osdir
    @$(%osdir)\KLIBC\BIN\date +"OPTION DESCRIPTION '@$#libsdl org:$(VERSION)$#@$#$#1$#$# %F               $(%HOSTNAME)::::::@@$(DESCRIPTION)'" >>$^@
!else
    @%append $@ OPTION DESCRIPTION '@$#libsdl org:$(VERSION)$#@$(DESCRIPTION)'
!endif
    @%append $@ OPTION QUIET
    @%append $@ OPTION ELIMINATE
    @%append $@ OPTION MANYAUTODATA
    @%append $@ OPTION OSNAME='OS/2 and eComStation'
    @%append $@ OPTION SHOWDEAD

clean : .SYMBOLIC
    @ echo * Clean: $(TITLENAME)
    @if exist .\obj\*.obj @del .\obj\*.obj
    @if exist .\obj @rd .\obj
    @if exist *.map @del *.map
    @if exist $(LIBHOME)\h\os2386tk.h @del $(LIBHOME)\h\os2386tk.h
    @if exist $(LNKFILE) @del $(LNKFILE)
    @if exist $(DLLFILE) @del $(DLLFILE)
    @if exist $(LIBFILE) @del $(LIBFILE)
