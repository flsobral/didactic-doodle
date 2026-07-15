# Magic Doodle Board

## Maintainer

Created and maintained by [Fabio Sobral](https://github.com/flsobral).

Copyright © 2026 Amalgam Solucoes em TI Ltda.

## License and contributions

This project is licensed under LGPL-2.1-only; see [LICENSE](LICENSE). See
[CONTRIBUTING.md](CONTRIBUTING.md) for the required SPDX headers and local
validation command.

Magic Doodle Board is a small, C-first, cross-platform runtime for 2D applications. It separates native application hosting, graphics context management, and Canvas 2D rendering into three independently built layers with stable public C APIs.

## Current implementation status

The repository is in the architectural migration described below. The currently
executable rendering paths are **Board Headless + Magic CPU + Doodle Skia**,
**Board SDL3 + Magic CPU/OpenGL/Metal/Vulkan + Doodle Skia on macOS**, and
**Board iOS native + Magic CPU/OpenGL ES/Metal + Doodle Skia in the iOS simulator**, and
**Board Android native + Magic CPU/OpenGL ES/Vulkan + Doodle Skia in an Android emulator**, and
**Board Web + Magic WebGL2 + Doodle Skia in a browser**. Board exposes either a
deterministic headless CPU surface, an SDL3 window surface, reusable native mobile
views, or a browser canvas; Magic acquires
and presents CPU frames through Board's versioned surface interface, and the
Skia provider binds Canvas operations through the selected versioned Magic
interop table.

Configure Skia with an artifact downloaded by
`bash scripts/fetch-totalcross-skia.sh .cache/skia-158dc9d7-r4 macos-arm64`:

```sh
cmake -S . -B build/headless-skia \
  -DBOARD_BACKEND=HEADLESS -DMAGIC_BACKEND=CPU \
  -DDOODLE_RENDERER=SKIA \
  -DDOODLE_SKIA_ROOT="$PWD/.cache/skia-158dc9d7-r4"
```

GLFW, winit, and Doodle providers other than Skia remain declared migration
targets. Selecting one fails during CMake configuration with an explicit
diagnostic; no backend is silently substituted.

Run the macOS desktop demo with:

```sh
cmake -S . -B build/desktop-cpu \
  -DBOARD_BACKEND=SDL3 -DMAGIC_BACKEND=CPU \
  -DDOODLE_RENDERER=SKIA \
  -DDOODLE_SKIA_ROOT="$PWD/.cache/skia-158dc9d7-r4" \
  -DMDB_BUILD_EXAMPLES=ON
cmake --build build/desktop-cpu --parallel
build/desktop-cpu/examples/desktop/magic_doodle_board_demo
```

For the OpenGL or macOS Metal variants, change the build directory and pass
`-DMAGIC_BACKEND=OPENGL` or `-DMAGIC_BACKEND=METAL`; `--frames 3` runs a finite
three-frame smoke test. Metal requires macOS.

For macOS Vulkan, install the Vulkan SDK and run the named smoke test with the
Skia r5 archive:

```sh
export VULKAN_SDK="$HOME/Library/VulkanSDK/1.4.350.1/macOS"
bash scripts/fetch-totalcross-skia.sh .cache/skia-158dc9d7-r5 macos-arm64
VULKAN_SDK="$VULKAN_SDK" ./scripts/test-desktop-vulkan-skia.sh
```

The wrapper uses the SDK's MoltenVK implementation and
`VK_LAYER_KHRONOS_validation`. Windows and Linux require separate runner
validation before they are documented as supported.

Run the iOS CPU demo in an arm64 simulator after making the matching external
Skia, libpng, and zlib artifacts available (the paths below are ignored caches):

```sh
bash scripts/fetch-totalcross-skia.sh .cache/skia-158dc9d7-r4 ios-simulator-arm64
cmake -S . -B build/ios-cpu \
  -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DBOARD_BACKEND=IOS_NATIVE -DMAGIC_BACKEND=CPU \
  -DDOODLE_RENDERER=SKIA \
  -DDOODLE_SKIA_ROOT="$PWD/.cache/skia-158dc9d7-r4" \
  -DDOODLE_IOS_PNG_ROOT="$PWD/.cache/libpng-ios-sim/libpng/ios-simulator/arm64" \
  -DDOODLE_IOS_ZLIB_ROOT="$PWD/.cache/zlib-ng-ios-sim/zlib-ng/ios-simulator/arm64" \
  -DMDB_BUILD_TESTS=OFF -DMDB_BUILD_EXAMPLES=ON
cmake --build build/ios-cpu --parallel
xcrun simctl install booted build/ios-cpu/magic_doodle_board_ios_demo.app
xcrun simctl launch booted com.amalgam.magicdoodleboard.demo
```

For the OpenGL ES or Metal simulator variants, use
`-DMAGIC_BACKEND=OPENGL` or `-DMAGIC_BACKEND=METAL` with a separate build
directory such as `build/ios-opengl` or `build/ios-metal`. The native Board
view uses a private `CAEAGLLayer` or `CAMetalLayer`; Magic owns the GPU context
or Metal device, queue, drawable, and presentation.

Run the Android CPU demo on an arm64 Android emulator with API 24 or newer:

```sh
cd android
ANDROID_HOME="$HOME/Library/Android/sdk" ./gradlew :app:assembleDebug \
  -PdoodleSkiaRoot="$PWD/../.cache/skia-android" \
  -PdoodleAndroidPngRoot="$PWD/../.cache/libpng-android/libpng/android/arm64-v8a" \
  -PdoodleAndroidZlibRoot="$PWD/../.cache/zlib-ng-android/zlib-ng/android/arm64-v8a"
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n com.amalgam.magicdoodleboard.demo/android.app.NativeActivity
```

For the OpenGL ES variant, add `-PmdbAndroidMagicBackend=OPENGL` to the Gradle
command. For Vulkan, use `-PmdbAndroidMagicBackend=VULKAN` and a Skia archive
downloaded by the current `scripts/fetch-totalcross-skia.sh` release, which
includes Ganesh Vulkan support. Board keeps its private EGL surface and OpenGL
ES context callbacks; for Vulkan it exposes only opaque Android-surface
creation callbacks. Magic owns the Vulkan instance, device, swapchain,
synchronization, and presentation, while Doodle Skia consumes the versioned
`MagicVulkanInterop` frame data.

The Android app has `minSdk 24`. Board keeps the NativeActivity, `ANativeWindow`,
input conversion, and `AChoreographer` frame loop private; Magic CPU presents
through the versioned Board CPU surface interface, while Magic OpenGL ES uses
the Board EGL capability.

Run the Web demo with the pinned wasm32 Skia archive and Emscripten 2.0.6:

```sh
bash scripts/fetch-totalcross-skia.sh .cache/skia-wasm32-r4 wasm32
emcmake cmake -S . -B build/web-skia \
  -DBOARD_BACKEND=WEB -DMAGIC_BACKEND=WEB \
  -DDOODLE_RENDERER=SKIA \
  -DDOODLE_SKIA_ROOT="$PWD/.cache/skia-wasm32-r4" \
  -DMDB_BUILD_TESTS=OFF -DMDB_BUILD_EXAMPLES=ON
cmake --build build/web-skia --parallel
cd build/web-skia/examples/web
python3 -m http.server 8080 --bind 127.0.0.1
# In another terminal: open http://127.0.0.1:8080/magic_doodle_board_web_demo.html
```

Board owns browser animation-frame scheduling plus mouse, touch, keyboard, and
resize conversion. Magic privately owns the WebGL2 context; presentation is
implicit in the browser frame callback;
Doodle Skia consumes only `MagicWebInterop` and renders the shared scene. The
preparation script also fetches Roboto, which the Web build preloads into
Emscripten's virtual font directory so Skia text is available in the browser.

## Backend-matrix test scripts

Each supported combination has a smoke-test script that builds, installs where
needed, runs the shared scene, and records its observable result. The scripts
expect their platform simulator or emulator to be booted; cache locations can
be overridden with the `MDB_*_SKIA_ROOT`, `MDB_*_PNG_ROOT`, and
`MDB_*_ZLIB_ROOT` environment variables.

```sh
scripts/test-headless-cpu-skia.sh
scripts/test-desktop-cpu-skia.sh
scripts/test-desktop-opengl-skia.sh
scripts/test-desktop-metal-skia.sh
scripts/test-desktop-vulkan-skia.sh
scripts/test-ios-cpu-skia.sh
scripts/test-ios-opengl-skia.sh
scripts/test-ios-metal-skia.sh
scripts/test-android-cpu-skia.sh
scripts/test-android-opengl-skia.sh
scripts/test-android-vulkan-skia.sh
scripts/test-web-skia.sh
```

`scripts/test-backend-matrix.sh <combination>` is the shared implementation.
Adding a supported entry to the backend matrix requires adding its matching
`scripts/test-<combination>.sh` wrapper in the same change. The Web wrapper
starts a local HTTP server on an available loopback port and uses Safari by
default; set `MDB_WEB_BROWSER`, `MDB_WEB_TIMEOUT_SECONDS`, or `MDB_WEB_PORT`
to select another browser, visible run duration, or server port.

Android wrappers wait up to 60 seconds for a connected, fully booted emulator
before failing clearly; set `MDB_ANDROID_BOOT_TIMEOUT_SECONDS` or `ADB` when
using a different timeout or device bridge.

The desktop Vulkan wrapper requires a Vulkan SDK through `VULKAN_SDK` or
`MDB_VULKAN_SDK`, a Vulkan-enabled macOS Skia r5 cache, and a visible desktop
session. It runs the demo with `VK_LAYER_KHRONOS_validation`; set
`MDB_DESKTOP_VULKAN_SKIA_ROOT`, `MDB_VULKAN_ICD`, or
`MDB_VULKAN_LAYER_PATH` to select non-default artifacts or SDK paths.

The `ios/` source directory is a convenience entry point for an iOS-only
consumer. It selects the same public composition as the root project rather
than compiling a legacy platform-specific demo directly:

```sh
cmake -S ios -B build/ios-demo \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT=iphonesimulator \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DMDB_IOS_MAGIC_BACKEND=CPU \
  -DMDB_IOS_SKIA_ROOT="$PWD/.cache/skia-158dc9d7-r4" \
  -DMDB_IOS_PNG_ROOT="$PWD/.cache/libpng-ios-sim/libpng/ios-simulator/arm64" \
  -DMDB_IOS_ZLIB_ROOT="$PWD/.cache/zlib-ng-ios-sim/zlib-ng/ios-simulator/arm64"
cmake --build build/ios-demo --parallel
```

The name is both a product metaphor and an architectural map:

- **Board** is the surface on which an application exists: windows, views, events, lifecycle, and frame timing.
- **Magic** is the machinery that turns a native surface into a usable CPU or GPU frame.
- **Doodle** is the drawing API and the renderer that turns Canvas commands into pixels.

The framework does not expose SDL, GLFW, Emscripten, Android, UIKit, Metal, OpenGL, Vulkan, Skia, C++, or Rust types through its portable public headers.

## Architecture

The runtime stack has exactly three public layers:

```text
Application
    |
    | Board callbacks and events
    v
+-------------------------------+
| Board                         |
| app host, window/view, input, |
| lifecycle, scheduler, surface |
+-------------------------------+
    |
    | Board public surface API
    v
+-------------------------------+
| Magic                         |
| device/context, frame target, |
| synchronization, presentation |
+-------------------------------+
    |
    | Magic public frame API
    v
+-------------------------------+
| Doodle                        |
| Canvas 2D and renderer        |
| Skia / Blend2D / NanoVG/Vello |
+-------------------------------+
```

The dependency direction is strict:

```text
Board
  ^
  |
Magic
  ^
  |
Doodle
```

Board does not depend on Magic or Doodle. Magic may consume only Board's installed public API. Doodle may consume only Magic's installed public API and does not depend directly on Board. The root CMake project only composes selected implementations; it is not a fourth runtime layer.

## Execution model

Magic Doodle Board is event-driven and frame-driven. Application code never owns a blocking platform loop.

```text
native Board backend
    -> converts input/lifecycle changes into BoardEvent
    -> dispatches the application event callback

native frame source
    -> Board frame callback(timestamp, delta)
    -> application update
    -> magic_context_begin_frame()
    -> doodle_renderer_begin_frame()
    -> application draws through DoodleCanvas
    -> doodle_renderer_end_frame()
    -> magic_context_end_frame()
    -> present
```

Frame sources remain platform-native:

- Web uses the browser animation-frame callback.
- Android uses `AChoreographer` directly.
- iOS uses `CADisplayLink`.
- SDL3, GLFW, and winit adapt their native event and frame mechanisms to the same Board callback contract.
- Headless mode supports manual or deterministic virtual-time frames for tests and server-side rendering.

A representative composition looks like this:

```c
static void app_on_frame(void *user_data,
                         uint64_t timestamp_ns,
                         double delta_seconds) {
    AppState *app = user_data;
    MagicFrame *frame = NULL;
    DoodleCanvas *canvas = NULL;

    app_update(app, delta_seconds);

    if (magic_context_begin_frame(app->magic, &frame) != MAGIC_OK) {
        return;
    }

    if (doodle_renderer_begin_frame(app->doodle, frame, &canvas) == DOODLE_OK) {
        app_draw(app, canvas);
        doodle_renderer_end_frame(app->doodle, canvas);
    }

    magic_context_end_frame(app->magic, frame);
}
```

Board never calls Doodle directly, and Doodle never presents a frame directly. The application composes the three public APIs.

### Runtime backend identity

After creating the three objects, an application can report the active runtime
combination without consulting build macros:

```c
printf("Board: %s %s | Magic: %s %s | Doodle: %s %s\n",
       board_backend_name(board), board_backend_version(board),
       magic_context_backend_name(magic), magic_context_backend_version(magic),
       doodle_renderer_name(renderer), doodle_renderer_version(renderer));
```

The shared demo renders these three labels. SDL, OpenGL/WebGL, and Vulkan
versions come from their active runtime objects; the Skia label identifies the
external Skia artifact selected when Doodle was configured.

To compile every currently supported matrix entry without launching devices or
opening a browser, run:

```sh
VULKAN_SDK="$HOME/Library/VulkanSDK/1.4.350.1/macOS" \
  scripts/test-backend-matrix.sh build-all
```

The command still requires each platform toolchain and external Skia artifact;
use an individual named command when a build should also run its smoke test.

## Layer 1: Board

Board owns platform and host integration.

### Responsibilities

- application startup, shutdown, pause, resume, focus, and visibility;
- logical windows and embeddable native views;
- native surface creation and destruction;
- size, scale, orientation, monitor, and safe-area information;
- keyboard, text input, mouse, touch, pen, and gamepad events;
- IME, clipboard, cursor, drag and drop, and accessibility hooks where supported;
- event queues and callback dispatch;
- frame scheduling and `request_frame`;
- UI-thread dispatch;
- native host services and native-view overlay slots;
- a versioned native-surface capability API consumed by Magic.

### Board backends

```text
SDL3
GLFW
Web
winit
Android native
IOS native
Headless
```

SDL3 is the default desktop backend. Android and iOS backends are native and embeddable. Web is implemented as a browser host rather than as a hidden substitution for another backend. Headless is a first-class backend for deterministic tests and offscreen workloads.

### Public API shape

```c
typedef struct BoardApp BoardApp;
typedef struct BoardWindow BoardWindow;
typedef struct BoardFrameScheduler BoardFrameScheduler;
typedef struct BoardNativeSurface BoardNativeSurface;

typedef struct BoardAppCallbacks {
    void (*on_start)(void *user_data);
    void (*on_event)(void *user_data, const BoardEvent *event);
    void (*on_update)(void *user_data, double delta_seconds);
    void (*on_frame)(void *user_data,
                     uint64_t timestamp_ns,
                     double delta_seconds);
    void (*on_shutdown)(void *user_data);
} BoardAppCallbacks;

BoardResult board_app_create(const BoardAppConfig *config,
                             const BoardAppCallbacks *callbacks,
                             void *user_data,
                             BoardApp **out_app);
void board_app_destroy(BoardApp *app);
void board_app_request_frame(BoardApp *app);
```

The Board frame callback does not receive a Canvas. This keeps Board independent from Doodle.

### Native surface capability contract

Magic needs more than a raw pointer to support SDL-created OpenGL contexts, Vulkan surfaces, Metal layers, Web canvases, and CPU presentation. Board therefore exposes a versioned capability query instead of leaking platform types.

Conceptually:

```c
typedef enum BoardSurfaceInterfaceId {
    BOARD_SURFACE_INTERFACE_CPU,
    BOARD_SURFACE_INTERFACE_OPENGL,
    BOARD_SURFACE_INTERFACE_VULKAN,
    BOARD_SURFACE_INTERFACE_METAL,
    BOARD_SURFACE_INTERFACE_WEB
} BoardSurfaceInterfaceId;

BoardResult board_surface_query_interface(
    BoardNativeSurface *surface,
    BoardSurfaceInterfaceId interface_id,
    uint32_t abi_version,
    void *out_interface,
    size_t out_interface_size);
```

Each returned interface is a C struct that begins with `struct_size` and `abi_version` and contains only primitive values, opaque pointers or integer handles, and callbacks. Magic performs backend-specific casts only in private implementation files.

## Layer 2: Magic

Magic owns graphics context and frame presentation.

### Responsibilities

- selecting CPU, OpenGL/OpenGL ES, Metal, Vulkan, or Web;
- creating and destroying devices and contexts;
- creating the graphics surface or swapchain from a Board surface capability;
- acquiring and ending frames;
- exposing renderer-compatible frame targets through versioned public interop structs;
- resize, sample count, color format, color space, and vsync;
- synchronization and resource-lifetime fences;
- device-loss detection and recovery;
- CPU buffers and upload/presentation where applicable.

### Magic backends

```text
CPU
OpenGL / OpenGL ES
Metal
Vulkan
Web
```

`MAGIC_BACKEND_WEB` is a browser graphics provider. Its initial implementation may use WebGL 2, while preserving a public API that can later support WebGPU without exposing Emscripten or browser types.

### Public API shape

```c
typedef struct MagicContext MagicContext;
typedef struct MagicFrame MagicFrame;

typedef enum MagicBackend {
    MAGIC_BACKEND_AUTO,
    MAGIC_BACKEND_CPU,
    MAGIC_BACKEND_OPENGL,
    MAGIC_BACKEND_METAL,
    MAGIC_BACKEND_VULKAN,
    MAGIC_BACKEND_WEB
} MagicBackend;

typedef struct MagicConfig {
    uint32_t struct_size;
    MagicBackend backend;
    bool vsync;
    uint32_t sample_count;
} MagicConfig;

MagicResult magic_context_create(BoardNativeSurface *surface,
                                 const MagicConfig *config,
                                 MagicContext **out_context);
void magic_context_destroy(MagicContext *context);
MagicResult magic_context_resize(MagicContext *context,
                                 uint32_t width,
                                 uint32_t height,
                                 float scale);
MagicResult magic_context_begin_frame(MagicContext *context,
                                      MagicFrame **out_frame);
MagicResult magic_context_end_frame(MagicContext *context,
                                    MagicFrame *frame);
```

Magic exposes renderer interop through versioned queries:

```c
typedef enum MagicInteropId {
    MAGIC_INTEROP_CPU_TARGET,
    MAGIC_INTEROP_OPENGL_TARGET,
    MAGIC_INTEROP_METAL_TARGET,
    MAGIC_INTEROP_VULKAN_TARGET,
    MAGIC_INTEROP_WEB_TARGET
} MagicInteropId;

MagicResult magic_frame_query_interop(const MagicFrame *frame,
                                      MagicInteropId interop_id,
                                      uint32_t abi_version,
                                      void *out_interop,
                                      size_t out_interop_size);
```

The interop tables contain no native graphics declarations. A private renderer adapter may reinterpret opaque handles after including its own backend headers.

## Layer 3: Doodle

Doodle owns the portable 2D drawing API and its renderer implementations.

### Responsibilities

- Canvas state, save and restore;
- clear, rectangles, rounded rectangles, lines, circles, and paths;
- transforms, clipping, layers, and blend modes;
- paints, strokes, gradients, shaders, and filters;
- images and image sampling;
- fonts, text measurement, shaping integration, and text drawing;
- renderer creation, resize, frame binding, flush, and teardown;
- feature reporting and compatibility checks.

### Doodle renderers

```text
Skia
Blend2D
NanoVG
Vello
```

Renderer adapters are Doodle submodules, not a fourth architectural layer.

- Skia is the first complete renderer and may use CPU raster, OpenGL/OpenGL ES, Metal, Vulkan, or Web targets supplied by Magic.
- Blend2D initially targets Magic CPU frames.
- NanoVG initially targets Magic OpenGL/OpenGL ES frames.
- Vello remains a clearly reported stub until a stable C adapter and compatible Magic interop path are implemented.

### Public API shape

```c
typedef struct DoodleRenderer DoodleRenderer;
typedef struct DoodleCanvas DoodleCanvas;
typedef struct DoodlePaint DoodlePaint;
typedef struct DoodlePath DoodlePath;
typedef struct DoodleImage DoodleImage;
typedef struct DoodleFont DoodleFont;

typedef enum DoodleRendererBackend {
    DOODLE_RENDERER_SKIA,
    DOODLE_RENDERER_BLEND2D,
    DOODLE_RENDERER_NANOVG,
    DOODLE_RENDERER_VELLO
} DoodleRendererBackend;

DoodleResult doodle_renderer_create(MagicContext *magic,
                                    const DoodleRendererConfig *config,
                                    DoodleRenderer **out_renderer);
void doodle_renderer_destroy(DoodleRenderer *renderer);
DoodleResult doodle_renderer_begin_frame(DoodleRenderer *renderer,
                                         MagicFrame *frame,
                                         DoodleCanvas **out_canvas);
DoodleResult doodle_renderer_end_frame(DoodleRenderer *renderer,
                                       DoodleCanvas *canvas);

void doodle_canvas_clear(DoodleCanvas *canvas, DoodleColor color);
void doodle_canvas_save(DoodleCanvas *canvas);
void doodle_canvas_restore(DoodleCanvas *canvas);
void doodle_canvas_translate(DoodleCanvas *canvas, float x, float y);
void doodle_canvas_scale(DoodleCanvas *canvas, float x, float y);
void doodle_canvas_rotate(DoodleCanvas *canvas, float radians);
void doodle_canvas_clip_rect(DoodleCanvas *canvas, DoodleRect rect);
void doodle_canvas_draw_rect(DoodleCanvas *canvas,
                             DoodleRect rect,
                             const DoodlePaint *paint);
```

## Mobile integration

Board supports three mobile hosting modes without adding platform concepts to Magic or Doodle.

### Fullscreen owned

A convenience activity or view controller hosts one Board view that fills the screen. Board still uses the same embeddable view internally.

### Embedded

The native application creates and positions a Board view inside any layout region.

Android concept:

```java
import org.magicdoodle.board.BoardView;

BoardView boardView = new BoardView(context);
boardView.setApplication(applicationHandle);
container.addView(boardView);
```

iOS concept:

```swift
import MagicDoodleBoard

let boardView = BoardView(frame: .zero)
boardView.applicationHandle = applicationHandle
view.addSubview(boardView)
```

The core C API never owns an `Activity`, `Fragment`, `UIViewController`, or navigation controller. Optional wrappers are convenience adapters only.

### Hybrid overlay

A Board host view contains both the rendering surface and a native overlay container:

```text
Board host view
├── rendering surface
└── native overlay container
    ├── text control
    ├── map view
    ├── camera preview
    └── web view
```

A `BoardNativeViewSlot` controls native view position, size, visibility, rectangular clipping, and a documented above-or-below-surface z-order. Arbitrary Doodle filters, perspective transforms, and path clipping do not apply to native controls.

Use `BOARD_HOST_MODE_HYBRID_OVERLAY` in `BoardBackendConfig` before creating
slots. The portable API deliberately receives the native control as an opaque
pointer, so no UIKit or Android types enter public headers. Slot frame and
clip values are logical host coordinates, and all slot operations must run on
the platform UI thread. The iOS host currently supports controls above the
renderer; requesting below-renderer order or requesting a slot from an
unimplemented host returns `BOARD_ERROR_UNAVAILABLE`.

Android provides `org.magicdoodle.board.BoardView` as an embeddable
`SurfaceView`. The demo Activity places it between ordinary Android controls;
its native application handle remains private to the JNI bridge. In hybrid
mode it also hosts an above-renderer native overlay through the same opaque
`BoardNativeViewSlot` API as iOS; below-renderer order remains unavailable.

## Target repository structure

```text
magic-doodle-board/
├── CMakeLists.txt
├── AGENTS.md
├── README.md
├── LICENSE
├── cmake/
│   ├── MagicDoodleBoardOptions.cmake
│   ├── MagicDoodleBoardCompatibility.cmake
│   └── package/
│
├── board/
│   ├── CMakeLists.txt
│   ├── include/board/
│   │   ├── board_app.h
│   │   ├── board_backend.h
│   │   ├── board_event.h
│   │   ├── board_host.h
│   │   ├── board_native_view.h
│   │   ├── board_scheduler.h
│   │   ├── board_surface.h
│   │   ├── board_types.h
│   │   └── board_version.h
│   ├── src/
│   │   ├── board_app.c
│   │   ├── board_event_queue.c
│   │   ├── board_scheduler.c
│   │   └── board_surface.c
│   ├── backends/
│   │   ├── sdl3/
│   │   ├── glfw/
│   │   ├── web/
│   │   ├── winit/
│   │   ├── android/
│   │   ├── ios/
│   │   └── headless/
│   └── tests/
│
├── magic/
│   ├── CMakeLists.txt
│   ├── include/magic/
│   │   ├── magic_backend.h
│   │   ├── magic_context.h
│   │   ├── magic_frame.h
│   │   ├── magic_interop.h
│   │   ├── magic_types.h
│   │   └── magic_version.h
│   ├── src/
│   │   ├── magic_context.c
│   │   ├── magic_frame.c
│   │   └── magic_registry.c
│   ├── backends/
│   │   ├── cpu/
│   │   ├── opengl/
│   │   ├── metal/
│   │   ├── vulkan/
│   │   └── web/
│   └── tests/
│
├── doodle/
│   ├── CMakeLists.txt
│   ├── include/doodle/
│   │   ├── doodle_canvas.h
│   │   ├── doodle_color.h
│   │   ├── doodle_font.h
│   │   ├── doodle_image.h
│   │   ├── doodle_paint.h
│   │   ├── doodle_path.h
│   │   ├── doodle_renderer.h
│   │   ├── doodle_types.h
│   │   └── doodle_version.h
│   ├── src/
│   │   ├── doodle_canvas.c
│   │   ├── doodle_registry.c
│   │   └── doodle_resources.c
│   ├── renderers/
│   │   ├── skia/
│   │   ├── blend2d/
│   │   ├── nanovg/
│   │   └── vello/
│   └── tests/
│
├── examples/
│   ├── common/
│   ├── desktop/
│   ├── android/
│   ├── ios/
│   ├── web/
│   └── headless/
│
├── tests/
│   ├── public_headers/
│   ├── architecture/
│   ├── configuration/
│   └── integration/
│
└── plans/
    └── convert-to-magic-doodle-board.md
```

## Build model

Each layer can be configured, built, installed, and consumed separately.

### Build Board independently

```sh
cmake -S board -B build/board \
  -DBOARD_BACKEND=HEADLESS \
  -DBOARD_BUILD_TESTS=ON
cmake --build build/board --parallel
ctest --test-dir build/board --output-on-failure
cmake --install build/board --prefix build/install
```

### Build Magic against installed Board

```sh
cmake -S magic -B build/magic \
  -DCMAKE_PREFIX_PATH="$PWD/build/install" \
  -DMAGIC_BACKEND=CPU \
  -DMAGIC_BUILD_TESTS=ON
cmake --build build/magic --parallel
ctest --test-dir build/magic --output-on-failure
cmake --install build/magic --prefix build/install
```

### Build Doodle against installed Magic

```sh
cmake -S doodle -B build/doodle \
  -DCMAKE_PREFIX_PATH="$PWD/build/install" \
  -DDOODLE_RENDERER=SKIA \
  -DDOODLE_BUILD_TESTS=ON
cmake --build build/doodle --parallel
ctest --test-dir build/doodle --output-on-failure
cmake --install build/doodle --prefix build/install
```

### Build the composed framework

```sh
cmake -S . -B build \
  -DBOARD_BACKEND=SDL3 \
  -DMAGIC_BACKEND=OPENGL \
  -DDOODLE_RENDERER=SKIA \
  -DMDB_BUILD_EXAMPLES=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Canonical CMake selections are:

```text
BOARD_BACKEND=SDL3|GLFW|WEB|WINIT|ANDROID_NATIVE|IOS_NATIVE|HEADLESS
MAGIC_BACKEND=CPU|OPENGL|METAL|VULKAN|WEB
DOODLE_RENDERER=SKIA|BLEND2D|NANOVG|VELLO
```

The root configuration validates the combination before generating build files. Unsupported combinations produce an error that names the unavailable pair or missing implementation.

## Initial compatibility matrix

The first complete migration preserves these known paths:

```text
Board SDL3          + Magic CPU      + Doodle Skia
Board SDL3          + Magic OpenGL   + Doodle Skia
Board SDL3 on macOS + Magic Metal    + Doodle Skia
Board SDL3 on macOS + Magic Vulkan   + Doodle Skia
Board Android       + Magic CPU      + Doodle Skia
Board Android       + Magic OpenGL ES+ Doodle Skia
Board Android       + Magic Vulkan   + Doodle Skia
Board iOS           + Magic CPU      + Doodle Skia
Board iOS           + Magic OpenGL ES+ Doodle Skia
Board iOS           + Magic Metal    + Doodle Skia
Board Web           + Magic Web      + Doodle Skia
Board Headless      + Magic CPU      + Doodle Skia
```

GLFW, winit, Blend2D, NanoVG, and Vello remain explicit stubs until implemented. Selecting a stub fails clearly at configure time.

## Dependencies

Large rendering dependencies remain external and are not vendored.

- Skia is consumed through a pinned external package.
- The current wasm32 Skia artifact is ABI-compatible with Emscripten 2.0.6; the Web build must keep that toolchain pin until the artifact is rebuilt with a newer Emscripten release.
- Android Skia packages may require matching external libpng and zlib-ng artifacts.
- Renderer-specific dependencies are private to their Doodle adapter and must not leak through installed headers.

## Public ABI rules

All public structs intended for cross-library exchange begin with size and version metadata where future extension is expected:

```c
typedef struct ExampleInterop {
    uint32_t struct_size;
    uint32_t abi_version;
    /* versioned fields follow */
} ExampleInterop;
```

Additional rules:

- use fixed-width integer types for ABI-visible numeric fields;
- use opaque structs for owned objects;
- document ownership of every pointer and callback;
- provide explicit create/destroy or retain/release pairs;
- do not pass C++ exceptions across the C boundary;
- translate backend failures into layer-specific result enums;
- keep symbols hidden by default and export only the documented C API;
- compile public-header smoke tests as C11 and C++17 or newer.

## Completed legacy migration

The former monolithic runtime and its duplicate platform demos have been
removed. Applications now compose `BoardApp`, `MagicContext`, and
`DoodleRenderer` explicitly through the installed public headers. Board owns
hosting and scheduling, Magic owns frame acquisition and presentation, and
Doodle owns the Canvas API and renderer providers.

The detailed, incremental conversion is specified in [`plans/convert-to-magic-doodle-board.md`](plans/convert-to-magic-doodle-board.md).

## Project status

The architecture in this README is the target contract for the conversion from the current `didactic-doodle` layout. Skia is the only renderer expected to be complete at the start of the migration. Stub selections must remain honest and fail clearly until their implementation and tests exist.
