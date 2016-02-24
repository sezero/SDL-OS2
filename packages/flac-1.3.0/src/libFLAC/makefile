LIBNAME = flac
VERSION = 1.3.0
INCPATH =.\include;..\..\include

!include ..\..\..\..\packages.mif

CFLAGS = $(CFLAGS_DLL) -DVERSION="$(VERSION)"
CFLAGS += -DHAVE_CONFIG_H -DFLAC_API_EXPORTS -DFLAC__NO_DLL -DFLAC__HAS_OGG
#CFLAGS = $(CFLAGS_STATIC) -DVERSION="$(VERSION)" -DHAVE_CONFIG_H -DFLAC_API_EXPORTS

SRCS = bitmath.c bitreader.c bitwriter.c cpu.c crc.c fixed.c float.c format.c &
       lpc.c md5.c memory.c metadata_iterators.c metadata_object.c &
       ogg_decoder_aspect.c ogg_encoder_aspect.c ogg_helper.c &
       ogg_mapping.c stream_decoder.c stream_encoder.c &
       stream_encoder_framing.c window.c

OBJS = $(SRCS:.c=.obj)

#all: $(LIBFILE) $(LIBFILE_STATIC)

$(LIBFILE_STATIC): $(OBJS)
  @echo * Create library: $@...
  @for %f in ($(OBJS)) do @wlib -b -q $* +%f >nul

#$(LIBFILE): $(DLLFILE)
#  wlib -b -n -q $(LIBFILE) $(DLLFILE)

$(DLLFILE): $(OBJS) $(LNKFILE)
  wlink @$(LNKFILE)

$(LNKFILE):
    @%create $@
    @%append $@ SYSTEM os2v2_dll INITINSTANCE TERMINSTANCE
    @%append $@ NAME $(DLLPATH)\$(LIBNAME)
    @for %i in ($(OBJS)) do @%append $@ FILE %i
    @%append $@ OPTION QUIET
!ifdef %osdir
    @$(%osdir)\KLIBC\BIN\date +"OPTION DESCRIPTION '@$#Xiph Org:$(VERSION)$#@$#$#1$#$# %F               $(%HOSTNAME)::::::@@Free Lossless Audio Codec (FLAC)'" >>$^@
!else
    @%append $@ OPTION DESCRIPTION '@$#Xiph Org:$(VERSION)$#@Free Lossless Audio Codec (FLAC)'
!endif
    @%append $@ LIBPATH $(LIBPATH)
    @%append $@ LIB ogg
    @%append $@ OPTION ELIMINATE
    @%append $@ OPTION MANYAUTODATA
    @%append $@ OPTION OSNAME='OS/2 and eComStation'
    @%append $@ OPTION SHOWDEAD

stream_decoder.obj: stream_decoder.c
  @wcc386 $(CFLAGS) -wcd=136 $<

.c.obj:
  @wcc386 $(CFLAGS) $<

clean: .SYMBOLIC
  @echo * Clean: $(LIBNAME)
  @if exist *.obj @del *.obj
  @if exist *.err @del *.err
  @if exist $(LNKFILE) @del $(LNKFILE)
  @if exist $(DLLFILE) @del $(DLLFILE)
  @if exist $(LIBFILE) @del $(LIBFILE)
  @if exist $(LIBFILE_STATIC) @del $(LIBFILE_STATIC)
