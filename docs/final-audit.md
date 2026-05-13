# Финальный аудит перед сдачей

Дата проверки: 2026-05-13.

## Таблица готовности

| # | Пункт | Статус | Файл реализации | Как проверить | Что сказать преподавателю |
|---|---|---|---|---|---|
| 1 | C++20 | Готово | `CMakeLists.txt` | `set(CMAKE_CXX_STANDARD 20)` и clean build | Проект собирается как C++20. |
| 2 | CMake | Готово | `CMakeLists.txt` | `cmake -S . -B build-local-winui-clean` | Сборка полностью описана в CMake. |
| 3 | WinUI GUI | Готово | `src/winui/main.cpp` | Запустить `AntivirusWinUi.exe` | Основной GUI перенесён на WinUI. |
| 4 | Windows Service | Готово | `src/service/ServiceMain.cpp` | `AntivirusService.exe --install` от администратора | Есть Windows Service с SCM install/uninstall. |
| 5 | Windows RPC | Готово | `rpc/AntivirusRpc.idl`, `src/service/RpcServer.cpp` | Сборка MIDL stubs, вызовы из GUI | GUI и service общаются через локальный RPC `ncalrpc`. |
| 6 | Авторизация | Готово | `src/service/AuthManager.cpp`, `src/winui/main.cpp` | Login `demo` / `demo` | Авторизация проходит через service RPC. |
| 7 | Лицензия | Готово | `src/service/LicenseManager.cpp`, `src/winui/main.cpp` | Activate `DEMO-1234` | Лицензия управляет доступом к функциям. |
| 8 | Feature blocking | Готово | `src/service/FeatureGate.cpp`, `src/winui/main.cpp` | Запустить GUI без входа/активации | Сканирование и мониторинг disabled при неактивной лицензии. |
| 9 | База в памяти | Готово | `src/service/scan/AvDatabase.*` | `AntivirusScanTests` | База загружается в память service. |
| 10 | `std::map` / красно-чёрное дерево | Готово | `src/service/scan/AvDatabase.h` | Проверить `std::map<std::uint64_t, ...>` | `std::map` в стандартных реализациях обычно RB-tree. |
| 11 | `AvRecord` поля | Готово | `src/service/scan/AvDatabase.h` | Просмотреть struct `AvRecord` | Запись содержит object type, offset range, hash/signature и threat name. |
| 12 | PE detection | Готово | `src/service/scan/FileScanner.cpp` | Скан demo PE сигнатуры | Тип PE определяется перед scan engine. |
| 13 | PowerShell detection | Готово | `src/service/scan/FileScanner.cpp` | Скан `.ps1` demo сигнатуры | PowerShell scripts распознаются как отдельный object type. |
| 14 | `ScanEngine` со `std::istream` | Готово | `src/service/scan/ScanEngine.*` | `AntivirusScanTests` | Engine принимает поток, не зависит от GUI. |
| 15 | Первые 8 байтов | Готово | `src/service/scan/ScanEngine.cpp` | Тесты prefix lookup | Кандидаты ищутся по 8-байтовому префиксу. |
| 16 | `ObjectType` | Готово | `src/service/scan/ScanEngine.cpp` | Тест wrong object type | Сигнатура проверяется только для нужного типа объекта. |
| 17 | `OffsetBegin/OffsetEnd` | Готово | `src/service/scan/ScanEngine.cpp` | Тест offset range | Engine проверяет допустимый диапазон смещений. |
| 18 | Hash/signature | Готово | `src/service/scan/ScanEngine.cpp` | Тест hash match/mismatch | Проверяется полная сигнатура и hash. |
| 19 | Сканирование файла | Готово | `src/service/ServiceMain.cpp`, `src/winui/main.cpp` | Кнопка `Сканировать файл` | GUI вызывает `AvScanFile`. |
| 20 | Сканирование папки | Готово | `src/service/ServiceMain.cpp`, `src/winui/main.cpp` | Кнопка `Сканировать папку` | GUI показывает count файлов, угрозы и detection list. |
| 21 | Сканирование fixed drives | Готово | `src/service/ServiceMain.cpp`, `src/winui/main.cpp` | Кнопка fixed drives | Service перечисляет `DRIVE_FIXED`. |
| 22 | Ахо-Корасик | Готово | `src/service/scan/AhoCorasickScanner.*` | `AntivirusScanTests` | Есть быстрый multi-pattern scan для demo signatures. |
| 23 | Бинарная база | Готово | `src/service/scan/AvDatabaseStorage.*` | Проверить `avdb.bin` | База хранится в binary format. |
| 24 | Manifest signature | Готово | `src/service/scan/AvDatabaseStorage.cpp` | `prepare-mock-update-recovery.ps1` | Manifest signature валидируется отдельно. |
| 25 | Record signature | Готово | `src/service/scan/AvDatabaseStorage.cpp` | Corrupt record scenario | Каждая запись имеет подпись и проверяется. |
| 26 | Повреждённые записи | Готово | `src/service/scan/AvDatabaseStorage.cpp` | `AntivirusScanTests` | Битые записи пропускаются или запускают repair path. |
| 27 | Backup `avdb.bak` | Готово | `src/service/scan/AvDatabaseStorage.cpp` | `prepare-backup-recovery.ps1` | Перед update есть backup базы. |
| 28 | Recovery из backup | Готово | `src/service/scan/AvDatabaseStorage.cpp` | `prepare-backup-recovery.ps1` | При ошибке primary service пробует backup. |
| 29 | Default database fallback | Готово | `src/service/ServiceMain.cpp` | Удалить базы и запустить service | Если recovery не сработал, загружается demo база. |
| 30 | Mock update server | Готово | `src/service/scan/AvUpdateClient.*` | `prepare-mock-update-recovery.ps1` | Mock server реализован локальной папкой без интернета. |
| 31 | Forced update при corrupted manifest | Готово | `src/service/scan/AvDatabaseStorage.cpp` | Corrupt manifest script | Invalid manifest сначала пытается восстановиться из mock update server. |
| 32 | Periodic update scheduler | Готово | `src/service/scan/AvUpdateScheduler.*` | Запуск service и `service.log` | Scheduler периодически проверяет mock update source. |
| 33 | Directory monitoring | Готово | `src/service/scan/DirectoryMonitor.*`, `src/winui/main.cpp` | WinUI start/stop monitoring | Service мониторит папку и сканирует изменённые файлы. |
| 34 | Secure Desktop confirmation | Готово | `src/common/secure_stop_confirmation.cpp` | WinUI `Остановить службу` | Остановка требует Secure Desktop prompt. |
| 35 | Process hardening | Готово | `src/common/process_hardening.cpp` | Логи service launch | Реализовано учебное DACL hardening без обхода безопасности Windows. |
| 36 | Installer | Готово | `installer/install.ps1`, `installer/uninstall.ps1`, `CMakeLists.txt` | `installer/install.ps1 -StartService` от администратора | Есть PowerShell installer и CMake install для WinUI/service/bootstrap. |
| 37 | Demo scripts | Готово | `scripts/demo/*` | Запустить порядок из `scripts/demo/README.md` | Есть scripts для build, service, license, threats, recovery, logs. |
| 38 | Документация защиты | Готово | `docs/defense.md` | Открыть документ | Есть структура, речь, demo order и пояснения. |
| 39 | PR descriptions | Готово | `docs/pr-descriptions.md` | Открыть документ | Есть summary для основных веток/проходов. |
| 40 | Mojibake отсутствует | Готово | `src`, `docs`, `scripts` | `rg -n "�|Ð|Ñ|Рџ|Рў|Рњ|Рђ" ...` | Явных следов битой кодировки не найдено, кроме исторической команды проверки. |
| 41 | Критические warnings отсутствуют | Готово | build output | Clean Release build | Проект собирается без критических compiler warnings; Visual Studio generator игнорирует `CMAKE_BUILD_TYPE`, это не ошибка. |
| 42 | Чистый git status | Готово | Git worktree | `git status --porcelain` после финального коммита | Финальное состояние должно быть clean. |

## Ограничения проверки

- SCM install/start требует elevated PowerShell. В обычной оболочке проверены build, tests, WinUI smoke launch и `cmake --install`; фактический `Start-Service` нужно запускать от администратора.
- Legacy Qt target сохранён, но не является основным. Основная сборка не требует `CMAKE_PREFIX_PATH` к Qt.

## Финальные команды демонстрации

```powershell
npx --yes @microsoft/winappcli restore
cmake -S . -B build-local-winui-clean -DCMAKE_BUILD_TYPE=Release
cmake --build build-local-winui-clean --config Release
ctest --test-dir build-local-winui-clean -C Release --output-on-failure

.\scripts\demo\build-release.ps1
.\scripts\demo\prepare-license.ps1
.\scripts\demo\install-service.ps1
.\scripts\demo\create-test-threats.ps1
.\build-local-extra-final\Release\AntivirusWinUi.exe
```

Команды для installer demo из PowerShell от администратора:

```powershell
.\installer\install.ps1 -StartService
.\installer\uninstall.ps1
```

## Краткая речь на 3-5 минут

Это учебный проект на C++20 под Windows. Он показывает связку WinUI GUI, Windows Service и локального Windows RPC. GUI не сканирует файлы сам, а вызывает service через IDL-контракт. Это важно, потому что бизнес-логика находится в service: авторизация, лицензия, база, scan engine, update/recovery и monitoring.

Авторизация учебная: логин `demo`, пароль `demo`. После входа пользователь активирует лицензию кодом `DEMO-1234`. Пока лицензия не активна, `FeatureGate` блокирует сканирование и мониторинг; WinUI визуально отключает эти кнопки.

Антивирусная часть безопасная и демонстрационная. База хранит `AvRecord` в `std::map`, где ключ — первые 8 байтов сигнатуры. `ScanEngine` принимает `std::istream`, проверяет object type, offset range, длину, hash и threat name. Для ускоренного множественного поиска добавлен Ахо-Корасик, а старый map-based алгоритм сохранён как fallback и как выполнение требования про красно-чёрное дерево.

База хранится на диске как `avdb.bin`, рядом есть `avdb.bak`. При повреждении primary базы service пробует mock update server, backup и default demo database. Повреждённый manifest обрабатывается отдельно и принудительно ведёт к mock-update recovery. Directory monitoring работает в service: GUI только выбирает папку и показывает статус.

В финальной версии основной GUI — `AntivirusWinUi.exe`. Qt target оставлен как legacy, но отключён по умолчанию. Сборка `build-local-winui-clean` проходит без Qt `CMAKE_PREFIX_PATH`. Для установки есть PowerShell installer, который копирует WinUI GUI, service и bootstrap DLL в Program Files и регистрирует службу.

## Вероятные вопросы

**Это настоящий антивирус?** Нет. Это безопасная учебная демонстрация архитектуры, IPC, базы, алгоритмов поиска и recovery.

**Где красно-чёрное дерево?** В `std::map` внутри `AvDatabase`. В стандартных реализациях `std::map` обычно реализован как сбалансированное красно-чёрное дерево.

**Почему одновременно `std::map` и Ахо-Корасик?** `std::map` закрывает требование по 8-байтовому префиксу и RB-tree. Ахо-Корасик добавлен как быстрый multi-pattern поиск demo-сигнатур за один проход.

**Почему mock update server — папка?** Чтобы показать update/recovery без внешних сервисов, интернета и нестабильных тестов.

**Что будет без лицензии?** Service вернёт blocked feature state, а WinUI отключит scan/monitoring buttons и покажет причину.

**Как останавливается служба?** WinUI сначала вызывает Secure Desktop confirmation, затем отправляет `AvRequestServiceStop` через RPC.

**Почему Qt ещё есть в репозитории?** Он оставлен как legacy для истории миграции. Основная сборка WinUI не требует Qt и не ищет Qt package.
