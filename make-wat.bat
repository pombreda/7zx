@echo off
if "%WATCOM%"=="" goto end

echo on
wcl386 /q /os /s /bcl=pmodew *.c /fe=7zdecwat
wcl386 /q /os /s /bcl=nt *.c /fe=7zdecw32
upx --ultra-brute --strip-relocs=0 7zdec*.exe
stubx -s 7zdecwat.exe
del *.obj
@echo off

cls
dir 7zdec*.exe

:end
set SEVENZIP=
