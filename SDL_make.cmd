@echo off
rem
rem This script will build libraries SDL with all required packages.
rem You must have installed Open Watcom and IBM OS/2 developer's toolkit.
rem As a result, next files should appear in .\dll :
rem	sdl12.dll
rem	sdlimage.dll
rem	sdlmixer.dll
rem	sdlnet.dll
rem	sdlttf.dll
rem
rem and LIB files for linking SDL with your programs in .\lib:
rem	sdl12.lib
rem	sdlimage.lib
rem	sdlmixer.lib
rem	sdlnet.lib
rem	sdlttf.lib
rem
rem Use path %LIBHOME%\h\SDL for include files.
rem
rem Andrey Vasilkin, 2014
rem digi@os2.snc.ru
rem

rem Specify the path to a directory where this script is located.
rem for ex.: SET LIBHOME = %HOME%\projects\port\LIBS
rem -------------------------------------------------------------------------

rem SET LIBHOME=%HOME%\projects\port\SDL

rem -------------------------------------------------------------------------


if .%LIBHOME%.==.. goto sethome
set makeSW=
if .%1.==.clean. goto l01
if .%1.==.make. goto l02

echo Usage: SDL_make.cmd [make or clean]
echo    make     Build all libraries for SDL
echo    clean    Delete all generated files
exit 1

:sethome
echo You should set variable LIBHOME in the file SDL_make.cmd.
exit 1

:l01
set makeSW=clean
goto l03

:l02
if not exist dll md dll
if not exist exe md exe
rem if not exist lib md lib

:l03
cd %LIBHOME%\packages\SDL-1.2.15
  wmake %makeSW% /h
cd %LIBHOME%\packages\ogg\src
  wmake %makeSW% /h
cd %LIBHOME%\packages\vorbis\lib
  wmake %makeSW% /h
cd %LIBHOME%\packages\flac-1.3.0\src\libFLAC
  wmake %makeSW% /h
cd %LIBHOME%\packages\smpeg-0.4.5
  wmake %makeSW% /h
cd %LIBHOME%\packages\SDL_mixer-1.2.12
  wmake %makeSW% /h
cd %LIBHOME%\packages\SDL_net-1.2.8
  wmake %makeSW% /h
cd %LIBHOME%\packages\zlib-1.2.8
  wmake %makeSW% /h
cd %LIBHOME%\packages\jpeg-9a
  wmake %makeSW% /h
cd %LIBHOME%\packages\lpng1612
  wmake %makeSW% /h
cd %LIBHOME%\packages\tiff-4.0.3\libtiff
  wmake %makeSW% /h
cd %LIBHOME%\packages\SDL_image-1.2.12
  wmake %makeSW% /h
cd %LIBHOME%\packages\freetype-2.5.3
  wmake %makeSW% /h
cd %LIBHOME%\packages\SDL_ttf-2.0.10
  wmake %makeSW% /h
cd %LIBHOME%\packages\SDL_sound-1.0.3
  wmake %makeSW% /h
cd %LIBHOME%\packages\SDL-1.2.15\test
  wmake %makeSW% /h
cd %LIBHOME%
