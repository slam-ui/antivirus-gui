# Testing

## Common Build Check

Run after each task:

```powershell
npx --yes @microsoft/winappcli restore
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Task 2.1 Manual Checks

1. Start `AntivirusWinUi.exe`.
2. Verify the WinUI window opens with service, account, license, database, scan, monitoring, and result sections.
3. Start the service and verify status polling changes from RPC unavailable to `Служба работает`.
4. Verify blocked scan/monitoring buttons are disabled until login and license activation.

## Task 2.2 Manual Checks

1. Install the Windows service as administrator.
2. Start the service.
3. Verify the service launches `AntivirusWinUi.exe` in user sessions.
4. Verify new user logons trigger GUI launch.
5. Use the WinUI `Остановить службу` button to request service stop through local RPC after Secure Desktop confirmation.
6. Verify service stop closes owned GUI child processes.

## Task 2.3 Manual Checks

1. Start service and GUI in mock authentication mode.
2. Verify unauthenticated state shows login.
3. Verify failed login shows an error.
4. Verify successful login shows the user.
5. Verify missing license shows activation.
6. Verify failed activation shows an error.
7. Verify successful activation unlocks features and shows expiration.
8. Verify logout clears authentication and license state.

## Extra Points Manual Checks

1. Verify all targets build through CMake.
2. Verify service stop asks for secure confirmation where supported.
3. Verify process DACL hardening blocks non-admin termination attempts where Windows security allows it.
4. Verify limitations around administrators are documented honestly.

## Task 2.6 Manual Checks

1. Build Release with `scripts/demo/build-release.ps1`.
2. Build the installer with `installer/build-installer.ps1 -BuildDir build-local-extra-final -OutputDir out/installer`.
3. Run PowerShell as Administrator.
4. Install with `out/installer/AntivirusGuiSetup.exe`.
5. Verify files are installed under `C:\Program Files\AntivirusGui`.
6. Verify the Windows service is registered for automatic startup.
7. Verify WinUI GUI and service launch.
8. Uninstall through Windows Apps settings or `C:\Program Files\AntivirusGui\Uninstall.exe`.
9. Verify service registration and installed files are removed.

GitHub Actions also publishes `AntivirusGuiSetup.exe` in the `antivirus-gui-installer` artifact.
