@echo off
rem Builds the Ogg test app and its unsigned .sis (S:\supertux2_symbian\tests\oggtest\oggtest.sis).
rem Run S:\supertux2_symbian\build_libs.bat first (and get sdl.dll repaired).
title SuperTux 2 - Ogg test
subst S: C:\Symbian\9.2\S60_3rd_FP1 >nul 2>&1
set EPOCROOT=\
set PATH=C:\Perl\bin;S:\epoc32\gcc\bin;S:\epoc32\tools;C:\Program Files (x86)\CSL Arm Toolchain\bin;%PATH%
set MAKEFLAGS=-j8
set LOG=S:\supertux2_symbian\tests\oggtest\build_log.txt
echo ==== started %date% %time% > %LOG%

cd /d S:\supertux2_symbian\tests\oggtest\group
echo [1/2] Compiling...
call bldmake bldfiles >> %LOG% 2>&1
call abld build gcce urel >> %LOG% 2>&1
echo [2/2] Making the .sis...
if exist ..\oggtest.sis del ..\oggtest.sis
call makesis oggtest.pkg ..\oggtest.sis >> %LOG% 2>&1

echo.
findstr /i /c:"error:" /c:"] Error" /c:"not recognized" %LOG% > nul
if %errorlevel%==0 (
  echo *** BUILD ERRORS. Send S:\supertux2_symbian\tests\oggtest\build_log.txt to Claude. ***
  findstr /i /c:"error:" /c:"] Error" /c:"not recognized" %LOG%
) else (
  dir ..\oggtest.sis | findstr /i "oggtest"
  echo Done: S:\supertux2_symbian\tests\oggtest\oggtest.sis
)
echo.
pause
