# AGENTS.md

## Purpose

This repository is intended to build a small C-first cross-platform runtime architecture for applications that need platform backends, lifecycle, input events, frame scheduling, graphics contexts, and interchangeable 2D renderers.

The initial and default implementation target is:

```text
SDL3 platform backend + Skia renderer
```

The architecture must be designed so additional backends and renderers can be added without changing application code.

## Core Goals

- Keep all public interfaces C-compatible.
- Avoid exposing SDL, Skia, GLFW, Android, iOS, Rust, C++, or renderer-specific types in public headers.
- Make the runtime event/frame-oriented from the beginning.
- Treat polling loops as backend implementation details.
- Keep SDL3 + Skia as the first working and default path.
- Provide a demo application that uses only generic runtime/canvas/event APIs.
- Build through GitHub Actions on the correct host for each platform.

## Priority Order

### Platform Backends

Implement in this order:

1. SDL3
2. Android native
3. iOS native
4. GLFW
5. winit, experimental

Rust-based integrations must be last and experimental.

### Graphics Contexts

Implement in this order:

1. CPU
2. OpenGL / OpenGL ES
3. Metal
4. Vulkan

### 2D Renderers

Implement in this order:

1. Skia
2. NanoVG
3. Blend2D
4. Vello, experimental

## Architectural Rules

### Runtime Model

The runtime must be oriented around events and scheduled frames:

```text
backend emits event
  -> runtime dispatches on_event

scheduler emits frame
  -> runtime computes dt
  -> runtime calls on_update
  -> runtime begins renderer frame
  -> runtime calls on_draw(canvas)
  -> runtime ends renderer frame
```

Do not design application code around a blocking loop such as:

```c
while (running) {
    poll_events();
    update();
    draw();
}
```

SDL3 and GLFW may use such loops internally, but only as adapters that feed events and frame callbacks into the runtime.

### Scheduler

Every backend must provide, or be compatible with, a frame scheduling strategy:

```text
SDL3:             poll/timer/pump adapter
GLFW:             glfwPollEvents adapter
Android native:   Choreographer
iOS native:       CADisplayLink
Web/Emscripten:   requestAnimationFrame / emscripten_set_main_loop_arg
winit:            RedrawRequested / AboutToWait
```

The application and demo must not know which mechanism is being used.

### Public API Constraints

Public headers under `include/tc_runtime/` may contain:

- C structs
- enums
- opaque pointer types
- primitive types
- function pointers
- `const char*`

Public headers must not contain:

- C++ classes
- C++ templates
- `std::` types
- Rust types
- SDL types
- Skia types
- GLFW types
- Objective-C types
- JNI types
- platform-native structs/classes

If a renderer requires C++ internally, hide it behind private C wrapper functions.

Skia and Blend2D may use C++ implementation files internally, but their public-facing adapter must remain C-compatible.

## Expected Repository Layout

Use this structure unless there is a strong reason to adapt it:

```text
include/tc_runtime/
  tc_app.h
  tc_backend.h
  tc_canvas.h
  tc_event.h
  tc_graphics.h
  tc_renderer.h
  tc_scheduler.h
  tc_types.h

src/runtime/
  tc_app.c
  tc_event_queue.c
  tc_scheduler.c

src/backends/sdl/
  tc_sdl_backend.c
  tc_sdl_backend.h

src/backends/android/
  tc_android_backend.c
  tc_android_backend.h

src/backends/ios/
  tc_ios_backend.c
  tc_ios_backend.h

src/backends/glfw/
  tc_glfw_backend.c
  tc_glfw_backend.h

src/backends/winit/
  README.md

src/graphics/
  tc_cpu_context.c
  tc_gl_context.c
  tc_metal_context.c
  tc_vulkan_context.c

src/renderers/skia/
  tc_skia_renderer.cpp
  tc_skia_renderer_c_api.h

src/renderers/nanovg/
  tc_nanovg_renderer.c

src/renderers/blend2d/
  tc_blend2d_renderer.cpp
  tc_blend2d_renderer_c_api.h

src/renderers/vello/
  README.md

examples/demo/
  demo_main.c
  demo_scene.c
  demo_scene.h

examples/demo_web/
  index.html

.github/workflows/
  build-linux.yml
  build-macos.yml
  build-windows.yml
  build-android.yml
  build-ios.yml
  build-web.yml

CMakeLists.txt
README.md
PLANS.md
AGENTS.md
```

## Implementation Guidance

### First Working Path

Build SDL3 + Skia first.

The first implementation may start with:

```text
SDL3 + Skia CPU
```

Then add:

```text
SDL3 + Skia OpenGL
```

The demo must not directly include SDL or Skia headers.

### Stubs

Backends/renderers not yet implemented should have clean stubs that either:

- compile when disabled, or
- fail at CMake configure time with a clear error when selected.

Do not leave broken half-compiled modules.

### Web Target

The Web demo must reuse the same demo source and runtime interfaces.

Preferred initial target:

```text
SDL3 + Emscripten + Skia + WebGL
```

Fallback:

```text
SDL3 + Emscripten + Skia CPU/WASM
```

The Web target must not use a blocking infinite loop.

### Rust Integrations

winit and Vello are experimental and must not be required by default builds.

Do not introduce Rust into the core runtime unless the explicit milestone for experimental Rust integration is being worked on.

## Build System Rules

Use CMake options like:

```text
-DTC_PLATFORM=DESKTOP|ANDROID|IOS|WEB
-DTC_BACKEND=SDL|ANDROID_NATIVE|IOS_NATIVE|GLFW|WINIT
-DTC_GRAPHICS=CPU|OPENGL|METAL|VULKAN
-DTC_RENDERER=SKIA|NANOVG|BLEND2D|VELLO
```

Unsupported combinations must fail with clear messages.

Examples:

```sh
cmake -S . -B build \
  -DTC_BACKEND=SDL \
  -DTC_RENDERER=SKIA \
  -DTC_GRAPHICS=CPU

cmake --build build --config Release
```

```sh
cmake -S . -B build-sdl-skia-gl \
  -DTC_BACKEND=SDL \
  -DTC_RENDERER=SKIA \
  -DTC_GRAPHICS=OPENGL

cmake --build build-sdl-skia-gl --config Release
```

```sh
emcmake cmake -S . -B build-web \
  -DTC_PLATFORM=WEB \
  -DTC_BACKEND=SDL \
  -DTC_RENDERER=SKIA \
  -DTC_GRAPHICS=OPENGL

cmake --build build-web
```

## GitHub Actions

Use separate workflows per platform.

Required hosts:

```text
Linux desktop:    ubuntu-latest
Windows desktop:  windows-latest
macOS desktop:    macos-latest
iOS build:        macos-latest
Android build:    ubuntu-latest
Web build:        ubuntu-latest with Emscripten
```

Each workflow should upload logs on failure.

The first CI target may build only the library and demo. Mobile workflows may initially compile stubs until app packaging exists.

## Demo Requirements

The demo must draw using only the generic canvas API.

It should show:

- background clear
- rectangle
- rounded rectangle
- stroked line
- text
- animated circle or rectangle
- transform using save/restore/translate/rotate
- resize-aware layout

It should react to:

- close event
- resize event
- pointer down/move/up
- keyboard input where available

The demo state must be plain C.

## Coding Style

- Prefer small files and narrow interfaces.
- Keep ownership rules explicit.
- Every create/init function must have a matching destroy/shutdown function.
- Avoid global mutable state unless required by a platform API.
- Return explicit error codes from public APIs.
- Log configuration choices at startup.
- Keep comments focused on why, not what.

## Definition of Done for First Version

The first version is done when:

- SDL3 + Skia CPU builds.
- SDL3 + Skia OpenGL builds where dependencies are available.
- The desktop demo opens a window and animates primitives.
- The demo uses no SDL or Skia API directly.
- The runtime exposes scheduler/event/canvas abstractions.
- CI builds on at least one desktop host.
- Stubs exist for planned backends/renderers with clear configuration errors.

