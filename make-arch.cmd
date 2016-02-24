@echo off
rem
rem This script will build distribution archives containing sources and
rem binaries files. Script SDL_make.cmd will be called to build SDL.
rem The file 7za.exe must be located somewhere in your system PATH.
rem You should set variable LIBHOME in the file SDL_make.cmd.
rem
rem Andrey Vasilkin, 2016
rem digi@os2.snc.ru
rem


rem Under eCS we can get curent date for archive file name.

if exist %osdir%\KLIBC\BIN\date.exe goto ecsdate
set archdate=""
goto enddate
:ecsdate
%osdir%\KLIBC\BIN\date +"set archdate=-%%Y%%m%%d" >archdate.cmd
call archdate.cmd
del archdate.cmd
:enddate

rem Make archives of sources and binaries.

rem Make archive - sources.

call SDL_make.cmd clean
set fname=SDL-1.2.15-src%archdate%.zip

echo Packing: %fname%.
del %fname%
cd ..
7za.exe a -tzip -mx7 -xr-!SDL\*.zip -xr-!SDL\SDL ./SDL/%fname% ./SDL
cd SDL

rem Make archive - binaries.

call SDL_make.cmd make
set fname=SDL-1.2.15%archdate%.zip

rem Exclude files for archive of binaies
echo SDL/packages >exclude.lst
echo SDL/exe >>exclude.lst
echo SDL/lib/flac_static.lib >>exclude.lst
echo SDL/lib/freetype_static.lib >>exclude.lst
echo SDL/lib/jpeg_static.lib >>exclude.lst
echo SDL/lib/libpng_static.lib >>exclude.lst
echo SDL/lib/libtiff_static.lib >>exclude.lst
echo SDL/lib/ogg_static.lib >>exclude.lst
echo SDL/lib/os2386.lib >>exclude.lst
echo SDL/lib/smpeg_static.lib >>exclude.lst
echo SDL/lib/vorbis_static.lib >>exclude.lst
echo SDL/lib/zlib_static.lib >>exclude.lst
echo SDL/h/FLAC >>exclude.lst
echo SDL/h/freetype >>exclude.lst
echo SDL/h/jpeg >>exclude.lst
echo SDL/h/ogg >>exclude.lst
echo SDL/h/vorbis >>exclude.lst
echo SDL/h/*.h >>exclude.lst
echo SDL/*.cmd >>exclude.lst
echo SDL/packages.mif >>exclude.lst
echo SDL/*.zip >>exclude.lst
echo SDL/SDL >>exclude.lst
echo SDL/exclude.lst >>exclude.lst
rem */

echo Packing: %fname%.
del %fname%
cd ..
7za.exe a -tzip -mx7 -xr-@SDL/exclude.lst ./SDL/%fname% ./SDL
cd SDL
del exclude.lst
