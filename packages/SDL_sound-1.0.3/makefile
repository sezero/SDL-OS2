DLLNAME = sdlsound
VERSION = 1.0.3

INCPATH = .\decoders\timidity;..\libmikmod-3.3.6\include

!include ..\..\packages.mif

SRCS_mpglib = dct64_i386.c decode_i386.c interface.c layer1.c layer2.c &
              layer3.c mpglib_common.c tabinit.c

SRCS_timidity = common.c instrum.c mix.c output.c playmidi.c &
                readmidi.c resample.c tables.c timidity.c instrum_dls.c

SRCS = aiff.c au.c flac.c midi.c mikmod.c modplug.c mpglib.c ogg.c quicktime.c &
       raw.c shn.c smpeg.c speex.c voc.c wav.c &
       alt_audio_convert.c audio_convert.c extra_rwops.c SDL_sound.c &
       $(SRCS_mpglib) $(SRCS_timidity)

OBJS = $(SRCS:.c=.obj)

CFLAGS  = $(CFLAGS_DLL) -ei -DBUILD_SDL -DSDL_SOUND_DLL_EXPORTS &
          -DSOUND_SUPPORTS_SMPEG &
          -DSOUND_SUPPORTS_MPGLIB &
          -DSOUND_SUPPORTS_MIKMOD &
          -DSOUND_SUPPORTS_WAV &
          -DSOUND_SUPPORTS_AIFF &
          -DSOUND_SUPPORTS_AU &
          -DSOUND_SUPPORTS_OGG &
          -DSOUND_SUPPORTS_VOC &
          -DSOUND_SUPPORTS_RAW &
          -DSOUND_SUPPORTS_FLAC &
          -DSOUND_SUPPORTS_MIDI
# SOUND_SUPPORTS_MODPLUG

LIBS = sdl12.lib flac_static.lib vorbis_static.lib ogg_static.lib &
       smpeg_static.lib mikmod.lib

.extensions:
.extensions: .lib .dll .obj .c

.c:  .\decoders
.c:  .\decoders\mpglib
.c:  .\decoders\timidity
.c:  .\playsound

$(EXEPATH)\playsound.exe: $(LIBFILE) playsound.obj
  @echo * Link: $@
  @wlink SYS os2v2 libpath $(LIBPATH) lib {sdl12.lib $(LIBFILE)} op q op el file {playsound} name $*

playsound.obj: .\playsound\playsound.c
  @wcc386 $(CFLAGS_EXE) -ei -wcd=136 -DHAVE_SIGNAL_H -DHAVE_SETBUF -DHAVE_ASSERT_H $[@

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
  @$(%osdir)\KLIBC\BIN\date +"OPTION DESCRIPTION '@$#libsdl org:$(VERSION)$#@$#$#1$#$# %F               $(%HOSTNAME)::::::@@Simple DirectMedia Layer abstract soundfile decoder'" >>$^@
!else
  @%append $@ OPTION DESCRIPTION '@$#libsdl org:$(VERSION)$#@Simple DirectMedia Layer abstract soundfile decoder'
!endif
  @%append $@ LIBPATH $(LIBPATH);..\libmikmod-3.3.6\os2
  @for %i in ($(LIBS)) do @%append $@ LIB %i
  @%append $@ OPTION MAP=$*
  @%append $@ OPTION ELIMINATE
  @%append $@ OPTION MANYAUTODATA
  @%append $@ OPTION OSNAME='OS/2 and eComStation'
  @%append $@ OPTION SHOWDEAD

wav.obj:
  @wcc386 $(CFLAGS) -wcd=1181 .\decoders\$*.c

mikmod.obj:
  @wcc386 $(CFLAGS) -wcd=102 .\decoders\$*.c

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
  @if exist $(EXEPATH)\playsound.exe @del $(EXEPATH)\playsound.exe