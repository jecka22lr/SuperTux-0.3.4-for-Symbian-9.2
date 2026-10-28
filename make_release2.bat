@echo off
rem Builds the signed all-in-one package (program, SDL, Open C, game data, add-ons):
rem   S:\supertux_release\%NAME%.sisx
rem Run build_supertux2.bat first. The game data comes from S:\supertux2_symbian\package.
rem The signing key lives in S:\supertux_signing and must NEVER go on GitHub.
title SuperTux 2 - full package
subst S: C:\Symbian\9.2\S60_3rd_FP1 >nul 2>&1
set EPOCROOT=\
set PATH=C:\Perl\bin;S:\epoc32\gcc\bin;S:\epoc32\tools;C:\Program Files (x86)\CSL Arm Toolchain\bin;%PATH%
set MAKEFLAGS=-j8
set KEYDIR=S:\supertux_signing
rem The key password is read from signing_password.txt next to this file (kept out of git),
rem or from the ST2_SIGN_PASS environment variable.
if not defined ST2_SIGN_PASS if exist S:\supertux2_symbian\signing_password.txt set /p ST2_SIGN_PASS=<S:\supertux2_symbian\signing_password.txt
set PASS=%ST2_SIGN_PASS%
set OUTDIR=S:\supertux_release
set NAME=SuperTux2_0.3.4-s60v3-test
rem make_release_r1.bat passes the release name and "nopause"
if not "%~1"=="" set NAME=%~1
cd /d S:\supertux2_symbian\game\symbian
if not exist %OUTDIR% mkdir %OUTDIR%
if not exist S:\supertux2_symbian\package\st2_4_music.zip (
  echo The game data is missing from S:\supertux2_symbian\package
  pause
  exit /b
)
if not exist %KEYDIR%\jecka.cer (
  echo The signing key is missing from %KEYDIR%
  pause
  exit /b 1
)
if "%PASS%"=="" (
  echo The signing key password is missing: put it in S:\supertux2_symbian\signing_password.txt
  pause
  exit /b 1
)

echo Packing about 32 MB, give it a minute...
if exist %OUTDIR%\%NAME%.sisx del %OUTDIR%\%NAME%.sisx
makesis supertux2_full.pkg %OUTDIR%\%NAME%.sis
signsis %OUTDIR%\%NAME%.sis %OUTDIR%\%NAME%.sisx %KEYDIR%\jecka.cer %KEYDIR%\jecka.key %PASS%
if exist %OUTDIR%\%NAME%.sis del %OUTDIR%\%NAME%.sis

echo.
dir %OUTDIR%\%NAME%.sisx | findstr /i "sisx"
echo Done: %OUTDIR%\%NAME%.sisx
if /i not "%~2"=="nopause" pause
if not exist %OUTDIR%\%NAME%.sisx exit /b 1
exit /b 0
