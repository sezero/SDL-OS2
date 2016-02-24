LIBNAME = jpeg
VERSION = 9a
INCPATH =.\include

!include ..\..\packages.mif

SRCS = jaricom.c jcapimin.c jcapistd.c jcarith.c jccoefct.c jccolor.c &
       jcdctmgr.c jchuff.c jcinit.c jcmainct.c jcmarker.c jcmaster.c &
       jcomapi.c jcparam.c jcprepct.c jcsample.c jctrans.c jdapimin.c &
       jdapistd.c jdarith.c jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c &
       jddctmgr.c jdhuff.c jdinput.c jdmainct.c jdmarker.c jdmaster.c &
       jdmerge.c jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c &
       jfdctfst.c jfdctint.c jidctflt.c jidctfst.c jidctint.c jquant1.c &
       jquant2.c jutils.c jmemmgr.c jmemansi.c

OBJS = $(SRCS:.c=.obj)

COBJECTS= cjpeg.obj rdppm.obj rdgif.obj rdtarga.obj rdrle.obj rdbmp.obj &
          rdswitch.obj cdjpeg.obj
DOBJECTS= djpeg.obj wrppm.obj wrgif.obj wrtarga.obj wrrle.obj wrbmp.obj &
          rdcolmap.obj cdjpeg.obj
TROBJECTS= jpegtran.obj rdswitch.obj cdjpeg.obj transupp.obj

# Added switch -ei for compatibility with SDL: 
# Function like jpeg_CreateDecompress() check the size of given structure that
# must be same at compile time and at runtime.
CFLAGS = $(CFLAGS_STATIC) -ei -wcd=136
# -4r -ort

all: $(LIBFILE_STATIC) $(EXEPATH)\cjpeg.exe $(EXEPATH)\djpeg.exe

$(LIBFILE_STATIC): $(OBJS)
  @echo * Create library: $(LIBFILE_STATIC)...
  @for %f in ($(OBJS)) do @wlib -b -q $* +%f >nul

$(EXEPATH)\cjpeg.exe: $(COBJECTS) $(LIBFILE_STATIC)
  @echo * Link: $@
  @wlink SYS os2v2 libpath $(LIBPATH) lib {$(LIBFILE_STATIC)} op q op el file {$(OBJS:.obj=) $(COBJECTS:.obj=)} name $*

$(EXEPATH)\djpeg.exe: $(DOBJECTS) $(LIBFILE_STATIC)
  @echo * Link: $@
  @wlink SYS os2v2 libpath $(LIBPATH) lib {$(LIBFILE_STATIC)} op q op el file {$(OBJS:.obj=) $(DOBJECTS:.obj=)} name $*

.c.obj:
  @wcc386 $(CFLAGS) $<

clean: .SYMBOLIC
  @echo * Clean: $(TITLENAME)
  @if exist *.obj @del *.obj
  @if exist *.err @del *.err
  @if exist $(LIBFILE_STATIC) @del $(LIBFILE_STATIC)
  @if exist $(EXEPATH)\cjpeg.exe @del $(EXEPATH)\cjpeg.exe
  @if exist $(EXEPATH)\djpeg.exe @del $(EXEPATH)\djpeg.exe
