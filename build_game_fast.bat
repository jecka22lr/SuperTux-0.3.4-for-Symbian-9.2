@echo off
rem Like build_game.bat, but compiles 8 files at once. Much faster; the log comes out jumbled,
rem and the old make can occasionally fail a file at random: if so, run build_game.bat.
rem Full output: S:\supertux2_symbian\build_game_log.txt (Claude reads it from here).
title SuperTux 2 - building the game (fast)
subst S: C:\Symbian\9.2\S60_3rd_FP1 >nul 2>&1
set EPOCROOT=\
set PATH=C:\Perl\bin;S:\epoc32\gcc\bin;S:\epoc32\tools;C:\Program Files (x86)\CSL Arm Toolchain\bin;%PATH%
set LOG=S:\supertux2_symbian\build_game_log.txt
set MAKEFLAGS=-j8
echo ==== started %date% %time% > %LOG%

cd /d S:\supertux2_symbian\game\symbian
echo Compiling, 8 files at a time...
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
