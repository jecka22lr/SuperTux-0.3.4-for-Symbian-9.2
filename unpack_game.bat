@echo off
rem Unpacks the SuperTux 0.3.4 source prepared for Symbian into S:\supertux2_symbian\game
rem (src, external libraries and the Symbian build files). Run once.
subst S: C:\Symbian\9.2\S60_3rd_FP1 >nul 2>&1
echo Unpacking into S:\supertux2_symbian\game ...
powershell -NoProfile -Command "Expand-Archive -Force -Path 'S:\supertux2_symbian\supertux2_game_src.zip' -DestinationPath 'S:\supertux2_symbian\game'"
if exist S:\supertux2_symbian\game\symbian\supertux2.mmp (
  echo Done.
) else (
  echo *** Unpacking failed. ***
)
pause
