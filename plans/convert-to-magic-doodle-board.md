# Convert didactic-doodle into the three-layer Magic Doodle Board framework

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds.

This plan follows the repository guidance in `AGENTS.md`. A future contributor must be able to resume the work using only the repository working tree and this file. Keep this document self-contained whenever implementation discoveries change the design or commands below.

## Purpose / Big Picture

After this change, the repository will provide a small, independent, C-first graphics framework named **Magic Doodle Board**. A native application will be able to select one Board host backend, one Magic graphics backend, and one Doodle renderer without including or exposing any implementation-library types. Board, Magic, and Doodle will each configure, compile, test, install, and export a CMake package separately, and they will communicate only through versioned public C APIs.

A developer will be able to prove the result in three ways. First, each layer will build by itself against only installed public dependencies. Second, the same backend-agnostic demo scene will run with supported desktop, mobile, web, and headless combinations. Third, an architecture test will show that public headers contain no forbidden SDL, Skia, Emscripten, Android, UIKit, Metal, Vulkan, OpenGL, C++, or Rust types and that no legacy `tc_` or `Tc...` framework symbols remain after the final migration milestone.

The new framework has exactly three public layers. **Board** owns application hosting, windows or embeddable native views, lifecycle, input, events, native surfaces, host services, native-view overlays, and frame scheduling. **Magic** owns CPU or GPU contexts, frame acquisition, render targets, synchronization, resize, device loss, and presentation. **Doodle** owns the public Canvas 2D API and renderer implementations. Skia, Blend2D, NanoVG, and Vello are Doodle renderer providers, not a fourth architectural layer.

## Progress

- [x] (2026-07-14 00:00Z) Captured the current runtime structure, supported combinations, public APIs, frame model, mobile integration requirements, naming goals, and target three-layer architecture in this ExecPlan.
- [x] (2026-07-14) Recorded the source and legacy-symbol baseline under `artifacts/baseline/`; desktop configuration was attempted and its missing Skia artifact recorded as an environment limitation.
- [ ] Inventory all `Tc...`, `tc_...`, `TC_...`, public headers, CMake options, target names, source files, and cross-directory private includes.
- [x] (2026-07-14) Added C11/C++ public-header tests, public-header foreign-type checks, and layer-boundary checks for the new layer trees.
- [x] (2026-07-14) Created independently configurable Board, Magic, and Doodle skeletons with CMake exports; staged standalone installation succeeds for Board Headless, Magic CPU, and Doodle core.
- [ ] Migrate application lifecycle, events, scheduling, surface hosting, and all window backends into Board; add the headless backend.
- [ ] Migrate CPU, OpenGL/OpenGL ES, Metal, Vulkan, and Web contexts into Magic and route all native-surface operations through Board's public capability API.
- [ ] Migrate the Canvas API and renderer lifecycle into Doodle; move Skia and the renderer stubs under Doodle renderer providers.
- [ ] Replace the existing application draw callback with explicit application composition of Board frame callbacks, Magic frames, and Doodle canvases.
- [ ] Add Android and iOS fullscreen-owned, embedded, and hybrid-overlay host modes based on reusable native Board views.
- [ ] Convert the shared demo and all platform entry points to the new public APIs and preserve one common scene across targets.
- [ ] Replace old CMake selections and target names with `BOARD_BACKEND`, `MAGIC_BACKEND`, and `DOODLE_RENDERER`; add compatibility validation and standalone layer builds.
- [ ] Remove temporary compatibility adapters, all framework-owned `tc_`/`Tc...` names, and obsolete source directories after all callers and tests use the new APIs.
- [ ] Complete the supported build matrix, CI updates, installation checks, documentation, and final observable acceptance runs.

## Surprises & Discoveries

- Observation: The current public `TcApp` callback model includes `on_draw(TcCanvas2D *)`, which directly couples application lifecycle to the rendering layer.
  Evidence: The supplied current architecture describes `tc_app_frame()` invoking `tc_renderer_begin_frame()`, `on_draw(TcCanvas2D *)`, and `tc_renderer_end_frame()` as one runtime operation. The new Board frame callback must contain timing only, and application composition must own Magic and Doodle calls.

- Observation: The current Web selection is expressed as SDL in public configuration but privately replaced by an Emscripten adapter.
  Evidence: The supplied Web configuration uses `TC_PLATFORM=WEB` and `TC_BACKEND=SDL` while the implementation substitutes a private Emscripten backend. The target architecture makes Web an explicit Board backend and an explicit Magic backend so configured behavior matches generated behavior.

- Observation: Graphics-context creation currently contains platform-specific variants in the same `src/graphics/` directory, while renderer-specific frame surfaces are created inside Skia code.
  Evidence: Current files include `tc_android_vulkan_context.cpp`, `tc_ios_metal_context.mm`, and a Skia adapter that creates raster, framebuffer, drawable, and swapchain-backed surfaces. The migration must define stable Board-to-Magic and Magic-to-Doodle interop contracts before moving these files.

- Observation: A raw native window pointer is insufficient for strict layer independence when SDL owns OpenGL context creation and Vulkan surface helpers.
  Evidence: SDL requires backend-specific helper calls for several graphics APIs. Board therefore needs a public, versioned surface capability query made of opaque handles and callbacks; Magic must not include Board backend-private SDL headers.

- Observation: Headless support is missing but is the best path for deterministic end-to-end architecture tests.
  Evidence: The supplied backend list omits headless. A manual scheduler, injected events, CPU target, and image hash allow testing Board, Magic, and Doodle without a display server or mobile device.

- Observation: The local development environment has SDL3 available but no configured Skia package or TotalCross Skia archive.
  Evidence: `cmake -S . -B build/baseline-desktop-cpu -DTC_PLATFORM=DESKTOP -DTC_BACKEND=SDL -DTC_GRAPHICS=CPU -DTC_RENDERER=SKIA` stops at the existing Skia package check.

- Observation: `scripts/fetch-totalcross-skia.sh` downloads the matching pinned Skia headers and library to an ignored cache directory.
  Evidence: `bash scripts/fetch-totalcross-skia.sh .cache/skia-158dc9d7-r4 macos-arm64` produced `headers/modules/skia/include/core/SkCanvas.h` and `libskia-macos-arm64.a`; the Headless + CPU + Skia integration test then linked and passed.

Update this section whenever implementation inspection reveals a fact that changes file ownership, API shape, backend compatibility, or validation strategy. Include a concise command result or file reference as evidence.

## Decision Log

- Decision: Use **Magic Doodle Board** as the umbrella project name, while the runtime dependency order is Board, then Magic, then Doodle.
  Rationale: “Magic Doodle Board” is natural English as a product name. The individual words also provide clear layer metaphors without forcing the brand phrase to mirror dependency order.
  Date/Author: 2026-07-14 / initial architecture plan.

- Decision: Keep exactly three public runtime layers and place renderer implementations inside Doodle.
  Rationale: A separate renderer layer would violate the requested three-layer model and would split the Canvas contract from the implementations that realize it. Doodle can contain a core Canvas API plus separately compiled renderer-provider libraries while remaining one architectural layer.
  Date/Author: 2026-07-14 / initial architecture plan.

- Decision: Put application lifecycle, frame scheduling, host services, native-view overlays, and mobile hosting in Board.
  Rationale: These facilities are platform and host integration concerns. Keeping them in Board permits fullscreen, embedded, and hybrid mobile modes without contaminating Magic or Doodle with Activity, Fragment, UIView, or view-controller concepts.
  Date/Author: 2026-07-14 / initial architecture plan.

- Decision: Make the fundamental Android and iOS integration object a reusable native Board view; make activity, fragment, and view-controller wrappers optional conveniences.
  Rationale: A view can be used fullscreen or embedded and can host an overlay container. Making a top-level screen object fundamental would prevent composition inside existing native interfaces.
  Date/Author: 2026-07-14 / initial architecture plan.

- Decision: Remove `on_draw(DoodleCanvas *)` from Board callbacks and perform drawing in application composition code after Board signals a frame.
  Rationale: Board cannot mention Doodle types without creating an upward dependency. Timing callbacks are sufficient for the application to begin a Magic frame and bind a Doodle renderer explicitly.
  Date/Author: 2026-07-14 / initial architecture plan.

- Decision: Use versioned public capability tables between Board and Magic, and versioned frame interop tables between Magic and Doodle.
  Rationale: Opaque raw handles alone cannot cover SDL-managed OpenGL contexts, Vulkan surface creation, Metal layers, Web canvases, CPU pixel presentation, and renderer-specific frame metadata. Versioned C tables preserve separate compilation and prevent foreign API types from leaking into public headers.
  Date/Author: 2026-07-14 / initial architecture plan.

- Decision: Do not create a shared public `common`, `runtime`, or `types` library.
  Rationale: A shared ABI package would become an undeclared fourth layer and blur ownership. Each producer layer defines the public values it exports; consumers include that layer's installed headers.
  Date/Author: 2026-07-14 / initial architecture plan.

- Decision: Use additive APIs and temporary compatibility adapters during migration, then remove all framework-owned `tc_`, `Tc...`, and `TC_...` names before completion.
  Rationale: Large source moves and symbol renames are safer when current builds remain runnable between milestones. The temporary bridge is not part of the final installed API.
  Date/Author: 2026-07-14 / initial architecture plan.

- Decision: Make Web explicit as `BOARD_BACKEND=WEB` and `MAGIC_BACKEND=WEB`.
  Rationale: Public configuration must describe the actual implementation. The Magic Web provider may initially use WebGL 2 privately and later add WebGPU without changing the Board contract.
  Date/Author: 2026-07-14 / initial architecture plan.

- Decision: Add Board Headless as a first-class backend and initially support it with Magic CPU and Doodle Skia.
  Rationale: This combination provides deterministic tests, offscreen rendering, and CI coverage without a native display. Unsupported headless GPU combinations can be added later through separate plans.
  Date/Author: 2026-07-14 / initial architecture plan.

- Decision: Until matching Skia headers and archive are supplied, expose Doodle core with `DOODLE_RENDERER=NONE` for package and provider-contract validation rather than treating a non-Skia implementation as Skia.
  Rationale: Naming a fallback renderer as Skia would be a silent backend substitution and would make configuration claims false. The real `DOODLE_RENDERER=SKIA` remains an explicit configuration failure until the external dependency is valid.
  Date/Author: 2026-07-14 / Codex.

- Decision: Implement the initial Skia provider for Magic CPU frames, sourced from the pinned artifact downloaded by `scripts/fetch-totalcross-skia.sh`.
  Rationale: The provider can make raster-direct Skia surfaces from `MagicCpuInterop` without accessing Magic private state. GPU Skia paths remain deferred until their corresponding Magic interop is implemented.
  Date/Author: 2026-07-14 / Codex.

## Outcomes & Retrospective

2026-07-14: The migration now has an executable lower-layer spine. `board_core`
implements a versioned headless CPU surface and deterministic coalescing frame
scheduler. `magic_core` consumes only Board's installed surface API and exposes
versioned CPU frame interop. `doodle_core` consumes only Magic's public API;
`doodle_renderer_skia` creates raster-direct Skia surfaces from that interop.
All three packages install and are consumable in dependency order. Existing
monolithic sources, GPU Skia paths, desktop/mobile/web backends, shared demos,
and legacy-name removal remain outstanding; this work deliberately does not
represent those as complete.

Validation recorded on 2026-07-14:

    cmake -S board -B build/standalone-board ... && ctest --test-dir build/standalone-board
    cmake -S magic -B build/standalone-magic -DCMAKE_PREFIX_PATH=$PWD/build/install ... && ctest --test-dir build/standalone-magic
    cmake -S doodle -B build/standalone-doodle -DCMAKE_PREFIX_PATH=$PWD/build/install ... && ctest --test-dir build/standalone-doodle
    cmake -S . -B build/root -DBOARD_BACKEND=HEADLESS -DMAGIC_BACKEND=CPU -DDOODLE_RENDERER=NONE && ctest --test-dir build/root

All of the listed CTest runs passed. The installed packages were also consumed
by `tests/integration/consumer`; the Skia package consumer passed
`-DDoodle_SKIA_ROOT=$PWD/.cache/skia-158dc9d7-r4`. The Headless + CPU + Skia
composition drew deterministically with hash `bff7964c10eaa55f`.
`DOODLE_RENDERER=VELLO` was confirmed to fail configuration with the required
explicit unimplemented diagnostic.

At the end of each milestone, append a short entry here describing what is now observable, what remains incomplete, and any design lesson that should guide later milestones. At final completion, compare the actual standalone build commands, supported backend matrix, demo behavior, and ABI checks against the purpose stated above.

## Context and Orientation

The current project is named `didactic-doodle`. It is a C-first, multiplatform 2D runtime whose public headers currently live under `include/tc_runtime/`. Its implementation mixes application runtime code under `src/runtime/`, native window backends under `src/backends/`, graphics contexts under `src/graphics/`, and renderer adapters under `src/renderers/`.

The current public concepts are:

- `TcApp`, which owns application lifecycle and callbacks named `on_event`, `on_update`, `on_draw`, and `on_shutdown`.
- `TcPlatformBackend`, which owns window or surface creation, events, and scheduling.
- `TcFrameScheduler`, which starts, requests, and stops frame callbacks.
- `TcGraphicsContext`, which represents CPU, OpenGL/OpenGL ES, Metal, or Vulkan context state.
- `TcRenderer2D`, which creates and resizes a renderer and begins or ends frames.
- `TcCanvas2D`, which provides basic 2D drawing and state operations.
- `TcEvent`, which represents quit, resize, pointer, key, text, pause, and resume events.

The current source layout is conceptually:

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
      tc_scheduler.c
      tc_event_queue.c

    src/backends/
      sdl/
      android/
      ios/
      web/
      glfw/
      winit/

    src/graphics/
      tc_cpu_context.c
      tc_gl_context.c
      tc_metal_context.mm
      tc_web_gl_context.c
      tc_android_cpu_context.c
      tc_android_gl_context.c
      tc_android_vulkan_context.cpp
      tc_ios_cpu_context.mm
      tc_ios_gl_context.mm
      tc_ios_metal_context.mm

    src/renderers/
      skia/
      nanovg/
      blend2d/
      vello/

    examples/
      demo/
      demo_android/
      demo_ios/
      demo_web/

The current event and frame model is callback-oriented. Native backends convert input into `TcEvent`, dispatch it to the application, and invoke frame callbacks from a platform-native scheduler. The application has no blocking loop. Web uses browser animation frames, Android uses `AChoreographer`, iOS uses `CADisplayLink`, and desktop uses an adapter around its window backend.

The current Skia renderer is the only implemented renderer. It uses raster surfaces for CPU, Ganesh GL for OpenGL and WebGL, Metal surfaces created per drawable, and an Android Vulkan swapchain path. NanoVG and Blend2D are stubs; Vello is a stub or documentation-only path. GLFW and winit are also stubs. Stub selections must remain explicit configuration failures until they are implemented.

The current root build uses these selections:

    TC_PLATFORM=DESKTOP|ANDROID|IOS|WEB
    TC_BACKEND=SDL|ANDROID_NATIVE|IOS_NATIVE|GLFW|WINIT
    TC_GRAPHICS=CPU|OPENGL|METAL|VULKAN
    TC_RENDERER=SKIA|NANOVG|BLEND2D|VELLO

The target build replaces them with layer-owned selections:

    BOARD_BACKEND=SDL3|GLFW|WEB|WINIT|ANDROID_NATIVE|IOS_NATIVE|HEADLESS
    MAGIC_BACKEND=CPU|OPENGL|METAL|VULKAN|WEB
    DOODLE_RENDERER=SKIA|BLEND2D|NANOVG|VELLO

A **backend** is a platform-specific or graphics-API-specific implementation of one layer's public contract. A **native surface capability** is a versioned Board public struct containing opaque handles and callbacks that Magic can use to create or control a graphics surface without including Board backend-private headers. A **frame interop table** is a versioned Magic public struct containing the opaque resources and metadata a Doodle renderer needs to bind one acquired frame. A **renderer provider** is a Doodle implementation library, such as Skia, that implements Doodle's Canvas contract using Magic frame interop.

The target repository structure is:

    board/
      CMakeLists.txt
      include/board/
        board_app.h
        board_backend.h
        board_event.h
        board_host.h
        board_native_view.h
        board_scheduler.h
        board_surface.h
        board_types.h
        board_version.h
      src/
        board_app.c
        board_event_queue.c
        board_scheduler.c
        board_surface.c
      backends/
        sdl3/
        glfw/
        web/
        winit/
        android/
        ios/
        headless/
      tests/

    magic/
      CMakeLists.txt
      include/magic/
        magic_backend.h
        magic_context.h
        magic_frame.h
        magic_interop.h
        magic_types.h
        magic_version.h
      src/
        magic_context.c
        magic_frame.c
        magic_registry.c
      backends/
        cpu/
        opengl/
        metal/
        vulkan/
        web/
      tests/

    doodle/
      CMakeLists.txt
      include/doodle/
        doodle_canvas.h
        doodle_color.h
        doodle_font.h
        doodle_image.h
        doodle_paint.h
        doodle_path.h
        doodle_renderer.h
        doodle_renderer_provider.h
        doodle_types.h
        doodle_version.h
      src/
        doodle_canvas.c
        doodle_registry.c
        doodle_resources.c
      renderers/
        skia/
        blend2d/
        nanovg/
        vello/
      tests/

    examples/
      common/
      desktop/
      android/
      ios/
      web/
      headless/

    tests/
      public_headers/
      architecture/
      configuration/
      integration/

    cmake/
    plans/

Each layer has its own top-level `CMakeLists.txt` and can be configured from its directory. Board installs `BoardConfig.cmake`; Magic finds installed Board and installs `MagicConfig.cmake`; Doodle finds installed Magic and installs `DoodleConfig.cmake`. The root project can add all three source directories for developer convenience, but it must link exported public targets and may not include private source paths across layers.

## Required public naming conversion

Use the owning layer as the prefix for every framework-owned type, function, enum value, file, build option, and target. Perform the following conceptual mapping and extend it consistently after the inventory identifies additional symbols.

Board ownership:

    TcApp                         -> BoardApp
    TcAppConfig                   -> BoardAppConfig
    TcAppCallbacks                -> BoardAppCallbacks
    TcPlatformBackend             -> BoardBackend
    TcFrameScheduler              -> BoardFrameScheduler
    TcEvent                       -> BoardEvent
    TcEventType                   -> BoardEventType
    TcNativeWindowHandle          -> BoardNativeSurface or BoardNativeHandle

    tc_app_*                      -> board_app_*
    tc_backend_*                  -> board_backend_*
    tc_scheduler_*                -> board_scheduler_*
    tc_event_*                    -> board_event_*
    tc_window_*                   -> board_window_*
    tc_native_view_*              -> board_native_view_*

    tc_app.h                      -> board_app.h
    tc_backend.h                  -> board_backend.h
    tc_scheduler.h                -> board_scheduler.h
    tc_event.h                    -> board_event.h

Magic ownership:

    TcGraphicsContext             -> MagicContext
    TcGraphicsConfig              -> MagicConfig
    TcGraphicsBackend             -> MagicBackend
    TcFrame or graphics frame     -> MagicFrame
    graphics surface state        -> MagicSurface or Magic frame interop

    tc_graphics_create            -> magic_context_create
    tc_graphics_destroy           -> magic_context_destroy
    tc_graphics_resize            -> magic_context_resize
    tc_graphics_begin_frame       -> magic_context_begin_frame
    tc_graphics_end_frame         -> magic_context_end_frame
    tc_graphics_*                 -> magic_context_* or magic_frame_*

    tc_graphics.h                 -> magic_context.h, magic_frame.h, and magic_interop.h
    tc_*_context.c/.cpp/.mm       -> magic_*_context.c/.cpp/.mm under the owning Magic backend

Doodle ownership:

    TcRenderer2D                  -> DoodleRenderer
    TcRendererConfig              -> DoodleRendererConfig
    TcCanvas2D                    -> DoodleCanvas
    TcPaint                       -> DoodlePaint
    TcPath                        -> DoodlePath
    TcImage                       -> DoodleImage
    TcFont                        -> DoodleFont

    tc_renderer_*                 -> doodle_renderer_*
    tc_canvas_*                   -> doodle_canvas_*
    tc_paint_*                    -> doodle_paint_*
    tc_path_*                     -> doodle_path_*
    tc_image_*                    -> doodle_image_*
    tc_font_*                     -> doodle_font_*

    tc_renderer.h                 -> doodle_renderer.h
    tc_canvas.h                   -> doodle_canvas.h
    src/renderers/skia/           -> doodle/renderers/skia/
    src/renderers/blend2d/        -> doodle/renderers/blend2d/
    src/renderers/nanovg/         -> doodle/renderers/nanovg/
    src/renderers/vello/          -> doodle/renderers/vello/

Do not mechanically rename `tc_types.h` into a new global type header. Split every type by ownership. Drawing geometry and colors belong to Doodle. Window metrics, input values, and lifecycle states belong to Board. Frame formats, target sizes, and synchronization values belong to Magic. Where two layers exchange a value, the producer layer's public header owns its declaration.

During migration only, place aliases or wrapper functions under `compat/tc_runtime/` and `compat/src/`. Mark them deprecated and exclude them from final installation by default. New implementation code, new tests, and new examples must never use compatibility names. Delete `compat/` before final acceptance.

## Target interfaces and dependencies

The signatures in this section define the required shape. Exact field additions are allowed when implementation inspection proves they are necessary, but changes must be recorded in the Decision Log and must preserve the ownership and foreign-type restrictions.

### Board public interfaces

In `board/include/board/board_event.h`, define a C-compatible `BoardEventType` enum and `BoardEvent` tagged union covering quit, resize, scale change, pointer down/move/up/cancel, scroll, key down/up, text input, composition/IME, focus, pause, resume, and low-memory events. Preserve existing event semantics before adding platform-specific extensions. Include timestamps and pointer identifiers using fixed-width integers.

In `board/include/board/board_app.h`, define opaque `BoardApp` and `BoardWindow` types and callbacks similar to:

    typedef struct BoardAppCallbacks {
        uint32_t struct_size;
        uint32_t abi_version;
        void (*on_start)(void *user_data);
        void (*on_event)(void *user_data, const BoardEvent *event);
        void (*on_update)(void *user_data, double delta_seconds);
        void (*on_frame)(void *user_data,
                         uint64_t timestamp_ns,
                         double delta_seconds);
        void (*on_shutdown)(void *user_data);
    } BoardAppCallbacks;

Board callbacks must not include `DoodleCanvas`, `MagicFrame`, renderer identifiers, or graphics API values.

In `board/include/board/board_scheduler.h`, define opaque `BoardFrameScheduler` and operations for create, destroy, start, request frame, and stop. A requested frame must coalesce with an already pending frame. Headless mode must support deterministic manual stepping with a timestamp supplied by the test.

In `board/include/board/board_surface.h`, define opaque `BoardNativeSurface`, `BoardSurfaceInterfaceId`, a common version header, and versioned interface structs for CPU, OpenGL, Vulkan, Metal, and Web. The public structs may contain `void *`, `uintptr_t`, `uint64_t`, primitive metadata, and function pointers, but may not name native API types.

The OpenGL capability must be sufficient for an SDL3 or native host to create and destroy a context, make or clear it current, swap buffers when Board owns swap, and resolve procedure addresses. The Vulkan capability must report required instance extension names and provide a callback that creates a surface from an opaque Vulkan instance handle and returns an opaque surface handle. The Metal capability must provide an opaque layer or view handle and scale/size information. The CPU capability must provide pixel-buffer presentation or a mapped host buffer contract. The Web capability must identify the browser canvas and context creation path without exposing Emscripten declarations.

In `board/include/board/board_host.h`, define lifecycle-state translation, host-service callbacks, UI-thread dispatch, and owned versus embedded host mode. In `board/include/board/board_native_view.h`, define opaque native-view slots with create/destroy, frame, visibility, rectangular clip, and z-order operations. Keep the C API independent from specific native classes.

Board build targets must be:

    board_core
    board_backend_sdl3
    board_backend_glfw
    board_backend_web
    board_backend_winit
    board_backend_android
    board_backend_ios
    board_backend_headless

Export aliases under the `Board::` namespace. At minimum export `Board::Board` for the core and one named alias per backend. Stubs must create no fake working target; configuration must fail with a message that identifies the unimplemented backend.

### Magic public interfaces

In `magic/include/magic/magic_context.h`, define opaque `MagicContext` and `MagicFrame`, a `MagicBackend` enum containing AUTO, CPU, OPENGL, METAL, VULKAN, and WEB, and a versioned `MagicConfig`. Define create/destroy, resize, begin frame, end frame, backend query, capability query, and device-loss status operations.

A required shape is:

    MagicResult magic_context_create(
        BoardNativeSurface *surface,
        const MagicConfig *config,
        MagicContext **out_context);

    MagicResult magic_context_begin_frame(
        MagicContext *context,
        MagicFrame **out_frame);

    MagicResult magic_context_end_frame(
        MagicContext *context,
        MagicFrame *frame);

`magic_context_create` consumes only Board public declarations. It may query the selected Board surface capability and retain any opaque callback context according to the documented ownership rules. It must not include a Board backend-private header.

In `magic/include/magic/magic_frame.h`, define frame size, scale, target format, color-space metadata, sample count, frame sequence, and validity. In `magic/include/magic/magic_interop.h`, define `MagicInteropId` and versioned CPU, OpenGL, Metal, Vulkan, and Web interop structs. These structs may contain opaque device, queue, command buffer, image, texture, framebuffer, and synchronization handles, but no foreign typedef names.

Doodle renderers must obtain all backend-specific resources by calling `magic_frame_query_interop()` or a context-level equivalent. Skia code may include Skia and graphics API headers privately, then cast opaque fields. It may not reach into a private `MagicContext` definition.

Magic build targets must be:

    magic_core
    magic_backend_cpu
    magic_backend_opengl
    magic_backend_metal
    magic_backend_vulkan
    magic_backend_web

Export aliases under the `Magic::` namespace. `magic/CMakeLists.txt` must work against an installed Board package by using `find_package(Board CONFIG REQUIRED)` when Board is not already present as a source target.

### Doodle public interfaces

In `doodle/include/doodle/doodle_canvas.h`, define opaque `DoodleCanvas` and public drawing operations for clear, save, restore, translate, scale, rotate, matrix concatenation, rectangular and path clipping, line, rectangle, rounded rectangle, circle, path, image, and text drawing. Preserve the existing minimum Canvas behavior and add only the resource APIs needed to represent it cleanly.

In separate headers under `doodle/include/doodle/`, define owned or reference-counted opaque resources for paint, path, image, font, text layout if introduced, shader, and optional picture/display-list support. Document ownership for every pointer passed across the C boundary.

In `doodle/include/doodle/doodle_renderer.h`, define opaque `DoodleRenderer`, a renderer configuration, begin-frame and end-frame operations, resize or context-change behavior, feature queries, and error reporting. A required shape is:

    DoodleResult doodle_renderer_create(
        const DoodleRendererProvider *provider,
        MagicContext *magic,
        const DoodleRendererConfig *config,
        DoodleRenderer **out_renderer);

    DoodleResult doodle_renderer_begin_frame(
        DoodleRenderer *renderer,
        MagicFrame *frame,
        DoodleCanvas **out_canvas);

    DoodleResult doodle_renderer_end_frame(
        DoodleRenderer *renderer,
        DoodleCanvas *canvas);

In `doodle/include/doodle/doodle_renderer_provider.h`, define a versioned Doodle service-provider interface. Each renderer adapter exports a unique provider getter, for example:

    const DoodleRendererProvider *doodle_skia_provider(void);
    const DoodleRendererProvider *doodle_blend2d_provider(void);
    const DoodleRendererProvider *doodle_nanovg_provider(void);
    const DoodleRendererProvider *doodle_vello_provider(void);

The provider interface allows separately compiled renderer libraries without duplicate generic factory symbols. The root composition chooses the configured provider; the backend-agnostic application scene receives a ready `DoodleRenderer` and never includes a renderer-specific header.

Doodle build targets must be:

    doodle_core
    doodle_renderer_skia
    doodle_renderer_blend2d
    doodle_renderer_nanovg
    doodle_renderer_vello

Export aliases under the `Doodle::` namespace. `doodle/CMakeLists.txt` must work against an installed Magic package by using `find_package(Magic CONFIG REQUIRED)` when Magic is not already present as a source target.

### Dependency boundaries

The final include and link rules are:

    Board public and private code
        may use platform dependencies privately
        must not include or link Magic or Doodle

    Magic public headers
        may include Board public headers
        must not include platform graphics headers

    Magic private backends
        may include their graphics API headers
        may link Board public targets
        must not include Board private headers
        must not include Doodle

    Doodle public headers
        may include Magic public headers where required
        must not include renderer implementation headers
        must not include Board directly

    Doodle private renderer providers
        may include their renderer and graphics API headers
        may link Magic public targets
        must not include Magic private headers
        must not include Board

    examples and composition code
        may include Board, Magic, and Doodle public headers
        must not include backend-private headers

Add an architecture test that scans compile commands and source includes for violations. A simpler initial implementation may use a Python script with explicit forbidden path and token rules, but it must run through CTest and report the exact offending file and include.

## Plan of Work

### Milestone 1: Establish a baseline and executable architecture checks

Begin by proving the current state before renaming anything. From the repository root, inspect `CMakeLists.txt`, all included CMake modules, `include/tc_runtime/`, `src/runtime/`, `src/backends/`, `src/graphics/`, `src/renderers/`, and every example entry point. Record the exact source files, public symbols, CMake targets, options, generated files, and current supported combinations in `artifacts/baseline/inventory.md`. Do not assume the supplied conceptual tree matches every filename.

Configure and run every currently working combination available on the development host. At minimum, establish desktop SDL plus Skia CPU and desktop SDL plus Skia OpenGL. On macOS, also establish SDL plus Metal if the current path is working. Build Web if the pinned Emscripten and wasm32 Skia artifacts are available. Record unavailable SDKs or artifacts as environment limitations, not implementation failures.

Add `tests/public_headers/` with tiny C11 and C++ translation units that include every current public header. Add `tests/architecture/check_boundaries.py` that can later enforce new include and naming rules. Initially, make it report findings without failing on known legacy names; make forbidden foreign types in installed public headers fail immediately. Add `tests/configuration/` scripts that configure known valid and invalid combinations and assert the expected result.

At the end of this milestone, the current implementation behaves exactly as before, but there is a concise baseline, public-header compilation coverage, and an architecture checker ready to become stricter as layers migrate.

Acceptance for this milestone is that the current desktop demo runs, CTest executes the new tests, and `artifacts/baseline/inventory.md` contains a reproducible command and result for each tested combination.

### Milestone 2: Create the three package skeletons and versioned boundaries

Create `board/`, `magic/`, and `doodle/` with the target include trees, private source directories, test directories, and independently usable `CMakeLists.txt` files. Add layer-specific result enums, version headers, symbol visibility macros, and the initial opaque public types. Do not move backend implementations yet. Build empty or minimal core libraries that expose version queries and compile as C11 and C++.

Create CMake package configuration and install rules for each layer. Board must configure and install without Magic or Doodle. Magic must configure against an installed Board package. Doodle must configure against an installed Magic package. Add a staged installation smoke script under `tests/integration/install_layers.cmake` or an equivalent test driver that builds and installs Board, then Magic against that prefix, then Doodle against the same prefix.

Define and test Board surface capability headers and Magic frame interop headers before moving implementation code. Write ABI tests that pass a supported `abi_version`, reject an unknown version, reject an undersized output struct, and preserve default values for fields added after an older version. These tests can use fake providers and do not need a real window or GPU.

Add the Doodle renderer-provider interface and a fake test provider that binds a fake Magic frame and records begin/end calls. This proves that renderer providers can be separately compiled before Skia is moved.

At the end of this milestone, the old runtime still drives examples, but the new packages can build and install independently and their cross-layer negotiation contracts are executable.

Acceptance is that the staged install test succeeds from a clean build directory and a test consumer can compile using only installed headers and CMake package exports.

### Milestone 3: Migrate Board and add headless hosting

Move application lifecycle, event queue, frame scheduler, window abstraction, host services, and native backend ownership into Board. Use `git mv` where practical so history remains visible. Rename every migrated public type, function, enum value, source file, and internal symbol to the Board prefix. Update includes to `#include <board/...>`.

Change the application callback contract so Board emits event, update, and frame-timing callbacks only. Remove rendering operations from the Board frame implementation. While old examples still expect `on_draw`, implement a temporary adapter in `compat/` that translates a Board frame callback into the old composition path. Do not put this adapter in Board.

For SDL3, GLFW, Web, winit, Android, iOS, and Headless, place backend code under `board/backends/<name>/`. Preserve stub status for GLFW and winit by making selection fail clearly. Make Web explicit and stop selecting SDL in public CMake configuration for browser builds. Keep browser event conversion and browser animation frames private to `board_backend_web`.

Implement `board_surface_query_interface()` for every working Board backend. SDL3 should provide the operations needed by Magic CPU, OpenGL, Vulkan, and Metal paths without making Magic include SDL headers. Android should provide opaque native window or surface handles, UI-thread dispatch, lifecycle state, and the operations needed by CPU, OpenGL ES, and Vulkan. iOS should provide opaque view/layer handles and the operations needed by CPU, OpenGL ES, and Metal. Web should provide browser canvas/context capability through opaque identifiers and callbacks.

Add `board/backends/headless/` as a real implementation. It creates a logical surface with width, height, scale, and CPU presentation support but no native window. It accepts injected `BoardEvent` values. Its scheduler supports manual deterministic stepping and an optional monotonic timer mode. The initial supported composed path is Headless plus Magic CPU plus Doodle Skia.

Update tests for event conversion, frame coalescing, pause/resume, resize, scale changes, and headless stepping. Make the architecture checker reject Magic or Doodle includes from all Board sources.

At the end of this milestone, Board is independently useful for host lifecycle and timing, current examples can still run through a compatibility composition adapter, and headless Board tests are deterministic.

Acceptance is that `cmake -S board` can build and test SDL3 or Headless without any Magic or Doodle source present, and that a headless scheduler test produces the exact requested timestamps and no extra frames.

### Milestone 4: Migrate graphics contexts into Magic

Move all graphics context files from `src/graphics/` into `magic/backends/` according to backend, then split generic context and frame lifecycle into `magic/src/`. Rename public and private framework-owned symbols and files to the Magic prefix. Keep graphics API names private when they are implementation facts, for example `magic_vulkan_context.cpp` inside the Vulkan backend.

Refactor each backend to create its context only from Board public surface capabilities. Remove direct includes of `src/backends/` headers and any direct reach into Board private structs. If a required operation is missing, extend the versioned Board surface capability, add its ABI tests, update all applicable Board providers, and record the decision here before proceeding.

Implement Magic CPU first against SDL3 and Headless Board CPU presentation. A Magic CPU frame exposes a buffer, row stride, pixel format, dimensions, and lifetime valid until `magic_context_end_frame()`. End frame presents through Board's CPU surface capability.

Implement Magic OpenGL/OpenGL ES next. Board owns the host operations required to create and bind the context when the window system demands that; Magic owns context policy, frame metadata, resize behavior, and the renderer interop exposed to Doodle. The Magic OpenGL frame interop must include framebuffer identifier, dimensions, sample count, origin convention, and any context token needed by the renderer provider without exposing GL typedefs.

Implement Magic Metal against the opaque Board Metal layer capability. Magic owns device, queue, drawable acquisition, and presentation. Expose only opaque handles and metadata through Magic interop. Implement Magic Vulkan against the Board Vulkan surface creation capability. Magic owns instance or receives a documented configured instance, then owns device selection, queues, swapchain, image acquisition, synchronization, resize, and presentation. Preserve the currently working Android Vulkan behavior while moving swapchain ownership out of Skia-specific code and into Magic.

Implement Magic Web explicitly. Initially it may create WebGL 2 privately and expose a Web or OpenGL-compatible frame interop, but configuration and diagnostics must use `MAGIC_BACKEND=WEB`. Preserve the pinned wasm32 renderer/toolchain compatibility until the external Skia artifact changes.

Add tests for begin/end ordering, double end, resize between frames, skipped or unavailable frames, device-loss status, and interop version negotiation. Use fake Board capabilities for unit tests and real Board backends for integration tests.

At the end of this milestone, all frame acquisition and presentation belong to Magic. Skia may still live in the old renderer directory, but it must consume Magic public frames rather than private graphics context definitions.

Acceptance is that Magic builds separately against installed Board, Headless plus CPU frame tests pass, and desktop CPU/OpenGL plus mobile or macOS supported paths preserve visible output through the compatibility renderer adapter.

### Milestone 5: Migrate Canvas and renderer providers into Doodle

Move the portable Canvas API, renderer lifecycle, and renderer-owned resources into Doodle. Split the current `tc_canvas.h`, `tc_renderer.h`, and any renderer-neutral types into `doodle/include/doodle/`. Rename all framework-owned symbols to `Doodle...`, `doodle_...`, and `DOODLE_...`. Ensure drawing geometry, colors, paints, paths, images, fonts, and text resources are owned by Doodle rather than a shared type header.

Move the Skia adapter to `doodle/renderers/skia/`. Replace access to private graphics context structs with calls to `magic_context_get_backend()` and `magic_frame_query_interop()`. The adapter includes Skia, OpenGL, Metal, Vulkan, or Web headers only in private implementation files. CPU creates or binds a raster surface over the Magic CPU frame. OpenGL binds Ganesh to the Magic framebuffer. Metal creates a Skia surface for the Magic drawable or texture. Vulkan binds Skia to the Magic-managed acquired image and synchronization contract rather than owning a private swapchain. Web uses the Magic Web frame and the pinned wasm32 Skia package.

Implement `doodle_skia_provider()` and use the public Doodle renderer-provider interface. Move Blend2D, NanoVG, and Vello directories under Doodle and give each a unique provider getter, but retain honest stub behavior. At configuration time, selecting an unimplemented renderer must fail with a message such as `DOODLE_RENDERER=VELLO is declared but not implemented for this repository revision`. Do not compile no-op renderers that report success.

Define an explicit renderer/backend compatibility function and CMake matrix. Initially document and validate at least these expectations:

    Skia     with CPU, OpenGL/OpenGL ES, Metal, Vulkan, and Web where current artifacts support them.
    Blend2D  with CPU only after its implementation exists.
    NanoVG   with OpenGL/OpenGL ES only after its implementation exists.
    Vello    unsupported until its C adapter and required Magic backend interop exist.

Move all renderer frame begin/end calls out of the old application runtime. The new application composition begins a Magic frame, begins the selected Doodle renderer on that frame, draws, ends Doodle, and ends Magic.

Add Canvas unit tests using the fake provider, Skia headless image tests, state-stack tests, transform and clip tests, text and image smoke tests, and error tests for incompatible Magic/Doodle combinations.

At the end of this milestone, Doodle is the only owner of Canvas and renderer APIs, Skia is a Doodle renderer provider, and there is no separate public renderer layer.

Acceptance is that Doodle builds separately against installed Magic, a headless Skia demo produces the approved deterministic image hash, and supported desktop outputs visually match baseline captures.

### Milestone 6: Implement mobile host composition modes in Board

Refactor Android hosting so the reusable native view is the fundamental object. Place Java or Kotlin host code under `board/backends/android/` in a package such as `org.magicdoodle.board`. Provide `BoardView` as the core embeddable component. It owns or contains the native rendering surface, translates lifecycle and input to Board, schedules frames through `AChoreographer`, and exposes a native overlay container. Keep a minimal NativeActivity or Activity wrapper only as a fullscreen convenience. A Fragment wrapper, if provided, must contain a BoardView rather than duplicate runtime logic.

Refactor iOS hosting similarly. Provide a reusable `BoardView` in the framework module and an optional `BoardViewController` convenience. `BoardView` owns the appropriate layer or raster surface, translates touch and lifecycle, schedules frames through `CADisplayLink`, and contains an overlay container. Keep Objective-C or Swift public names inside the Board platform package while the portable C core receives only opaque handles and callbacks.

Implement explicit Board host modes: FULLSCREEN_OWNED, EMBEDDED, and HYBRID_OVERLAY. Fullscreen is a convenience configuration of the reusable view. Embedded permits arbitrary native constraints or layout sizing. Hybrid overlay allows native controls above or below the rendering surface through `BoardNativeViewSlot`.

Document and enforce slot limitations: position, size, visibility, rectangular clipping, and a limited z-order are supported; arbitrary Doodle transforms, path clipping, perspective, filters, and guaranteed inclusion in renderer screenshots are not. Route native control hit testing through the platform hierarchy. Dispatch lifecycle and view mutations on the UI thread. Keep rendering-thread policy backend-specific and explicit.

Add one Android demo showing a BoardView between native controls and one iOS demo showing the same. Add a hybrid demo or test with one native overlay slot whose bounds are updated from application layout data. These platform demos may be excluded when SDKs are unavailable, but CI configuration and local commands must be documented.

At the end of this milestone, mobile hosts no longer require the framework to own the whole screen, while fullscreen applications still have a convenience path.

Acceptance is that the embedded demos display the shared Doodle scene inside a bounded native view, surrounding native controls remain interactive, resize reaches Magic and Doodle, and a native overlay slot receives input independently of the Doodle surface.

### Milestone 7: Convert examples and root composition

Create `examples/common/` containing all backend-agnostic application state, event handling, update logic, and drawing. It may include only installed Board, Magic, and Doodle public headers. It must not contain SDL, Emscripten, JNI, UIKit, graphics API, Skia, C++, or Rust implementation declarations.

The common example receives or creates a Board app, obtains the Board native surface, creates a Magic context using the configured Magic backend, chooses the configured Doodle provider in composition code, and creates a Doodle renderer. Its Board frame callback follows this exact sequence:

    update application state using delta_seconds
    call magic_context_begin_frame()
    call doodle_renderer_begin_frame()
    invoke common scene drawing with DoodleCanvas
    call doodle_renderer_end_frame()
    call magic_context_end_frame()

Handle unavailable frames without calling Doodle. On resize or scale events, update Magic and any Doodle size-dependent state before the next draw. On pause, stop frame requests and release transient frames. On resume, restore context state as required and request a frame.

Keep platform entry points minimal. Desktop entry chooses the Board backend compiled by CMake. Android and iOS entry points instantiate their Board view wrappers. Web exports the browser host and creates `magic_doodle_board_demo.html`, `.js`, and `.wasm`. Headless renders a fixed number of deterministic frames and writes a PNG or raw image plus a hash.

Delete old example copies only after the common scene runs through every available platform entry point.

At the end of this milestone, one application scene proves that the public APIs, not private implementation details, connect the framework.

Acceptance is that changing the root CMake selections changes Board, Magic, or Doodle implementations without changing any file under `examples/common/`.

### Milestone 8: Replace the build surface, remove legacy names, and complete validation

Update the root `CMakeLists.txt` and modules under `cmake/` to use `BOARD_BACKEND`, `MAGIC_BACKEND`, and `DOODLE_RENDERER`. Detect platform defaults only when the user did not provide an explicit value. Validate unsupported combinations before generating source targets. Do not silently map Web to SDL or fall back from a selected stub.

Provide a transitional CMake compatibility module that recognizes old `TC_*` options, emits a deprecation warning, and maps them to the new values only during intermediate milestones. Remove that module before final completion unless a release policy explicitly requires one compatibility release; if retained, record that policy in the Decision Log and exclude old names from public code symbols and installed headers.

Make each layer install public headers, libraries, version files, and CMake package exports. Add test consumers that use `find_package(Board)`, `find_package(Magic)`, and `find_package(Doodle)` from a clean prefix. Ensure source builds and installed-package builds use the same target names and public include paths.

Enable the architecture checker to fail on all framework-owned `Tc...`, `tc_...`, and legacy `TC_...` names outside intentionally documented third-party or historical artifact files. Remove `include/tc_runtime/`, `src/runtime/`, `src/backends/`, `src/graphics/`, `src/renderers/`, and `compat/` only when their migrated counterparts are complete and no build references remain. Use `git grep`, generated build files, install manifests, exported symbol inspection, and clean builds to prove removal.

Update CI to run focused jobs for public header compilation, boundary checks, standalone staged installation, headless CPU plus Skia, desktop SDL3 CPU plus Skia, desktop SDL3 OpenGL plus Skia, macOS Metal where available, Android CPU/OpenGL ES/Vulkan where available, iOS CPU/Metal where available, and Web. Stub configuration tests run on a lightweight host and assert clear failures.

Update `README.md`, `AGENTS.md`, package documentation, example documentation, and release notes to match actual implemented combinations. Do not advertise stubs as supported.

At the end of this milestone, the repository contains only the new architecture and names, all supported paths are validated, and the project can be consumed layer by layer or as a root-composed build.

Acceptance is the complete set of commands and observable outcomes in the next section.

## Concrete Steps

Run commands from the repository root unless a command explicitly changes the source directory. Update this section as actual target names and environment requirements are confirmed during implementation.

### Baseline inventory

Create a clean branch and inspect status:

    git status --short
    git switch -c refactor/magic-doodle-board

Inventory legacy names and files:

    mkdir -p artifacts/baseline
    rg -n --hidden \
      --glob '!build/**' \
      --glob '!artifacts/**' \
      '\bTc[A-Za-z0-9_]*|\btc_[A-Za-z0-9_]*|\bTC_[A-Za-z0-9_]*' \
      . > artifacts/baseline/legacy-symbols.txt

    find include src examples cmake -type f -print | sort \
      > artifacts/baseline/files.txt

Configure the current desktop CPU path using the existing options, adapting only if inspection proves an option spelling differs:

    cmake -S . -B build/baseline-desktop-cpu \
      -DTC_PLATFORM=DESKTOP \
      -DTC_BACKEND=SDL \
      -DTC_GRAPHICS=CPU \
      -DTC_RENDERER=SKIA \
      -DCMAKE_BUILD_TYPE=Release
    cmake --build build/baseline-desktop-cpu --parallel \
      > artifacts/baseline/desktop-cpu-build.log 2>&1
    ctest --test-dir build/baseline-desktop-cpu --output-on-failure

Repeat for desktop OpenGL and any host-supported Metal path. Run the demo and save one screenshot or deterministic output reference per path.

### Focused layer builds after skeleton creation

Build and install Board Headless independently:

    rm -rf build/standalone-board build/install
    cmake -S board -B build/standalone-board \
      -DBOARD_BACKEND=HEADLESS \
      -DBOARD_BUILD_TESTS=ON \
      -DCMAKE_BUILD_TYPE=Debug
    cmake --build build/standalone-board --parallel
    ctest --test-dir build/standalone-board --output-on-failure
    cmake --install build/standalone-board --prefix "$PWD/build/install"

Expected concise result:

    100% tests passed, 0 tests failed
    -- Installing: .../build/install/include/board/board_app.h
    -- Installing: .../build/install/lib/cmake/Board/BoardConfig.cmake

Build and install Magic CPU against only the installed Board package:

    rm -rf build/standalone-magic
    cmake -S magic -B build/standalone-magic \
      -DCMAKE_PREFIX_PATH="$PWD/build/install" \
      -DMAGIC_BACKEND=CPU \
      -DMAGIC_BUILD_TESTS=ON \
      -DCMAKE_BUILD_TYPE=Debug
    cmake --build build/standalone-magic --parallel
    ctest --test-dir build/standalone-magic --output-on-failure
    cmake --install build/standalone-magic --prefix "$PWD/build/install"

Build and install Doodle Skia against only installed public packages:

    rm -rf build/standalone-doodle
    cmake -S doodle -B build/standalone-doodle \
      -DCMAKE_PREFIX_PATH="$PWD/build/install" \
      -DDOODLE_RENDERER=SKIA \
      -DDOODLE_BUILD_TESTS=ON \
      -DCMAKE_BUILD_TYPE=Debug
    cmake --build build/standalone-doodle --parallel
    ctest --test-dir build/standalone-doodle --output-on-failure
    cmake --install build/standalone-doodle --prefix "$PWD/build/install"

The standalone source directories must not refer to sibling private source paths. Prove this with the architecture test and by temporarily copying or checking out each source directory without the others if the staged package test does not already isolate them.

### Root composed builds

Headless deterministic path:

    rm -rf build/headless
    cmake -S . -B build/headless \
      -DBOARD_BACKEND=HEADLESS \
      -DMAGIC_BACKEND=CPU \
      -DDOODLE_RENDERER=SKIA \
      -DMDB_BUILD_EXAMPLES=ON \
      -DMDB_BUILD_TESTS=ON
    cmake --build build/headless --parallel
    ctest --test-dir build/headless --output-on-failure
    ./build/headless/examples/headless/magic_doodle_board_headless \
      --frames 3 \
      --output build/headless/demo.png

The command must report a stable image hash recorded by the approved test. If Skia or font versions make cross-host pixel identity impractical, use a repository-pinned font and a tolerance-based image comparison; record that decision and evidence in this plan.

Desktop SDL3 plus CPU:

    rm -rf build/desktop-cpu
    cmake -S . -B build/desktop-cpu \
      -DBOARD_BACKEND=SDL3 \
      -DMAGIC_BACKEND=CPU \
      -DDOODLE_RENDERER=SKIA \
      -DMDB_BUILD_EXAMPLES=ON
    cmake --build build/desktop-cpu --parallel
    ctest --test-dir build/desktop-cpu --output-on-failure
    ./build/desktop-cpu/examples/desktop/magic_doodle_board_demo

Observe a resizable window containing the shared scene. Pointer, key, text, resize, pause or focus where applicable, update timing, and drawing must work.

Desktop SDL3 plus OpenGL:

    rm -rf build/desktop-opengl
    cmake -S . -B build/desktop-opengl \
      -DBOARD_BACKEND=SDL3 \
      -DMAGIC_BACKEND=OPENGL \
      -DDOODLE_RENDERER=SKIA \
      -DMDB_BUILD_EXAMPLES=ON
    cmake --build build/desktop-opengl --parallel
    ctest --test-dir build/desktop-opengl --output-on-failure
    ./build/desktop-opengl/examples/desktop/magic_doodle_board_demo

The displayed scene and interaction must match the CPU path. Logs may report selected backend names but must not expose implementation types to the application.

Web:

    emcmake cmake -S . -B build/web \
      -DBOARD_BACKEND=WEB \
      -DMAGIC_BACKEND=WEB \
      -DDOODLE_RENDERER=SKIA \
      -DMDB_BUILD_EXAMPLES=ON
    cmake --build build/web --parallel

Expected output files are:

    build/web/examples/web/magic_doodle_board_demo.html
    build/web/examples/web/magic_doodle_board_demo.js
    build/web/examples/web/magic_doodle_board_demo.wasm

Serve the directory through a local HTTP server, open the HTML file, and verify resize, mouse, touch where available, keyboard, and animation-frame scheduling. The application must not run a blocking loop.

### Unsupported selection checks

Run one configure command per declared stub and expect a nonzero result with an exact explanatory message. For example:

    cmake -S . -B build/invalid-vello \
      -DBOARD_BACKEND=HEADLESS \
      -DMAGIC_BACKEND=CPU \
      -DDOODLE_RENDERER=VELLO

Expected message shape:

    DOODLE_RENDERER=VELLO is declared but not implemented for this repository revision

Likewise test GLFW, winit, Blend2D, and NanoVG until they are actually implemented. Once a stub becomes implemented, replace its failure test with a working smoke test and update this plan.

### Final naming and public-header checks

Compile installed headers as C and C++ through CTest. Then scan framework-owned sources:

    rg -n --hidden \
      --glob '!build/**' \
      --glob '!artifacts/**' \
      --glob '!third_party/**' \
      '\bTc[A-Za-z0-9_]*|\btc_[A-Za-z0-9_]*|\bTC_(PLATFORM|BACKEND|GRAPHICS|RENDERER)\b' \
      .

Expected result: no matches.

Scan installed public headers for forbidden implementation names:

    rg -n \
      'SDL|GLFW|Emscripten|JNIEnv|jobject|UIKit|UIView|CAMetal|MTL|Vk[A-Z]|GL[A-Z]|Sk[A-Z]|std::|rust::' \
      build/install/include/board \
      build/install/include/magic \
      build/install/include/doodle

Expected result: no matches. If an ordinary English comment creates a false positive, tighten the checker to tokens or declarations rather than weakening the architectural rule.

Inspect exported C symbols using the platform's symbol tool. On ELF systems use `nm -D`; on macOS use `nm -gU`; on Windows use `dumpbin /exports`. The public symbol set must contain only documented `board_`, `magic_`, and `doodle_` entries plus required platform runtime symbols.

## Validation and Acceptance

The conversion is accepted only when all statements below are demonstrated by commands or human-observable behavior and concise evidence is recorded in `artifacts/final/`.

1. Board configures, builds, tests, installs, and can be consumed without Magic or Doodle source directories. Its installed public headers compile as C11 and C++ and contain no foreign implementation types.

2. Magic configures, builds, tests, installs, and can be consumed against only an installed Board package. No Magic source includes Board private headers. CPU frame tests prove buffer lifetime and presentation. Backend-specific tests prove supported OpenGL/OpenGL ES, Metal, Vulkan, and Web paths where the platform is available.

3. Doodle configures, builds, tests, installs, and can be consumed against only installed Magic and Board packages. No Doodle source includes Magic private headers or Board headers. Skia obtains frame resources only through Magic public interop queries.

4. The root project exposes `BOARD_BACKEND`, `MAGIC_BACKEND`, and `DOODLE_RENDERER`. Explicit unsupported selections fail clearly. Web is never described publicly as SDL. Headless is selectable.

5. The shared demo source under `examples/common/` is unchanged across backend selections and contains no backend implementation includes. Desktop SDL3 plus CPU and desktop SDL3 plus OpenGL show the same scene and accept the same input. macOS Metal, Android, iOS, and Web preserve the same scene when their toolchains are available.

6. The headless CPU plus Skia demo renders a deterministic approved image or passes a documented tolerance comparison. It runs in CI without a display server.

7. Android and iOS expose reusable Board views. A fullscreen wrapper uses that view. An embedded demo places the view among native controls. A hybrid-overlay demonstration positions at least one native view slot and preserves independent hit testing.

8. Board frame callbacks contain timing and events only. Application composition explicitly begins Magic, begins Doodle, draws, ends Doodle, and ends Magic. Board does not call Doodle and Doodle does not present.

9. The repository contains no framework-owned public `Tc...` types, `tc_...` functions or files, or old build selections. The final install prefix contains only `board/`, `magic/`, and `doodle/` public include trees and their CMake package files.

10. CI and local full-matrix validation pass for every combination documented as implemented. Stubs are documented as stubs and are tested to fail clearly.

Do not treat compilation alone as acceptance. Run the demos, interact with windowed and embedded hosts, inspect headless output, resize surfaces, pause and resume mobile hosts, and confirm the visible or recorded behavior.

## Idempotence and Recovery

All configure commands use disposable `build/` subdirectories and can be repeated after deleting the corresponding directory. Installation uses `build/install`, not a system prefix. Never overwrite a developer's global CMake packages during this plan.

Use additive migration and compatibility adapters to keep the tree buildable. Before deleting a legacy file or directory, prove that `rg`, CMake dependency graphs, compile commands, and install manifests no longer reference it. Use `git mv` for file relocation when the content remains substantially the same. If a source move fails halfway, reset only the affected paths or restore them from Git, then repeat the move; do not use destructive repository-wide reset commands when unrelated work exists.

Keep baseline artifacts under `artifacts/baseline/` and final evidence under `artifacts/final/`. Generated binaries and large logs remain ignored. Preserve concise summaries and hashes if the repository policy permits tracked artifacts; otherwise reference local paths in this living plan during implementation and remove them before final commit.

If a new public capability is required between layers, first add a new versioned field or interface version and tests. Do not make a private cross-layer include as a temporary shortcut. If the current ABI cannot be extended safely, add a new interface identifier while preserving old behavior until all providers and consumers migrate.

If a platform SDK is unavailable, complete portable code, fake-provider tests, CMake configuration, and documented commands. Mark the platform validation item incomplete in Progress rather than claiming success. Do not silently remove the platform from the target architecture.

If the current pinned Web renderer artifact cannot work with a modern Emscripten version, retain the known compatible Emscripten 2.0.6 pin for the migration. A toolchain upgrade is a separate change unless rebuilding the renderer artifact is required to complete the new architecture; if so, record the decision and validation evidence here.

## Artifacts and Notes

Keep concise evidence that proves architectural behavior rather than retaining verbose build output in the plan. Useful artifacts include:

    artifacts/baseline/inventory.md
    artifacts/baseline/legacy-symbols.txt
    artifacts/baseline/desktop-cpu-build.log
    artifacts/baseline/desktop-cpu.png
    artifacts/baseline/desktop-opengl.png

    artifacts/final/standalone-install-summary.txt
    artifacts/final/public-header-check.txt
    artifacts/final/boundary-check.txt
    artifacts/final/headless-image-hash.txt
    artifacts/final/backend-matrix.md
    artifacts/final/exported-symbols.txt

As work proceeds, add short excerpts here only when they explain a discovery or prove a milestone. Avoid pasting full build logs. For a successful staged install, an appropriate excerpt is:

    Board standalone tests: 100% passed
    Magic standalone tests: 100% passed
    Doodle standalone tests: 100% passed
    Consumer configure: found Board, Magic, Doodle from build/install
    Consumer run: rendered 3 headless frames, hash <approved-value>

For an architecture failure, record the exact diagnostic shape expected from the checker:

    boundary violation: doodle/renderers/skia/skia_surface.cc includes
    magic/src/magic_context_private.h; Doodle may include only Magic public headers

The checker must provide enough information to fix the violation without rerunning with verbose tracing.

## Final target build matrix

The migration must preserve or establish these initially implemented combinations unless baseline inspection proves one was never functional. Record any correction in the Decision Log.

    Board SDL3          + Magic CPU       + Doodle Skia
    Board SDL3          + Magic OpenGL    + Doodle Skia
    Board SDL3 on macOS + Magic Metal     + Doodle Skia
    Board Android       + Magic CPU       + Doodle Skia
    Board Android       + Magic OpenGL ES + Doodle Skia
    Board Android       + Magic Vulkan    + Doodle Skia
    Board iOS           + Magic CPU       + Doodle Skia
    Board iOS           + Magic OpenGL ES + Doodle Skia
    Board iOS           + Magic Metal     + Doodle Skia
    Board Web           + Magic Web       + Doodle Skia
    Board Headless      + Magic CPU       + Doodle Skia

GLFW, winit, Blend2D, NanoVG, and Vello remain explicit stubs at the start of this plan. Headless GPU contexts are outside the initial scope. Runtime backend auto-selection may be added after explicit selections are stable; it must not replace configuration validation or silently hide unsupported paths.

## Revision note

2026-07-14: Created the initial self-contained ExecPlan to convert the current `didactic-doodle` runtime into the independent Board, Magic, and Doodle layer architecture. The plan resolves renderer ownership, mobile embedding, headless support, Web naming, cross-layer ABI negotiation, prefix migration, standalone builds, and observable validation so implementation can proceed without relying on prior conversation context.
