SRCS_getopt = getopt.c getopt1.c
SRCS_grabbag = alloc.c cuesheet.c file.c picture.c replaygain.c seektable.c &
               snprintf.c
SRCS_replaygain_analysis = replaygain_analysis.c
SRCS_replaygain_synthesis = replaygain_synthesis.c
SRCS_utf8 = charset.c iconvert.c utf8.c
# charset_test.c makemap.c

SRCS = $(SRCS_grabbag) $(SRCS_replaygain_analysis) &
       $(SRCS_replaygain_synthesis) $(SRCS_utf8)
# $(SRCS_getopt)

OBJS = $(SRCS:.c=.obj)

INCPATH = $(%WATCOM)\H\os2;$(%WATCOM)\H;.;..\..\include;..\..\include\share;.\replaygain_synthesis\include

CFLAGS = -i=$(INCPATH) -bt=os2 -d0 -q -s
CFLAGS += -DHAVE_CONFIG_H

share.lib: $(OBJS)
  @for %f in ($(OBJS)) do @wlib -b -q $* +%f >nul

.extensions:
.extensions: .lib .obj .c

.c: .\getopt;.\grabbag;.\replaygain_analysis;.\replaygain_synthesis;.\utf8

.c.obj:
  @wcc386 $(CFLAGS) $<

clean: .SYMBOLIC
  @if exist *.obj del *.obj
  @if exist *.err del *.err
  @if exist share.lib del share.lib
