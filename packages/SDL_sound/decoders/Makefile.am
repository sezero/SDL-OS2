noinst_LTLIBRARIES = libdecoders.la

SUBDIRS = timidity

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/decoders/timidity

libdecoders_la_SOURCES =	\
	aiff.c		\
	au.c		\
	mikmod.c	\
	modplug.c	\
	mpg123.c	\
	ogg.c		\
	raw.c		\
	shn.c		\
	voc.c		\
	midi.c		\
	flac.c		\
	speex.c		\
	quicktime.c \
	wav.c

EXTRA_DIST = skeleton.c
