if USE_TIMIDITY
noinst_LTLIBRARIES = libtimidity.la
endif

AM_CPPFLAGS = -I$(top_srcdir)

libtimidity_la_SOURCES =	\
	common.c		\
	common.h		\
	instrum.c		\
	instrum.h		\
	mix.c			\
	mix.h			\
	options.h		\
	output.c		\
	output.h		\
	playmidi.c		\
	playmidi.h		\
	readmidi.c		\
	readmidi.h		\
	resample.c		\
	resample.h		\
	tables.c		\
	tables.h		\
	timidity.c		\
	timidity.h

EXTRA_DIST = CHANGES COPYING FAQ README TODO Makefile.testmidi testmidi.c
