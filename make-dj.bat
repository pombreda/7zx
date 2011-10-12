@echo off
REM GCC 2.95.3 + DJGPP 2.03p2 + hacked DOPRNT.C + TINY.C (no glob, no env.
REM    file) + UPX 3.03 + D3X 0.90.h
REM
REM 42,572 bytes

if "%DJGPP%"=="" goto end
set SEVENZIP=7zdecdj2

echo on
gcc -s -O -mcpu=i686 -malign-jumps=2 -malign-loops=2 -malign-functions=2 -x c *.c -o %SEVENZIP%.exe
upx --ultra-brute --small %SEVENZIP%.exe
stubx -s %SEVENZIP%.exe
@echo off

cls
dir %SEVENZIP%.exe

:end
set SEVENZIP=
