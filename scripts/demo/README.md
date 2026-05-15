# Демо-скрипты

Для сценариев установки, перезапуска службы и проверки recovery запускайте PowerShell от имени администратора.

Рекомендуемый порядок:

1. `.\scripts\demo\build-release.ps1`
2. `.\scripts\demo\prepare-license.ps1`
3. `.\scripts\demo\install-service.ps1`
4. `.\scripts\demo\create-test-threats.ps1`
5. Открыть `build-local-extra-final\Release\AntivirusWinUi.exe`, войти как `demo` / `demo`, активировать кодом `DEMO-1234`, затем просканировать `C:\ProgramData\AntivirusGuiScanTest`.
6. Запустить мониторинг папки `C:\ProgramData\AntivirusGuiScanTest` из GUI, создать или изменить demo-файл и выполнить `.\scripts\demo\show-logs.ps1`.
7. Выполнить `.\scripts\demo\prepare-backup-recovery.ps1`.
8. При необходимости пересобрать или перезапустить службу, затем выполнить `.\scripts\demo\prepare-mock-update-recovery.ps1`.
9. Выполнить `.\scripts\demo\show-logs.ps1`.

Заметки:

- `prepare-backup-recovery.ps1` копирует `avdb.bin` в `avdb.bak`, повреждает основную базу, перезапускает службу и показывает строки лога восстановления.
- `prepare-mock-update-recovery.ps1` копирует валидную базу в `C:\ProgramData\AntivirusGuiMockServer\avdb.bin`, повреждает подпись manifest основной базы, перезапускает службу и показывает forced update через mock server.
- Demo-сигнатуры безопасны: `MZAVGUI-PE-TEST` и `Invoke-AvGuiTest`.
