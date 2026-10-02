@echo off
setlocal enabledelayedexpansion

set OUTDIR=build
set EXE=%OUTDIR%\outfit_combo_test.exe

if not exist "%OUTDIR%" mkdir "%OUTDIR%"

echo Compiling OutfitCombo tests...
g++ -std=c++17 -Wall -Wextra -Wpedantic -O0 -g ^
  ".\clothing_item.cpp" ".\outfit_combo.cpp" ".\outfit_combo_test.cpp" ^
  -o "%EXE%"
if errorlevel 1 (
  echo.
  echo Build failed.
  exit /b 1
)

echo Running tests...
"%EXE%"

