Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$OutDir = "build"
if (-not (Test-Path $OutDir)) { New-Item -ItemType Directory -Path $OutDir | Out-Null }

$exe = Join-Path $OutDir "clothing_item_test.exe"

Write-Host "Compiling ClothingItem tests..."

# Compile statement:
g++ -std=c++17 -Wall -Wextra -Wpedantic -O0 -g `
  ".\clothing_item.cpp" ".\clothing_item_test.cpp" `
  -o $exe

Write-Host "Running tests..."
& $exe

