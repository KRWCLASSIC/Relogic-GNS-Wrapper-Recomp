# Relogic-GNS-Wrapper-Recomp

x86 and x64 rebuild of Terraria 1.4.6.0's `GNS_Wrapper.dll` using Valve's [GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets).

Terraria on Windows only ships a 32-bit `GNS_Wrapper.dll` and `steamnetworkingsockets.dll`, so 64-bit loaders/runtimes can't load it. This repo rebuilds the wrapper for both x86 and x64 with matching exports.

## Build (Windows + vcpkg)

```powershell
# x64
cmake -B build-x64 -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build-x64 --config Release

# x86
cmake -B build-x86 -A Win32 `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x86-windows
cmake --build build-x86 --config Release
```

Prebuilt x64 and x86 DLLs are also built by GitHub Actions (`.github/workflows/build.yml`).
