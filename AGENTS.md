# AGENTS.md

## Project identity

This repository contains **Magic Doodle Board**, a small, C-first, cross-platform application and 2D graphics framework. The framework is divided into exactly three architectural layers:

1. **Board** owns application hosting, windows, native surfaces, lifecycle, input, event delivery, frame scheduling, and native-view hosting.
2. **Magic** owns graphics devices and contexts, frame acquisition, render targets, synchronization, resize, device loss, and presentation.
3. **Doodle** owns the public Canvas 2D API and renderer implementations such as Skia, Blend2D, NanoVG, and Vello.

The runtime stack is `Board -> Magic -> Doodle`, while application code orchestrates the frame and depends only on public C APIs.

## Architectural invariants

Preserve these rules in every change:

- Board, Magic, and Doodle are separate libraries with separate public include trees, build targets, tests, install rules, and package exports.
- A layer may communicate with the layer below it only through that layer's installed public headers and exported targets.
- Board must not include or link Magic or Doodle.
- Magic may depend on Board's public API, but must not include Board private headers or backend implementation files.
- Doodle may depend on Magic's public API, but must not include Magic private headers or backend implementation files.
- Doodle must not depend directly on Board.
- The root project is a composition and packaging superproject, not a fourth runtime layer.
- Renderer implementations belong to Doodle. Do not create a fourth public `renderer` layer.
- No public header may expose SDL, GLFW, Emscripten, JNI, Java, Objective-C, UIKit, Metal, Vulkan, OpenGL, Skia, C++, or Rust types. Use C structs, enums, fixed-width integers, primitive values, opaque pointers, opaque handles, and callbacks.
- Public headers must compile as both C11 and C++ without requiring implementation-language headers.
- The application must not contain a blocking event loop. Native polling and platform loops remain private backend details.
- Web frames use the browser frame callback, Android frames use `AChoreographer`, iOS frames use `CADisplayLink`, and desktop adapters translate their native loop into Board callbacks.
- The same backend-agnostic demo scene must run on desktop, mobile, web, and headless targets.
- Unsupported backend combinations must fail during configuration with a clear message. Never silently substitute a different backend.

## Public naming rules

Use the owning layer as the only C symbol and file prefix.

- Board types use `Board...`; Board functions use `board_...`; Board files use `board_....h` and `board_....c`.
- Magic types use `Magic...`; Magic functions use `magic_...`; Magic files use `magic_....h` and `magic_....c`, `.cc`, or `.mm`.
- Doodle types use `Doodle...`; Doodle functions use `doodle_...`; Doodle files use `doodle_....h` and `doodle_....c`, `.cc`, `.mm`, or implementation-specific extensions.
- Enum values use `BOARD_...`, `MAGIC_...`, or `DOODLE_...`.
- Preprocessor definitions and include guards use the same owning-layer prefix.
- Do not add new `tc_` symbols, `Tc...` types, or `tc_...` files. During the migration, compatibility aliases may exist only in an explicitly temporary compatibility directory and must not be used by new code.
- Do not create a generic `common`, `core_types`, or `runtime` ABI shared by all three layers. Each layer owns its public types. Cross-layer values are defined by the producer layer's public API.

Examples:

    BoardEvent
    board_window_create()
    board_event.h

    MagicContext
    magic_context_begin_frame()
    magic_context.h

    DoodleCanvas
    doodle_canvas_draw_rect()
    doodle_canvas.h

## Layer responsibilities

### Board

Board owns:

- application lifecycle and callback dispatch;
- logical windows and native hosting surfaces;
- window size, scale, orientation, monitors, focus, and visibility;
- keyboard, text input, mouse, touch, pen, and gamepad events;
- IME, clipboard, cursor, drag and drop, and accessibility hooks where implemented;
- the frame scheduler and `request_frame` behavior;
- fullscreen-owned, embedded, and hybrid-overlay mobile hosting modes;
- native-view slots and host-service callbacks;
- the public surface capability interface consumed by Magic;
- backends for SDL3, GLFW, Web, winit, native Android, native iOS, and headless operation.

Board does not own a graphics device, GPU command submission, a Canvas API, or renderer resources.

### Magic

Magic owns:

- backend selection for CPU, OpenGL/OpenGL ES, Metal, Vulkan, and Web;
- graphics device and context creation;
- surface and swapchain management;
- frame acquisition and presentation;
- render-target dimensions, formats, sample counts, and color-space metadata;
- resize, synchronization, device loss, and recovery;
- stable public interop tables that Doodle renderers query without exposing native graphics API types.

Magic does not own paths, paints, text shaping, Canvas commands, widgets, or application events.

### Doodle

Doodle owns:

- `DoodleCanvas` and its state stack;
- geometry, paths, paints, images, fonts, text, gradients, shaders, clipping, transforms, blend modes, filters, and pictures/display lists where supported;
- renderer lifecycle and frame binding;
- Skia, Blend2D, NanoVG, and Vello renderer adapters;
- feature and compatibility reporting for renderer/backend combinations.

Doodle does not create windows, poll events, schedule frames, or present a swapchain.

## Frame contract

Keep the frame sequence explicit and observable:

    Board backend or host callback
        -> Board frame callback(timestamp_ns, delta_seconds)
        -> application update
        -> magic_context_begin_frame()
        -> doodle_renderer_begin_frame()
        -> application draw using DoodleCanvas
        -> doodle_renderer_end_frame()
        -> magic_context_end_frame()
        -> presentation

Board must never call Doodle directly. Doodle must never present a frame directly. The application or demo composition code coordinates the three public APIs.

## Surface and renderer interoperability

Board and Magic communicate through a versioned Board surface capability API. Magic and Doodle communicate through versioned Magic frame and backend interop APIs.

- Interop structs must start with `struct_size` and `abi_version` fields.
- Query functions must reject unsupported interface versions cleanly.
- Opaque native handles may use `void *` or fixed-width integer storage, but public declarations must not name foreign API types.
- Backend-specific casts belong only in private implementation files.
- Adding a new backend-specific interop table requires tests for version mismatch, missing capability, and successful negotiation.

## Mobile hosting

The fundamental mobile integration unit is a native view, not an activity or view controller.

- Android exposes a reusable `BoardView`; an optional activity or fragment wrapper may host it.
- iOS exposes a reusable `BoardView`; an optional view-controller wrapper may host it.
- Fullscreen-owned mode is a convenience built on the embeddable view.
- Embedded mode allows the Board view to occupy any native layout region.
- Hybrid-overlay mode adds a native overlay container for maps, camera previews, web views, text controls, and similar system components.
- Native view slots support position, size, visibility, rectangular clipping, and a documented z-order. Do not promise arbitrary Doodle transforms or filters over native controls.
- All native view and lifecycle operations execute on the platform UI thread.

## Backend status and compatibility

Keep backend availability explicit in CMake and documentation.

Board choices:

- SDL3
- GLFW
- Web
- winit
- native Android
- native iOS
- headless

Magic choices:

- CPU
- OpenGL/OpenGL ES
- Metal
- Vulkan
- Web

Doodle renderer choices:

- Skia
- Blend2D
- NanoVG
- Vello

A backend listed as a stub must configure-fail with a message that names the missing implementation. Do not add placeholder success paths.

## Repository layout

Place code under the owning layer:

    board/include/board/
    board/src/
    board/backends/
    board/tests/

    magic/include/magic/
    magic/src/
    magic/backends/
    magic/tests/

    doodle/include/doodle/
    doodle/src/
    doodle/renderers/
    doodle/tests/

    examples/
    cmake/
    plans/

Private headers belong under the corresponding layer's `src` or backend directory and must not be installed.

## Build-system rules

- Each layer must configure and build from its own directory.
- Each layer must install its public headers and export a CMake package.
- The root CMake project may use `add_subdirectory` for a source build, but downstream layers must depend on exported public targets rather than source paths.
- Canonical target families are `board_core` and `board_backend_*`, `magic_core` and `magic_backend_*`, and `doodle_core` and `doodle_renderer_*`.
- Canonical root selections are `BOARD_BACKEND`, `MAGIC_BACKEND`, and `DOODLE_RENDERER`.
- Platform detection must not silently override an explicit selection.
- Keep third-party dependencies external. Do not vendor Skia or other large renderers unless a separate approved plan changes this policy.

## Testing requirements

Every architectural change must add or update tests at the narrowest useful level.

At minimum, preserve:

- public-header C and C++ compile tests;
- ABI/version negotiation tests for cross-layer interfaces;
- layer-boundary checks that reject private or forbidden includes;
- unit tests for event conversion, scheduler state, resize, and frame lifecycle;
- headless CPU rendering tests with deterministic pixel or image-hash output;
- one shared demo scene exercised across supported platform combinations;
- configuration tests proving unsupported selections fail clearly;
- platform smoke tests where the relevant SDK and runner are available.

Run focused tests after each edit. Run the full supported matrix before completing a milestone. Prefer quiet build output and preserve full logs in files when diagnosis is needed.

## Change discipline

- Inspect the current implementation before renaming or moving code.
- Make additive changes first, migrate callers, validate, and only then delete legacy paths.
- Keep the tree buildable at the end of every milestone.
- Do not mix broad formatting changes with architectural moves.
- Preserve history with `git mv` where practical.
- Update README, package exports, examples, tests, and CI whenever a public name or build option changes.
- Record any necessary deviation from these rules in the active ExecPlan's Decision Log before implementing it.

## ExecPlans

For significant refactors, new backends, public ABI changes, or work expected to span multiple sessions, create and maintain an ExecPlan under `plans/`. Follow the structure demonstrated by `plans/convert-to-magic-doodle-board.md`.

An ExecPlan is a living, self-contained implementation document. Keep `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` current. Do not stop at a design-only result: every plan must define commands and observable acceptance behavior. Resolve ordinary implementation ambiguities inside the plan instead of waiting for user direction.
