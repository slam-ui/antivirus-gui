# Документация для защиты

## 1. Назначение проекта

`antivirus-gui` — учебный C++20/Windows проект, который демонстрирует GUI, Windows Service, Windows RPC, авторизацию, лицензирование, загрузку антивирусной базы, сканирование файлов/папок и восстановление базы. Проект не является настоящим антивирусом и использует безопасные demo-сигнатуры.

## 2. Общая архитектура

- GUI: основной WinUI target `AntivirusWinUi`, файлы `src/winui/*`. Старый Qt Widgets target `AntivirusGui` оставлен как legacy и отключён по умолчанию.
- Windows Service: target `AntivirusService`, файлы `src/service/*`.
- Windows RPC: контракт `rpc/AntivirusRpc.idl`, generated client/server stubs создаются через MIDL.
- CMake: основной файл сборки `CMakeLists.txt`, Release build проверяется через `build-local-winui-clean` или demo build `build-local-extra-final`.
- Windows API: service control manager, RPC over `ncalrpc`, session launch, process hardening, `ProgramData`, fixed drive enumeration.

## 3. Авторизация и лицензия

Авторизация находится в `AuthManager`: учебный login/password — `demo` / `demo`. Лицензия находится в `LicenseManager`: код активации — `DEMO-1234`. Состояние лицензии дополнительно читается из mock server файла `C:\ProgramData\AntivirusGuiMockServer\license-status.txt`.

Функции сканирования блокируются через `FeatureGate`, если пользователь не вошёл или лицензия не активна.

## 4. Антивирусная база в памяти

`AvDatabase` хранит записи в `std::map<std::uint64_t, std::vector<AvRecord>>`. Ключ — первые 8 байтов сигнатуры, превращённые в `uint64_t`. Значение — список `AvRecord` с одинаковым префиксом.

## 5. Почему std::map подходит

`std::map` в стандартных реализациях построен как сбалансированное дерево, обычно красно-чёрное дерево. Поэтому структура закрывает требование по красно-чёрному дереву и даёт логарифмический поиск по 8-байтовому префиксу.

## 6. Сканирование

- Файл: `FileScanner::scanFile` определяет тип объекта и передаёт поток в `ScanEngine`.
- Папка: `FileScanner::scanDirectory` рекурсивно обходит файлы с `skip_permission_denied`.
- Все несъёмные диски: service перечисляет `GetLogicalDrives` + `GetDriveTypeW(DRIVE_FIXED)` и сканирует каждый корень.

## 7. Проверка сигнатуры

`ScanEngine` проверяет:

- первые 8 байтов и lookup в `std::map`;
- `ObjectType` (`PE file`, `PowerShell script`);
- диапазон `OffsetBegin`/`OffsetEnd`;
- длину сигнатуры;
- hash полной кандидатной сигнатуры;
- `threatName` возвращается в результате обнаружения.

## 8. Ахо-Корасик

`AhoCorasickScanner` реализован отдельными файлами `AhoCorasickScanner.h/.cpp`. Он строит trie по байтам demo-сигнатур, добавляет failure-ссылки и ищет несколько сигнатур за один проход по потоку. Старый `std::map`-алгоритм сохранён как fallback и как часть требования 2.4.

Так как текущий бинарный формат хранит hash полной сигнатуры, а не raw bytes, Ахо-Корасик используется для известных учебных demo-сигнатур. Это не ломает формат `avdb.bin`.

## 9. База на диске

Основная база: `C:\ProgramData\AntivirusGui\bases\avdb.bin`.
Backup: `C:\ProgramData\AntivirusGui\bases\avdb.bak`.

Формат содержит:

- `magic` = `AVDB`;
- `version`;
- `releaseDate`;
- `recordCount`;
- `manifestSignature`;
- набор `AvRecord`;
- `AvRecordSignature` для каждой записи.

`AvDatabaseStorage` проверяет manifest signature и подпись каждой записи. Повреждённые записи пропускаются и запускают попытку repair через mock update server.

## 10. Backup/recovery

При ошибке primary базы порядок восстановления такой:

1. primary `avdb.bin`;
2. mock update server;
3. backup `avdb.bak`;
4. default demo database.

Перед scheduled update создаётся backup, затем база перечитывается.

## 11. Mock update server

Mock update server — локальная папка `C:\ProgramData\AntivirusGuiMockServer`. Если там есть корректный `avdb.bin`, service использует его для восстановления повреждённой primary базы. Это учебная имитация сервера обновлений без настоящего интернета.

Если повреждён `manifestSignature`, это отдельный reason `InvalidManifestSignature`, и recovery принудительно начинает с mock update server.

## 12. Directory monitoring

`DirectoryMonitor` работает в service, не в GUI. GUI только выбирает папку и вызывает RPC:

- `AvStartDirectoryMonitor`;
- `AvStopDirectoryMonitor`;
- `AvGetDirectoryMonitorStatus`.

Реализация использует polling worker thread. Новые или изменённые файлы сканируются текущей базой, результат пишется в `service.log`.

## 13. Secure Desktop confirmation

Остановка службы из GUI проходит через общий `common/secure_stop_confirmation.cpp`. Это учебная демонстрация подтверждения опасного действия перед RPC-вызовом `AvRequestServiceStop`. WinUI использует эту же Secure Desktop логику, чтобы не ломать старый Qt GUI.

## 14. Process hardening

`process_hardening.cpp` включает учебное hardening-поведение процесса: логирует применённые ограничения и демонстрирует Windows-specific защитные настройки без обхода системной безопасности.

## 15. Демонстрация

Основной порядок:

1. `.\scripts\demo\build-release.ps1`
2. `.\scripts\demo\prepare-license.ps1`
3. `.\scripts\demo\install-service.ps1`
4. `.\scripts\demo\create-test-threats.ps1`
5. Запустить `build-local-extra-final\Release\AntivirusWinUi.exe`.
6. Войти: `demo` / `demo`.
7. Активировать: `DEMO-1234`.
8. Сканировать `C:\ProgramData\AntivirusGuiScanTest`.
9. Проверить monitoring через GUI и `.\scripts\demo\show-logs.ps1`.
10. Показать recovery: `prepare-backup-recovery.ps1` и `prepare-mock-update-recovery.ps1`.

Для демонстрации установки можно вместо шага 3 скачать GitHub Actions artifact `antivirus-gui-installer` и запустить от администратора:

```powershell
.\AntivirusGuiSetup.exe
```

EXE-инсталлер ставит VC++ Runtime, Windows App Runtime 2.0, `AntivirusWinUi.exe`, `AntivirusService.exe`, документацию и demo scripts, регистрирует `AntivirusGuiService` и создаёт uninstall.

## 16. Частые вопросы

**Это настоящий антивирус?** Нет, это учебная демонстрация архитектуры и алгоритмов на безопасных строковых сигнатурах.

**Где красно-чёрное дерево?** В `std::map` внутри `AvDatabase`.

**Зачем Ахо-Корасик, если есть std::map?** `std::map` проверяет кандидатов по 8-байтовому префиксу, а Ахо-Корасик демонстрирует множественный поиск сигнатур за один проход.

**Почему mock server — папка?** Чтобы показать update/recovery без внешнего интернета и нестабильных сетевых зависимостей.

**Что происходит при битом manifest?** Ошибка классифицируется как `InvalidManifestSignature`, после чего service сначала пытается восстановиться из mock update server.
