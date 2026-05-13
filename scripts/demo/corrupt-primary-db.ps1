# Портит primary avdb.bin, переворачивая первый байт файла.
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$DbPath = Join-Path $env:ProgramData 'AntivirusGui\bases\avdb.bin'
if (-not (Test-Path $DbPath)) {
    throw "Primary database not found: $DbPath"
}

$bytes = [System.IO.File]::ReadAllBytes($DbPath)
if ($bytes.Length -lt 1) {
    throw "Primary database is empty: $DbPath"
}

$bytes[0] = $bytes[0] -bxor 0xff
[System.IO.File]::WriteAllBytes($DbPath, $bytes)

Write-Host "Primary database corrupted: $DbPath"
