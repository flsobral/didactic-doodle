# PLANS.md

# Build a C platform/rendering architecture with SDL3 + Skia as the default path

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds.

## Purpose / Big Picture

Create a small C-first cross-platform application runtime architecture that separates platform backends, lifecycle, input events, frame scheduling, graphics contexts, 2D renderers, canvas drawing, and demo application code.

After this work, the repository must build a demo application using the default combination:

```text
SDL3 platform backend + Skia renderer
```

The first working version must support at least:

```text
SDL3 + Skia CPU
SDL3 + Skia OpenGL
```

The demo must open a window, handle resize/input events, animate content, and draw using backend-agnostic canvas primitives instead of calling Skia directly from application code.

The architecture must already be prepared for Android native, iOS native, GLFW, winit, NanoVG, Blend2D, Vello, CPU, OpenGL, Metal, Vulkan, and Web/Emscripten targets, even when some of those are initially stubs or experimental.

## Progress

- [x] Create repository structure for public C interfaces, runtime, platform backends, graphics contexts, renderers, demo app, and CI.
- [x] Define public C-only interfaces for app callbacks, platform backend, scheduler, graphics context, renderer, canvas, events, images, paths, paints, and basic types.
- [x] Implement event/frame-oriented runtime.
- [x] Implement SDL3 platform backend as the default backend.
- [x] Implement CPU graphics context.
- [x] Implement Skia renderer through a C-compatible adapter.
- [x] Implement SDL3 + Skia CPU demo path.
- [x] Implement Android native lifecycle/input/scheduling adapter with CPU and OpenGL ES graphics paths.
- [x] Implement Android NativeActivity CPU demo library using the shared generic canvas demo source.
- [x] Package and sign an arm64-v8a Android API 24 debug APK using the Gradle Wrapper.
- [x] Build the iOS UIKit demo bundle for the arm64 simulator with CPU, OpenGL ES, and Metal graphics paths.
- [x] Implement OpenGL graphics context.
- [x] Implement SDL3 + Skia OpenGL demo path.
- [x] Implement the macOS SDL3 + Skia Metal demo path.
- [x] Implement demo application using only generic canvas/event/runtime APIs.
- [ ] Add Web/Emscripten demo target.
- [x] Add GitHub Actions workflow scaffolds for Linux, Windows, macOS, Android, iOS, and Web.
- [x] Add clear stubs for Android native, iOS native, GLFW, winit, NanoVG, Blend2D, Vello, and Vulkan where not yet implemented.
- [x] Document supported and unsupported combinations.

## Surprises & Discoveries

Record unexpected implementation facts here.

- Observation: SDL3 and Skia were initially absent from the development machine.
  Evidence: the default configure stopped with the explicit SDL3 dependency message.

- Observation: the TotalCross Skia release supplies matching development headers and a macOS arm64 static library, with GPU enabled.
  Evidence: `libskia-macos-arm64.a`, `skia-dev-headers-158dc9d7.zip`, and its build manifest were downloaded from release `skia-158dc9d7-r3`.

- Observation: the pinned macOS Skia archive implements Ganesh OpenGL, but its shipped headers require consumers to define `SK_GL` before exposing the API.
  Evidence: `nm -gU libskia-macos-arm64.a | c++filt` exports `GrDirectContext::MakeGL(...)` and the archive contains `GrGL*` objects; compiling the adapter with `SK_GL=1` exposes and links that API.

- Observation: the matching TotalCross Android archive includes Ganesh OpenGL ES support.
  Evidence: the release manifest was not retained in the unpacked archive, but `nm` shows exported `GrDirectContext::MakeGL(...)`, `GrGLMakeNativeInterface()`, and `GrGL*` objects. The Android NDK is installed locally and built the EGL/OpenGL ES 3 adapter.

- Observation: `AChoreographer` is only present from Android API 24.
  Evidence: Android NDK API level documentation marks its frame-callback API as API 24+; the Android minimum was raised accordingly.

- Observation: the local Android SDK includes NDK 28.2.13676358 and can cross-compile the native adapter for arm64 API 24.
  Evidence: `aarch64-linux-android24-clang` produced an AArch64 ELF object from `src/backends/android/tc_android_backend.c` with warnings treated as errors.

- Observation: the Android Skia archive requires companion libpng plus EGL/GLESv2 linkage even for the CPU raster demo.
  Evidence: the first shared-library link reported unresolved PNG and EGL/GLES symbols; linking the matching TotalCross `libpng-android-arm64-v8a` archive, `EGL`, and `GLESv2` produced `libtc_demo_android.so`.

- Observation: the matching TotalCross libpng archive was built against the corresponding zlib-ng prebuilt.
  Evidence: the libpng release describes that dependency and the Android `libz.a` archive is published by `zlib-ng-2.1.6-r2`.

- Observation: Android Gradle Plugin attempted unsupported ABIs by default.
  Evidence: it tried to link the arm64 Skia archive for `armeabi-v7a`, which failed. Restricting `abiFilters` to `arm64-v8a` produced a signed 6.8 MB debug APK verified with APK Signature Scheme v2.

- Observation: the iOS simulator Skia archive has an iOS 18.5 deployment floor.
  Evidence: linking at iOS 17 emitted version warnings; compiling the UIKit bundle with an 18.5 simulator deployment target succeeded.

- Observation: the iOS Skia raster surface is RGBA with a top-left origin, whereas the CoreGraphics image path needs explicit format and vertical-coordinate conversion.
  Evidence: the first simulator presentation was vertically inverted and red/blue-swizzled; `kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast` plus a saved vertical CGContext flip produced correctly oriented, correctly colored output.

- Observation: SDL's macOS CPU window surface is BGRA while the selected Skia build's native N32 raster format is RGBA.
  Evidence: the macOS demo reported `SDL_PIXELFORMAT_ARGB8888` (BGRA bytes on little-endian macOS) and showed the same red/blue channel swap previously observed on iOS; creating the Skia raster surface with the SDL surface's explicit pixel format removes the mismatch.

- Observation: the macOS SDL3 OpenGL path can create a 3.2 core context and drive Skia's Ganesh GL backend from the pinned archive.
  Evidence: configuring with `-DTC_GRAPHICS=OPENGL` built `tc_demo`; its runtime log reported `OpenGL 3.2 core context created` and completed the demo frame loop.

- Observation: `SDL_GetWindowSurface` must not be called for an SDL OpenGL window, including during resize processing.
  Evidence: the unconditional CPU-surface refresh caused `SDL_GL_SwapWindow` to fail with `The specified window isn't an OpenGL window`, leaving the rendered framebuffer unpresented.

- Observation: the pinned Android Skia archive can render through EGL/OpenGL ES 3 on the API 34 arm64 emulator.
  Evidence: an APK built with `-PtcAndroidGraphics=OPENGL` logged `EGL 1.4 OpenGL ES 3 context created`; emulator logcat reported continuous `EGL_emulation` frame statistics and an `adb screencap` showed the animated generic Skia demo.

- Observation: the iOS simulator Skia archive exports Ganesh OpenGL ES support, and its UIKit drawable uses a non-default framebuffer.
  Evidence: `nm -gU libskia-ios-simulator-arm64.a | c++filt` exports `GrDirectContext::MakeGL(...)` and `GrGLMakeNativeInterface()`. An arm64 iPhone 16 simulator displayed the animated generic demo through an `EAGLContext`/`CAEAGLLayer` path after Skia was explicitly rebound to the context-owned framebuffer.

- Observation: TotalCross Skia revision `skia-158dc9d7-r4` adds Metal-enabled Apple artifacts, Vulkan-enabled Android arm64-v8a, and a wasm32 WebGL archive.
  Evidence: the r4 release publishes `libskia-wasm32.a`, Android/iOS/macOS manifests, and reports WebAssembly/WebGL, Android arm64-v8a, and iOS device/simulator artifacts. Its manifests enable `skia_use_metal=true` for Apple, `skia_use_vulkan=true` for Android, and `skia_use_webgl=true` for wasm32.

- Observation: r4's public Ganesh value types have feature-dependent layouts, so consumers must define every GPU backend compiled into their platform archive.
  Evidence: the iOS OpenGL simulator app built without `SK_METAL` aborted in `gl_surface` with a stack-protector failure. Defining `SK_METAL=1` for Apple and `SK_VULKAN=1` for the Android archive aligns consumer headers with the r4 library configuration.

- Observation: a macOS Metal drawable is not reliably available while the SDL window is being constructed.
  Evidence: `SkSurface::MakeFromCAMetalLayer` returned no surface during renderer creation, while an LLDB breakpoint at `tc_metal_context_set_drawable` was hit from the first `tc_renderer_begin_frame` after the SDL event loop started.

- Observation: presenting a `CAMetalDrawable` directly can race the Skia command buffer and leave the macOS Metal window black.
  Evidence: Skia's own Metal window context enqueues `presentDrawable:` in a new command buffer on the same queue after Skia flushes, rather than calling the drawable's immediate `present` method.

- Observation: the r4 macOS archive enables both Ganesh OpenGL and Metal, even when the application selects only Metal.
  Evidence: its published macOS arm64 build manifest sets both `skia_use_gl=true` and `skia_use_metal=true`; those backend macros alter public Ganesh value-type layouts and therefore must be defined together by consumers.

- Observation: `MakeFromCAMetalLayer` acquires and retains its drawable lazily, when Skia first materializes the surface during drawing or flush.
  Evidence: its lazy-proxy callback calls `nextDrawable` and writes the retained handle only at materialization time. The output slot must remain valid through `on_draw` and `flush`; reading it in `begin_frame` yields null and prevents rendering.

- Observation: the pinned Skia Metal implementation cannot create a render pipeline with a raster sample count of zero.
  Evidence: the Metal runtime reported `Error creating pipeline: rasterSampleCount (0) is not supported by device` once the demo reached the presentation callback. Passing one creates a valid single-sample, non-MSAA render target.

- Observation: the r4 iOS simulator archive enables both Ganesh OpenGL ES and Metal.
  Evidence: its published arm64 simulator manifest sets `skia_use_gl=true`, `skia_gl_standard="gles"`, and `skia_use_metal=true`, so both public feature macros must be visible to every iOS consumer.

- Observation: the UIKit Metal path presents the generic Skia demo correctly on the arm64 iPhone 16 simulator.
  Evidence: the r4 Metal bundle installed and launched through `simctl`; a simulator screenshot showed the background, primitives, text, and animated shape without orientation or color-channel errors.

## Decision Log

- Decision: SDL3 + Skia is the default implementation path.
  Rationale: SDL3 provides broad initial platform coverage through a C API. Skia is the most complete and production-proven 2D renderer among the planned renderer options.
  Date/Author: Initial ExecPlan.

- Decision: All public interfaces must be C-only.
  Rationale: The architecture must be callable from C, JNI, and other runtimes without exposing C++ ABI or Rust ABI concerns.
  Date/Author: Initial ExecPlan.

- Decision: The runtime must be event/frame-oriented from the start.
  Rationale: Android, iOS, Web, and winit are naturally callback-driven. SDL3 and GLFW polling loops should be treated as adapters that feed events and frame ticks into the same callback-based runtime.
  Date/Author: Initial ExecPlan.

- Decision: Introduce a `FrameScheduler` abstraction.
  Rationale: The runtime should not assume ownership of a blocking `while` loop. Each backend should schedule frames using its native mechanism: SDL polling/timers, GLFW polling, Android `Choreographer`, iOS `CADisplayLink`, Web `requestAnimationFrame`, and winit redraw callbacks.
  Date/Author: Initial ExecPlan.

- Decision: Add a Web/Emscripten demo target.
  Rationale: The same architecture should prove that the demo can run in browsers without changing app code or exposing SDL/Emscripten/Skia details to the demo.
  Date/Author: Initial ExecPlan.

- Decision: Rust-based combinations are experimental and last.
  Rationale: Vello and winit may require Rust integration and should not complicate the default C build.
  Date/Author: Initial ExecPlan.

- Decision: Support the pinned TotalCross Skia release as an external, non-vendored dependency.
  Rationale: The release supplies the headers and platform static libraries needed to prove the CPU vertical slice without adding large generated binaries to the repository. `scripts/fetch-totalcross-skia.sh` downloads it into an ignored cache and `TC_SKIA_ROOT` makes the selection explicit.
  Date/Author: 2026-07-13 / Codex.

- Decision: Keep the SDL event/frame loop in the SDL backend rather than `tc_app_run`.
  Rationale: Application and runtime APIs stay callback-oriented; a polling loop is strictly an adapter detail for SDL and can be replaced by native schedulers on callback-driven platforms.
  Date/Author: 2026-07-13 / Codex.

- Decision: Use an SDL OpenGL 3.2 core context and wrap its default framebuffer in a Skia Ganesh `SkSurface`.
  Rationale: the verified macOS archive exports the required Ganesh GL API. Keeping the SDL context and framebuffer metadata private preserves the C-only public runtime surface.
  Date/Author: 2026-07-13 / Codex.

- Decision: Refresh SDL window surfaces only for the CPU graphics configuration.
  Rationale: the OpenGL swapchain is owned by the GL context, while the CPU path uses `SDL_Surface`; separating these avoids converting an OpenGL window into the incompatible software-surface path.
  Date/Author: 2026-07-13 / Codex.
  Date/Author: 2026-07-13 / Codex.

- Decision: Implement Android platform lifecycle, pointer translation, and Choreographer scheduling independently from renderer integration.
  Rationale: These NDK responsibilities are backend-specific and can be completed without leaking Android types into public headers. The CPU path was completed first; the compatible archive subsequently enabled an EGL/OpenGL ES 3 renderer integration.
  Date/Author: 2026-07-13 / Codex.

- Decision: Select Android CPU or OpenGL ES rendering at CMake/Gradle build time through `TC_ANDROID_GRAPHICS` / `tcAndroidGraphics`.
  Rationale: the NativeActivity, Choreographer adapter, and generic demo source remain identical while CPU continues to offer a dependable fallback and EGL/Ganesh GL is available where the packaged Skia archive supports it.
  Date/Author: 2026-07-13 / Codex.

- Decision: Require Android API 24 and call `AChoreographer` directly.
  Rationale: API 24 is now the supported baseline, allowing a simple direct vsync scheduler without dynamic symbol resolution or an API 23 fallback.
  Date/Author: 2026-07-13 / Codex.

- Decision: Use a Skia CPU raster buffer and copy it into `ANativeWindow` for the first Android demo.
  Rationale: it proved NativeActivity lifecycle, input, scheduling, the generic canvas API, and the reusable demo before the EGL/OpenGL ES 3 backend was added; it remains the default fallback configuration.
  Date/Author: 2026-07-13 / Codex.

- Decision: Link the Android demo to pinned TotalCross libpng and zlib-ng archives rather than an NDK-provided zlib.
  Rationale: libpng was built against that zlib-ng release, so linking the matching pair makes the Android Skia dependency graph explicit and reproducible.
  Date/Author: 2026-07-13 / Codex.

- Decision: Limit the first APK to `arm64-v8a`.
  Rationale: all three supplied prebuilt dependencies are arm64-v8a archives. Other Android ABIs must receive their own matching Skia, libpng, and zlib-ng artifacts before being enabled.
  Date/Author: 2026-07-13 / Codex.

- Decision: Implement the first iOS demo as a UIKit CPU-raster surface driven by `CADisplayLink`.
  Rationale: it reuses the generic canvas scene and Skia raster adapter while keeping UIKit/CoreGraphics details in private Objective-C++ sources.
  Date/Author: 2026-07-13 / Codex.

- Decision: Present the iOS Skia CPU buffer as a big-endian, premultiplied-last CoreGraphics image and flip only the drawing context's vertical axis.
  Rationale: the selected Skia build stores N32 pixels as RGBA. This preserves the generic top-left canvas coordinate system while avoiding a horizontal mirror and color-channel swaps in UIKit.
  Date/Author: 2026-07-13 / Codex.

- Decision: Offer the iOS simulator's OpenGL ES path through `TC_IOS_GRAPHICS=OPENGL`, while leaving CPU as the default and Metal as the future iOS GPU target.
  Rationale: the selected archive and simulator validated the legacy EAGL/Ganesh path without exposing native types in the public API. OpenGL ES is deprecated on iOS, so it cannot replace the planned Metal backend.
  Date/Author: 2026-07-13 / Codex.

- Decision: Pin the external Skia downloader to `skia-158dc9d7-r4` and expose every r4 static-library target it publishes.
  Rationale: this preserves the current explicit, non-vendored dependency model while making the Metal, Vulkan, and wasm32-capable archives available to their future platform integrations.
  Date/Author: 2026-07-13 / Codex.

- Decision: Compile consumers against r4's enabled GPU feature macros, even when a different supported backend is selected at runtime.
  Rationale: Skia's public backend surface types include feature-specific members. Matching the static library's feature set prevents ABI corruption while retaining build-time selection of CPU, OpenGL, Metal, and Vulkan paths.
  Date/Author: 2026-07-13 / Codex.

- Decision: Implement the macOS Metal path with SDL's `CAMetalLayer` view and Skia's `MakeFromCAMetalLayer` surface factory.
  Rationale: SDL keeps windowing private, while Skia obtains and submits the native Metal drawable through a per-frame surface. The renderer delays drawable acquisition until `begin_frame`, so its public C API remains free of Metal types and the normal SDL scheduler controls frame timing.
  Date/Author: 2026-07-13 / Codex.

- Decision: Schedule each macOS Metal drawable's presentation in a command buffer from the queue shared with Skia.
  Rationale: Metal command queues are ordered. Committing the presentation command after `flushAndSubmit` makes the drawable visible only after Skia's rendering commands, avoiding an otherwise-racy black frame.
  Date/Author: 2026-07-13 / Codex.

- Decision: Give `MakeFromCAMetalLayer` the private Metal context's drawable slot directly.
  Rationale: Skia fills the slot once it materializes the surface during frame submission. The graphics context then queues presentation and releases that retained reference exactly once, returning every drawable to `CAMetalLayer` for reuse.
  Date/Author: 2026-07-13 / Codex.

- Decision: Request one sample for the macOS Skia Metal surface until configurable MSAA is introduced.
  Rationale: One is Metal's valid single-sample value for this Skia build; zero fails pipeline creation despite the older public API describing it as disabling MSAA.
  Date/Author: 2026-07-13 / Codex.

- Decision: Implement iOS Metal with a UIKit `CAMetalLayer`, a private `MTLDevice`/`MTLCommandQueue`, and the shared Skia Metal renderer adapter.
  Rationale: UIKit owns the native display lifecycle through `CADisplayLink`; the graphics context owns Metal handles and presentation while the generic demo and public C headers remain independent of Objective-C and Metal types.
  Date/Author: 2026-07-13 / Codex.

- Decision: Carry the CPU target's private RGBA/BGRA format through the graphics context and construct each Skia raster surface explicitly from it.
  Rationale: native CPU surfaces vary by backend. Keeping this implementation detail private prevents platform types from leaking into public APIs and avoids assuming Skia's N32 format matches every destination.
  Date/Author: 2026-07-13 / Codex.

## Outcomes & Retrospective

The SDL3 + Skia CPU, SDL3 + Skia OpenGL, and macOS SDL3 + Skia Metal desktop vertical slices are implemented and validated locally. A 2026-07-13 build using the pinned TotalCross Skia release compiled successfully for all three paths; the Metal demo created an SDL `CAMetalLayer`, initialized Ganesh Metal, and acquired its first drawable from the scheduled frame callback. Its presentation now follows Skia's ordered command-queue pattern. Public headers passed standalone C11 syntax validation. The Android NativeActivity supports both CPU and EGL/OpenGL ES 3 Skia paths; the arm64 API 34 emulator displayed the OpenGL demo correctly. The iOS UIKit demo supports CPU, an arm64 simulator OpenGL ES path, and a native Metal path using the same per-frame Skia surface and presentation flow; the Metal path displayed the animated generic demo on an iPhone 16 simulator.

Web remains the next execution milestone. Its CMake selection deliberately fails clearly while its adapter is incomplete, avoiding an apparently successful but unusable build.

## Context and Orientation

This project provides a minimal runtime architecture. The application must not know whether it is running on SDL3, Android native, iOS native, GLFW, winit, Web/Emscripten, or another future backend.

The application must also not know whether drawing is handled by Skia, NanoVG, Blend2D, Vello, or another future renderer.

A platform backend owns operating-system integration:

```text
window/surface creation
lifecycle
input events
timers/frame scheduling
redraw requests
native surface handles
```

A graphics context represents the low-level drawing target:

```text
CPU bitmap
OpenGL / OpenGL ES
Metal
Vulkan
```

A renderer is the 2D drawing engine:

```text
Skia
NanoVG
Blend2D
Vello
```

A canvas is the generic drawing interface used by application/UI code. It exposes operations such as clear, save, restore, transform, clipping, rectangles, rounded rectangles, lines, paths, text, and images.

The first build must prove the complete path:

```text
demo app
  -> generic app runtime
  -> SDL3 backend
  -> frame scheduler
  -> graphics context
  -> Skia renderer
  -> generic Canvas2D API
  -> visible animated window
```

## Target Priority

### Platform Backends

Implement platform backends in this order:

```text
1. SDL3
2. Android native
3. iOS native
4. GLFW
5. winit, experimental
```

SDL3 is required for the first working version. Android native and iOS native are specialized mobile backends that implement the same interfaces. GLFW is useful for desktop/dev tooling. winit must remain experimental because it introduces Rust and a different native loop model.

### Graphics Contexts

Implement graphics contexts in this order:

```text
1. CPU
2. OpenGL / OpenGL ES
3. Metal
4. Vulkan
```

The first version may use Skia CPU first because it is easier to validate and useful for headless tests. SDL3 + Skia OpenGL should follow because it proves accelerated rendering.

### Renderers

Implement renderers in this order:

```text
1. Skia
2. NanoVG
3. Blend2D
4. Vello, experimental
```

Skia is the default renderer. NanoVG is an optional lightweight OpenGL renderer. Blend2D is an optional CPU renderer and useful for fallback/headless rendering. Vello is experimental and should be isolated because of Rust/wgpu integration.

## Repository Layout

Create this structure:

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
AGENTS.md
PLANS.md
```

## Public Interface Design

Create `include/tc_runtime/tc_types.h` with plain C structs for rectangles, colors, points, sizes, transforms, and handles.

Use opaque handles for runtime objects:

```c
typedef struct TcApp TcApp;
typedef struct TcPlatformBackend TcPlatformBackend;
typedef struct TcGraphicsContext TcGraphicsContext;
typedef struct TcRenderer2D TcRenderer2D;
typedef struct TcCanvas2D TcCanvas2D;
typedef struct TcFrameScheduler TcFrameScheduler;
```

Native handles must also be opaque:

```c
typedef struct TcNativeWindowHandle TcNativeWindowHandle;
typedef struct TcNativeSurfaceHandle TcNativeSurfaceHandle;
```

No SDL, Skia, GLFW, Objective-C, Android, C++, or Rust type may appear in public headers.

## Event Interface

Create `include/tc_runtime/tc_event.h` with backend-neutral event types:

```c
typedef enum TcEventType {
    TC_EVENT_NONE = 0,
    TC_EVENT_QUIT,
    TC_EVENT_RESIZE,
    TC_EVENT_REDRAW_REQUESTED,
    TC_EVENT_POINTER_DOWN,
    TC_EVENT_POINTER_MOVE,
    TC_EVENT_POINTER_UP,
    TC_EVENT_KEY_DOWN,
    TC_EVENT_KEY_UP,
    TC_EVENT_TEXT_INPUT,
    TC_EVENT_PAUSE,
    TC_EVENT_RESUME
} TcEventType;
```

Events must include enough data for resize, pointer, key, text, lifecycle, and redraw dispatch.

## Scheduler Interface

Create `include/tc_runtime/tc_scheduler.h`:

```c
typedef struct TcFrameScheduler TcFrameScheduler;

typedef void (*TcFrameCallback)(
    void* user_data,
    double timestamp_seconds,
    double delta_seconds
);

int tc_scheduler_start(
    TcFrameScheduler* scheduler,
    TcFrameCallback callback,
    void* user_data
);

int tc_scheduler_request_frame(TcFrameScheduler* scheduler);

void tc_scheduler_stop(TcFrameScheduler* scheduler);
```

Backend mappings:

```text
SDL3:
  SDL_PollEvent + timer/per-frame pump adapter

GLFW:
  glfwPollEvents + per-frame pump adapter

Android native:
  Choreographer

iOS native:
  CADisplayLink

Web/Emscripten:
  requestAnimationFrame / emscripten_set_main_loop_arg

winit:
  RedrawRequested / AboutToWait callbacks
```

The runtime must not assume that it owns a permanent blocking loop.

## Backend Interface

Create `include/tc_runtime/tc_backend.h` with a backend abstraction that provides:

```c
int tc_backend_init(TcPlatformBackend* backend, const TcBackendConfig* config);
TcNativeWindowHandle* tc_backend_get_native_window(TcPlatformBackend* backend);
TcNativeSurfaceHandle* tc_backend_get_native_surface(TcPlatformBackend* backend);
TcFrameScheduler* tc_backend_get_scheduler(TcPlatformBackend* backend);
int tc_backend_pump_events(TcPlatformBackend* backend, TcEventSink sink, void* user_data);
void tc_backend_request_redraw(TcPlatformBackend* backend);
void tc_backend_shutdown(TcPlatformBackend* backend);
```

`tc_backend_pump_events` may be meaningful for SDL3 and GLFW. Callback-driven platforms may implement it as a no-op or as a platform-specific dispatch hook.

## Runtime Callback Model

Create `include/tc_runtime/tc_app.h`:

```c
typedef struct TcAppCallbacks {
    void (*on_event)(void* user_data, const TcEvent* event);
    void (*on_update)(void* user_data, double delta_seconds);
    void (*on_draw)(void* user_data, TcCanvas2D* canvas);
    void (*on_shutdown)(void* user_data);
} TcAppCallbacks;
```

The frame flow must be:

```text
scheduler emits frame
  -> runtime computes dt
  -> runtime calls app on_update
  -> runtime begins renderer frame
  -> runtime calls app on_draw(canvas)
  -> runtime ends renderer frame
```

Do not let demo code control SDL polling directly.

## Graphics Context Interface

Create `include/tc_runtime/tc_graphics.h`:

```c
typedef enum TcGraphicsApi {
    TC_GRAPHICS_CPU = 0,
    TC_GRAPHICS_OPENGL,
    TC_GRAPHICS_METAL,
    TC_GRAPHICS_VULKAN
} TcGraphicsApi;

int tc_graphics_context_create(
    TcGraphicsApi api,
    TcNativeWindowHandle* window,
    TcNativeSurfaceHandle* surface,
    TcGraphicsContext** out_context
);

void tc_graphics_context_resize(
    TcGraphicsContext* context,
    int width,
    int height,
    float scale
);

void tc_graphics_context_destroy(TcGraphicsContext* context);
```

The CPU context should be implemented first. OpenGL follows. Metal and Vulkan may start as stubs with clear configure-time errors.

## Renderer Interface

Create `include/tc_runtime/tc_renderer.h`:

```c
typedef enum TcRendererKind {
    TC_RENDERER_SKIA = 0,
    TC_RENDERER_NANOVG,
    TC_RENDERER_BLEND2D,
    TC_RENDERER_VELLO
} TcRendererKind;

int tc_renderer_create(
    TcRendererKind kind,
    TcGraphicsContext* context,
    TcRenderer2D** out_renderer
);

int tc_renderer_attach(TcRenderer2D* renderer, TcGraphicsContext* context);
int tc_renderer_resize(TcRenderer2D* renderer, int width, int height, float scale);
TcCanvas2D* tc_renderer_begin_frame(TcRenderer2D* renderer);
int tc_renderer_end_frame(TcRenderer2D* renderer);
void tc_renderer_destroy(TcRenderer2D* renderer);
```

Skia is first. NanoVG, Blend2D, and Vello may start as unavailable options with clean configuration errors.

## Canvas Interface

Create `include/tc_runtime/tc_canvas.h` with backend-agnostic drawing:

```c
void tc_canvas_save(TcCanvas2D* canvas);
void tc_canvas_restore(TcCanvas2D* canvas);

void tc_canvas_translate(TcCanvas2D* canvas, float x, float y);
void tc_canvas_scale(TcCanvas2D* canvas, float x, float y);
void tc_canvas_rotate(TcCanvas2D* canvas, float degrees);

void tc_canvas_clip_rect(TcCanvas2D* canvas, TcRect rect);
void tc_canvas_clear(TcCanvas2D* canvas, TcColor color);

void tc_canvas_draw_rect(TcCanvas2D* canvas, TcRect rect, TcPaint paint);
void tc_canvas_draw_round_rect(TcCanvas2D* canvas, TcRect rect, float radius, TcPaint paint);
void tc_canvas_draw_line(TcCanvas2D* canvas, TcPoint a, TcPoint b, TcPaint paint);
void tc_canvas_draw_text(TcCanvas2D* canvas, const char* text, float x, float y, TcTextStyle style);
```

Reserve opaque handles for paths and images:

```c
typedef struct TcPath TcPath;
typedef struct TcImage TcImage;
```

Keep path/image APIs minimal in the first milestone.

## Event and Frame Requirements

Do not design the application around this model:

```c
while (running) {
    poll_events();
    update();
    draw();
}
```

Instead, design it around this model:

```text
backend emits event
  -> runtime dispatches to app on_event

scheduler emits frame
  -> runtime computes dt
  -> runtime calls app on_update
  -> runtime begins renderer frame
  -> runtime calls app on_draw(canvas)
  -> runtime ends renderer frame
```

Polling backends are allowed to implement polling internally, but polling must not leak into the app API.

## Plan of Work

### Milestone 1: Skeleton and Public Interfaces

Create the repository layout, CMake root, public headers, and no-op runtime.

Acceptance:

```sh
cmake -S . -B build
cmake --build build
```

The build produces a runtime library with no real backend selected.

### Milestone 2: Runtime, Events, and Scheduler

Implement app callbacks, event dispatch, frame scheduler interface, and runtime frame flow.

Acceptance:

- Runtime can receive synthetic events.
- Runtime can receive synthetic frame callbacks.
- Demo scene can be called through `on_update` and `on_draw` without SDL/Skia.

### Milestone 3: SDL3 Backend

Implement SDL3 window creation, event translation, lifecycle approximation, resize, pointer, keyboard, text input, close event, and redraw request.

Acceptance:

- SDL3 backend creates a window.
- SDL events are translated to `TcEvent`.
- SDL polling remains inside the backend/scheduler adapter.

### Milestone 4: CPU Graphics Context + Skia CPU Renderer

Implement CPU graphics context and Skia CPU renderer behind a C-compatible adapter.

Acceptance:

```sh
cmake -S . -B build \
  -DTC_BACKEND=SDL \
  -DTC_RENDERER=SKIA \
  -DTC_GRAPHICS=CPU

cmake --build build --config Release
```

The demo can draw through generic canvas APIs using Skia CPU.

### Milestone 5: OpenGL Graphics Context + SDL3 + Skia OpenGL

Implement SDL3 OpenGL context creation and Skia OpenGL renderer path.

Acceptance:

```sh
cmake -S . -B build-sdl-skia-gl \
  -DTC_BACKEND=SDL \
  -DTC_RENDERER=SKIA \
  -DTC_GRAPHICS=OPENGL

cmake --build build-sdl-skia-gl --config Release
```

Run:

```sh
./build-sdl-skia-gl/examples/demo/tc_demo
```

Expected output:

```text
tc_demo: backend=sdl
tc_demo: renderer=skia
tc_demo: graphics=opengl
tc_demo: window created
tc_demo: running
tc_demo: shutdown
```

Expected visual result:

- A window opens.
- The background is cleared.
- Rectangles, rounded rectangles, lines, and text are visible.
- At least one shape animates.
- Resizing updates the drawing area.
- Closing exits cleanly.

### Milestone 6: Demo Application

Implement `examples/demo` using only generic runtime/canvas/event APIs.

The demo must draw:

- background clear
- rectangle
- rounded rectangle
- stroked line
- text
- animated circle or rectangle
- transform using save/restore/translate/rotate
- resize-aware layout

The demo must react to:

- close event
- resize event
- pointer down/move/up
- keyboard input where available

The demo state must be a plain C struct.

### Milestone 7: Web/Emscripten Demo Target

Add a Web/Emscripten build of the demo application.

The Web target must reuse the same generic runtime, event, canvas, renderer, graphics, scheduler, and backend interfaces.

Initial Web target:

```text
SDL3 backend
Emscripten platform
FrameScheduler backed by browser-compatible animation frames
Skia renderer
WebGL graphics context when available
CPU/WASM fallback where practical
```

Expected output:

```text
examples/demo_web/index.html
examples/demo_web/tc_demo.js
examples/demo_web/tc_demo.wasm
examples/demo_web/tc_demo.data, if needed
```

The browser demo must:

- open inside an HTML canvas
- render the same animated primitives as the desktop demo
- handle resize
- handle mouse/touch input
- use requestAnimationFrame-compatible scheduling
- avoid blocking infinite loops

Build command example:

```sh
emcmake cmake -S . -B build-web \
  -DTC_PLATFORM=WEB \
  -DTC_BACKEND=SDL \
  -DTC_RENDERER=SKIA \
  -DTC_GRAPHICS=OPENGL

cmake --build build-web
```

Acceptance:

- Browser-loadable artifacts are produced.
- The demo runs in the browser.
- The same demo source is used as desktop.
- The demo does not call SDL, Emscripten, or Skia directly.

### Milestone 8: CI Workflows

Add separate GitHub Actions workflows.

Required workflows:

```text
.github/workflows/build-linux.yml
.github/workflows/build-windows.yml
.github/workflows/build-macos.yml
.github/workflows/build-android.yml
.github/workflows/build-ios.yml
.github/workflows/build-web.yml
```

Required hosts:

```text
Linux desktop:    ubuntu-latest
Windows desktop:  windows-latest
macOS desktop:    macos-latest
iOS build:        macos-latest
Android build:    ubuntu-latest with Android NDK
Web build:        ubuntu-latest with Emscripten
```

CI must at least build the library and demo for desktop platforms. Mobile workflows may initially compile platform/backend stubs if full app packaging is not ready, but they must be present and documented.

Each workflow must upload build logs when builds fail.

### Milestone 9: Stubs and Unsupported Combinations

Add placeholder or stub modules for:

```text
Android native
iOS native
GLFW
winit
NanoVG
Blend2D
Vello
Metal
Vulkan
```

Stubs must either:

- compile when disabled, or
- fail at CMake configure time with a clear message when selected.

Unsupported combinations must be rejected during configuration with readable errors.

### Milestone 10: Optional Backends and Renderers

After SDL3 + Skia works, add optional implementations in priority order:

```text
Android native backend
iOS native backend
GLFW backend
NanoVG renderer
Blend2D renderer
Metal graphics context
Vulkan graphics context
winit + Vello experimental path
```

Do not make Rust required for normal builds.

## CMake Configuration

Add options:

```text
-DTC_PLATFORM=DESKTOP|ANDROID|IOS|WEB
-DTC_BACKEND=SDL|ANDROID_NATIVE|IOS_NATIVE|GLFW|WINIT
-DTC_GRAPHICS=CPU|OPENGL|METAL|VULKAN
-DTC_RENDERER=SKIA|NANOVG|BLEND2D|VELLO
```

Default values:

```text
TC_PLATFORM=DESKTOP
TC_BACKEND=SDL
TC_GRAPHICS=CPU
TC_RENDERER=SKIA
```

Unsupported combinations must fail clearly.

Example desktop CPU build:

```sh
cmake -S . -B build \
  -DTC_BACKEND=SDL \
  -DTC_RENDERER=SKIA \
  -DTC_GRAPHICS=CPU

cmake --build build --config Release
```

Example desktop OpenGL build:

```sh
cmake -S . -B build-sdl-skia-gl \
  -DTC_BACKEND=SDL \
  -DTC_RENDERER=SKIA \
  -DTC_GRAPHICS=OPENGL

cmake --build build-sdl-skia-gl --config Release
```

Example Web build:

```sh
emcmake cmake -S . -B build-web \
  -DTC_PLATFORM=WEB \
  -DTC_BACKEND=SDL \
  -DTC_RENDERER=SKIA \
  -DTC_GRAPHICS=OPENGL

cmake --build build-web
```

## Backend and Renderer Matrix

Intended matrix:

```text
SDL3:
  CPU + Skia
  OpenGL + Skia
  OpenGL + NanoVG
  CPU + Blend2D

Android native:
  OpenGL/Vulkan + Skia
  CPU + Blend2D

iOS native:
  Metal + Skia
  CPU + Blend2D

GLFW:
  OpenGL + Skia
  OpenGL + NanoVG

Web:
  SDL3/Emscripten + Skia WebGL
  SDL3/Emscripten + Skia CPU/WASM fallback
  SDL3/Emscripten + NanoVG WebGL, later
  winit/wgpu + Vello WebGPU, experimental and last

winit:
  Vello/wgpu only as experimental
  Rust required
  not part of default build
```

## Idempotence and Recovery

All generated files must go under `build*` directories and may be deleted safely.

CMake configuration must be repeatable.

If SDL3, Skia, Emscripten, or another selected dependency is not available, CMake must fail with a clear message explaining:

- which dependency is missing
- which option selected it
- how to disable or change that option

Do not vendor large dependencies unless the repository already uses that pattern. Prefer CMake options that allow dependency paths to be supplied.

## Artifacts and Notes

The first successful implementation should capture these artifacts in CI:

```text
build logs
CMake configure logs
demo executable where applicable
Web HTML/JS/WASM artifacts
screenshots only if the CI environment supports headless capture
```

Add a short `README.md` showing:

```text
how to build SDL3 + Skia CPU
how to build SDL3 + Skia OpenGL
how to build Web/Emscripten demo
how to run the demo
which backends/renderers are implemented
which are stubs or experimental
```

## Validation and Acceptance

The implementation is accepted for the first desktop version when all of the following are true:

- The default CMake configuration builds successfully.
- SDL3 + Skia CPU builds.
- SDL3 + Skia OpenGL builds where dependencies are available.
- The demo application uses only `tc_canvas_*`, `tc_event_*`, scheduler, and runtime APIs.
- The demo does not include SDL or Skia headers directly.
- The SDL3 + Skia build opens a window and displays animated drawing.
- The demo responds to resize and close events.
- The runtime exposes scheduler/event/canvas abstractions.
- CI builds on at least one desktop host.

The Web target is accepted when:

```sh
emcmake cmake -S . -B build-web \
  -DTC_PLATFORM=WEB \
  -DTC_BACKEND=SDL \
  -DTC_RENDERER=SKIA \
  -DTC_GRAPHICS=OPENGL

cmake --build build-web
```

produces browser-loadable artifacts and the demo runs using the same app/demo source code as desktop, without direct SDL, Emscripten, or Skia calls in the demo.

The architecture is complete enough when the public API can represent all planned backends, graphics contexts, renderers, scheduler models, events, and canvas operations without exposing SDL, Skia, NanoVG, Blend2D, Vello, GLFW, winit, Android, or iOS types in public headers.
