# tc_runtime

## Maintainer

Created and maintained by [Fabio Sobral](https://github.com/flsobral).

Copyright © 2026 Amalgam Solucoes em TI Ltda.

A small, C-first application runtime that keeps platform, scheduler, graphics, renderer, and canvas APIs independent. The initial implementation targets are SDL3 + Skia CPU and SDL3 + Skia OpenGL. Application code, including the demo, never includes SDL or Skia headers.

## Current status

The public C API, event/frame runtime, SDL3 event adapter, CPU, OpenGL, and macOS Metal graphics contexts, Skia adapters, and generic-canvas demo are implemented. Android-native supports CPU, OpenGL ES 3, and Vulkan; iOS UIKit supports CPU, OpenGL ES, and Metal; and the web demo uses Emscripten, WebGL 2, and the wasm32 Skia archive. The default desktop build requires externally supplied SDL3 and Skia CMake packages; neither dependency is vendored. GLFW, other renderers, winit, and Vello are intentional stubs and CMake explains when one is selected.

## Build the CPU demo

Install SDL3 and supply a Skia build. A pinned TotalCross release is supported directly and can be fetched without adding binaries to the repository:

```sh
brew install sdl3 # macOS
./scripts/fetch-totalcross-skia.sh
```

Then configure and build:

```sh
cmake -S . -B build -DTC_BACKEND=SDL -DTC_RENDERER=SKIA -DTC_GRAPHICS=CPU \
  -DTC_SKIA_ROOT="$PWD/.cache/skia-158dc9d7-r4"
cmake --build build --config Release
./build/examples/demo/tc_demo
```

Alternatively, provide a Skia CMake package and set `CMAKE_PREFIX_PATH` (and `TC_SKIA_TARGET` if its exported target is not `skia`). This is intentionally a configure-time error rather than a partial build.

## Build the OpenGL demo

The pinned macOS Skia archive contains Ganesh OpenGL symbols. Build the shared demo source with an SDL OpenGL 3.2 core context:

```sh
cmake -S . -B build-sdl-skia-gl \
  -DTC_BACKEND=SDL -DTC_RENDERER=SKIA -DTC_GRAPHICS=OPENGL \
  -DTC_SKIA_ROOT="$PWD/.cache/skia-158dc9d7-r4"
cmake --build build-sdl-skia-gl
./build-sdl-skia-gl/examples/demo/tc_demo
```

## Build the Metal demo (macOS)

The r4 macOS archive includes Ganesh Metal support. The same demo uses SDL's private `CAMetalLayer` and acquires a Skia surface per frame:

```sh
cmake -S . -B build-sdl-skia-metal \
  -DTC_BACKEND=SDL -DTC_RENDERER=SKIA -DTC_GRAPHICS=METAL \
  -DTC_SKIA_ROOT="$PWD/.cache/skia-158dc9d7-r4"
cmake --build build-sdl-skia-metal
./build-sdl-skia-metal/examples/demo/tc_demo
```

## Planned configurations

`TC_PLATFORM` accepts `DESKTOP`, `ANDROID`, `IOS`, and `WEB`; `TC_BACKEND` accepts `SDL`, `ANDROID_NATIVE`, `IOS_NATIVE`, `GLFW`, and `WINIT`; `TC_GRAPHICS` accepts `CPU`, `OPENGL`, `METAL`, and `VULKAN`; and `TC_RENDERER` accepts `SKIA`, `NANOVG`, `BLEND2D`, and `VELLO`.

`DESKTOP + SDL + CPU + SKIA`, `DESKTOP + SDL + OPENGL + SKIA`, macOS `DESKTOP + SDL + METAL + SKIA`, and `WEB + SDL + OPENGL + SKIA` are implemented. The web selection uses a private Emscripten event/animation-frame adapter while preserving the same public backend API and shared demo source. Other unsupported selections fail clearly at CMake configuration time.

## Web demo

The web demo uses WebGL 2 and Emscripten's `requestAnimationFrame` scheduler. It needs the wasm32 r4 Skia archive and Emscripten **2.0.6**, the version used to produce that archive. Newer SDKs are not binary-compatible with this static archive.

```sh
./scripts/fetch-totalcross-skia.sh "$PWD/.cache/skia-wasm32-r4" wasm32
git clone https://github.com/emscripten-core/emsdk.git .cache/emsdk
.cache/emsdk/emsdk install 2.0.6
.cache/emsdk/emsdk activate 2.0.6
source .cache/emsdk/emsdk_env.sh

emcmake cmake -S . -B build-web -DCMAKE_BUILD_TYPE=Release \
  -DTC_PLATFORM=WEB -DTC_BACKEND=SDL -DTC_RENDERER=SKIA -DTC_GRAPHICS=OPENGL \
  -DTC_SKIA_ROOT="$PWD/.cache/skia-wasm32-r4"
cmake --build build-web
python3 -m http.server --directory build-web/examples/demo 8000
```

Open `http://localhost:8000/tc_demo.html`. The generated HTML, JavaScript, and WebAssembly files remain together in `build-web/examples/demo/`.

The private Android-native adapter translates lifecycle and pointer events and requires Android API 24 or newer. It uses `AChoreographer` directly. The default APK uses the CPU raster path; the same shared demo can be built with EGL/OpenGL ES 3 and Skia Ganesh when the selected Skia archive exports GL support.

## Android native library

The Android CPU demo reuses `examples/demo/demo_scene.c` and presents the Skia raster buffer through `ANativeWindow`. It targets `arm64-v8a` with API 24 as the minimum. Fetch the required external archives, then build the native library:

```sh
./scripts/fetch-totalcross-skia.sh "$PWD/.cache/skia-android" android-arm64-v8a
curl -L -o /tmp/libpng-android.tar.gz https://github.com/TotalCross/totalcross-depot-tools/releases/download/libpng-1.6.48-r2/libpng-android-arm64-v8a.tar.gz
mkdir -p .cache/libpng-android && tar -xzf /tmp/libpng-android.tar.gz -C .cache/libpng-android
curl -L -o /tmp/zlib-ng-android.tar.gz https://github.com/TotalCross/totalcross-depot-tools/releases/download/zlib-ng-2.1.6-r2/zlib-ng-android-arm64-v8a.tar.gz
mkdir -p .cache/zlib-ng-android && tar -xzf /tmp/zlib-ng-android.tar.gz -C .cache/zlib-ng-android

cmake -S android/app/src/main/cpp -B build-android-cpu \
  -DCMAKE_TOOLCHAIN_FILE="$HOME/Library/Android/sdk/ndk/28.2.13676358/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24 \
  -DTC_SKIA_ROOT="$PWD/.cache/skia-android" \
  -DTC_LIBPNG_ROOT="$PWD/.cache/libpng-android" \
  -DTC_ZLIB_NG_ROOT="$PWD/.cache/zlib-ng-android"
cmake --build build-android-cpu
```

This produces `build-android-cpu/libtc_demo_android.so`. The Gradle/NativeActivity project is under `android/`; APK packaging still requires a local Gradle installation or wrapper.

Build the debug APK with the versioned Gradle Wrapper and the same dependency roots:

```sh
cd android
./gradlew assembleDebug \
  -PtcSkiaRoot="$PWD/../.cache/skia-android" \
  -PtcLibpngRoot="$PWD/../.cache/libpng-android" \
  -PtcZlibNgRoot="$PWD/../.cache/zlib-ng-android"
```

The resulting signed debug artifact is `android/app/build/outputs/apk/debug/app-debug.apk`. It contains only `arm64-v8a`, matching the published Skia/libpng/zlib-ng archives.

Pass `-PtcAndroidGraphics=OPENGL` to build the EGL/OpenGL ES 3 variant. Pass `-PtcAndroidGraphics=VULKAN` to use the native Android Vulkan swapchain and Skia Ganesh Vulkan renderer. The Vulkan path requires the r4 Android Skia archive and runs on Android API 24 or newer.

## iOS simulator demo

The UIKit demo is driven by `CADisplayLink` and shares the generic Skia canvas scene. CPU is the default; OpenGL ES remains available for compatibility, while Metal is the native GPU path.

```sh
cmake -S ios -B build-ios-sim-metal \
  -DCMAKE_OSX_SYSROOT=iphonesimulator \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=18.5 \
  -DTC_IOS_GRAPHICS=METAL \
  -DTC_SKIA_ROOT="$PWD/.cache/skia-ios-sim-r4" \
  -DTC_LIBPNG_ROOT="$PWD/.cache/libpng-ios-sim" \
  -DTC_ZLIB_NG_ROOT="$PWD/.cache/zlib-ng-ios-sim"
cmake --build build-ios-sim-metal --parallel
```

Install and launch `build-ios-sim-metal/TCdemo.app` with `xcrun simctl install` and `xcrun simctl launch` for a booted arm64 simulator. Set `TC_IOS_GRAPHICS=CPU` or `OPENGL` to select the alternate paths.
