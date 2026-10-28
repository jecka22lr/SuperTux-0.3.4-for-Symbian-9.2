@echo off
rem SuperTux Milestone 2 (0.3.x) libraries:
rem   Tremor (integer Ogg decoder), SDL_mixer with Ogg (sdl_mixer_ogg.lib), PhysicsFS (physfs.lib).
rem Milestone 1 (supertux_symbian_src) is not touched. Full output: build_libs_log.txt next to this file.
rem (sdl.dll with 64 KB thread stacks was built and repaired once already; it is not rebuilt here.)
title SuperTux 2 - building libraries
subst S: C:\Symbian\9.2\S60_3rd_FP1 >nul 2>&1
set EPOCROOT=\
set PATH=C:\Perl\bin;S:\epoc32\gcc\bin;S:\epoc32\tools;C:\Program Files (x86)\CSL Arm Toolchain\bin;%PATH%
set MAKEFLAGS=-j8
set LOG=S:\supertux2_symbian\build_libs_log.txt
echo ==== started %date% %time% > %LOG%

echo [1/3] Building Tremor...
cd /d S:\supertux2_symbian\libs\tremor\symbian
echo ==== Tremor >> %LOG%
call bldmake bldfiles >> %LOG% 2>&1
call abld export >> %LOG% 2>&1
call abld build gcce urel >> %LOG% 2>&1

echo [2/3] Building SDL_mixer with Ogg...
cd /d S:\supertux2_symbian\libs\sdl_mixer_ogg
echo ==== SDL_mixer (Ogg) >> %LOG%
call bldmake bldfiles >> %LOG% 2>&1
call abld build gcce urel >> %LOG% 2>&1

echo [3/3] Building PhysicsFS...
cd /d S:\supertux2_symbian\libs\physfs\symbian
echo ==== PhysicsFS >> %LOG%
call bldmake bldfiles >> %LOG% 2>&1
call abld reallyclean gcce urel >> %LOG% 2>&1
call abld export >> %LOG% 2>&1
call abld build gcce urel >> %LOG% 2>&1

echo ==== finished %date% %time% >> %LOG%
dir S:\epoc32\release\armv5\urel\tremor.lib S:\epoc32\release\armv5\urel\sdl_mixer_ogg.lib S:\epoc32\release\armv5\urel\physfs.lib >> %LOG% 2>&1
echo.
findstr /i /c:"error:" /c:"] Error" /c:"not recognized" %LOG% > nul
if %errorlevel%==0 (
  echo *** THE BUILD HAD ERRORS. Send S:\supertux2_symbian\build_libs_log.txt to Claude. ***
  findstr /i /c:"error:" /c:"] Error" /c:"not recognized" %LOG%
) else (
  echo Libraries OK.
)
echo.
pause
