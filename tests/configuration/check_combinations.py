#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only
"""Exercise root backend-selection diagnostics from a disposable build tree."""

import pathlib
import subprocess
import sys
import tempfile


root = pathlib.Path(sys.argv[1]).resolve()
cmake = sys.argv[2]
base_arguments = ["-DMDB_BUILD_TESTS=OFF", "-DMDB_BUILD_EXAMPLES=OFF"]
cases = (
    ("headless_cpu", ("-DBOARD_BACKEND=HEADLESS", "-DMAGIC_BACKEND=CPU", "-DDOODLE_RENDERER=NONE"), True, ""),
    ("headless_opengl", ("-DBOARD_BACKEND=HEADLESS", "-DMAGIC_BACKEND=OPENGL", "-DDOODLE_RENDERER=NONE"), False, "BOARD_BACKEND=HEADLESS supports only MAGIC_BACKEND=CPU"),
    ("glfw_stub", ("-DBOARD_BACKEND=GLFW", "-DMAGIC_BACKEND=CPU", "-DDOODLE_RENDERER=NONE"), False, "BOARD_BACKEND=GLFW is declared but not implemented"),
    ("vello_stub", ("-DBOARD_BACKEND=HEADLESS", "-DMAGIC_BACKEND=CPU", "-DDOODLE_RENDERER=VELLO"), False, "DOODLE_RENDERER=VELLO is declared but not implemented"),
    ("web_without_emscripten", ("-DBOARD_BACKEND=WEB", "-DMAGIC_BACKEND=WEB", "-DDOODLE_RENDERER=NONE"), False, "BOARD_BACKEND=WEB requires the Emscripten CMake toolchain"),
)

with tempfile.TemporaryDirectory(prefix="magic-doodle-board-configuration-") as temporary:
    build_root = pathlib.Path(temporary)
    for name, arguments, expected_success, diagnostic in cases:
        result = subprocess.run(
            [cmake, "-S", str(root), "-B", str(build_root / name), *base_arguments, *arguments],
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        if (result.returncode == 0) != expected_success:
            print(f"configuration case {name} returned {result.returncode}; expected success={expected_success}")
            print(result.stdout)
            sys.exit(1)
        if diagnostic and diagnostic not in result.stdout:
            print(f"configuration case {name} did not report: {diagnostic}")
            print(result.stdout)
            sys.exit(1)

print("configuration matrix check passed")
