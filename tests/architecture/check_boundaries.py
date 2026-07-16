#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only
"""Reject public foreign declarations and private cross-layer includes."""
import pathlib
import re
import sys

root = pathlib.Path(sys.argv[1])
violations = []
foreign = re.compile(r"\b(SDL|GLFW|Emscripten|JNIEnv|jobject|UIKit|UIView|CAMetal|MTL[A-Z]|Vk[A-Z]|GL[A-Z]|Sk[A-Z]|std::|rust::)")
legacy = re.compile(r"\b(?:Tc[A-Za-z0-9_]*|tc_[A-Za-z0-9_]*|TC_[A-Z][A-Z0-9_]*)")
generated_directories = {"build", ".cxx", "intermediates", ".gradle"}
for layer in ("board", "magic", "doodle"):
    for path in (root / layer / "include" / layer).glob("*.h"):
        if foreign.search(path.read_text()):
            violations.append(f"public foreign-type violation: {path.relative_to(root)}")
for path in (root / "board").rglob("*"):
    if path.suffix not in {".c", ".cc", ".cpp", ".mm", ".h"}: continue
    if "#include <magic/" in path.read_text() or "#include <doodle/" in path.read_text():
        violations.append(f"boundary violation: {path.relative_to(root)} includes an upper layer")
for path in (root / "magic").rglob("*"):
    if path.suffix not in {".c", ".cc", ".cpp", ".mm", ".h"}: continue
    source = path.read_text()
    if "#include <doodle/" in source or "../board/" in source or "board/src/" in source:
        violations.append(f"boundary violation: {path.relative_to(root)} includes a forbidden layer or private path")
for path in (root / "doodle").rglob("*"):
    if path.suffix not in {".c", ".cc", ".cpp", ".mm", ".h"}: continue
    source = path.read_text()
    if "#include <board/" in source or "../magic/" in source or "magic/src/" in source:
        violations.append(f"boundary violation: {path.relative_to(root)} includes Board or Magic private code")
for directory in ("board", "magic", "doodle", "examples", "android", "ios"):
    for path in (root / directory).rglob("*"):
        if generated_directories.intersection(path.relative_to(root).parts):
            continue
        if path.suffix not in {".c", ".cc", ".cpp", ".mm", ".h", ".cmake", ".txt"}:
            continue
        if legacy.search(path.read_text()):
            violations.append(f"legacy naming violation: {path.relative_to(root)}")
for path in (root / "CMakeLists.txt",):
    if legacy.search(path.read_text()):
        violations.append(f"legacy naming violation: {path.relative_to(root)}")
if violations:
    print("\n".join(violations))
    sys.exit(1)
print("boundary check passed")
