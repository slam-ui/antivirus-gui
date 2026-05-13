# Builds a single admin EXE installer with bundled prerequisites.
param(
    [string]$BuildDir = (Join-Path $PSScriptRoot '..\build-local-winui-clean'),
    [string]$OutputDir = (Join-Path $PSScriptRoot '..\out\installer')
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Find-MakeNsis {
    $command = Get-Command makensis.exe -ErrorAction SilentlyContinue
    if ($null -ne $command) {
        return $command.Source
    }

    $candidates = @(
        "${env:ProgramFiles(x86)}\NSIS\makensis.exe",
        "${env:ProgramFiles(x86)}\NSIS\Bin\makensis.exe",
        "$env:ProgramFiles\NSIS\makensis.exe",
        "$env:ProgramFiles\NSIS\Bin\makensis.exe"
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    throw "makensis.exe not found. Install NSIS 3.x or let GitHub Actions install it."
}

function Download-File {
    param(
        [string]$Uri,
        [string]$Destination,
        [string]$Sha256 = ''
    )

    if (-not (Test-Path $Destination)) {
        Invoke-WebRequest -Uri $Uri -OutFile $Destination
    }

    if ($Sha256.Length -gt 0) {
        $actual = (Get-FileHash -Algorithm SHA256 -LiteralPath $Destination).Hash.ToLowerInvariant()
        if ($actual -ne $Sha256.ToLowerInvariant()) {
            throw "SHA256 mismatch for $Destination. Expected $Sha256, got $actual."
        }
    }
}

$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$BuildDir = (Resolve-Path $BuildDir).Path
$OutputDirInfo = New-Item -ItemType Directory -Force -Path $OutputDir
$OutputDir = $OutputDirInfo.FullName
$DepsDirInfo = New-Item -ItemType Directory -Force -Path (Join-Path $OutputDir 'deps')
$DepsDir = $DepsDirInfo.FullName
$MakeNsis = Find-MakeNsis

$requiredBuildFiles = @(
    'AntivirusWinUi.exe',
    'AntivirusService.exe',
    'Microsoft.WindowsAppRuntime.Bootstrap.dll'
)

foreach ($file in $requiredBuildFiles) {
    $path = Join-Path $BuildDir "Release\$file"
    if (-not (Test-Path $path)) {
        throw "Required build output is missing: $path"
    }
}

Download-File `
    -Uri 'https://aka.ms/vs/17/release/vc_redist.x64.exe' `
    -Destination (Join-Path $DepsDir 'vc_redist.x64.exe')

Download-File `
    -Uri 'https://aka.ms/windowsappsdk/2.0/2.0.1/windowsappruntimeinstall-x64.exe' `
    -Destination (Join-Path $DepsDir 'windowsappruntimeinstall-x64.exe') `
    -Sha256 '8b3164dacac14650a45ce7b1990a8a43ca4427a5a4751abc5611978e75df2810'

$OutputExe = Join-Path $OutputDir 'AntivirusGuiSetup.exe'
$ScriptPath = Join-Path $ProjectRoot 'installer\AntivirusGui.nsi'

& $MakeNsis `
    '/V2' `
    "/DPROJECT_ROOT=$ProjectRoot" `
    "/DBUILD_DIR=$BuildDir" `
    "/DDEPS_DIR=$DepsDir" `
    "/DOUTPUT_EXE=$OutputExe" `
    $ScriptPath

if ($LASTEXITCODE -ne 0) {
    throw "makensis failed with exit code $LASTEXITCODE."
}

$readme = Join-Path $OutputDir 'README-INSTALLER.md'
@"
# Antivirus GUI installer

Run `AntivirusGuiSetup.exe` as Administrator.

The installer:

- installs Microsoft Visual C++ Runtime;
- installs Windows App Runtime 2.0;
- copies `AntivirusWinUi.exe`, `AntivirusService.exe`, docs, and demo scripts to `C:\Program Files\AntivirusGui`;
- registers `AntivirusGuiService` for automatic start;
- starts the service;
- creates Start Menu shortcuts;
- registers an uninstaller.

Demo credentials:

- login: `demo`
- password: `demo`
- activation code: `DEMO-1234`

Use installed docs `docs\final-audit.md` and `docs\defense.md` for the 2.4, 2.5, extra points, and 2.6 checklist.
"@ | Set-Content -Path $readme -Encoding UTF8

Write-Host "Installer: $OutputExe"
