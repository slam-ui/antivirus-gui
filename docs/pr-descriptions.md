# PR descriptions

## task-2.4-av-engine

### Summary

Implemented the in-memory antivirus database and scan engine required for task 2.4.

### Details

- Added `AvRecord` with signature prefix, signature hash, offset range, object type, record signature, and threat name.
- Stored records in `std::map<std::uint64_t, std::vector<AvRecord>>` by the first 8 signature bytes.
- Added `ScanEngine` that accepts `std::istream` and validates prefix, object type, offset range, signature length, and hash.
- Added file and directory scanning through the service.
- Exposed database info, file scan, directory scan, and fixed-drive scan through Windows RPC.
- Updated Qt GUI to display database state and scan results.

### Verification

- `cmake --build build-local-extra-final --config Release`
- `ctest --test-dir build-local-extra-final -C Release --output-on-failure`

## task-2.5-av-updates

### Summary

Implemented disk database storage, validation, backup/recovery, and scheduled update support for task 2.5.

### Details

- Added binary `avdb.bin` format in `C:\ProgramData\AntivirusGui\bases`.
- Added `avdb.bak` backup support.
- Validated magic, version, release date, record count, manifest signature, and record signatures.
- Skipped corrupted records and classified load errors.
- Added recovery order: primary, mock update server, backup, default database.
- Reloaded database after activation and scheduled update.
- Added forced update path for corrupted `manifestSignature`.

### Verification

- `cmake --build build-local-extra-final --config Release`
- `ctest --test-dir build-local-extra-final -C Release --output-on-failure`

## extra-2.1-2.5-final

### Summary

Finalized the extra 2.1-2.5 branch with stabilization, Aho-Corasick demo scanning, directory monitoring, mock update recovery, demo scripts, and defense documentation.

### Details

- Fixed Russian mojibake strings and localized fixed-drive scan UI.
- Removed relevant MSVC warnings from project code and generated RPC path handling.
- Added Aho-Corasick scanner while preserving the `std::map` fallback.
- Added service-side directory monitoring with start/stop/status RPC and GUI controls.
- Added mock update server recovery from `C:\ProgramData\AntivirusGuiMockServer\avdb.bin`.
- Added PowerShell demo scripts under `scripts/demo`.
- Added defense documentation under `docs/defense.md`.

### Verification

- `cmake -S . -B build-local-extra-final -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Users/13372/QtCodex/6.7.3/msvc2019_64"`
- `cmake --build build-local-extra-final --config Release`
- `ctest --test-dir build-local-extra-final -C Release --output-on-failure`

## feature/winui-gui

### Summary

Migrated the primary GUI from Qt Widgets to WinUI while keeping the Windows Service and RPC contract intact.

### Details

- Restored Windows App SDK packages through `winapp.yaml`.
- Added `AntivirusWinUi` as the primary GUI target.
- Added a non-Qt RPC client for WinUI over the existing generated MIDL client stubs.
- Ported status polling, login, logout, license activation, scan file/folder/fixed drives, directory monitoring, and secure service stop controls.
- Switched the service session launcher to `AntivirusWinUi.exe`.
- Disabled the legacy Qt target by default; it remains available with `-DANTIVIRUS_BUILD_GUI=ON`.
- Updated demo scripts, CI, README, architecture notes, and defense notes for the WinUI build.

### Verification

- `npx --yes @microsoft/winappcli restore`
- `cmake -S . -B build-local-winui-clean -DCMAKE_BUILD_TYPE=Release`
- `cmake --build build-local-winui-clean --config Release`
- `ctest --test-dir build-local-winui-clean -C Release --output-on-failure`
