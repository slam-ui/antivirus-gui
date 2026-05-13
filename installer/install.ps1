# Installs the coursework WinUI GUI and Windows service from a Release build.
param(
    [string]$SourceDir = (Join-Path $PSScriptRoot '..\build-local-extra-final\Release'),
    [string]$InstallDir = (Join-Path $env:ProgramFiles 'AntivirusGui'),
    [switch]$StartService
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$ServiceName = 'AntivirusGuiService'
$SourceDir = (Resolve-Path $SourceDir).Path

$RequiredFiles = @(
    'AntivirusWinUi.exe',
    'AntivirusService.exe',
    'Microsoft.WindowsAppRuntime.Bootstrap.dll'
)

foreach ($file in $RequiredFiles) {
    $path = Join-Path $SourceDir $file
    if (-not (Test-Path $path)) {
        throw "Required file not found: $path"
    }
}

$existing = Get-Service -Name $ServiceName -ErrorAction SilentlyContinue
if ($null -ne $existing -and $existing.Status -ne 'Stopped') {
    Stop-Service -Name $ServiceName -Force
    Start-Sleep -Seconds 2
}

New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null

foreach ($file in $RequiredFiles) {
    Copy-Item -LiteralPath (Join-Path $SourceDir $file) -Destination (Join-Path $InstallDir $file) -Force
}

$serviceExe = Join-Path $InstallDir 'AntivirusService.exe'
if ($null -ne $existing) {
    & $serviceExe --uninstall
    if ($LASTEXITCODE -ne 0) {
        throw "Service uninstall failed with exit code $LASTEXITCODE."
    }
}

& $serviceExe --install
if ($LASTEXITCODE -ne 0) {
    throw "Service install failed with exit code $LASTEXITCODE."
}

if ($StartService) {
    Start-Service -Name $ServiceName
}

Write-Host "Installed to $InstallDir"
