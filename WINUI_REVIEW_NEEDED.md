## R-001: Windows App SDK is missing for real WinUI 3 migration

- **Coordinates:** CMakeLists.txt:1
- **Hypothesis:** The repository can add a parallel GUI target, but this machine does not currently have the Windows App SDK / WinUI 3 development files needed for a real C++/WinRT WinUI target.
- **Why not fixed:** `cppwinrt.exe` exists in the Windows SDK, but `Microsoft.UI.Xaml.winmd`, Windows App SDK headers/libraries, NuGet package cache, and `winapp` CLI were not found. Creating a Win32-only executable named `AntivirusWinUi` would not satisfy the WinUI requirement and would make the project documentation misleading.
- **Suggested fix:** Install or restore Windows App SDK tooling, then add `AntivirusWinUi` as a parallel target. The expected setup is one of: Visual Studio Windows app development components with `Microsoft.WindowsAppSDK` NuGet support, or `winapp init` / `winapp restore` creating `.winapp` headers and package metadata for the CMake target. After that, continue prompts 12-18 on `feature/winui-gui`.
- **Confidence:** Confirmed

Observed checks:

- `Get-ChildItem ... -Filter cppwinrt.exe` found Windows SDK C++/WinRT tool.
- `Get-ChildItem ... -Filter Microsoft.UI.Xaml.winmd` found no WinUI metadata under Windows Kits, Microsoft SDKs, or the user NuGet cache.
- `Get-Command winapp` returned no installed CLI.
- `dotnet new list winui` returned no WinUI templates.
