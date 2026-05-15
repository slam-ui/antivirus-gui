# Готовит forced update: кладёт корректную базу в mock server, портит manifest primary и показывает recovery log.
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Corrupt-ManifestSignature {
    param(
        [Parameter(Mandatory = $true)]
        [string] $Path
    )

    $bytes = [System.IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -lt 48) {
        throw "База слишком мала, чтобы содержать подпись manifest: $Path"
    }

    $releaseDateLength = [System.BitConverter]::ToUInt32($bytes, 8)
    $manifestOffset = 4 + 4 + 4 + ([int]$releaseDateLength * 2) + 4
    if ($manifestOffset -ge $bytes.Length) {
        throw "Смещение manifest находится вне файла базы: $manifestOffset"
    }

    $bytes[$manifestOffset] = $bytes[$manifestOffset] -bxor 0xff
    [System.IO.File]::WriteAllBytes($Path, $bytes)
}

$ServiceName = 'AntivirusGuiService'
$BaseDir = Join-Path $env:ProgramData 'AntivirusGui\bases'
$DbPath = Join-Path $BaseDir 'avdb.bin'
$MockServerDir = Join-Path $env:ProgramData 'AntivirusGuiMockServer'
$MockDbPath = Join-Path $MockServerDir 'avdb.bin'
$LogPath = Join-Path $env:ProgramData 'AntivirusGui\service.log'

if (-not (Test-Path $DbPath)) {
    throw "Основная база не найдена: $DbPath"
}

New-Item -ItemType Directory -Force -Path $MockServerDir | Out-Null
Copy-Item -LiteralPath $DbPath -Destination $MockDbPath -Force

Corrupt-ManifestSignature -Path $DbPath

Restart-Service -Name $ServiceName -Force
Start-Sleep -Seconds 3

Write-Host "Mock update база подготовлена: $MockDbPath"
if (Test-Path $LogPath) {
    Get-Content -LiteralPath $LogPath -Tail 120
}
