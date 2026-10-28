@echo off
rem Builds everything for a GitHub release of the SuperTux 0.3.4 S60v3 port, in S:\supertux_release:
rem   %REL%.sisx                  all-in-one, no log file (for players)
rem   %REL%_split.zip             the same, split for phones that refuse the big package
rem   %REL%-debuglog.sisx         all-in-one, writes c:\data\supertux2\log.txt (for testers)
rem   %REL%-debuglog_split.zip    the same, split
rem Needs build_libs.bat to have been run once, and the signing key in S:\supertux_signing.
title SuperTux 2 - release builds
subst S: C:\Symbian\9.2\S60_3rd_FP1 >nul 2>&1
set REL=SuperTux2_0.3.4-s60v3-r1
set OPT=S:\supertux2_symbian\game\symbian\build_options.h
set MAIN=S:\supertux2_symbian\game\symbian\symbian_main.cpp
set FAILED=

call :build 0 %REL%
if defined FAILED goto end
call :build 1 %REL%-debuglog
if defined FAILED goto end

echo.
echo All done:
dir /b S:\supertux_release\%REL%*.sisx S:\supertux_release\%REL%*.zip
goto end

:build
echo.
echo ===== %2 (logging %1) =====
call :options %1
rem touch symbian_main.cpp so it is recompiled with the new option
copy /b %MAIN% +,, %MAIN% >nul
call S:\supertux2_symbian\build_supertux2.bat nopause
if errorlevel 1 (set FAILED=1& echo *** Build failed, stopping. & exit /b)
call S:\supertux2_symbian\make_release2.bat %2 nopause
if errorlevel 1 (set FAILED=1& exit /b)
call S:\supertux2_symbian\make_split2.bat %2 nopause
if errorlevel 1 (set FAILED=1& exit /b)
exit /b

:options
> %OPT% echo // Build options for the Symbian port. make_release_r1.bat rewrites this file
>> %OPT% echo // for each build; the version kept in the repository is the debug one.
>> %OPT% echo //   ST2_LOGGING 1: writes c:\data\supertux2\log.txt (game messages, checkpoints, stack use)
>> %OPT% echo //   ST2_LOGGING 0: no log file (normal release)
>> %OPT% echo #define ST2_LOGGING %1
exit /b

:end
rem leave the source on the debug setting, as kept in the repository
call :options 1
title SuperTux 2 - release builds
echo.
pause
