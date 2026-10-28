@echo off
rem Compiles SuperTux 0.3.4 for Symbian, 8 files at a time. Keeps going after errors so one run shows them all.
rem If a file ever fails at random (the old make can trip over parallel builds), just run it again.
rem Full output: S:\supertux2_symbian\build_game_log.txt (Claude reads it from here).
title SuperTux 2 - building the game
subst S: C:\Symbian\9.2\S60_3rd_FP1 >nul 2>&1
set EPOCROOT=\
set PATH=C:\Perl\bin;S:\epoc32\gcc\bin;S:\epoc32\tools;C:\Program Files (x86)\CSL Arm Toolchain\bin;%PATH%
set MAKEFLAGS=-j8
set LOG=S:\supertux2_symbian\build_game_log.txt
echo ==== started %date% %time% > %LOG%

cd /d S:\supertux2_symbian\game\symbian
echo Compiling, 8 files at a time...
if exist S:\epoc32\release\gcce\urel\supertux2.exe del S:\epoc32\release\gcce\urel\supertux2.exe
call bldmake bldfiles >> %LOG% 2>&1
call abld build -k gcce urel >> %LOG% 2>&1
echo ==== finished %date% %time% >> %LOG%

echo.
findstr /i /c:"error:" /c:"] Error" /c:"not recognized" %LOG% > nul
if %errorlevel%==0 (
  echo Build finished with errors. Tell Claude: it reads build_game_log.txt directly.
) else (
  dir S:\epoc32\release\gcce\urel\supertux2.exe | findstr /i "supertux2"
  echo Build OK.
)
echo.
pause
