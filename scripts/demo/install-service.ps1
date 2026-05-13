# Переустанавливает Windows-службу из Release-сборки, запускает её и показывает sc query.
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$ServiceName = 'AntivirusGuiService'
$ProjectRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$ServiceExe = Join-Path $ProjectRoot 'build-local-extra-final\Release\AntivirusService.exe'

if (-not (Test-Path $ServiceExe)) {
    throw "Service executable not found. Run scripts/demo/build-release.ps1 first."
}

Stop-Process -Name 'AntivirusGui' -Force -ErrorAction SilentlyContinue

$existing = Get-Service -Name $ServiceName -ErrorAction SilentlyContinue
if ($null -ne $existing) {
    if ($existing.Status -ne 'Stopped') {
        Stop-Service -Name $ServiceName -Force -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 2
    }

    & $ServiceExe --uninstall
    Start-Sleep -Seconds 2
}

& $ServiceExe --install
if ($LASTEXITCODE -ne 0) {
    throw "Service install failed with exit code $LASTEXITCODE."
}

Start-Service -Name $ServiceName
Start-Sleep -Seconds 2
sc.exe query $ServiceName
