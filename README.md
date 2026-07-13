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

The private Android-native adapter translates lifecycle and pointer events and is designed for an Android API 23 minimum. It dynamically uses `AChoreographer` only on API 24+; graphics integration remains unavailable until an Android GPU-enabled Skia archive is supplied.
