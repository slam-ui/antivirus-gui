# Architecture

## Component Overview

```text
User session
  AntivirusWinUi.exe
    - WinUI main window
    - Service/RPC client
    - Auth, activation, scan, monitor, and service stop controls
  AntivirusGui.exe (legacy, optional)
    - Qt Widgets implementation kept for history

Session 0
  AntivirusService.exe
    - Windows Service entry point
    - Session manager
    - GUI process launcher
    - Local RPC server
    - Auth/license managers

Local IPC
  Windows RPC endpoint over ncalrpc
    - Local RPC transport on modern Windows uses LPC/ALPC internally

Packaging
  Installer
    - Copies binaries and runtime files
    - Registers/removes the Windows service
```

## Project Layout

- `src/common`: shared utilities for paths, logging, Secure Desktop confirmation, process hardening, and Windows error formatting.
- `src/winui`: primary WinUI GUI executable.
- `src/gui`: legacy Qt Widgets GUI executable, disabled by default.
- `src/service`: Windows service executable.
- `rpc`: RPC IDL and generated MIDL stubs.
- `installer`: installer packaging placeholder.
- `docs`: assignment documentation and manual verification notes.

## Safety Boundaries

This is an educational project. It does not implement hidden malicious activity, process hiding, UAC bypasses, anti-debugging, unauthorized persistence, or system protection disabling. Service/session behavior in later tasks is explicit and documented for coursework demonstration.

The GUI and service use normal Windows APIs and transparent logs. Security-related hardening is documented as best-effort DACL configuration, not malware-like self-defense.
