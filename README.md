# tc_runtime

## Maintainer

Created and maintained by [Fabio Sobral](https://github.com/flsobral).

Copyright © 2026 Amalgam Solucoes em TI Ltda.

A small, C-first application runtime that keeps platform, scheduler, graphics, renderer, and canvas APIs independent. The initial implementation target is SDL3 + Skia CPU. Application code, including the demo, never includes SDL or Skia headers.

## Current status

The public C API, event/frame runtime, SDL3 event adapter, CPU graphics context, Skia raster adapter, and generic-canvas demo are implemented. The default build requires externally supplied SDL3 and Skia CMake packages; neither dependency is vendored. OpenGL, Web, mobile backends, GLFW, other renderers, Metal, Vulkan, winit, and Vello are intentional stubs and CMake explains when one is selected.

## Build the CPU demo

Install SDL3 and supply a Skia build. A pinned TotalCross release is supported directly and can be fetched without adding binaries to the repository:

```sh
brew install sdl3 # macOS
./scripts/fetch-totalcross-skia.sh
```

Then configure and build:

```sh
cmake -S . -B build -DTC_BACKEND=SDL -DTC_RENDERER=SKIA -DTC_GRAPHICS=CPU \
  -DTC_SKIA_ROOT="$PWD/.cache/skia-158dc9d7"
cmake --build build --config Release
./build/examples/demo/tc_demo
```

Alternatively, provide a Skia CMake package and set `CMAKE_PREFIX_PATH` (and `TC_SKIA_TARGET` if its exported target is not `skia`). This is intentionally a configure-time error rather than a partial build.

## Planned configurations

`TC_PLATFORM` accepts `DESKTOP`, `ANDROID`, `IOS`, and `WEB`; `TC_BACKEND` accepts `SDL`, `ANDROID_NATIVE`, `IOS_NATIVE`, `GLFW`, and `WINIT`; `TC_GRAPHICS` accepts `CPU`, `OPENGL`, `METAL`, and `VULKAN`; and `TC_RENDERER` accepts `SKIA`, `NANOVG`, `BLEND2D`, and `VELLO`.

Only `DESKTOP + SDL + CPU + SKIA` is implemented today. The other selections fail clearly at CMake configuration time, rather than compiling incomplete adapters. OpenGL and the Emscripten demo remain the next milestones.

The private Android-native adapter translates lifecycle and pointer events and requires Android API 24 or newer. It uses `AChoreographer` directly; graphics integration remains unavailable until an Android GPU-enabled Skia archive is supplied.

## Android CPU native library

The Android CPU demo reuses `examples/demo/demo_scene.c` and presents the Skia raster buffer through `ANativeWindow`. It targets `arm64-v8a` with API 24 as the minimum. Fetch the required external archives, then build the native library:

```sh
./scripts/fetch-totalcross-skia.sh "$PWD/.cache/skia-android" android-arm64-v8a
curl -L -o /tmp/libpng-android.tar.gz https://github.com/TotalCross/totalcross-depot-tools/releases/download/libpng-1.6.48-r2/libpng-android-arm64-v8a.tar.gz
mkdir -p .cache/libpng-android && tar -xzf /tmp/libpng-android.tar.gz -C .cache/libpng-android

cmake -S android/app/src/main/cpp -B build-android-cpu \
  -DCMAKE_TOOLCHAIN_FILE="$HOME/Library/Android/sdk/ndk/28.2.13676358/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24 \
  -DTC_SKIA_ROOT="$PWD/.cache/skia-android" \
  -DTC_LIBPNG_ROOT="$PWD/.cache/libpng-android"
cmake --build build-android-cpu
```

This produces `build-android-cpu/libtc_demo_android.so`. The Gradle/NativeActivity project is under `android/`; APK packaging still requires a local Gradle installation or wrapper.
