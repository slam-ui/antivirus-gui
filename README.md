# Antivirus GUI

[![Windows](https://github.com/slam-ui/antivirus-gui/actions/workflows/windows.yml/badge.svg)](https://github.com/slam-ui/antivirus-gui/actions/workflows/windows.yml)

Educational Windows C++20 project for antivirus GUI coursework. The project is intentionally limited to benign UI, service, IPC, authentication/licensing, and installer exercises. It does not implement malware behavior, process hiding, system protection bypasses, or real antivirus scanning.

## Covered Assignments

- Task 2.1: Qt Widgets GUI, tray lifecycle, hidden mode, single-instance behavior.
- Task 2.2: Windows service, session launch, and local RPC interaction.
- Task 2.3: account login, activation, and license-gated features.
- Extra points: CMake build, secure stop confirmation, and documented DACL hardening.
- Task 2.6: Windows installer packaging.

## Architecture

- `AntivirusGui.exe`: Qt 6 Widgets GUI with tray icon, hidden mode, account/login UI, activation UI, and RPC client.
- `AntivirusService.exe`: native Windows service with SCM install/uninstall, WTS session launch, local RPC server, auth/license state, secure stop confirmation, and process DACL hardening.
- `rpc/AntivirusRpc.idl`: Windows RPC interface generated through MIDL and used over `ncalrpc`.
- `installer`: CPack ZIP installer package with elevated install/uninstall scripts.

## Build

Prerequisites:

- Windows with Visual Studio 2022 Build Tools or Visual Studio 2022.
- CMake 3.24 or newer.
- Qt 6 Widgets development package available to CMake.

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Run GUI

After building:

```powershell
.\build\Release\AntivirusGui.exe --allow-standalone-debug
```

Production GUI instances are launched by the service with `--hidden --service-child`.

## Run Service

After building:

```powershell
.\build\Release\AntivirusService.exe --console
```

Service install/uninstall requires an elevated shell:

```powershell
.\build\Release\AntivirusService.exe --install
sc.exe start AntivirusGuiService
.\build\Release\AntivirusService.exe --uninstall
```

## Install

Build the package:

```powershell
cmake --build build --config Release --target package
```

Extract `AntivirusInstaller-0.1.0-win64.zip`, open elevated PowerShell, and run:

```powershell
.\installer\install.ps1
```

Uninstall:

```powershell
.\installer\uninstall.ps1
```

## Demo

Use [docs/DEMO_SCRIPT.md](docs/DEMO_SCRIPT.md) for the teacher-facing walkthrough and [docs/SUBMISSION_CHECKLIST.md](docs/SUBMISSION_CHECKLIST.md) for requirement-by-requirement evidence.

## CI Artifacts

The Windows workflow configures CMake, builds Release targets, packages the installer ZIP, and uploads the executables plus installer artifact.
