@echo off
rem Builds the split release for phones that refuse the all-in-one package:
rem   S:\supertux_release\%NAME%_split.zip (program + 3 data packages + README)
rem Run build_supertux2.bat first. The game data comes from S:\supertux2_symbian\package.
rem The signing key lives in S:\supertux_signing and must NEVER go on GitHub.
title SuperTux 2 - split packages
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
set SPLIT=%OUTDIR%\%NAME%_split
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
if exist %SPLIT% rmdir /s /q %SPLIT%
mkdir %SPLIT%

echo [1/2] Program and data packages...
makesis supertux2_program.pkg %SPLIT%\1_SuperTux2_program.sis
signsis %SPLIT%\1_SuperTux2_program.sis %SPLIT%\1_SuperTux2_program.sisx %KEYDIR%\jecka.cer %KEYDIR%\jecka.key %PASS%
del %SPLIT%\1_SuperTux2_program.sis
for %%N in (1 2 3 4) do (
  makesis supertux2_data%%N.pkg %SPLIT%\2_SuperTux2_data%%N.sis
  signsis %SPLIT%\2_SuperTux2_data%%N.sis %SPLIT%\2_SuperTux2_data%%N.sisx %KEYDIR%\jecka.cer %KEYDIR%\jecka.key %PASS%
  del %SPLIT%\2_SuperTux2_data%%N.sis
)
copy /y split_install_readme.txt %SPLIT%\README.txt >nul

echo [2/2] Zipping...
if exist %SPLIT%.zip del %SPLIT%.zip
powershell -NoProfile -Command "Compress-Archive -Path '%SPLIT%\*' -DestinationPath '%SPLIT%.zip'"

echo.
dir /b %SPLIT%
echo Done: %SPLIT%.zip
if /i not "%~2"=="nopause" pause
if not exist %SPLIT%.zip exit /b 1
exit /b 0
