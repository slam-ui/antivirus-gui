# Task 2.1: Basic Graphical Application Functions

## Requirements

| # | Requirement | Implementation |
|---|-------------|----------------|
| 1 | Add an icon to the taskbar notification area on startup. | `src/winui/main.cpp` creates a native Win32 tray icon with `Shell_NotifyIconW`. |
| 2 | Left-clicking the tray icon shows the main window. | The tray callback handles `WM_LBUTTONUP`, `WM_LBUTTONDBLCLK`, `NIN_SELECT`, and calls `showMainWindow()`. |
| 3 | Right-clicking the tray icon shows a context menu. | The tray callback handles `WM_RBUTTONUP` / `WM_CONTEXTMENU` and opens a Win32 popup menu. |
| 4 | Context menu contains `Открыть`, which shows the main window. | Tray command `kTrayOpenCommand` calls `showMainWindow()`. |
| 5 | Context menu contains `Выход`, which terminates the app. | Tray command `kTrayExitCommand` requests service stop through RPC and closes the GUI. |
| 6 | Tray icon is restored after taskbar recreation. | The WinUI window subclass listens for the registered `TaskbarCreated` message and re-adds the tray icon. |
| 7 | App supports startup with hidden main window. | Production `--service-child` and debug `--hidden` start with tray icon only. |
| 8 | Closing the main window keeps the app running in background. | The WinUI `Closed` handler marks the event handled and hides the AppWindow. |
| 9 | Main menu contains `Файл -> Выход`, which terminates the app. | The native Win32 menu command uses the same RPC stop-and-exit path as tray exit. |
| 10 | Single-instance mode prevents a second tray icon in the same user session. | `SingleInstanceGuard` uses `Local\\AntivirusGuiSingleton` before tray creation. |
| 11 | Build works through CMake/MSBuild on CI. | `.github/workflows/windows.yml` configures and builds the WinUI Release target. |
| 12 | Build artifacts are saved. | CI uploads WinUI/service binaries and the NSIS installer artifact. |

## Build

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Manual Verification

1. Run `AntivirusWinUi.exe --allow-standalone-debug --show` for local debug, or run the installed app through the service.
2. Verify the tray icon appears.
3. Close the main window with the window close button and verify the process stays alive.
4. Left-click the tray icon and verify the main window appears.
5. Right-click the tray icon and verify the context menu appears.
6. Click `Открыть` and verify the main window appears.
7. Click `Выход` and verify the process exits.
8. Run `AntivirusWinUi.exe --allow-standalone-debug --hidden` and verify the window does not appear while the tray icon exists.
9. Start a second installed `AntivirusWinUi.exe` instance and verify it opens the existing service-owned GUI without adding another tray icon.
10. Restart `Explorer.exe` and verify the tray icon is restored.

## Artifacts

GitHub Actions uploads Release executables in the `antivirus-gui-windows-release` artifact:

- `AntivirusWinUi.exe`
- `AntivirusService.exe`
- `AntivirusCtl.exe`

The `antivirus-gui-installer` artifact contains `AntivirusGuiSetup.exe` for Task 2.6.

## Limitations

Task 2.1 tray/window behavior is implemented in the current WinUI target. Service lifecycle and RPC stop behavior are covered by Task 2.2.
