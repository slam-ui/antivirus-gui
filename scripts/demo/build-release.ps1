# Собирает Release-версию проекта через CMake и выводит пути к готовым exe.
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$ProjectRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$BuildDir = Join-Path $ProjectRoot 'build-local-extra-final'
$QtPath = 'C:/Users/13372/QtCodex/6.7.3/msvc2019_64'

Push-Location $ProjectRoot
try {
    cmake -S . -B $BuildDir -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$QtPath"
    cmake --build $BuildDir --config Release

    $GuiExe = Join-Path $BuildDir 'Release\AntivirusGui.exe'
    $ServiceExe = Join-Path $BuildDir 'Release\AntivirusService.exe'

    Write-Host "GUI:     $GuiExe"
    Write-Host "Service: $ServiceExe"
} finally {
    Pop-Location
}
