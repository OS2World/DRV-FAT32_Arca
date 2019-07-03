rem
rem Build with OpenWatcom tools
rem
setlocal
@echo off
set d=p
SET DEBUG=0
set WATCOM=e:\compilers\watcom
set TOOLKIT=e:\os2tk45
set DDKBASE=%TOOLKIT%\DDK\BASE
set PATH=%toolkit%\bin;%watcom%\binp;\tools\bin;%path%
set BEGINLIBPATH=%toolkit%\dll;%watcom%\binp\dll;\tools\dll
call %WATCOM%\owsetenv.cmd
call svnrev.cmd
call envwic.cmd
wmake -h -f makefile.wcc %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
