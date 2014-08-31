DLLNAME = sdlmixer
VERSION = 1.2.12
INCPATH = .\timidity;..\smpeg-0.4.5;..\libmikmod-3.3.6\include
!include ..\..\packages.mif

SRCS = dynamic_flac.c dynamic_fluidsynth.c dynamic_mod.c dynamic_mp3.c &
       dynamic_ogg.c effects_internal.c effect_position.c &
       effect_stereoreverse.c fluidsynth.c load_aiff.c load_flac.c load_ogg.c &
       load_voc.c mixer.c music.c music_flac.c music_mad.c &
       music_mod.c music_modplug.c music_ogg.c &
       wavestream.c music_cmd.c

SRCS_native_midi = native_midi_common.c & native_midi_mac.c &
       native_midi_macosx.c native_midi_win32.c

SDL_timidity = common.c ctrlmode.c filter.c instrum.c mix.c output.c &
       playmidi.c readmidi.c resample.c sdl_a.c sdl_c.c tables.c timidity.c

SRCS = $(SRCS) $(SDL_timidity)

LIBS = flac_static.lib ogg_static.lib sdl12.lib smpeg_static.lib &
       vorbis_static.lib mikmod.lib
# Tremor_static.lib

CFLAGS  = $(CFLAGS_DLL) -ei -DBUILD_SDL -DWAV_MUSIC -DMOD_MUSIC -DFLAC_MUSIC &
          -DMP3_MUSIC -DOGG_MUSIC -DMID_MUSIC -DUSE_TIMIDITY_MIDI
# -DUSE_FLUIDSYNTH_MIDI -DMODPLUG_MUSIC -DMP3_MAD_MUSIC

.extensions:
.extensions: .lib .dll .obj .c

.c: .\timidity

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
    @$(%osdir)\KLIBC\BIN\date +"OPTION DESCRIPTION '@$#libsdl org:$(VERSION)$#@$#$#1$#$# %F               $(%HOSTNAME)::::::@@Simple DirectMedia Layer Mixer'" >>$^@
!else
    @%append $@ OPTION DESCRIPTION '@$#libsdl org:$(VERSION)$#@Simple DirectMedia Layer Mixer'
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
