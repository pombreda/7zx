@echo off
if "%DJGPP%"=="" goto end
set SEVENZIP=7zdecrsx

echo on
gccw32 -Zrsx32 -O -march=i386 -x c *.c -o %SEVENZIP%.exe
pestack -s1024 -c100 %SEVENZIP%.exe
@echo off

cls
dir %SEVENZIP%.exe

:end
set SEVENZIP=
