# Antivirus GUI

Educational Windows C++20 project for antivirus GUI coursework. The project is intentionally limited to benign UI, service, IPC, authentication/licensing, and installer exercises. It does not implement malware behavior, process hiding, system protection bypasses, or real antivirus scanning.

## Covered Assignments

- Task 2.1: GUI lifecycle foundation; the primary GUI is now WinUI.
- Task 2.2: Windows service, session launch, and local RPC interaction.
- Task 2.3: account login, activation, and license-gated features.
- Task 2.4: antivirus database loading, file/folder/fixed-drive scanning, monitoring, and Aho-Corasick extra point.
- Task 2.5: on-disk antivirus database storage, update/recovery, backup, corrupted records, and forced update for a bad manifest.
- Extra points: CMake build, secure stop confirmation, and documented DACL hardening.
- Task 2.6: Windows installer packaging.

The project provides a C++20/CMake Windows Service, local RPC, WinUI GUI, database scanning, demo update/recovery scripts, and coursework documentation.

## Build

Prerequisites:

- Windows with Visual Studio 2022 Build Tools or Visual Studio 2022.
- CMake 3.24 or newer.
- Node.js/npm for `npx @microsoft/winappcli restore`.
- Windows App SDK packages restored by `winappcli` from `winapp.yaml`.

```powershell
npx --yes @microsoft/winappcli restore
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The legacy Qt Widgets target is disabled by default. Build it only when needed with `-DANTIVIRUS_BUILD_GUI=ON` and a Qt 6 `CMAKE_PREFIX_PATH`.

## Run GUI

After building:

```powershell
.\build\Release\AntivirusWinUi.exe --allow-standalone-debug --show
```

The WinUI GUI talks to the service through Windows RPC. In the installed flow, run `AntivirusWinUi.exe` normally: it starts the service if needed, asks the service-owned hidden GUI window to show, and exits without creating a second tray icon.

## Run Service

After building:

```powershell
.\build\Release\AntivirusService.exe --console
```

For the real Windows Service install/start flow, run PowerShell as Administrator and use `scripts/demo/install-service.ps1`.

## EXE Installer

GitHub Actions builds a real administrator installer artifact:

- artifact name: `antivirus-gui-installer`
- file: `AntivirusGuiSetup.exe`

The installer includes the app, service, RPC stop helper, demo docs/scripts, Microsoft Visual C++ Runtime installer, and Windows App Runtime 2.0 installer. During upgrades it requests service stop through RPC before replacing files.

Build it locally after a Release build:

```powershell
.\installer\build-installer.ps1 -BuildDir build-local-winui-ui -OutputDir out\installer
```

Run the generated installer as Administrator:

```powershell
.\out\installer\AntivirusGuiSetup.exe
```

## CI Artifacts

The Windows workflow restores WinApp SDK packages, configures CMake, builds the Release targets, and uploads the WinUI GUI, service, tests, and bootstrap DLL as artifacts.
