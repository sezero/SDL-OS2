# Makefile template for use under OS/2
#

AR=wlib
ARFLAGS=-b -c -n 
CC=wcc386
#CFLAGS=-5r -bt=os2 -fp5 -fpi87 -mf -oeatxh -w4 -zp8 -DHAVE_FCNTL_H -DHAVE_LIMITS_H -DHAVE_UNISTD_H -DHAVE_SYS_IOCTL_H -DHAVE_SYS_TIME_H -DMIKMOD_BUILD -d1 -DDRV_OS2 -DDRV_DART -DDRV_WAV -DDRV_AIFF -DDRV_STDOUT
CFLAGS=-bt=os2 -d0 -q -bm -5s -fp5 -sg -oteanbmier -DHAVE_FCNTL_H -DHAVE_LIMITS_H -DHAVE_UNISTD_H -DHAVE_SYS_IOCTL_H -DHAVE_SYS_TIME_H -DMIKMOD_BUILD -d1 -DDRV_OS2 -DDRV_DART -DDRV_WAV -DDRV_AIFF -DDRV_STDOUT
CFLAGS+=-i$(%WATCOM)\H\os2;$(%WATCOM)\H -DHAVE_LIMITS_H
COMPILE=$(CC) $(CFLAGS) -I..\include -I.. -I..\os2
IMPLIB=

.SUFFIXES:
.SUFFIXES: .o .c

LIBS=-lmmpm2
LIBNAME=mikmod.lib
DLLNAME=mikmod.dll
DEFNAME=mikmod.def
OBJ=drv_os2.o drv_dart.o \
    drv_raw.o drv_aiff.o drv_wav.o \
    drv_nos.o drv_stdout.o \
    load_669.o load_amf.o load_asy.o load_dsm.o load_far.o load_gdm.o load_gt2.o \
    load_it.o load_imf.o load_m15.o load_med.o load_mod.o load_mtm.o load_okt.o \
    load_s3m.o load_stm.o load_stx.o load_ult.o load_umx.o load_uni.o load_xm.o \
    mmalloc.o mmerror.o mmio.o \
    mdriver.o mdreg.o mloader.o mlreg.o mlutil.o mplayer.o munitrk.o mwav.o \
    npertab.o sloader.o virtch.o virtch2.o virtch_common.o

all: mikmod.lib

clean: .SYMBOLIC
	FOR %F IN ( $(LIBNAME) $(DLLNAME) $(OBJ) ) DO IF EXIST %F ERASE %F

distclean: clean
	FOR %F IN ( $(DEFNAME) ..\include\mikmod.h ..\include\mikmod_build.h Makefile ) DO IF EXIST %F ERASE %F

$(DLLNAME): $(OBJ) $(DEFNAME)
	$(CC) $(CFLAGS) -Zso -Zsys -o $(DLLNAME) $(DEFNAME) $(OBJ) $(LIBS)
	$(IMPLIB) -o $(LIBNAME) $(DEFNAME)

$(LIBNAME): $(OBJ)
	$(AR) $(ARFLAGS) $(LIBNAME) $(OBJ)

HEADER_DEPS=..\include\mikmod_build.h ..\include\mikmod_internals.h
drv_dart.o: ..\drivers\drv_dart.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\drivers\drv_dart.c
drv_nos.o: ..\drivers\drv_nos.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\drivers\drv_nos.c
drv_os2.o: ..\drivers\drv_os2.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\drivers\drv_os2.c
drv_raw.o: ..\drivers\drv_raw.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\drivers\drv_raw.c
drv_stdout.o: ..\drivers\drv_stdout.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\drivers\drv_stdout.c
drv_aiff.o: ..\drivers\drv_aiff.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\drivers\drv_aiff.c
drv_wav.o: ..\drivers\drv_wav.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\drivers\drv_wav.c
load_669.o: ..\loaders\load_669.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_669.c
load_amf.o: ..\loaders\load_amf.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_amf.c
load_asy.o: ..\loaders\load_asy.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_asy.c
load_dsm.o: ..\loaders\load_dsm.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_dsm.c
load_far.o: ..\loaders\load_far.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_far.c
load_gdm.o: ..\loaders\load_gdm.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_gdm.c
load_gt2.o: ..\loaders\load_gt2.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_gt2.c
load_it.o: ..\loaders\load_it.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_it.c
load_imf.o: ..\loaders\load_imf.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_imf.c
load_m15.o: ..\loaders\load_m15.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_m15.c
load_med.o: ..\loaders\load_med.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_med.c
load_mod.o: ..\loaders\load_mod.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_mod.c
load_mtm.o: ..\loaders\load_mtm.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_mtm.c
load_okt.o: ..\loaders\load_okt.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_okt.c
load_s3m.o: ..\loaders\load_s3m.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_s3m.c
load_stm.o: ..\loaders\load_stm.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_stm.c
load_stx.o: ..\loaders\load_stx.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_stx.c
load_ult.o: ..\loaders\load_ult.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_ult.c
load_umx.o: ..\loaders\load_umx.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_umx.c
load_uni.o: ..\loaders\load_uni.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_uni.c
load_xm.o: ..\loaders\load_xm.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\loaders\load_xm.c
mmalloc.o: ..\mmio\mmalloc.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\mmio\mmalloc.c
mmerror.o: ..\mmio\mmerror.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\mmio\mmerror.c
mmio.o: ..\mmio\mmio.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\mmio\mmio.c
mdriver.o: ..\playercode\mdriver.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\mdriver.c
mdreg.o: ..\playercode\mdreg.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\mdreg.c
mloader.o: ..\playercode\mloader.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\mloader.c
mlreg.o: ..\playercode\mlreg.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\mlreg.c
mlutil.o: ..\playercode\mlutil.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\mlutil.c
mplayer.o: ..\playercode\mplayer.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\mplayer.c
munitrk.o: ..\playercode\munitrk.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\munitrk.c
mwav.o: ..\playercode\mwav.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\mwav.c
npertab.o: ..\playercode\npertab.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\npertab.c
sloader.o: ..\playercode\sloader.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\sloader.c
virtch.o: ..\playercode\virtch.c ..\playercode\virtch_common.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\virtch.c
virtch2.o: ..\playercode\virtch2.c ..\playercode\virtch_common.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\virtch2.c
virtch_common.o: ..\playercode\virtch_common.c $(HEADER_DEPS)
	$(COMPILE) -fo=$^@ ..\playercode\virtch_common.c

FORCE:
