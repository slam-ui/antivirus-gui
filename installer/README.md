# Installer

This project ships a CPack ZIP installer package for Windows coursework demonstration.

## Build

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --build build --config Release --target package
```

The generated package is named like:

```text
AntivirusInstaller-0.1.0-win64.zip
```

## Install

Extract the ZIP package, open an elevated PowerShell window, and run:

```powershell
.\installer\install.ps1
```

The script copies files to `Program Files\Antivirus GUI`, registers `AntivirusGuiService`, and starts it.

## Uninstall

Open an elevated PowerShell window from the extracted package or installed directory and run:

```powershell
.\installer\uninstall.ps1
```

The script stops and removes the service, then removes installed files.
