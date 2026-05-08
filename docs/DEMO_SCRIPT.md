# Demo Script

## 1. CI Artifacts

1. Open GitHub Actions `Windows`.
2. Open the latest successful run for the top stacked PR.
3. Download `antivirus-gui-windows-release`.
4. Verify it contains:
   - `AntivirusGui.exe`
   - `AntivirusService.exe`
   - `AntivirusInstaller-0.1.0-win64.zip`

## 2. Local Build

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --build build --config Release --target package
```

## 3. Install

1. Extract `AntivirusInstaller-0.1.0-win64.zip`.
2. Open PowerShell as Administrator.
3. Run:

```powershell
.\installer\install.ps1
```

4. Confirm files exist in `C:\Program Files\Antivirus GUI`.
5. Confirm service exists:

```powershell
sc.exe query AntivirusGuiService
```

## 4. Service and GUI

1. Start the service if it is not already running:

```powershell
sc.exe start AntivirusGuiService
```

2. Verify GUI is launched in the user session hidden with tray icon.
3. Left-click tray icon and show the main window.
4. Close the main window and verify the process remains in background.
5. Right-click tray icon and show the context menu.

## 5. Task 2.1 Checks

1. Run `AntivirusGui.exe --allow-standalone-debug --hidden`.
2. Verify no window appears initially.
3. Use tray icon to open the window.
4. Start a second instance and verify it exits without another tray icon.
5. Restart Explorer and verify tray icon restores.

## 6. Task 2.2 Checks

1. Verify production GUI requires service parent.
2. Use `Файл -> Выход` in GUI.
3. Confirm the secure credential prompt.
4. Verify service stop is requested over RPC.
5. Start service again and repeat via tray `Выход`.

## 7. Task 2.3 Checks

1. Open GUI.
2. In login dialog enter any login and password `fail`; verify error.
3. Enter any login and a non-empty password; verify account label updates.
4. In activation dialog enter invalid code; verify error.
5. Enter `DEMO-1234`; verify license expiry appears.
6. Use `Аккаунт -> Выйти из аккаунта`; verify account/license state clears.

## 8. Extra Checks

1. Attempt to terminate service process as a non-admin user.
2. Attempt to terminate GUI process as a non-admin user.
3. Explain that administrators can still take ownership/use debug privileges and the project does not implement malware-like self-defense.

## 9. Uninstall

1. Open elevated PowerShell.
2. Run:

```powershell
.\installer\uninstall.ps1
```

3. Verify service is gone:

```powershell
sc.exe query AntivirusGuiService
```

4. Verify `C:\Program Files\Antivirus GUI` is removed.
