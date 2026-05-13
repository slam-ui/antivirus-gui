# Pre-WinUI checklist

Ветка: `extra-2.1-2.5-final`

| Пункт | Команда проверки | Ожидаемый результат | Фактический результат |
| --- | --- | --- | --- |
| Чистый git status | `git status --porcelain` | Нет вывода | Пройдено перед проверкой |
| Сборка проекта | `cmake --build build-local-extra-final --config Release` | `AntivirusGui.exe`, `AntivirusService.exe`, tests built | Пройдено |
| Unit/regression tests | `ctest --test-dir build-local-extra-final -C Release --output-on-failure` | 100% tests passed | Пройдено |
| GUI открывается | `AntivirusGui.exe --allow-standalone-debug --show` | Процесс GUI запускается | Пройдено: GUI держался 3 секунды, затем остановлен |
| Service runtime | `AntivirusService.exe --console` | Service runtime стартует | Пройдено: console mode держался 4 секунды, затем остановлен console control event |
| Service install/start | `scripts/demo/install-service.ps1` | Служба установлена и запущена | Не выполнялось: текущая оболочка не elevated/admin |
| RPC | Service console + GUI/RPC client | RPC endpoint доступен | Частично: service runtime стартует; полный RPC smoke вручную через GUI требует service/admin demo |
| Авторизация | GUI login `demo` / `demo` | Вход успешен | Не выполнялось в автоматической проверке; логика покрыта demo flow |
| Активация лицензии | GUI activation `DEMO-1234` | Лицензия активна | Не выполнялось в автоматической проверке; логика покрыта demo flow |
| База загружается | Service startup / CTest storage scenario | База загружена или восстановлена | Пройдено через service startup и CTest |
| Сканирование файла | `AntivirusScanTests` | PE/PowerShell demo signatures detect | Пройдено |
| Сканирование папки | GUI или service RPC | Папка сканируется | Не выполнялось вручную; код path доступен через demo scripts |
| Fixed drives scan | GUI или service RPC | Fixed drives сканируются | Не выполнялось вручную в этой среде |
| Directory monitoring | GUI monitor controls + `show-logs.ps1` | Новые/изменённые файлы сканируются | Не выполнялось вручную; service-side код собирается |
| Backup/recovery | `AntivirusScanTests` forced recovery scenario | Recovery проходит | Пройдено для mock update/manifest path |
| Mock update recovery | `AntivirusScanTests` | Corrupted manifest repaired from mock server | Пройдено |
| Mojibake | `rg -n "Рђ|Рџ|Р¤|Рћ|РЎ|СЃР|СЊ|С‹" src rpc docs scripts CMakeLists.txt README.md` | Нет вывода | Пройдено |
| Критические warnings | Release build output | Нет критических warnings | Пройдено: текущая сборка без критических предупреждений |

Для полной ручной демонстрации с установкой службы запустить PowerShell от администратора и пройти порядок из `scripts/demo/README.md`.
