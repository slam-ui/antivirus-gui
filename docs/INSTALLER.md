# Task 2.6: Installer Creation

## Requirements

| # | Requirement | Implementation |
|---|-------------|----------------|
| 1 | Include executable files, dynamic libraries, resources, and configuration files needed to run. | CPack ZIP installs `AntivirusGui.exe`, `AntivirusService.exe`, Qt runtime files deployed by `windeployqt`, installer scripts, docs, and `config.example.json`. |
| 2 | Install necessary dependencies. | MSVC runtime is included through `InstallRequiredSystemLibraries`; Qt runtime/plugins are copied through `windeployqt`. |
| 3 | Register Windows service for automatic startup. | `installer/install.ps1` runs `AntivirusService.exe --install`, which registers `AntivirusGuiService` as `SERVICE_AUTO_START`, then starts it. |
| 4 | Remove all application files on uninstall. | `installer/uninstall.ps1` removes the installation root after service cleanup. |
| 5 | Remove dependencies if not used elsewhere. | The package keeps app-local Qt/MSVC runtime copies under the installation root, so uninstall removes only this app's copies. |
| 6 | Stop and remove Windows service from SCM on uninstall. | `uninstall.ps1` stops `AntivirusGuiService` and runs `AntivirusService.exe --uninstall`, falling back to `sc.exe delete`. |
| 7 | Build installer in CI and upload artifacts. | GitHub Actions builds Release, runs `package`, and uploads `.zip` package artifacts. |

## Build Locally

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --build build --config Release --target package
```

## Manual Verification

1. Build `AntivirusInstaller-0.1.0-win64.zip`.
2. Extract the ZIP package.
3. Open elevated PowerShell.
4. Run `.\installer\install.ps1`.
5. Verify files exist under `Program Files\Antivirus GUI`.
6. Verify `AntivirusGuiService` is registered.
7. Verify the service is set to automatic startup.
8. Verify GUI/tray starts through the service in a user session.
9. Run `.\installer\uninstall.ps1`.
10. Verify the service is removed.
11. Verify installed files are removed.

## Security Notes

- Installation requires administrator rights because service registration requires SCM write access.
- Installer scripts perform transparent file copy and service registration only.
- The installer does not hide files, bypass UAC, or disable Windows protections.
