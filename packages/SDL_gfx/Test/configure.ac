dnl Process this file with autoconf to produce a configure script.
AC_INIT(TestGfxPrimitives.c)
AC_CONFIG_MACRO_DIR([m4])

dnl Detect the canonical host and target build environment
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Setup for automake
AM_INIT_AUTOMAKE(TestGfxPrimitives, 1.0)

dnl SDL version required
SDL_VERSION=1.2.0

dnl Check for compilers
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_INSTALL

dnl Check for compiler environment
AC_C_CONST

dnl Set for C compiler by default
AC_LANG_C

dnl Setup target flags
case "$target" in
    *-*-cygwin* | *-*-mingw32*)
        if test "$build" != "$target"; then # cross-compiling
            ac_default_prefix=/usr/local/cross-tools/i386-mingw32msvc
        fi
        ;;
esac

dnl Figure out which math library to use
case "$target" in
    *-*-cygwin* | *-*-mingw32*)
        MATHLIB=""
        ;;
    *-*-beos* | *-*-haiku*)
        MATHLIB=""
        ;;
    *-*-darwin*)
        MATHLIB=""
        ;;
    *-*-aix*)
        MATHLIB="-lm"
        if test x$ac_cv_c_compiler_gnu = xyes; then
            CFLAGS="-mthreads"
        fi
        ;;
    *)
        MATHLIB="-lm"
        ;;
esac
AC_SUBST(MATHLIB)
LIBS="$LIBS $MATHLIB"

dnl Check for SDL

AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

dnl Check for the SDL_gfx lib
have_sdlgfx=no
AC_CHECK_LIB(SDL_gfx, pixelColor , have_sdlgfx=yes)
if test x$have_sdlgfx = xyes; then
    LIBS="$LIBS -lSDL_gfx"
else
    AC_MSG_ERROR([
*** Unable to find SDL_gfx library
])
fi

# Finally create all the generated files
AC_OUTPUT([Makefile])
