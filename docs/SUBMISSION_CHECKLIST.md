# Submission Checklist

## Task 2.1 GUI Basics

| Requirement | Implementation | File/Class | How to test | Status |
|-------------|----------------|------------|-------------|--------|
| Tray icon on startup | `QSystemTrayIcon` is shown by `TrayController`. | `src/gui/TrayController.*` | Start GUI and inspect notification area. | Done |
| Left click opens window | Trigger activation shows and raises `MainWindow`. | `TrayController::showMainWindow` | Left-click tray icon. | Done |
| Right click menu | Tray context menu with `Открыть` and `Выход`. | `TrayController` | Right-click tray icon. | Done |
| Hidden mode | `--hidden` skips initial window show. | `src/gui/main.cpp` | Run `AntivirusGui.exe --hidden`. | Done |
| Close hides window | `closeEvent` ignores close and hides. | `MainWindow::closeEvent` | Close window and verify process remains. | Done |
| File exit | `Файл -> Выход` goes through app lifecycle. | `MainWindow`, `AppLifecycle` | Click menu exit. | Done |
| Single instance | Local named mutex prevents second tray icon. | `SingleInstanceGuard` | Start second GUI in same session. | Done |
| Explorer restart restore | Native filter handles `TaskbarCreated`. | `TrayController` | Restart Explorer. | Done |

## Task 2.2 Windows Service and RPC

| Requirement | Implementation | File/Class | How to test | Status |
|-------------|----------------|------------|-------------|--------|
| Native Windows service | SCM entry point and service status handling. | `ServiceMain` | `AntivirusService.exe --install`, `sc start`. | Done |
| Launch GUI per user session | WTS enumeration and `CreateProcessAsUserW`. | `SessionManager`, `ProcessLauncher` | Start service as LocalSystem and inspect user session tray. | Done |
| Session change handling | Handles logon/connect notifications. | `ServiceMain::control` | Log on/connect new session. | Done |
| Local RPC server | `ncalrpc` endpoint `AntivirusGuiRpc`. | `RpcServer`, `rpc/AntivirusRpc.idl` | GUI exit calls service stop RPC. | Done |
| Stop closes GUI children | Service tracks process handles and closes owned children. | `SessionManager` | Stop service and inspect child processes. | Done |
| GUI service checks | SCM check/start/wait and parent process validation. | `ServiceClient`, `ParentProcessCheck` | Run GUI manually without debug override. | Done |

## Task 2.3 Accounts and Activation

| Requirement | Implementation | File/Class | How to test | Status |
|-------------|----------------|------------|-------------|--------|
| Auth state in service | In-memory auth manager. | `AuthManager` | Login through GUI. | Done |
| HTTPS backend boundary | URL validation in WinHTTP client wrapper. | `AuthHttpClient` | Inspect config/client behavior. | Done |
| No raw secrets to GUI | RPC DTOs expose only safe state. | `AntivirusRpc.idl`, `RpcServer` | Inspect DTO fields. | Done |
| Login form | Dialog calls `AvLogin`. | `AuthDialog`, `RpcClient` | Use password `fail`, then valid input. | Done |
| Activation form | Dialog calls `AvActivateProduct`. | `ActivationDialog` | Use invalid code, then `DEMO-1234`. | Done |
| License polling | 7 second timer updates account/license/feature labels. | `MainWindow` | Activate/logout and watch UI update. | Done |
| Logout clears state | RPC logout clears auth and license state. | `AuthManager`, `LicenseManager` | Use `Аккаунт -> Выйти из аккаунта`. | Done |

## Extra Points

| Requirement | Implementation | File/Class | How to test | Status |
|-------------|----------------|------------|-------------|--------|
| CMake build | All targets and MIDL-generated RPC sources build through CMake. | `CMakeLists.txt` | Run configure/build/package. | Done |
| Secure stop confirmation | Credential UI secure prompt validates Windows credentials. | `SecureStopConfirmation` | Exit from GUI/tray and confirm prompt. | Done |
| Service process DACL | Deny `PROCESS_TERMINATE` to Builtin Users. | `service/security_hardening`, `process_dacl` | Try non-admin termination. | Done |
| GUI process DACL | Same process hardening for GUI. | `gui/security_hardening` | Try non-admin termination. | Done |
| Admin limitation | Documented as Windows-model limitation. | `docs/EXTRA_POINTS.md` | Review docs. | Done |

## Task 2.6 Installer

| Requirement | Implementation | File/Class | How to test | Status |
|-------------|----------------|------------|-------------|--------|
| Include binaries/resources | CPack ZIP includes exe, Qt DLLs/plugins, docs, scripts. | `CMakeLists.txt`, `installer/*` | Inspect ZIP contents. | Done |
| Runtime dependencies | `windeployqt` and `InstallRequiredSystemLibraries`. | `CMakeLists.txt` | Build package and inspect `bin`. | Done |
| Service registration | Elevated install script runs `--install`. | `installer/install.ps1` | Run install script as admin. | Done |
| Service removal | Uninstall script stops/deletes service. | `installer/uninstall.ps1` | Run uninstall script as admin. | Done |
| CI artifact | Workflow builds package and uploads ZIP. | `.github/workflows/windows.yml` | Check Actions artifacts. | Done |

## Open Limitations

| Limitation | Reason | Impact |
|------------|--------|--------|
| Task 2.4 and 2.5 are not covered as separate PRs. | Their conditions were not provided in the prompt file. | Need additional assignment screenshots if they exist. |
| Installer is CPack ZIP, not MSI. | ZIP is more reliable for service script demonstration without fragile MSI custom actions. | Teacher should run `installer/install.ps1` from extracted package. |
| Mock auth backend is deterministic. | No real previous web-service URL was supplied. | Demonstrates GUI/service flow without external dependency. |
