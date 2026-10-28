@echo off
rem Builds SuperTux 2 and its unsigned test package S:\supertux2_symbian\supertux2.sis
rem (program, its own SDL and Open C; the game data is not included, see make_release2.bat).
rem Only recompiles what changed. Run S:\supertux2_symbian\build_libs.bat once before.
title SuperTux 2 - build
subst S: C:\Symbian\9.2\S60_3rd_FP1 >nul 2>&1
set EPOCROOT=\
set PATH=C:\Perl\bin;S:\epoc32\gcc\bin;S:\epoc32\tools;C:\Program Files (x86)\CSL Arm Toolchain\bin;%PATH%
set MAKEFLAGS=-j8
set LOG=S:\supertux2_symbian\build_supertux2_log.txt
set URel=S:\epoc32\release\gcce\urel
set ZApps=S:\epoc32\data\z\resource\apps
echo ==== started %date% %time% > %LOG%

echo [1/4] SuperTux 2's own SDL (st2sdl.dll, st2sdlexe.dll)...
cd /d S:\supertux2_symbian\sdl\group
echo ==== SDL >> %LOG%
call bldmake bldfiles >> %LOG% 2>&1
call abld build -k gcce urel >> %LOG% 2>&1
rem a freshly linked st2sdl.dll has broken static data: repair it (skipped when already done)
perl fix_st2sdl.pl %URel%\st2sdl.dll >> %LOG% 2>&1

echo [2/4] Icon and pointer bitmaps...
echo ==== bitmaps >> %LOG%
call bmconv %ZApps%\st2sdlexe.mbm /c16S:\official\SDL-1.2.14\symbian\src\pointer.bmp /8S:\official\SDL-1.2.14\symbian\src\pointer-alpha.bmp >> %LOG% 2>&1
call bmconv %ZApps%\supertux2_icon.mbm /c24S:\supertux2_symbian\game\symbian\supertux2_icon.bmp /8S:\supertux2_symbian\game\symbian\supertux2_icon_mask.bmp >> %LOG% 2>&1

echo [3/4] Compiling the game (only what changed)...
cd /d S:\supertux2_symbian\game\symbian
echo ==== game >> %LOG%
rem always relink, otherwise rebuilt libraries (physfs.lib etc.) are not picked up
if exist %URel%\supertux2.exe del %URel%\supertux2.exe
call bldmake bldfiles >> %LOG% 2>&1
call abld build -k gcce urel >> %LOG% 2>&1

echo [4/4] Making the .sis...
if exist S:\supertux2_symbian\supertux2.sis del S:\supertux2_symbian\supertux2.sis
call makesis supertux2.pkg S:\supertux2_symbian\supertux2.sis >> %LOG% 2>&1

echo.
findstr /i /c:"error:" /c:"] Error" /c:"not recognized" /c:"cannot find" %LOG% > nul
set BUILD_RESULT=0
if %errorlevel%==0 (
  set BUILD_RESULT=1
  echo *** BUILD ERRORS. Tell Claude, the log is S:\supertux2_symbian\build_supertux2_log.txt ***
  findstr /i /c:"error:" /c:"] Error" /c:"not recognized" /c:"cannot find" %LOG%
) else (
  dir S:\supertux2_symbian\supertux2.sis | findstr /i "supertux2"
  echo Done: S:\supertux2_symbian\supertux2.sis
)
echo.
rem "nopause" is passed by make_release_r1.bat
if /i not "%~1"=="nopause" pause
exit /b %BUILD_RESULT%
