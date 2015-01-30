@echo off
if exist *.o del *.o
set cfbin=e:\apps\lang\coldfire\gcc-m68k\bin
::set cfbin=c:\coldfire\gcc-m68k\bin
%cfbin%\make.exe %1
%cfbin%\m68k-coff-objdump.exe -d -S -l testapp.cof >.\testapp.lst
