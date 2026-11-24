# Relogic-GNS-Wrapper-Recomp

Rebuild of Terraria 1.4.6.0's Windows `GNS_Wrapper.dll` targeting Windows (x86/x64) and Linux (x64), using Valve's [GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets).

Terraria on Windows only ships a 32-bit `GNS_Wrapper.dll` and `steamnetworkingsockets.dll`, so 64-bit loaders/runtimes can't load it, and the official Linux `.so` depends on old OpenSSL 1.1 (`libssl.so.1.1`). This repo rebuilds the wrapper for Windows x86/x64 and Linux x64 with matching exports.

## Changes vs. official build

- **Windows:** builds both x86 and x64 against current vcpkg `GameNetworkingSockets` (OpenSSL 3).
- **Linux:** builds x64 `libGNS_Wrapper.so` with OpenSSL 3 and protobuf statically linked (no `libssl.so.1.1` dependency).
- **Code:** replaced MSVC-only `memcpy_s` with bounds-checked `std::memcpy`, guarded `__cdecl` to `_WIN32`, and exported only the `GNS_*` symbols on Linux.

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

## Build (Linux x64 + vcpkg)

```bash
cmake -B build-linux-x64 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-linux
cmake --build build-linux-x64 --parallel
```

Prebuilt Windows (x64/x86) and Linux (x64) binaries are also built by GitHub Actions (`.github/workflows/build.yml`).
