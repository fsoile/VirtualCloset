@echo off
setlocal enabledelayedexpansion

rem Always run from this script's directory (repo root).
pushd "%~dp0"

for %%I in ("%~dp0.") do set "ROOT=%%~fI"

set OUTDIR=build
set EXE=%OUTDIR%\virtual_closet.exe

if not exist "%OUTDIR%" mkdir "%OUTDIR%"

rem ---- SFML (manual zip) setup ----
rem Preferred: set SFML_DIR to the extracted SFML folder (the one containing include\ and lib\).
rem Example:
rem   setx SFML_DIR C:\libs\SFML-2.6.1
rem Or: put the extracted folder inside this repo as .\SFML\

rem Default to local .\SFML\ folder (repo root).
set "SFML_DIR_CHOSEN=SFML"

rem If user provided SFML_DIR env var (outside this script), prefer it.
rem Use delayed expansion to avoid odd expansion edge-cases in some shells.
if not "!SFML_DIR!"=="" set "SFML_DIR_CHOSEN=!SFML_DIR!"

rem Normalize (removes wrapping quotes / odd spacing).
for %%I in ("!SFML_DIR_CHOSEN!") do set "SFML_DIR_CHOSEN=%%~fI"

echo CHECKING_SFML_DIR=[!SFML_DIR_CHOSEN!]
dir /b "!SFML_DIR_CHOSEN!" >NUL 2>&1

rem SFML sanity check: show expected header path (do not hard-fail here).
set "SFML_HEADER=%SFML_DIR_CHOSEN%\include\SFML\Graphics.hpp"
echo CHECKING_HEADER_PATH=[%SFML_HEADER%]
if exist "%SFML_HEADER%" (echo SFML_HEADER=FOUND) else (echo SFML_HEADER=MISSING)

rem Windows will not overwrite an .exe that is still running.
taskkill /F /IM virtual_closet.exe >NUL 2>&1

echo Compiling Virtual Closet (SFML)...
echo.
echo Using SFML_DIR=!SFML_DIR_CHOSEN!
echo.

g++ -std=c++17 -Wall -Wextra -Wpedantic -O0 -g -static-libgcc -static-libstdc++ -static ^
  ".\main.cpp" ".\clothing_item.cpp" ".\outfit_combo.cpp" ".\closet.cpp" ^
  -o "%EXE%" ^
  -I"!SFML_DIR_CHOSEN!\include" -L"!SFML_DIR_CHOSEN!\lib" ^
  -lsfml-graphics -lsfml-window -lsfml-system

if errorlevel 1 (
  echo.
  echo Build failed.
  exit /b 1
)

rem Copy SFML runtime DLLs next to the executable (so it can run).
rem SFML 3 MinGW ships sfml-*-3.dll; older layouts used libsfml-*-3.dll.
for %%D in (
  "sfml-graphics-2.dll" "sfml-window-2.dll" "sfml-system-2.dll"
  "sfml-graphics-3.dll" "sfml-window-3.dll" "sfml-system-3.dll"
  "libsfml-graphics-3.dll" "libsfml-window-3.dll" "libsfml-system-3.dll"
) do (
  if exist "!SFML_DIR_CHOSEN!\bin\%%~D" copy /Y "!SFML_DIR_CHOSEN!\bin\%%~D" "%OUTDIR%\" >NUL
)

rem SFML DLLs also need the MinGW C++ runtime. Copy them from g++'s folder.
for /f "delims=" %%G in ('where g++ 2^>nul') do (
  set "MINGW_BIN=%%~dpG"
  goto :found_gpp
)
:found_gpp
if defined MINGW_BIN (
  for %%D in ("libgcc_s_seh-1.dll" "libstdc++-6.dll" "libwinpthread-1.dll") do (
    if exist "!MINGW_BIN!%%~D" copy /Y "!MINGW_BIN!%%~D" "%OUTDIR%\" >NUL
  )
)

echo.
echo Build succeeded: %EXE%
echo Running...
"%EXE%"

popd

