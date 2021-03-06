dnl Process this file with autoconf to produce a configure script.

dnl Set various version strings - taken gratefully from the GTk sources
# Making releases:
#   MICRO_VERSION_MACRO += 1;
#   INTERFACE_AGE_MACRO += 1;
#   BINARY_AGE_MACRO += 1;
# if any functions have been added, set INTERFACE_AGE_MACRO to 0.
# if backwards compatibility has been broken,
# set BINARY_AGE_MACRO and INTERFACE_AGE_MACRO to 0.
m4_define([MAJOR_VERSION_MACRO], [2])
m4_define([MINOR_VERSION_MACRO], [0])
m4_define([MICRO_VERSION_MACRO], [11])
dnl 'patch' version added to not bump version to 2.0.12,
dnl  because SDL2_ttf does have a 2.0.12 release.  this
dnl  affects only the package and libtool versions,  but
dnl  not SDL_TTF_VERSION which still reads 2.0.11..
m4_define([PATCH_VERSION_MACRO], [1])
m4_define([INTERFACE_AGE_MACRO], [1])
m4_define([BINARY_AGE_MACRO], [11])

AC_INIT([SDL_ttf],
        MAJOR_VERSION_MACRO.MINOR_VERSION_MACRO.MICRO_VERSION_MACRO.PATCH_VERSION_MACRO,
        [https://github.com/libsdl-org/SDL_ttf/issues],
        [SDL_ttf])
AC_CONFIG_MACRO_DIR([acinclude])
AC_CONFIG_SRCDIR([SDL_ttf.c])

AC_SUBST([MAJOR_VERSION], MAJOR_VERSION_MACRO)
AC_SUBST([MINOR_VERSION], MINOR_VERSION_MACRO)
AC_SUBST([MICRO_VERSION], MICRO_VERSION_MACRO)
AC_SUBST([PATCH_VERSION], PATCH_VERSION_MACRO)
AC_SUBST([INTERFACE_AGE], INTERFACE_AGE_MACRO)
AC_SUBST([BINARY_AGE], BINARY_AGE_MACRO)

# libtool versioning
LT_INIT([win32-dll])

LT_RELEASE=$MAJOR_VERSION.$MINOR_VERSION
LT_CURRENT=`expr $MICRO_VERSION - $INTERFACE_AGE`
LT_REVISION=`expr $INTERFACE_AGE + $PATCH_VERSION`
LT_AGE=`expr $BINARY_AGE - $INTERFACE_AGE`

AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

dnl Detect the canonical build and host environments
AC_CANONICAL_HOST

dnl Setup for automake
AM_INIT_AUTOMAKE([foreign tar-ustar])

dnl Check for tools
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
if test -z "$host_alias"; then
    hostaliaswindres=
else
    hostaliaswindres="$host_alias-windres"
fi
AC_CHECK_PROGS(WINDRES, [windres $hostaliaswindres $host_os-windres])

case "$host" in
    *-*-cygwin* | *-*-mingw*)
        ;;

    *)  AC_FUNC_ALLOCA
        ;;
esac

case "$host" in
    *-*-beos*)
        ac_default_prefix=/boot/develop/tools/gnupro
        ;;
    *-*-cygwin* | *-*-mingw*)
        if test "$build" != "$host"; then # cross-compiling
            # Default cross-compile location
            ac_default_prefix=/usr/local/cross-tools/i386-mingw32
        else
            # Look for the location of the tools and install there
            if test "$BUILD_PREFIX" != ""; then
                ac_default_prefix=$BUILD_PREFIX
            fi
        fi
        if test x$WINDRES != x; then
            use_version_rc=true
        fi
        ;;
esac
AM_CONDITIONAL(USE_VERSION_RC, test x$use_version_rc = xtrue)

dnl Check for the FreeType 2 library
PKG_CHECK_MODULES([FT2], [freetype2 >= 7.0.1], [], [dnl
    AC_CHECK_FT2(,,[AC_MSG_ERROR([dnl
*** Unable to find FreeType2 library (http://www.freetype.org/)])]
    )
])
CFLAGS="$CFLAGS $FT2_CFLAGS"
LIBS="$LIBS $FT2_LIBS"

dnl Check for SDL
SDL_VERSION=1.2.4
AC_SUBST(SDL_VERSION)
AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

dnl Check for OpenGL
case "$host" in
    *-*-cygwin* | *-*-mingw*)
        MATHLIB=""
        SYS_GL_LIBS="-lopengl32"
        ;;
    *-*-beos*)
        MATHLIB=""
        SYS_GL_LIBS="-lGL"
        ;;
    *-*-darwin*)
        MATHLIB=""
        SYS_GL_LIBS="-Wl,-framework,OpenGL"
        ;;
    *-*-aix*)
        if test x$ac_cv_c_compiler_gnu = xyes; then
            CFLAGS="-mthreads"
        fi
        SYS_GL_LIBS=""
        ;;
    *)
        MATHLIB="-lm"
        AC_PATH_X
        AC_PATH_XTRA
        if test x$have_x = xyes; then
            CFLAGS="$CFLAGS $X_CFLAGS"
            SYS_GL_LIBS="$X_LIBS -lGL"
        else
            SYS_GL_LIBS="-lGL"
        fi
        ;;
esac
AC_MSG_CHECKING(for OpenGL support)
have_opengl=no
save_LIBS="$LIBS"
LIBS="$LIBS $SYS_GL_LIBS"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
 #include "SDL_opengl.h"
]], [[
 glOrtho( -2.0, 2.0, -2.0, 2.0, -20.0, 20.0 );
]])], [have_opengl=yes],[])
AC_MSG_RESULT($have_opengl)
LIBS="$save_LIBS"
if test x$have_opengl = xyes; then
    CFLAGS="$CFLAGS -DHAVE_OPENGL"
    GL_LIBS="$SYS_GL_LIBS"
else
    GL_LIBS=""
fi
AC_CHECK_LIB(iconv, libiconv_open, [ICONV_LIBS="-liconv"])
AC_SUBST([GL_LIBS])
AC_SUBST([ICONV_LIBS])
AC_SUBST([MATHLIB])
AC_SUBST([WINDRES])

# Finally create all the generated files
AC_CONFIG_FILES([
Makefile
SDL_ttf.spec
SDL_ttf.pc
])
AC_OUTPUT
