@ECHO OFF
IF EXIST Makefile GOTO BUILD
CALL configure.cmd
:BUILD
wmake -ms %1
IF ERRORLEVEL 1 GOTO ERROR
ECHO Compilation finished.
GOTO END
:ERROR
ECHO Compilation aborted !
:END
