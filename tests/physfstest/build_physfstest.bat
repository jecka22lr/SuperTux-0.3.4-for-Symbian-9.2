@echo off
rem Builds the PhysicsFS test app and its unsigned .sis (S:\supertux2_symbian\tests\physfstest\physfstest.sis).
rem Run S:\supertux2_symbian\build_libs.bat first.
title SuperTux 2 - PhysicsFS test
subst S: C:\Symbian\9.2\S60_3rd_FP1 >nul 2>&1
set EPOCROOT=\
set PATH=C:\Perl\bin;S:\epoc32\gcc\bin;S:\epoc32\tools;C:\Program Files (x86)\CSL Arm Toolchain\bin;%PATH%
set MAKEFLAGS=-j8
set LOG=S:\supertux2_symbian\tests\physfstest\build_log.txt
echo ==== started %date% %time% > %LOG%

cd /d S:\supertux2_symbian\tests\physfstest\group
echo [1/2] Compiling...
call bldmake bldfiles >> %LOG% 2>&1
call abld build gcce urel >> %LOG% 2>&1
echo [2/2] Making the .sis...
if exist ..\physfstest.sis del ..\physfstest.sis
call makesis physfstest.pkg ..\physfstest.sis >> %LOG% 2>&1

echo.
findstr /i /c:"error:" /c:"] Error" /c:"not recognized" %LOG% > nul
if %errorlevel%==0 (
  echo *** BUILD ERRORS. Send S:\supertux2_symbian\tests\physfstest\build_log.txt to Claude. ***
  findstr /i /c:"error:" /c:"] Error" /c:"not recognized" %LOG%
) else (
  dir ..\physfstest.sis | findstr /i "physfstest"
  echo Done: S:\supertux2_symbian\tests\physfstest\physfstest.sis
)
echo.
pause
