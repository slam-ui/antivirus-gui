# Extra Points: CMake Secure Stop Confirmation and DACL Hardening

## Criteria

| Criterion | Status | Implementation |
|-----------|--------|----------------|
| CMake build for the whole app | Implemented | GUI, service, common library, and generated RPC source files are built from `CMakeLists.txt`. |
| WinUI point | Not implemented | The project intentionally uses Qt 6 Widgets, which already satisfies the allowed GUI framework requirement. |
| Secure Desktop stop confirmation | Implemented with standard Windows credential UI | RPC service stop calls `CredUIPromptForWindowsCredentialsW` with `CREDUIWIN_SECURE_PROMPT`, unpacks credentials, validates them with `LogonUserW`, zeroes password buffers, and proceeds only after success. |
| DACL for service process | Implemented best-effort | `applyServiceSecurityHardening()` applies a process DACL denying `PROCESS_TERMINATE` to Builtin Users while allowing LocalSystem and Administrators. |
| DACL for GUI process | Implemented best-effort | `applyGuiSecurityHardening()` applies the same DACL policy to the GUI process. |
| Prevent administrator termination | Documented limitation | Windows administrators can take ownership or use debug privileges; this project does not implement malware-like self-defense. |

## Files

- `src/common/security/process_dacl.*`
- `src/service/security_hardening.*`
- `src/gui/security_hardening.*`
- `src/service/SecureStopConfirmation.*`

## Manual Verification

1. Build Release binaries.
2. Install the service as administrator.
3. Start the service.
4. Log in as a non-admin user.
5. Attempt to terminate `AntivirusService.exe` from Task Manager without elevation and verify access is denied where Windows policy enforces the DACL.
6. Attempt to terminate `AntivirusGui.exe` from Task Manager without elevation and verify access is denied where Windows policy enforces the DACL.
7. Use GUI `Файл -> Выход` or tray `Выход`.
8. Verify the Windows secure credential prompt appears.
9. Enter valid Windows credentials and verify service stop proceeds.
10. Cancel or enter invalid credentials and verify service stop is denied.

## Security Notes

- Credentials are not stored.
- Password buffers are zeroed with `SecureZeroMemory`.
- The project does not hide processes, bypass UAC, disable system protections, or fight administrators.
- DACL hardening is best-effort and limited by the Windows security model.
- If Secure Desktop credential UI is unavailable in the current session, service stop is denied and a warning is logged.

## Build

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
