# Makefile.am for the SDL Rich Text Format loading library and viewer
ACLOCAL_AMFLAGS = -I acinclude
AUTOMAKE_OPTIONS= foreign

lib_LTLIBRARIES = libSDL_rtf.la

libSDL_rtfincludedir = $(includedir)/SDL
libSDL_rtfinclude_HEADERS =	\
	SDL_rtf.h

libSDL_rtf_la_SOURCES =		\
	rtfactn.c		\
	rtfdecl.h		\
	rtfreadr.c		\
	rtftype.h		\
	SDL_rtfreadr.c		\
	SDL_rtfreadr.h		\
	SDL_rtf.c

EXTRA_DIST =			\
	CHANGES			\
	version.rc		\
	SDL_rtf.spec		\
	SDL_rtf.spec.in		\
	gcc-fat.sh		\
	autogen.sh

libSDL_rtf_la_LDFLAGS = 	\
	-no-undefined		\
	-release $(LT_RELEASE)	\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) $(LT_EXTRA)
if USE_VERSION_RC
libSDL_rtf_la_DEPENDENCIES = version.o
endif

.rc.o:
	$(RC) $< $@

if HAVE_SDL_TTF
EXAMPLES = showrtf
else
EXAMPLES =
endif
noinst_PROGRAMS = $(EXAMPLES)

showrtf_SOURCES = showrtf.c
showrtf_LDADD = libSDL_rtf.la @SDLTTF_LIBS@
showrtf_CFLAGS = @SDLTTF_CFLAGS@

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: distcheck

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz

