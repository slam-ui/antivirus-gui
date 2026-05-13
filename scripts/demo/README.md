# Demo scripts

Run PowerShell as Administrator for service install/restart scripts.

Recommended order:

1. `.\scripts\demo\build-release.ps1`
2. `.\scripts\demo\prepare-license.ps1`
3. `.\scripts\demo\install-service.ps1`
4. `.\scripts\demo\create-test-threats.ps1`
5. Open `build-local-extra-final\Release\AntivirusGui.exe`, login as `demo` / `demo`, activate with `DEMO-1234`, and scan `C:\ProgramData\AntivirusGuiScanTest`.
6. Use GUI monitoring controls on `C:\ProgramData\AntivirusGuiScanTest`, then create or edit a demo file and run `.\scripts\demo\show-logs.ps1`.
7. `.\scripts\demo\prepare-backup-recovery.ps1`
8. Rebuild or restart service if needed, then run `.\scripts\demo\prepare-mock-update-recovery.ps1`
9. `.\scripts\demo\show-logs.ps1`

Notes:

- `prepare-backup-recovery.ps1` copies `avdb.bin` to `avdb.bak`, corrupts the primary database, restarts the service, and shows recovery log lines.
- `prepare-mock-update-recovery.ps1` copies a valid database to `C:\ProgramData\AntivirusGuiMockServer\avdb.bin`, corrupts the primary manifest signature, restarts the service, and shows forced mock-update recovery logs.
- The demo signatures are benign strings: `MZAVGUI-PE-TEST` and `Invoke-AvGuiTest`.
