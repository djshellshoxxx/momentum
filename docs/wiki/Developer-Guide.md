# Developer Guide

Windows, MSVC 2022 (CMake 3.22+, Git). JUCE 7.0.9 is fetched by CMake.

```sh
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Lite edition: `cmake -B build-lite -G "Visual Studio 17 2022" -A x64 -DMOMENTUM_EDITION=Lite`.

The top of `Momentum/CMakeLists.txt` sets `CMAKE_POLICY_VERSION_MINIMUM` for CMake 4.x. This snapshot was not built in this pass.

Hygiene: `Momentum/build-lite/` (about 256 files, including `CMakeCache.txt`) is committed. It should be removed from the index and ignored.
