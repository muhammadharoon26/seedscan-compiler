# Add Scoop shims and GCC to the current session PATH
$scoopShims = Join-Path $env:USERPROFILE "scoop\shims"
$scoopGcc   = Join-Path $env:USERPROFILE "scoop\apps\gcc\current\bin"

if ($env:PATH -notlike "*$scoopShims*") {
    $env:PATH = "$scoopShims;" + $env:PATH
}
if ($env:PATH -notlike "*$scoopGcc*") {
    $env:PATH = "$scoopGcc;" + $env:PATH
}

Set-Location "d:\seedscan-compiler"

Write-Host "=== Prerequisites ===" -ForegroundColor Yellow
g++ --version | Select-Object -First 1
cmake --version | Select-Object -First 1
bison --version | Select-Object -First 1

# Check for make, install if missing
if (-not (Get-Command make -ErrorAction SilentlyContinue)) {
    Write-Host "`n[!] 'make' not found. Installing via scoop..." -ForegroundColor Yellow
    scoop install make
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Failed to install make. Please run 'scoop install make' manually." -ForegroundColor Red
        exit
    }
}
make --version | Select-Object -First 1

Write-Host "`n=== Configuring CMake ===" -ForegroundColor Cyan
cmake -S . -B build -G "Unix Makefiles"

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    exit
}

Write-Host "`n=== Building Project ===" -ForegroundColor Cyan
cmake --build build

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit
}

Write-Host "`n=== Running Tests ===" -ForegroundColor Cyan
Set-Location build
ctest --output-on-failure

Write-Host "`n=== All Done! ===" -ForegroundColor Green
