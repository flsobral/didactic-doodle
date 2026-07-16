# Convert didactic-doodle into the three-layer Magic Doodle Board framework

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, `Outcomes & Retrospective`, and `Editorial Report` must be kept up to date as work proceeds.

This plan follows the repository guidance in `AGENTS.md` and `.agent/PLANS.md`. A future contributor must be able to resume the work using only the repository working tree and this file. Keep this document self-contained whenever implementation discoveries change the design or commands below.

## Purpose / Big Picture

After this change, the repository will provide a small, independent, C-first graphics framework named **Magic Doodle Board**. A native application will be able to select one Board host backend, one Magic graphics backend, and one Doodle renderer without including or exposing any implementation-library types. Board, Magic, and Doodle will each configure, compile, test, install, and export a CMake package separately, and they will communicate only through versioned public C APIs.

A developer will be able to prove the result in three ways. First, each layer will build by itself against only installed public dependencies. Second, the same backend-agnostic demo scene will run with supported desktop, mobile, web, and headless combinations. Third, an architecture test will show that public headers contain no forbidden SDL, Skia, Emscripten, Android, UIKit, Metal, Vulkan, OpenGL, C++, or Rust types and that no legacy `tc_` or `Tc...` framework symbols remain after the final migration milestone.

The new framework has exactly three public layers. **Board** owns application hosting, windows or embeddable native views, lifecycle, input, events, native surfaces, host services, native-view overlays, and frame scheduling. **Magic** owns CPU or GPU contexts, frame acquisition, render targets, synchronization, resize, device loss, and presentation. **Doodle** owns the public Canvas 2D API and renderer implementations. Skia, Blend2D, NanoVG, and Vello are Doodle renderer providers, not a fourth architectural layer.

## Progress

- [x] (2026-07-14 00:00Z) Captured the current runtime structure, supported combinations, public APIs, frame model, mobile integration requirements, naming goals, and target three-layer architecture in this ExecPlan.
- [x] (2026-07-14) Recorded the source and legacy-symbol baseline under `artifacts/baseline/`; desktop configuration was attempted and its missing Skia artifact recorded as an environment limitation.
- [x] (2026-07-15) Inventoried all `Tc...`, `tc_...`, `TC_...`, public headers, CMake options, target names, source files, and cross-directory private includes under `artifacts/baseline/`; the architecture checks now reject retired framework names and private cross-layer includes in active sources.
- [x] (2026-07-14) Added C11/C++ public-header tests, public-header foreign-type checks, and layer-boundary checks for the new layer trees.
- [x] (2026-07-15) Added root configuration-matrix tests that confirm the valid Headless + CPU selection and explicit diagnostics for unsupported backend or renderer selections.
- [x] (2026-07-14) Created independently configurable Board, Magic, and Doodle skeletons with CMake exports; staged standalone installation succeeds for Board Headless, Magic CPU, and Doodle core, and the SDL3 Metal Board → Magic Metal → Doodle Skia package chain now configures and tests on macOS.
- [x] (2026-07-15) Migrated application lifecycle, events, scheduling, and surface hosting into Board for every implemented host: Headless, SDL3, native Android, native iOS, and Web. GLFW and winit remain explicit configuration-fail stubs rather than silent substitutions.
- [x] (2026-07-15) Migrated the supported Magic contexts and routed their native-surface operations through Board's public capability API: CPU for Headless, SDL3, Android, and iOS; OpenGL/OpenGL ES for SDL3 macOS, Android, and iOS; Metal for SDL3 macOS and iOS; Android Vulkan; and WebGL2.
- [x] (2026-07-15) Implemented and validated Board SDL3 + Magic Vulkan + Doodle Skia on macOS with Vulkan SDK 1.4.350.1, MoltenVK, and Skia r5. The named smoke script runs three frames under `VK_LAYER_KHRONOS_validation`; Windows and Linux remain unvalidated until their own runners execute it.
- [x] (2026-07-15) Migrated the portable Canvas API, renderer lifecycle, and Skia provider into Doodle; the active demos draw only through `DoodleCanvas`.
- [x] (2026-07-15) Added provider-owned Blend2D, NanoVG, and Vello stubs under Doodle. Their getters return unavailable, CMake selections fail explicitly, and focused getter/configuration tests prevent a silent no-op renderer.
- [x] (2026-07-15) Replaced the application draw callback with explicit composition of Board frame callbacks, Magic frames, and Doodle canvases in every active demo; the legacy callback runtime was removed.
- [x] (2026-07-15) Added public runtime identity queries for the active Board backend, Magic context, and Doodle renderer. The shared scene displays their names and versions without consulting application build macros.
- [x] (2026-07-15) Added and executed `scripts/test-backend-matrix.sh build-all`, which compiled all 12 supported matrix entries without requiring a booted mobile runner or opening the Web demo; the individual commands retain their launch smoke tests.
- [x] (2026-07-15) Made Android build artifacts observable in the matrix build directory: each Gradle variant is copied to its own `build/android/native-<backend>-skia/magic_doodle_board_android_<backend>_demo.apk` path instead of being overwritten by the next variant.
- [x] (2026-07-15) Normalized matrix build paths to `build/<platform>/<board>-<magic>-<renderer>` and re-ran `build-all`; for example, desktop Vulkan is `build/desktop/sdl3-vulkan-skia` and iOS Metal is `build/ios/native-metal-skia`.
- [x] (2026-07-16) Added Android and iOS fullscreen-owned, embedded, and hybrid-overlay host modes based on reusable native Board views. Both native Board views support embedded hybrid overlays above the renderer; below-renderer ordering remains explicitly unavailable. The current-tree build matrix compiled all six mobile renderer entries; fresh visible device acceptance remains part of the final acceptance item.
- [x] (2026-07-15) Converted the shared demo and every supported platform entry point to the new public APIs around `examples/common/magic_doodle_board_scene.c`; removed the unbuilt duplicate legacy demos.
- [x] (2026-07-15) Replaced old CMake selections and target names with `BOARD_BACKEND`, `MAGIC_BACKEND`, and `DOODLE_RENDERER`; standalone layer builds and the iOS convenience entry use only the new selections.
- [x] (2026-07-15) Removed temporary compatibility adapters, all framework-owned `tc_`/`Tc...` names, and obsolete legacy source directories after verifying no build references remained.
- [x] (2026-07-16) Obtained clean Android CPU, OpenGL ES, and Vulkan visual smoke evidence on the arm64 AVD. Each named script now waits for the real Activity window before capturing; the inspected images show the requested runtime identity and the above-renderer native overlay. Updated hosted CI still awaits its external runners.
- [ ] Execute the updated CI on its hosted runners. Commits `db2a1e8`, `430deac`, and `2730a03` were pushed to `main` after correcting the previous runner diagnostics. The Windows correction fetches in Git Bash, verifies the staged `SkCanvas.h` and `libskia-windows-x64.lib`, and supplies PowerShell CMake with `GITHUB_WORKSPACE` paths; its artifact staging was reproduced locally. GitHub Actions API requests for the commit still return HTTP 503, so no hosted result is claimable yet. The local full build matrix, package-install chain, consumer, documentation update, all iOS simulator smokes, Android CPU/OpenGL ES/Vulkan AVD smokes, all desktop smoke scripts, and the Web HTTP/browser smoke are complete; compilation alone does not close hosted CI acceptance.
- [x] (2026-07-14) Added named smoke-test scripts for every currently supported matrix combination. `test-headless-cpu-skia.sh` and `test-android-opengl-skia.sh` were executed locally; the latter installed, launched, and captured the visible emulator scene.
- [x] (2026-07-14) Migrated Board Web + Magic Web + Doodle Skia. The Emscripten 2.0.6 build produced the browser demo and Safari completed an eight-second smoke run over a local HTTP server; `scripts/test-web-skia.sh` records its generated artifacts.
- [x] (2026-07-15) Converted `ios/CMakeLists.txt` into an iOS-only convenience entry point for the root Board + Magic + Doodle composition; it no longer compiles the legacy `Tc*` demo or graphics contexts directly.
- [ ] Finalize the Editorial Report after the remaining host-mode and full-matrix acceptance work; reconcile it with the final validation evidence and `Outcomes & Retrospective` before marking this ExecPlan complete.

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

- Observation: An installed Board package built with SDL3 must find SDL3 before importing its exported targets.
  Evidence: configuring standalone Magic Metal against the initial SDL3 Board install failed because `SDL3::SDL3-shared` was absent. `BoardConfig.cmake` now calls `find_dependency(SDL3 CONFIG)` for SDL3 exports, and the standalone Board → Magic Metal → Doodle Skia chain configures and tests successfully.

- Observation: The pinned iOS simulator Skia archive references external libpng and zlib symbols.
  Evidence: linking `libskia-ios-simulator-arm64.a` initially failed on `_png_*`; the matching arm64 simulator libpng and zlib artifacts resolve those symbols through explicit `DOODLE_IOS_PNG_ROOT` and `DOODLE_IOS_ZLIB_ROOT` CMake inputs.

- Observation: The iOS simulator still provides EAGL/OpenGL ES 3 even though Apple deprecates it for new applications.
  Evidence: the native Board view created a `CAEAGLLayer`, Magic acquired the opaque context through the Board OpenGL capability table, and the simulator displayed the Skia scene. The backend defines `GLES_SILENCE_DEPRECATION` privately; no EAGL declarations enter public headers.

- Observation: A `CAMetalLayer` can be hosted by the reusable iOS Board view without adding UIKit or Metal declarations to the public ABI.
  Evidence: Board published the layer as an opaque value through the existing Metal surface capability; Magic created the device, queue, drawable, and presentation command buffer, and the arm64 simulator rendered the Skia scene.

- Observation: `AChoreographer` is available from Android API 24, so a native Android Board host based on it cannot support a lower minimum SDK.
  Evidence: the NDK marks `AChoreographer_getInstance` and `AChoreographer_postFrameCallback` unavailable below API 24. Board configuration now rejects lower API levels and the Gradle application declares `minSdk 24`.

- Observation: Android OpenGL ES 3 can use the existing Board OpenGL surface capability without placing EGL types in any public header.
  Evidence: Board privately created and rebuilt the EGL window surface and opaque context callbacks, Magic published the default framebuffer through `MagicOpenGLInterop`, and the Pixel 3a API 34 emulator rendered the Skia scene.

- Observation: only the current `skia-158dc9d7-r4` Android archive contains the Ganesh Vulkan symbols required by the Skia provider.
  Evidence: `llvm-nm .cache/skia-android-r4/libskia-android-arm64-v8a.a` reports `GrDirectContext::MakeVulkan`, while the older local `skia-android` cache does not; the Vulkan APK linked successfully with the current archive.

- Observation: Android API 24 does not export `vkGetPhysicalDeviceFeatures2` as a linkable loader symbol, and Skia must receive the exact feature structure enabled on the Magic-owned device.
  Evidence: the original Android Vulkan implementation obtains the function through `vkGetInstanceProcAddr`, passes that structure through `VkDeviceCreateInfo::pNext`, and gives the same pointer to `GrVkBackendContext`; Magic now preserves that contract through an opaque interop field.

- Observation: The pinned macOS Skia archive does not export Ganesh Vulkan entry points, and this environment has no Vulkan loader or MoltenVK development package.
  Evidence: `nm -gU .cache/skia-158dc9d7-r4/libskia-macos-arm64.a | grep MakeVulkan` produced no symbols, while neither `vulkan.h` nor `libvulkan*.dylib` was present under the local Homebrew prefixes. SDL3 desktop Vulkan therefore cannot be an honest supported Skia combination with the current external artifacts.

- Observation: Vulkan SDK 1.4.350.1 is now installed locally and enumerates the Apple M1 Pro through MoltenVK, including `VK_LAYER_KHRONOS_validation` and `VK_KHR_portability_enumeration`.
  Evidence: `/Users/flsobral/Library/VulkanSDK/1.4.350.1/macOS/bin/vulkaninfo --summary` reported loader 1.4.350, `driverName = MoltenVK`, and the validation layer. The preceding desktop-Vulkan limitation no longer applies to the loader. `nm -gU .cache/skia-158dc9d7-r5/libskia-macos-arm64.a | grep MakeVulkan` subsequently returned Ganesh Vulkan symbols, enabling the combination.

- Observation: The initial desktop Vulkan demo target defined `MDB_DEMO_VULKAN` but did not select `MAGIC_BACKEND_VULKAN`, so it silently created a CPU Magic context.
  Evidence: the new runtime label printed `Board: SDL3 3.4.12 | Magic: CPU 0.1.0 | Doodle: Skia 158dc9d7-r5` from a `MAGIC_BACKEND=VULKAN` build. Adding the missing branch exposed MoltenVK's required `VK_KHR_portability_subset` device extension. After Magic enabled and passed the actual device-extension list to Skia, the label reported `Magic: Vulkan 1.1.334` and the validation log was empty.

- Observation: Emscripten's current WebGL glue rejects contexts requested with `explicitSwapControl`.
  Evidence: its generated `_emscripten_webgl_do_create_context` returns zero when that attribute is true because browser explicit swap was removed. Magic must request the normal implicit browser presentation path and treat end-frame as successful after Doodle flushes the current context.

- Observation: A consumer of an installed Doodle package configured with Skia must supply the matching external Skia artifact when it calls `find_package(Doodle)`.
  Evidence: a 2026-07-16 consumer configuration against `build/plan-final-install` failed with `Doodle::Skia requires -DDoodle_SKIA_ROOT=...`; it then configured, linked, and exited zero with `-DDoodle_SKIA_ROOT=$PWD/.cache/skia-158dc9d7-r4`. This is an intentional external-renderer dependency, now documented in the README and CI package-consumer command.

- Observation: The available Android AVD can stall unrelated system apps and `system_server`, contaminating screenshots or delaying ADB even while the Magic Doodle Board CPU demo remains alive.
  Evidence: on 2026-07-16 the AVD displayed ANRs for Messages and System UI, while `logcat` recorded long `system_server` contentions and a live demo PID without a Magic Doodle Board fatal exception. After an emulator restart, the CPU demo produced a clean capture. The OpenGL ES APK built but this runner did not remain stable long enough for a trustworthy visual capture; do not treat its older capture as current-tree acceptance.

- Observation: A child overlay inside Android `SurfaceView` or the renderer-owning `TextureView` can be composed beneath the changing renderer pixels even when the Java hierarchy and Board slot negotiation succeed.
  Evidence: `uiautomator` reported the attached overlay and its tap changed its text, but the first screenshots showed only the portion outside the renderer. Routing the slot into a sibling `FrameLayout` above the `BoardView` produced a fully visible overlay across CPU, OpenGL ES, and Vulkan captures.

- Observation: The Android matrix runner could report success after building without launching a smoke, or capture the Android splash screen before the Activity content was ready.
  Evidence: under `set -e`, the former `[[ ... ]] && return` build-only guards made a false condition terminate the shell path. The runner now uses `if` guards, waits for the app PID, removes stale UI-dump data, and waits for the Activity title in a fresh `uiautomator` dump before `screencap`.

- Observation: Git Bash and PowerShell do not provide an interchangeable representation of the Windows workspace path to a native CMake configure step.
  Evidence: the Windows hosted configure reported that the Skia cache lacked `headers/modules/skia/include/core/SkCanvas.h` and the archive after a PowerShell step invoked the Bash fetch adapter with `$PWD`. Running the fetch and artifact assertions entirely with `shell: bash` successfully staged `SkCanvas.h` and `libskia-windows-x64.lib` locally; the following PowerShell configure now uses `$env:GITHUB_WORKSPACE` explicitly.

- Observation: the depot's `skia/fetch.sh --install-dev` invokes native Python to enumerate all platform build manifests; on Git Bash for Windows, its CRLF output leaves a carriage return in each manifest URL.
  Evidence: hosted Windows run `29541808320` successfully installed `libskia.lib` and then failed with curl exit 3 while downloading `build_config_manifest-android-arm64-v8a.md`, before CMake ran. The project adapter now uses the executable depot fetcher for the selected archive and reads the declared shared development bundle metadata directly, avoiding unrelated manifest downloads. The adapter stages the resulting headers and archive locally for `windows-x64`.

- Observation: a source build of SDL 3.4.12 on `ubuntu-latest` needs Linux window-system development packages even though the CI job does not open a window.
  Evidence: hosted Linux run `29541804704` stopped in SDL configuration with `SDL could not find X11 or Wayland development libraries on your system`; after those were installed, run `29542084404` identified the remaining XTEST prerequisite. The workflow now installs the SDL X11 (including XTEST), Wayland, EGL, and OpenGL development prerequisites before building the pinned source.

- Observation: the pinned Windows Skia archive uses the static MSVC runtime and references the Windows OpenGL loader.
  Evidence: hosted Windows run `29542085623` reached link time and reported `MT_StaticRelease` versus `MD_DynamicRelease` mismatches, followed by unresolved `wglGetCurrentContext` and `wglGetProcAddress`. The Windows CI configuration now selects `CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded`, and the private Skia external target propagates the system `opengl32` dependency to link consumers.

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

- Decision: Remove the retired monolithic runtime, duplicate demos, and renderer placeholder files once the iOS convenience entry no longer used them.
  Rationale: Every supported platform now composes the shared scene through Board, Magic, and Doodle. Keeping unreferenced duplicate implementations would preserve obsolete public names and invite an accidental bypass of the layer boundaries. The architecture checker now rejects those names in active source and build files.
  Date/Author: 2026-07-15 / Codex.

- Decision: Add the host-mode and native-slot C ABI before the Android reusable view, and initially implement slots only above the iOS renderer.
  Rationale: The public API needs stable, native-type-free ownership and geometry rules before each platform can provide it. iOS can host an overlay container inside its existing reusable Board view; placing a control below its render layer would require a separate rendering sibling, so that order returns `BOARD_ERROR_UNAVAILABLE` rather than silently changing z-order.
  Date/Author: 2026-07-15 / Codex.

- Decision: Make Android BoardView a SurfaceView that owns a private JNI application handle, replacing the demo NativeActivity entry point.
  Rationale: SurfaceView supplies an ANativeWindow usable by the existing Board-to-Magic capability contract while allowing a native Android layout to position BoardView between ordinary controls. The C ABI accepts only an opaque window pointer; JNI and Java remain private implementation details.
  Date/Author: 2026-07-15 / Codex.

- Decision: Have Android BoardView own the render SurfaceView and native overlay children, with JNI translating opaque slot references on the UI thread.
  Rationale: This gives Android the same portable BoardNativeViewSlot contract as iOS while keeping Java View and JNI declarations out of installed headers. The initial implementation supports the documented above-renderer order, frame, rectangular clip, and visibility; below-renderer order reports unavailable.
  Date/Author: 2026-07-15 / Codex.

- Decision: Supersede the render-SurfaceView implementation with a private `TextureView`-backed `Surface`, and place hybrid overlays in a caller-supplied sibling `FrameLayout` above `BoardView`.
  Rationale: Both implementations preserve the opaque Java `Surface` to `ANativeWindow` boundary used by Board and Magic, but a sibling container gives Android compositor ordering that visibly places native slots above the renderer. BoardView translates Board-relative slot geometry into the container coordinate space and retains the JNI handle privately. The documented below-renderer limitation is unchanged.
  Date/Author: 2026-07-16 / Codex.

- Decision: Obtain Skia and native compression/image dependencies through the tag pinned in `deps/totalcross-depot-tools.ref`, while retaining `scripts/fetch-totalcross-skia.sh` as a compatibility adapter for the existing CMake layout.
  Rationale: The depot owns versioned release fetchers and now supplies executable scripts. The adapter keeps downstream CMake callers stable while CI gets reproducible Skia headers and archives. Where Skia needs a compression dependency, use the depot's `zlib-ng` prefix; libpng is fetched from the same release family. `minizip-ng` remains preferred if an archive dependency is later needed, but it is not part of the current Skia link closure.
  Date/Author: 2026-07-16 / Codex.

- Decision: Permit manual dispatch for every hosted validation workflow in addition to push and pull-request triggers.
  Rationale: A manual run is a controlled recovery path when a hosted event is delayed or unavailable. It changes neither the build matrix nor its selection inputs, while allowing the exact same validation definitions to be rerun against a pinned `main` revision.
  Date/Author: 2026-07-16 / Codex.

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

- Decision: Extract Magic CPU into a backend-private provider before moving the GPU context implementations.
  Rationale: This replaces the monolithic Magic context state with a backend operation table and verifies the Board-to-Magic boundary on the deterministic path. Each GPU migration can now add its provider without exposing backend state to Doodle.
  Date/Author: 2026-07-14 / Codex.

- Decision: Complete the desktop CPU milestone by adding an SDL3 Board provider rather than retaining the old monolithic SDL graphics context.
  Rationale: Board owns SDL window/event-loop details and exposes a CPU presentation callback; Magic then remains unaware of SDL while Doodle receives only Magic CPU frame interop.
  Date/Author: 2026-07-14 / Codex.

- Decision: Add SDL3 OpenGL capability callbacks to Board and consume them through a Magic OpenGL provider for the desktop GPU milestone.
  Rationale: SDL context creation, current-context control, procedure lookup, drawable sizing, and swapping remain Board-private callbacks. Magic and Doodle exchange only opaque OpenGL context/framebuffer values through their versioned public interop tables.
  Date/Author: 2026-07-14 / Codex.

- Decision: Add an SDL3 Metal layer capability to Board; keep Metal device, queue, drawable acquisition, and presentation in Magic.
  Rationale: The SDL Metal view is host-surface state, while GPU resources and presentation synchronization belong to Magic. Doodle Skia receives the opaque layer/device/queue/drawable-slot values only through `MagicMetalInterop`.
  Date/Author: 2026-07-14 / Codex.

- Decision: Implement the initial iOS host as a reusable opaque Board view that owns a CPU pixel buffer and a `CADisplayLink` scheduler.
  Rationale: UIKit details remain private to Board while the application receives ordinary Board events and frame callbacks. Magic continues to consume only the versioned CPU surface table and Doodle sees only `MagicCpuInterop`.
  Date/Author: 2026-07-14 / Codex.

- Decision: Extend the reusable iOS Board view with an optional private `CAEAGLLayer` and EAGL callbacks when `MAGIC_BACKEND=OPENGL` is selected.
  Rationale: Board retains platform-created drawable ownership, while Magic drives context creation, binding, frame sizing, and presentation exclusively through the public Board capability table. This preserves the same Magic-to-Doodle OpenGL interop contract used on desktop.
  Date/Author: 2026-07-14 / Codex.

- Decision: Extend the reusable iOS Board view with an optional private `CAMetalLayer` when `MAGIC_BACKEND=METAL` is selected.
  Rationale: Board remains the owner of the platform host layer, while Magic owns Metal device selection, command queue creation, drawable acquisition, and presentation. Doodle receives only the opaque `MagicMetalInterop` table, preserving the layer boundaries and portable public ABI.
  Date/Author: 2026-07-14 / Codex.

- Decision: Implement the initial Android host as a NativeActivity attachment that accepts the platform `android_app` only as an opaque `void *` in the public Board API.
  Rationale: Board privately owns NativeActivity callbacks, `ANativeWindow` CPU presentation, input conversion, and the `AChoreographer` loop. Application composition remains limited to Board, Magic, and Doodle public headers while Android NDK declarations stay out of the portable ABI.
  Date/Author: 2026-07-14 / Codex.

- Decision: Extend the Android Board host with optional private EGL/OpenGL ES 3 callbacks only when `MAGIC_BACKEND=OPENGL` is selected.
  Rationale: Board retains ownership of the Android window surface and its lifecycle, while Magic drives opaque context creation, binding, sizing, and swapping through the public Board table. Doodle sees only the resulting `MagicOpenGLInterop` values.
  Date/Author: 2026-07-14 / Codex.

- Decision: Require a runnable backend-matrix smoke-test script for every supported combination.
  Rationale: A supported configuration is not complete until its build, installation where needed, launch, and observable output can be repeated without reconstructing platform-specific commands. The shared matrix runner centralizes common setup while one named wrapper per combination makes matrix coverage reviewable.
  Date/Author: 2026-07-14 / Codex.

- Decision: implement the initial Magic Web provider with a private WebGL2 context and expose a versioned `MagicWebInterop` table to Doodle.
  Rationale: the pinned wasm32 Skia artifact contains the Ganesh OpenGL implementation. Board exposes only the browser canvas selector, Magic owns WebGL2 creation/current-context setup and browser-implicit presentation, and Doodle creates its Skia backend render target from opaque frame values without including browser headers.
  Date/Author: 2026-07-14 / Codex.

- Decision: Keep SDL3 desktop Vulkan outside the supported matrix until the external Vulkan loader and a matching macOS Skia Ganesh Vulkan artifact are available. Superseded after the prerequisites were verified on 2026-07-15.
  Rationale: the plan's acceptance matrix initially listed Android Vulkan but not desktop Vulkan. Advertising a desktop path without those external artifacts would have violated the explicit-backend rule and could not have had a runnable smoke-test script.
  Date/Author: 2026-07-15 / Codex.

- Decision: Add SDL3 desktop Vulkan to the supported macOS matrix after the validation-layer smoke run succeeds, using Vulkan SDK 1.4.350.1 and the TotalCross Skia r5 archive.
  Rationale: the locally installed SDK proved that MoltenVK and a validation layer are available, the r5 archive exported Ganesh Vulkan symbols, and the three-frame smoke run completed without a validation error. macOS is the only validated desktop runner; Windows and Linux must receive their own runner validation before they are claimed as supported.
  Date/Author: 2026-07-15 / Codex.

- Decision: Report selected runtime backends through object-bound public name and version queries, and have the shared scene render those results.
  Rationale: build selections alone did not prove the demo created the intended context. Object-bound queries keep foreign runtime types out of public headers, let every entry point use the same backend-agnostic scene, and made a desktop Vulkan fallback visible during validation. The appended Board Vulkan-surface, Magic Vulkan-interop, and Doodle renderer-provider fields require ABI version 2 so old binary consumers are rejected rather than accepting incompatible layouts.
  Date/Author: 2026-07-15 / Codex.

- Decision: Treat the Skia artifact location as an explicit installed-Doodle package input rather than embedding or copying the archive into the package prefix.
  Rationale: Skia remains an external dependency by project policy. Rejecting a consumer that omits `Doodle_SKIA_ROOT` is clearer and safer than exporting a target with an unresolved archive path; the consumer command and README now make the requirement explicit.
  Date/Author: 2026-07-16 / Codex.

## Outcomes & Retrospective

2026-07-15: A progress review reconciled the plan with the checked-in tree,
the named backend-matrix scripts, and the recorded smoke-test commits. At the
time of that review, Board was complete for every implemented host and Magic
was complete for every currently supported context except desktop Vulkan. The Canvas and Skia
provider migration is complete; the remaining Doodle-provider task is the
declared Blend2D, NanoVG, and Vello provider/stub work. Mobile embedded and
above-renderer overlays are implemented on Android and iOS, while
below-renderer ordering remains an explicit unavailable capability. Full
matrix/CI acceptance remains open because it has not been rerun after the
latest mobile-host changes.

2026-07-15: Blend2D, NanoVG, and Vello now each have a Doodle-owned provider
stub source and public getter. The getters return unavailable rather than a
no-op renderer; selecting any of the renderers continues to fail during CMake
configuration. Focused tests cover both the getters and all three
configuration diagnostics. Desktop Vulkan was deliberately not promoted to
the supported matrix: the pinned macOS Skia archive lacks Ganesh Vulkan and
the local environment lacks a Vulkan loader.

2026-07-14: The migration now has an executable lower-layer spine. `board_core`
implements a versioned headless CPU surface and deterministic coalescing frame
scheduler. `magic_core` consumes only Board's installed surface API and exposes
versioned CPU frame interop. `doodle_core` consumes only Magic's public API;
`doodle_renderer_skia` creates raster-direct Skia surfaces from that interop.
The CPU implementation now resides in `magic/backends/cpu/`; the generic
context dispatcher retains no Board surface details.
All three packages install and are consumable in dependency order. Existing
monolithic sources, GPU Skia paths, desktop/mobile/web backends, shared demos,
and legacy-name removal remain outstanding; this work deliberately does not
represent those as complete.

2026-07-14: Board SDL3 now owns a native macOS window, SDL event conversion,
the private desktop loop, and CPU pixel presentation. The desktop demo composes
Board, Magic, and Doodle entirely through public APIs and was run for three
seconds after configuration with `BOARD_BACKEND=SDL3`, `MAGIC_BACKEND=CPU`, and
`DOODLE_RENDERER=SKIA`. It printed its running confirmation and created the
native frame loop; closing the window remains the normal exit mechanism.

2026-07-14: The SDL3 OpenGL path now creates its context through Board's
versioned OpenGL capability table. Magic makes that opaque context current,
publishes its default framebuffer through `MagicOpenGLInterop`, and Doodle
Skia binds it as a backend render target. The three-frame desktop demo smoke
test exited successfully with code 0. The pinned macOS Skia archive requires
both `SK_GL` and `SK_METAL` private layout macros even for the OpenGL path.

2026-07-14: The macOS SDL3 Metal path now creates an SDL Metal view in Board,
while Magic owns the `CAMetalLayer` device, command queue, drawable lifecycle,
and presentation. Doodle Skia consumes only opaque `MagicMetalInterop` values
to create its layer-backed frame surface. The composed demo completed its
three-frame smoke test with `BOARD_BACKEND=SDL3`, `MAGIC_BACKEND=METAL`, and
`DOODLE_RENDERER=SKIA`; the matching CTest configuration passed all 8 tests.
Board SDL3 Metal, Magic Metal, and Doodle Skia also configured and tested as
separate packages through an installed `build/install-metal` prefix.

2026-07-14: The iOS native CPU milestone adds a reusable Board view privately
implemented with `UIView` and `CADisplayLink`. It owns the CPU pixel buffer,
dispatches resize and touch events, and presents CPU frames through Core
Graphics. The iOS demo composes that Board backend with Magic CPU and Doodle
Skia around the same common scene used by the SDL3 demo. It was built for the
arm64 iOS simulator, installed on iPhone 15 Pro, launched successfully, and
captured in `artifacts/final/ios-cpu-simulator.png`.
The Board iOS, Magic CPU, and Doodle Skia packages also build separately and
their installed CMake packages configure a simulator consumer.

2026-07-14: The iOS native OpenGL ES milestone adds an optional private
`CAEAGLLayer` to the reusable Board view. Board supplies opaque context,
current-context, drawable-size, and swap callbacks; Magic uses them to manage
its OpenGL frame; Doodle Skia binds the resulting `MagicOpenGLInterop` target.
The arm64 simulator demo was built, installed on iPhone 15 Pro, launched, and
captured in `artifacts/final/ios-opengl-simulator.png`. The iOS Board → Magic
OpenGL → Doodle Skia packages also build separately.

2026-07-14: The iOS native Metal milestone adds an optional private
`CAMetalLayer` to the reusable Board view. Magic owns the Metal device, command
queue, drawable acquisition, and command-buffer presentation; Doodle Skia
binds the opaque `MagicMetalInterop` values as its layer-backed frame surface.
The arm64 simulator demo was built, installed on iPhone 15 Pro, launched, and
captured in `artifacts/final/ios-metal-simulator.png`. The iOS Board → Magic
Metal → Doodle Skia packages also build independently and their installed
packages configure a simulator consumer through `build/install-ios-metal`.

2026-07-14: The Android native CPU milestone adds a Board NativeActivity host
with private `ANativeWindow` CPU presentation, input conversion, resize and
lifecycle events, and an `AChoreographer` scheduler. It requires Android API
24 and keeps that requirement aligned with the Gradle `minSdk 24`. The arm64
Android demo composes Board, Magic CPU, and Doodle Skia around the shared scene,
was installed and launched on the Pixel 3a API 34 emulator, and was captured in
`artifacts/final/android-cpu-emulator.png`. The Android Board → Magic CPU →
Doodle Skia packages also build independently and their installed packages
configure an arm64 Android consumer through `build/install-android-cpu`.

2026-07-14: The Android native OpenGL ES milestone adds optional private EGL
surface and context callbacks to the Board host. Magic creates, binds, sizes,
and swaps the opaque OpenGL ES 3 context; Doodle Skia binds the default
framebuffer through `MagicOpenGLInterop`. The arm64 Android demo was built,
installed and launched on the Pixel 3a API 34 emulator, and captured in
`artifacts/final/android-opengl-emulator.png`. The Android Board → Magic
OpenGL → Doodle Skia packages also build independently and their installed
packages configure an arm64 consumer through `build/install-android-opengl`.

2026-07-14: The Android native Vulkan milestone adds a versioned Board Vulkan
surface capability whose callbacks use only opaque values. Magic owns the
Android Vulkan instance, physical-device selection, queue, swapchain image
acquisition, semaphores, resize/recreation, and presentation. Doodle Skia
creates a Vulkan Ganesh context from `MagicVulkanInterop`, waits for Magic's
acquire semaphore, signals Magic's present semaphore, and never owns the
swapchain. The acquire semaphore is transferred to Skia for each frame, while
Magic retains only presentation semaphores; this matches the original Android
Vulkan ownership contract and prevents a double destruction during teardown.
The arm64 APK and root Android composition both build with Android API 24 and
the current pinned Skia archive. The visible Pixel 3a API 34 emulator installed
and ran the shared scene, captured in
`artifacts/final/android-vulkan-emulator.png`.

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

2026-07-14: The Web milestone makes `BOARD_BACKEND=WEB` and
`MAGIC_BACKEND=WEB` real selections under the Emscripten 2.0.6 toolchain.
Board privately adapts browser resize, mouse, touch, keyboard, and animation
frames to its public callbacks. Magic owns the WebGL2 context and the browser's
implicit presentation boundary, and publishes a
versioned opaque Web frame table; Doodle Skia binds the default framebuffer
through that table. The common scene linked against the pinned wasm32 Skia
archive, producing `magic_doodle_board_web_demo.html`, `.js`, and `.wasm`.
The demo must be served over HTTP so that the Emscripten loader can fetch its
`.wasm` asset; opening the generated HTML through `file://` aborts before
initialization. `scripts/test-web-skia.sh` starts a loopback Python HTTP server
on an available port, launches Safari at the generated URL, and records
generated-artifact metadata under `artifacts/final/`; the CI Web workflow now
uses the same public CMake selections.

2026-07-15: Safari exposed a WebAssembly indirect-call signature mismatch in
the Doodle Web path. The Board animation callback and scheduler were verified
with the generated WebAssembly name section; the failing call was instead the
application-side invocation of Skia's `GrGLFunction::fBindFramebuffer` wrapper.
Magic already makes the WebGL context current and records its currently bound
framebuffer before Doodle begins the frame, so Doodle now consumes that value
without invoking the incompatible wrapper.

2026-07-15: The published wasm32 Skia archive includes FreeType and a custom
directory font manager, but the browser's Emscripten filesystem starts without
the `/usr/share/fonts` directory that its default manager scans. The Web demo
therefore preloads an externally fetched Roboto font at that path; text remains
owned by Doodle's Skia renderer and requires no browser-specific Canvas API.

2026-07-15: `ios/CMakeLists.txt` now selects `IOS_NATIVE`, a chosen Magic
backend, and Skia before adding the root project as a subdirectory. This keeps
the convenience iOS entry point useful to platform-only consumers without
retaining a second app implementation that included `src/graphics/` and the
legacy public headers. The CPU simulator bundle configured and built with
`cmake -S ios` using the same cached Skia, libpng, and zlib prefixes as the
backend-matrix smoke script.

2026-07-15: The retired monolithic headers and implementation directories,
their duplicate desktop, Android, and iOS demos, and inactive renderer
placeholder files were removed after a repository search showed that only the
old iOS convenience CMake file still referenced them. The architecture checker
now treats retired naming in active Board, Magic, Doodle, example, Android,
iOS, and root-CMake sources as a failure. Historical names remain only in this
ExecPlan and baseline artifacts, where they document the completed migration.

2026-07-15: Configuration validation is now executable rather than relying on
documentation alone. The root CTest case configures a disposable valid
Headless + CPU build and verifies clear failures for a Headless GPU request,
the GLFW stub, the Vello stub, and a Web request outside Emscripten.

2026-07-15: Board now exposes native-type-free host modes, lifecycle and UI
dispatch values, plus opaque native-view slots. The existing iOS Board view
contains an overlay container; hybrid hosts can attach an above-renderer
UIKit control with frame, visibility, and rectangular clipping managed by a
slot. The iOS demo embeds that Board view between native controls and adds an
interactive native overlay button. Android has no misleading slot success
path: it reports the capability as unavailable until its reusable BoardView
is implemented.

2026-07-15: Android now has `org.magicdoodle.board.BoardView`, an embeddable
SurfaceView that creates, starts, resizes, and destroys the private native
Board/Magic/Doodle composition from its surface lifecycle. The demo Activity
places the BoardView between Android controls and the Android CPU APK builds
successfully. The OpenGL ES and Vulkan APK variants also compile with the same
BoardView entry point. A visible AVD run remains required to validate each renderer.

2026-07-15: Android BoardView is now a render SurfaceView plus an overlay
container. A hybrid demo button is created through the opaque
BoardNativeViewSlot contract; private JNI methods apply its frame, clip, and
visibility on the Android UI thread and remove it during teardown. The CPU APK
build validates the Java/JNI bridge; an AVD run is still required to exercise
the visible control and renderer lifecycle.

2026-07-15: Android smoke wrappers no longer wait indefinitely when no device
is connected. They poll for a fully booted ADB target up to a configurable
timeout and then name the missing prerequisite, making the runner's status
observable before Gradle work begins.

2026-07-15: SDL3 desktop Vulkan is now an implemented macOS path. Board
creates an SDL Vulkan window and owns the SDL-specific surface destruction
callback; Magic owns instance portability enumeration, device, swapchain, and
presentation; Doodle Skia receives the exact enabled instance- and
device-extension lists through `MagicVulkanInterop` instead of assuming
Android extensions. The runtime identity labels caught and corrected an
initial demo CPU fallback, then exposed the required MoltenVK portability
device extension. With Vulkan SDK 1.4.350.1, MoltenVK, and Skia r5, the
corrected three-frame demo reported `Board: SDL3 3.4.12 | Magic: Vulkan
1.1.334 | Doodle: Skia 158dc9d7-r5`; its validation log was empty and the
root CTest suite passed 10 of 10 tests. After the ABI 2 update, Board, Magic,
and Doodle also configured, tested, installed, and configured an external
consumer in dependency order; the consumer linked and exited 0. The new
`scripts/test-desktop-vulkan-skia.sh` preserves those prerequisites in the
backend matrix. Windows and Linux have not yet run this script.

2026-07-15: `scripts/test-backend-matrix.sh build-all` compiled the 12 current
supported entries: Headless CPU; SDL3 CPU, OpenGL, Metal, and Vulkan; iOS CPU,
OpenGL ES, and Metal; Android CPU, OpenGL ES, and Vulkan; and Web. Build-only
mode deliberately skipped simulator/AVD startup, APK installation, browser
launch, and rendering smoke checks, so it supplements rather than replaces
the named per-combination tests.

2026-07-16: The current tree passed a fresh root Headless + CPU + Skia run
(9 of 9 tests: public C/C++ headers, boundaries, configuration, headless
rendering, and focused layer/provider tests). A fresh local
`scripts/test-backend-matrix.sh build-all` then compiled all 12 supported
entries with the macOS Vulkan SDK. Board, Magic, and Doodle were subsequently
built, tested, and installed in dependency order (1 of 1, 1 of 1, and 2 of 2
tests); an external consumer configured with that prefix and the explicit
`Doodle_SKIA_ROOT`, linked, and exited zero. The macOS and Linux CI workflows
now run CTest; macOS additionally validates this staged package-consumer
chain. No iOS simulator or Android AVD was booted for this run, so fresh
visible mobile acceptance remains outstanding.

2026-07-16: An iPhone 15 Pro iOS 17.0 simulator was booted and all three
named iOS smoke scripts rebuilt, installed, launched, and captured the
current CPU, OpenGL ES, and Metal demos. Visual inspection of the refreshed
CPU and Metal captures confirmed the shared scene, runtime identity, embedded
Board view, above-renderer native overlay, and native control below it. During
this run the original title was found beneath the status-bar area; the iOS demo
now lays out its title, Board view, and bottom control from `safeAreaInsets`,
and the refreshed captures show the corrected layout. Android still lacks a
booted AVD in this environment.

2026-07-16: An Android arm64 AVD was made available after the earlier matrix
run. The named CPU smoke rebuilt and installed the current APK; after the AVD
was restarted to clear unrelated System UI and Messages ANRs, the running demo
produced a clean `artifacts/final/android-cpu-emulator.png` capture showing
the embedded BoardView, CPU runtime label, native overlay, and surrounding
host controls. The demo Activity now assigns explicit contrasting label colors
to its two native buttons so the overlay and below-renderer control remain
legible on the emulator. The OpenGL ES APK also rebuilt, but AVD-wide ANRs
prevented a trustworthy current visual capture. Android Vulkan remains build
validated only for this revision.

2026-07-16: The remaining local non-Android acceptance commands were rerun on
the current tree. Headless CPU passed `mdb_headless_skia`; SDL3 CPU, OpenGL,
and Metal each reported the requested runtime identity and exited after three
frames; and SDL3 Vulkan reported `Magic: Vulkan 1.1.334` with an empty
validation log. The Web smoke rebuilt the Emscripten demo, served it over
loopback HTTP in Safari for eight seconds, and refreshed its artifact report.
An Android cold boot did not solve the GPU visual gap: OpenGL ES reported its
runtime identity and continued rendering in logcat, but System UI immediately
ANRed; Vulkan rebuilt the current APK but the same AVD stalled before a
trustworthy launch/capture. Those generated, contaminated screenshots were
discarded rather than recorded as acceptance evidence.

2026-07-16: A subsequent cold-booted arm64 AVD completed all three named
Android smokes after the reusable host was changed to a private
`TextureView`-backed `Surface` and the overlay was placed in a sibling
container above it. `android-cpu-emulator.png`, `android-opengl-emulator.png`,
and `android-vulkan-emulator.png` were visually inspected and show the
requested CPU, OpenGL ES, and Vulkan identities, the shared scene, the native
overlay above the renderer, and the native control below it. A tap at the
overlay coordinates changed its text to `Tapped` in a fresh UI hierarchy,
proving independent native input. The Android matrix runner now rejects a
build-only false guard, waits for the app PID, and waits for the real Activity
title before capturing. Hosted CI is the remaining external acceptance item.

At the end of each milestone, append a short entry here describing what is now observable, what remains incomplete, and any design lesson that should guide later milestones. At final completion, compare the actual standalone build commands, supported backend matrix, demo behavior, and ABI checks against the purpose stated above.

## Editorial Report

This report is an in-progress factual handoff maintained under `.agent/PLANS.md`. It describes execution evidence available through 2026-07-16; it is not a completion claim and must be finalized after the remaining Progress items and final acceptance runs.

### Editorial Summary

The original engineering problem was a monolithic C-first graphics runtime whose lifecycle, platform hosting, graphics contexts, Canvas API, and renderer behavior were coupled through legacy `Tc*` interfaces. The migration set out to make the same demo scene composable from three independent public libraries: Board for hosting and events, Magic for frame and graphics-context ownership, and Doodle for Canvas and renderers.

The working tree now contains those three package trees, public versioned capability boundaries, a shared demo scene, and recorded runnable paths for headless CPU, macOS SDL3 CPU/OpenGL/Metal/Vulkan, iOS CPU/OpenGL ES/Metal, Android CPU/OpenGL ES/Vulkan, and Web. The result is developer-visible: supported builds choose `BOARD_BACKEND`, `MAGIC_BACKEND`, and `DOODLE_RENDERER` explicitly and application code composes a Board frame with a Magic frame and a Doodle Canvas. Fresh mobile-host acceptance, including Android GPU captures, is now recorded; hosted CI remains the only open external acceptance.

### Original Plan versus Actual Outcome

The plan intended to replace the retired runtime with exactly three public layers, preserve the shared demo across supported platforms, keep unsupported choices honest, and establish independent CMake packages with public-only dependency boundaries. Those core structural goals have been implemented: `board/`, `magic/`, and `doodle/` configure as packages; retired framework names were removed from active sources; and the demos use `examples/common/magic_doodle_board_scene.c` through the explicit Board-to-Magic-to-Doodle frame sequence.

The executed result changed direction in several material ways. Android hosting began as a NativeActivity path but became an embeddable `org.magicdoodle.board.BoardView`; this better satisfies the reusable-view requirement. Web changed from direct local-file opening to an HTTP-served smoke test and gained a preloaded Roboto font because the browser loader and the pinned Skia font manager required those conditions. SDL3 desktop Vulkan was initially excluded because the r4 macOS archive lacked Ganesh Vulkan symbols and no loader was present; Vulkan SDK 1.4.350.1 and Skia r5 removed those prerequisites, so the macOS path was implemented and validated. Blend2D, NanoVG, and Vello remain deliberately unavailable rather than becoming no-op implementations; provider-owned getter stubs and configuration tests now make that status explicit.

The remaining work is not represented as delivered. Native slots currently support documented above-renderer ordering; below-renderer ordering is unavailable. Local supported-matrix, installation, and observable acceptance evidence is recorded, but the updated hosted CI has not yet run. The Editorial Report is consequently incremental rather than final.

### What Changed

The public architecture is represented by `board/include/board/`, `magic/include/magic/`, and `doodle/include/doodle/`. `board/include/board/board_surface.h` exposes versioned, native-type-free surface capability tables; `magic/include/magic/magic_interop.h` carries versioned frame interop; and `doodle/include/doodle/` owns the Canvas and renderer-provider APIs. `board_backend_name`/`board_backend_version`, `magic_context_backend_name`/`magic_context_backend_version`, and `doodle_renderer_name`/`doodle_renderer_version` identify the active objects without exposing native types. Private platform and graphics implementation files remain in their owning layer, including `board/backends/`, `magic/backends/`, and `doodle/renderers/`.

`examples/common/magic_doodle_board_scene.c` is the backend-agnostic scene used by the desktop, iOS, Android, and Web entry points. Root `CMakeLists.txt` selects the composition through `BOARD_BACKEND`, `MAGIC_BACKEND`, and `DOODLE_RENDERER`; `ios/CMakeLists.txt` is an iOS-only convenience entry for that same composition. `board/backends/sdl3/board_sdl3_backend.c`, `magic/backends/vulkan/magic_vulkan_context.cpp`, and `doodle/renderers/skia/doodle_skia_renderer.cpp` now form the desktop Vulkan path. `scripts/test-backend-matrix.sh` and its named wrappers provide repeatable smoke-test entry points for the recorded supported combinations, including a loopback HTTP server for Web and validation-layer setup for desktop Vulkan.

Recent evidence-bearing commits include `c709e46` (three-layer foundation), `4451fce` (Skia CPU provider), `9118840` (Android Vulkan path), `fa34684` through `a2dfb6a` (Web migration and compatibility fixes), `05ecc9e` (legacy runtime removal), `466d1c7`, `3f0e65e`, and `c56711b` (mobile embedding and overlays), and `f76b5c5` (provider-owned renderer stubs). These commits are evidence pointers, not a claim that every combination has received final acceptance after the last commit.

### Decisions and Trade-offs

The central decision was to use versioned public capability tables between Board and Magic and versioned frame interop tables between Magic and Doodle. This keeps SDL, UIKit, Metal, Vulkan, EGL, Skia, JNI, and browser declarations out of installed headers, at the cost of maintaining explicit negotiation structs and backend adapters.

Application composition owns the frame sequence instead of Board invoking Doodle. This enforces the dependency direction and makes lifecycle behavior observable, but application entry points must coordinate begin-frame and end-frame calls. Unsupported backend and renderer selections fail during configuration rather than falling back; this produces clearer behavior but requires a maintained compatibility matrix and named validation scripts.

Mobile integration uses reusable Board views and native overlay slots. The implemented initial z-order is above the renderer; returning `BOARD_ERROR_UNAVAILABLE` for below-renderer requests is intentionally less capable than silently changing the requested order. The Web path remains pinned to Emscripten 2.0.6 and a matching external Skia artifact, trading toolchain currency for recorded compatibility.

### Unexpected Problems and Discoveries

The external Skia artifacts materially constrained the implementation. The iOS simulator archive required separate libpng and zlib inputs; the Android Vulkan path required the newer `skia-158dc9d7-r4` archive because older local artifacts lacked `GrDirectContext::MakeVulkan`; and the macOS r4 archive did not expose Ganesh Vulkan symbols. The later r5 macOS archive did export those symbols and enabled the desktop implementation. These findings are recorded in `Surprises & Discoveries` with the relevant commands and artifact names.

Browser execution uncovered two different failures. Loading the Web demo through `file://` prevented Emscripten from fetching the `.wasm` file, so the test runner now serves it over loopback HTTP. Safari then exposed a WebAssembly indirect-call signature mismatch when application code invoked Skia's framebuffer wrapper; the implementation instead consumes the framebuffer already made current by Magic. The wasm32 Skia package also required a preloaded Roboto font because its expected font directory was absent from the initial Emscripten filesystem.

Android minimum SDK 24 is a functional constraint, not an arbitrary build setting: the native scheduler uses `AChoreographer`, whose required API is unavailable below that level. The Android runner also previously appeared not to execute when no emulator was booted; it now reports a bounded, explicit readiness failure instead of waiting indefinitely.

The runtime labels found two desktop Vulkan issues that configuration and a three-frame exit status alone had hidden: the demo omitted its Vulkan `MagicConfig` branch and then MoltenVK required `VK_KHR_portability_subset`. Magic now enables that advertised device extension and Doodle receives the same device-extension list for Skia negotiation. The corrected validation-layer run is the applicable Vulkan evidence.

### Validation and Measurable Results

Only observed results are recorded here. A Headless + CPU + Skia composition produced the deterministic hash `bff7964c10eaa55f`, as recorded in `artifacts/final/headless-image-hash.txt` and the retrospective entry dated 2026-07-14. Staged Board, Magic, and Doodle installation tests and their consumer configuration were recorded as passing on 2026-07-14; the exact command shapes are preserved under `Concrete Steps` and in the retrospective rather than being reconstructed here.

The focused provider-stub validation was observed with:

    cmake -S . -B build/plan-doodle-stubs -DBOARD_BACKEND=HEADLESS -DMAGIC_BACKEND=CPU -DDOODLE_RENDERER=NONE -DMDB_BUILD_TESTS=ON -DMDB_BUILD_EXAMPLES=OFF
    cmake --build build/plan-doodle-stubs --parallel
    ctest --test-dir build/plan-doodle-stubs --output-on-failure

That run reported 8 of 8 tests passed, including `mdb_public_headers_c`, `mdb_public_headers_cpp`, `mdb_boundaries`, `mdb_configuration`, `board_headless`, `magic_cpu`, `doodle_provider`, and `doodle_provider_stubs`. A subsequent focused run reported 3 of 3 passed for `mdb_boundaries`, `mdb_configuration`, and `doodle_provider_stubs`.

The recorded Web smoke run served the demo over HTTP in Safari for eight seconds. `artifacts/final/web-skia-artifacts.txt` now records 811 bytes for the HTML, 384440 bytes for JavaScript, 4822912 bytes for WebAssembly, and 35408 bytes for the data file. These are generated artifact sizes from that refreshed run, not performance measurements. No meaningful frame-time, memory, binary-size comparison, or rendering-performance benchmark has been taken.

With `VULKAN_SDK=/Users/flsobral/Library/VulkanSDK/1.4.350.1/macOS`, `VK_ICD_FILENAMES` set to the SDK MoltenVK JSON, and `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`, the corrected `scripts/test-desktop-vulkan-skia.sh` configured and built a fresh matrix directory, printed `Board: SDL3 3.4.12 | Magic: Vulkan 1.1.334 | Doodle: Skia 158dc9d7-r5`, and completed the three-frame demo without a validation error. `vulkan-validation.log` was empty, and a separate root CTest run reported 10 of 10 tests passed. Standalone Board, Magic, and Doodle Vulkan builds each reported all of their focused tests passed, and the installed consumer linked and exited 0. These are macOS observations only.

`scripts/test-backend-matrix.sh build-all` subsequently completed the build of all 12 currently supported entries in this macOS environment. It is compilation evidence only: the command intentionally does not boot a simulator or AVD, install an APK, launch a browser, or inspect a rendered result.

The plan records builds, launches, and screenshots for the listed iOS and Android simulator/emulator paths under `artifacts/final/`, and three-frame desktop smoke runs for OpenGL and Metal. Because the latest mobile-host changes have not received a final complete visible-device rerun, those earlier captures are historical evidence rather than final acceptance of the current tree.

On 2026-07-16, the current root Headless + CPU + Skia configuration passed 9
of 9 CTest cases. The same revision compiled all 12 entries through
`scripts/test-backend-matrix.sh build-all`; that mode intentionally did not
launch mobile devices or a browser. Fresh staged Board, Magic, and Doodle
package tests passed 1 of 1, 1 of 1, and 2 of 2 respectively, and the
external CMake consumer linked and exited zero when supplied the matching
`Doodle_SKIA_ROOT`. The macOS and Linux CI definitions now run CTest, and
macOS additionally exercises this installed package chain. These are local
configuration and execution results until the updated CI runs on its hosted
runners.

The same date also produced fresh iPhone 15 Pro simulator captures for iOS
CPU, OpenGL ES, and Metal after the demo was corrected to respect safe-area
insets. The CPU and Metal captures were visually inspected: both show the
shared scene and active runtime identity inside the embedded Board view, an
above-renderer native overlay, and an independently placed native control
below it. The Android visible smoke remains unavailable locally because no
AVD is configured or booted.

An Android arm64 AVD became available later on 2026-07-16. After restarting it
to clear unrelated Messages and System UI ANRs, the current CPU, OpenGL ES,
and Vulkan APKs installed, launched, and produced clean visual captures. The
screens show their runtime identities, the embedded BoardView, a native overlay
above the renderer, and a surrounding native control. The overlay's text also
changed to `Tapped` after an ADB tap, independently of the Doodle surface.

The final local desktop and Web pass on the same tree was successful: the
headless integration test passed; SDL3 CPU, OpenGL, Metal, and Vulkan each
completed their three-frame smoke; the Vulkan validation log was empty; and
the Web demo was served to Safari over loopback HTTP for eight seconds. The
remaining evidence gap is updated hosted CI execution, not Android visual
acceptance or local configuration and compilation.

### Useful Evidence and Examples

Useful concise evidence includes `tests/architecture/check_boundaries.py` for public and layer-boundary enforcement, `tests/configuration/check_combinations.py` for explicit configuration diagnostics, and `tests/integration/headless_skia.cpp` for the deterministic composed rendering path. `doodle/tests/doodle_provider_stubs_test.c` demonstrates that the Blend2D, NanoVG, and Vello getters report unavailable rather than supplying a false renderer.

For human-visible evidence, inspect `artifacts/final/android-cpu-emulator.png`, `artifacts/final/android-opengl-emulator.png`, `artifacts/final/android-vulkan-emulator.png`, `artifacts/final/ios-cpu-simulator.png`, `artifacts/final/ios-opengl-simulator.png`, and `artifacts/final/ios-metal-simulator.png`. Their presence records captures made during execution; visual correctness still needs human inspection. For Web, the reproducible command is `scripts/test-web-skia.sh`, which starts the local HTTP server and records `artifacts/final/web-skia-artifacts.txt`.

### Limitations, Remaining Work, and Open Questions

The plan remains in progress only until the updated CI runs on its hosted
runners and this report is finalized from that evidence. The current mobile
overlay implementation does not support below-renderer ordering. GLFW and
winit remain configuration-fail Board stubs; Blend2D, NanoVG, and Vello remain
configuration-fail Doodle renderer selections backed only by unavailable getter
stubs. Headless GPU contexts are outside the initial scope.

Desktop SDL3 Vulkan is validated only on macOS with Vulkan SDK 1.4.350.1,
MoltenVK, and Skia r5. Windows and Linux need their own SDK/driver runners and
smoke-script executions before this repository can claim support there. Android
CPU, OpenGL ES, and Vulkan have current-tree visible-device verification.

### Possible Article Angles

For C and CMake framework maintainers: “Separating a cross-platform graphics runtime into host, frame, and Canvas layers.” The problem is preventing private platform and renderer types from crossing a public API; the takeaway is the use of versioned opaque capability tables.

For mobile graphics developers: “Making a renderer host embeddable without giving up native controls.” The problem is combining a reusable render view with ordinary Android and iOS controls; the takeaway is an explicit overlay-slot contract and honest z-order limitations.

For WebAssembly maintainers: “Why a browser graphics demo needs an HTTP server, a pinned toolchain, and a font asset.” The problem is that a compiled Web demo can still fail at load or text rendering time; the takeaway is to validate the full asset-loading path rather than treating a generated HTML file as a runnable artifact.

### Suggested Narrative

The strongest narrative starts with the monolithic callback that handed a Canvas directly to application lifecycle code and explains why that made platform hosting and renderer ownership inseparable. It then introduces the three constraints: public headers must remain C-compatible and free of foreign types, every backend choice must be explicit, and one demo scene must remain portable. The implementation sequence is Board surface capabilities, Magic frame interop, then Doodle renderer binding; the Android and macOS Vulkan paths plus mobile overlay slots provide concrete examples. The unexpected Web loader, function-table, and font failures demonstrate why end-to-end validation mattered. Close with the recorded headless hash, configuration tests, the macOS validation-layer smoke result, generated Web artifact evidence, and the remaining mobile/full-matrix work.

### Claims Requiring Human Review

Any external statement that the current tree runs correctly on all mobile combinations requires a new visible simulator/emulator run after the latest BoardView and overlay commits. Screenshot-based visual claims require inspection of the files in `artifacts/final/`; the report records their existence and prior capture, not a fresh visual review. Claims about Windows or Linux desktop Vulkan support, performance, binary-size improvement, production readiness, or external dependency licensing require separate technical and editorial review. Normal review is also required before publishing claims about Apple, Android, SDL3, Skia, Emscripten, or TotalCross behavior.

## Context and Orientation

At the baseline captured for this plan, the project was named `didactic-doodle` and its public headers lived under `include/tc_runtime/`. That baseline implementation mixed application runtime code under `src/runtime/`, native window backends under `src/backends/`, graphics contexts under `src/graphics/`, and renderer adapters under `src/renderers/`. Those retired paths have since been removed from active source; the baseline names remain below only to explain the migration starting point.

The baseline public concepts were:

- `TcApp`, which owns application lifecycle and callbacks named `on_event`, `on_update`, `on_draw`, and `on_shutdown`.
- `TcPlatformBackend`, which owns window or surface creation, events, and scheduling.
- `TcFrameScheduler`, which starts, requests, and stops frame callbacks.
- `TcGraphicsContext`, which represents CPU, OpenGL/OpenGL ES, Metal, or Vulkan context state.
- `TcRenderer2D`, which creates and resizes a renderer and begins or ends frames.
- `TcCanvas2D`, which provides basic 2D drawing and state operations.
- `TcEvent`, which represents quit, resize, pointer, key, text, pause, and resume events.

The baseline source layout was conceptually:

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

The baseline event and frame model was callback-oriented. Native backends converted input into `TcEvent`, dispatched it to the application, and invoked frame callbacks from a platform-native scheduler. The application had no blocking loop. Web used browser animation frames, Android used `AChoreographer`, iOS used `CADisplayLink`, and desktop used an adapter around its window backend.

At the baseline, Skia was the only implemented renderer. It used raster surfaces for CPU, Ganesh GL for OpenGL and WebGL, Metal surfaces created per drawable, and an Android Vulkan swapchain path. NanoVG and Blend2D were stubs; Vello was a stub or documentation-only path. GLFW and winit were also stubs. The migrated tree retains the same explicit-failure policy for unavailable selections.

The baseline root build used these selections:

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

### Milestone 8: Replace the build surface, remove legacy names, complete validation, and finalize the Editorial Report

Update the root `CMakeLists.txt` and modules under `cmake/` to use `BOARD_BACKEND`, `MAGIC_BACKEND`, and `DOODLE_RENDERER`. Detect platform defaults only when the user did not provide an explicit value. Validate unsupported combinations before generating source targets. Do not silently map Web to SDL or fall back from a selected stub.

Provide a transitional CMake compatibility module that recognizes old `TC_*` options, emits a deprecation warning, and maps them to the new values only during intermediate milestones. Remove that module before final completion unless a release policy explicitly requires one compatibility release; if retained, record that policy in the Decision Log and exclude old names from public code symbols and installed headers.

Make each layer install public headers, libraries, version files, and CMake package exports. Add test consumers that use `find_package(Board)`, `find_package(Magic)`, and `find_package(Doodle)` from a clean prefix. Ensure source builds and installed-package builds use the same target names and public include paths.

Enable the architecture checker to fail on all framework-owned `Tc...`, `tc_...`, and legacy `TC_...` names outside intentionally documented third-party or historical artifact files. Remove `include/tc_runtime/`, `src/runtime/`, `src/backends/`, `src/graphics/`, `src/renderers/`, and `compat/` only when their migrated counterparts are complete and no build references remain. Use `git grep`, generated build files, install manifests, exported symbol inspection, and clean builds to prove removal.

Update CI to run focused jobs for public header compilation, boundary checks, standalone staged installation, headless CPU plus Skia, desktop SDL3 CPU plus Skia, desktop SDL3 OpenGL plus Skia, macOS Metal where available, Android CPU/OpenGL ES/Vulkan where available, iOS CPU/Metal where available, and Web. Stub configuration tests run on a lightweight host and assert clear failures. Before marking this milestone complete, reconcile the `Editorial Report` with the final Progress state, `Outcomes & Retrospective`, exact validation results, commits, and generated artifacts; it must distinguish observed results from work that remains unverified.

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

    rm -rf build/desktop/sdl3-cpu-skia
    cmake -S . -B build/desktop/sdl3-cpu-skia \
      -DBOARD_BACKEND=SDL3 \
      -DMAGIC_BACKEND=CPU \
      -DDOODLE_RENDERER=SKIA \
      -DMDB_BUILD_EXAMPLES=ON
    cmake --build build/desktop/sdl3-cpu-skia --parallel
    ctest --test-dir build/desktop/sdl3-cpu-skia --output-on-failure
    ./build/desktop/sdl3-cpu-skia/examples/desktop/magic_doodle_board_demo

Observe a resizable window containing the shared scene. Pointer, key, text, resize, pause or focus where applicable, update timing, and drawing must work.

Desktop SDL3 plus OpenGL:

    rm -rf build/desktop/sdl3-opengl-skia
    cmake -S . -B build/desktop/sdl3-opengl-skia \
      -DBOARD_BACKEND=SDL3 \
      -DMAGIC_BACKEND=OPENGL \
      -DDOODLE_RENDERER=SKIA \
      -DMDB_BUILD_EXAMPLES=ON
    cmake --build build/desktop/sdl3-opengl-skia --parallel
    ctest --test-dir build/desktop/sdl3-opengl-skia --output-on-failure
    ./build/desktop/sdl3-opengl-skia/examples/desktop/magic_doodle_board_demo

The displayed scene and interaction must match the CPU path. Logs may report selected backend names but must not expose implementation types to the application.

Desktop SDL3 plus Metal on macOS:

    cmake -S . -B build/desktop/sdl3-metal-skia \
      -DBOARD_BACKEND=SDL3 \
      -DMAGIC_BACKEND=METAL \
      -DDOODLE_RENDERER=SKIA \
      -DDOODLE_SKIA_ROOT=$PWD/.cache/skia-158dc9d7-r4 \
      -DMDB_BUILD_TESTS=ON \
      -DMDB_BUILD_EXAMPLES=ON
    cmake --build build/desktop/sdl3-metal-skia --parallel
    ctest --test-dir build/desktop/sdl3-metal-skia --output-on-failure
    ./build/desktop/sdl3-metal-skia/examples/desktop/magic_doodle_board_demo --frames 3

The demo must open the SDL3 Metal window, render the shared scene for three
frames, and exit with status 0. This combination is macOS-only.

Desktop SDL3 plus Vulkan on macOS:

    export VULKAN_SDK="$HOME/Library/VulkanSDK/1.4.350.1/macOS"
    bash scripts/fetch-totalcross-skia.sh .cache/skia-158dc9d7-r5 macos-arm64
    VULKAN_SDK="$VULKAN_SDK" ./scripts/test-desktop-vulkan-skia.sh

The wrapper must find the SDK's MoltenVK ICD, configure and build a fresh
`build/desktop/sdl3-vulkan-skia` directory with Skia r5, run the shared
scene for three frames, report `Board: SDL3 ... | Magic: Vulkan ... | Doodle:
Skia ...` in `build/desktop/sdl3-vulkan-skia/runtime-identity.log`, and
leave no `Validation Error` or `ERROR` line in
`build/desktop/sdl3-vulkan-skia/vulkan-validation.log`. This is a
macOS-only validated path; Windows and Linux require separate runner evidence.

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
    Board SDL3 on macOS + Magic Vulkan    + Doodle Skia
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

2026-07-15: Incorporated the mandatory in-progress `Editorial Report` required by `.agent/PLANS.md`. The report synthesizes only recorded implementation, commit, test, artifact, and validation evidence; it identifies remaining work and claims needing human review, and Progress now requires final reconciliation before this ExecPlan can be completed.

2026-07-15: Revised the in-progress plan after Vulkan SDK 1.4.350.1 and the TotalCross Skia r5 macOS archive made desktop Vulkan testable. The revision records the SDL3 + Magic + Doodle implementation, the macOS-only validation-layer result, its named matrix script, standalone package evidence, and the remaining Windows/Linux runner requirement.

2026-07-15: Revised the in-progress plan after refreshed Android OpenGL/Vulkan and iOS Metal captures plus a refreshed Web artifact manifest were recorded. The revision also records the normalized matrix output convention, which preserves each platform and backend combination under `build/<platform>/<board>-<magic>-<renderer>`.
