# Показывает последние 120 строк service.log.
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$LogPath = Join-Path $env:ProgramData 'AntivirusGui\service.log'
if (-not (Test-Path $LogPath)) {
    Write-Host "Файл лога не найден: $LogPath"
    exit 0
}

Get-Content -LiteralPath $LogPath -Tail 120
