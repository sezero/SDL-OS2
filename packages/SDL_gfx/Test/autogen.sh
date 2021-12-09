#!/bin/sh
#
aclocal -I m4
automake --foreign --include-deps --add-missing --copy
autoconf
rm aclocal.m4

#./configure $*
echo "Now you are ready to run ./configure"
