# Готовит сценарий восстановления из avdb.bak: копирует backup, портит primary, перезапускает службу и показывает лог.
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$ServiceName = 'AntivirusGuiService'
$BaseDir = Join-Path $env:ProgramData 'AntivirusGui\bases'
$DbPath = Join-Path $BaseDir 'avdb.bin'
$BackupPath = Join-Path $BaseDir 'avdb.bak'
$LogPath = Join-Path $env:ProgramData 'AntivirusGui\service.log'

if (-not (Test-Path $DbPath)) {
    throw "Primary database not found: $DbPath"
}

Copy-Item -LiteralPath $DbPath -Destination $BackupPath -Force
& (Join-Path $PSScriptRoot 'corrupt-primary-db.ps1')

Restart-Service -Name $ServiceName -Force
Start-Sleep -Seconds 3

Write-Host "Backup prepared: $BackupPath"
if (Test-Path $LogPath) {
    Get-Content -LiteralPath $LogPath -Tail 120
}
