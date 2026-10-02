Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$OutDir = "build"
if (-not (Test-Path $OutDir)) { New-Item -ItemType Directory -Path $OutDir | Out-Null }

$exe = Join-Path $OutDir "outfit_combo_test.exe"

Write-Host "Compiling OutfitCombo tests..."

# Compile statement:
g++ -std=c++17 -Wall -Wextra -Wpedantic -O0 -g `
  ".\clothing_item.cpp" ".\outfit_combo.cpp" ".\outfit_combo_test.cpp" `
  -o $exe

Write-Host "Running tests..."
& $exe

