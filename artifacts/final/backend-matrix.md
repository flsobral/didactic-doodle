BOARD_BACKEND=HEADLESS + MAGIC_BACKEND=CPU + DOODLE_RENDERER=SKIA: passes; deterministic hash bff7964c10eaa55f
BOARD_BACKEND=SDL3 + MAGIC_BACKEND=CPU + DOODLE_RENDERER=SKIA on macOS: demo builds and starts a native window
BOARD_BACKEND=SDL3 + MAGIC_BACKEND=OPENGL + DOODLE_RENDERER=SKIA on macOS: demo renders three frames and exits 0
BOARD_BACKEND=SDL3 + MAGIC_BACKEND=METAL + DOODLE_RENDERER=SKIA on macOS: demo renders three frames and exits 0
BOARD_BACKEND=SDL3 + MAGIC_BACKEND=VULKAN + DOODLE_RENDERER=SKIA on macOS: demo renders three frames and exits 0 with VK_LAYER_KHRONOS_validation enabled
BOARD_BACKEND=IOS_NATIVE + MAGIC_BACKEND=CPU + DOODLE_RENDERER=SKIA in iOS simulator: app installs, launches, and renders the shared scene
BOARD_BACKEND=IOS_NATIVE + MAGIC_BACKEND=OPENGL + DOODLE_RENDERER=SKIA in iOS simulator: app installs, launches, and renders the shared scene through OpenGL ES
BOARD_BACKEND=IOS_NATIVE + MAGIC_BACKEND=METAL + DOODLE_RENDERER=SKIA in iOS simulator: app installs, launches, and renders the shared scene through Metal
BOARD_BACKEND=ANDROID_NATIVE + MAGIC_BACKEND=CPU + DOODLE_RENDERER=SKIA in Android emulator: app installs, launches, and renders the shared scene through the native CPU surface
BOARD_BACKEND=ANDROID_NATIVE + MAGIC_BACKEND=OPENGL + DOODLE_RENDERER=SKIA in Android emulator: app installs, launches, and renders the shared scene through OpenGL ES
BOARD_BACKEND=ANDROID_NATIVE + MAGIC_BACKEND=VULKAN + DOODLE_RENDERER=SKIA in Android emulator: app installs, launches, and renders the shared scene through Vulkan
BOARD_BACKEND=WEB + MAGIC_BACKEND=WEB + DOODLE_RENDERER=SKIA in Safari: emrun serves and launches the shared scene through WebGL2
DOODLE_RENDERER=VELLO: configuration fails clearly as unimplemented
