BOARD_BACKEND=HEADLESS + MAGIC_BACKEND=CPU + DOODLE_RENDERER=SKIA: passes; deterministic hash bff7964c10eaa55f
BOARD_BACKEND=SDL3 + MAGIC_BACKEND=CPU + DOODLE_RENDERER=SKIA on macOS: demo builds and starts a native window
BOARD_BACKEND=SDL3 + MAGIC_BACKEND=OPENGL + DOODLE_RENDERER=SKIA on macOS: demo renders three frames and exits 0
BOARD_BACKEND=SDL3 + MAGIC_BACKEND=METAL + DOODLE_RENDERER=SKIA on macOS: demo renders three frames and exits 0
DOODLE_RENDERER=VELLO: configuration fails clearly as unimplemented
