dnl Process this file with autoconf to produce a configure script.

AC_INIT(SDL_gfxPrimitives.c)
AC_CONFIG_MACRO_DIR([m4])

dnl SDL version required
SDL_VERSION=1.2.7
AC_SUBST(SDL_VERSION)

dnl Setup for automake

# Making releases:
#   MICRO_VERSION += 1;
#   INTERFACE_AGE += 1;
#   BINARY_AGE += 1;
# if any functions have been added, set INTERFACE_AGE to 0.
# if backwards compatibility has been broken,
# set BINARY_AGE and INTERFACE_AGE to 0.

MAJOR_VERSION=2
MINOR_VERSION=0
MICRO_VERSION=26
INTERFACE_AGE=1
BINARY_AGE=10
VERSION=$MAJOR_VERSION.$MINOR_VERSION.$MICRO_VERSION

AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(MICRO_VERSION)
AC_SUBST(INTERFACE_AGE)
AC_SUBST(BINARY_AGE)
AC_SUBST(VERSION)

# libtool versioning
LT_RELEASE=$MAJOR_VERSION.$MINOR_VERSION
LT_CURRENT=`expr $MICRO_VERSION - $INTERFACE_AGE`
LT_REVISION=$INTERFACE_AGE
LT_AGE=`expr $BINARY_AGE - $INTERFACE_AGE`

AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

dnl Detect the canonical host and target build environment
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(SDL_gfx, $VERSION)

dnl Check for compilers
LT_INIT([win32-dll])
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

if test x$ac_cv_c_compiler_gnu = xyes; then
  CFLAGS="$CFLAGS -Wall"
fi

AC_ARG_ENABLE(mmx, [AS_HELP_STRING([--enable-mmx],[Use mmx assembly on x86 [default=yes]])],
            , enable_mmx=yes)
if test x$enable_mmx = xyes; then
  case "$host" in
    x86_64-*)
      CFLAGS="$CFLAGS -DUSE_MMX"
      ;;

    i?86-*)
      AC_MSG_CHECKING(for GCC -mmmx option)
      saved_CFLAGS="$CFLAGS"
      CFLAGS="$CFLAGS -mmmx"
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
         #include <mmintrin.h>
         int foo (__m64 m) {
            return _mm_cvtsi64_si32(m);
         }]],[])],
         [have_gcc_mmx=yes],
         [have_gcc_mmx=no])
      AC_MSG_RESULT($have_gcc_mmx)

      if test x$have_gcc_mmx = xyes; then
         CFLAGS="$CFLAGS -DUSE_MMX"
      else
         enable_mmx=no
         CFLAGS="$saved_CFLAGS"
      fi
      ;;

    *) enable_mmx=no
      ;;
  esac
fi
AC_MSG_CHECKING(whether MMX can be enabled)
AC_MSG_RESULT($enable_mmx)

dnl Check for SDL
AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)
CFLAGS="$CFLAGS $SDL_CFLAGS"
# Remove dependencies from 'sdl-config' not meant for us libraries:
SDL_LIBS=$(echo "$SDL_LIBS" | sed -e 's/-lmingw32//' -e 's/-lSDLmain//')
LIBS="$LIBS $SDL_LIBS"

# Finally create all the generated files
AC_OUTPUT([
Makefile
SDL_gfx.pc
])
