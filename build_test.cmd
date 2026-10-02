@echo off
setlocal enabledelayedexpansion

set OUTDIR=build
set EXE=%OUTDIR%\clothing_item_test.exe

if not exist "%OUTDIR%" mkdir "%OUTDIR%"

echo Compiling ClothingItem tests...
g++ -std=c++17 -Wall -Wextra -Wpedantic -O0 -g ^
  ".\clothing_item.cpp" ".\clothing_item_test.cpp" ^
  -o "%EXE%"
if errorlevel 1 (
  echo.
  echo Build failed.
  exit /b 1
)

echo Running tests...
"%EXE%"

