#!/usr/bin/env python3
"""Reject public foreign declarations and private cross-layer includes."""
import pathlib
import re
import sys

root = pathlib.Path(sys.argv[1])
violations = []
foreign = re.compile(r"\b(SDL|GLFW|Emscripten|JNIEnv|jobject|UIKit|UIView|CAMetal|MTL[A-Z]|Vk[A-Z]|GL[A-Z]|Sk[A-Z]|std::|rust::)")
for layer in ("board", "magic", "doodle"):
    for path in (root / layer / "include" / layer).glob("*.h"):
        if foreign.search(path.read_text()):
            violations.append(f"public foreign-type violation: {path.relative_to(root)}")
for path in (root / "board").rglob("*.[ch]"):
    if "#include <magic/" in path.read_text() or "#include <doodle/" in path.read_text():
        violations.append(f"boundary violation: {path.relative_to(root)} includes an upper layer")
for path in (root / "magic").rglob("*.[ch]"):
    source = path.read_text()
    if "#include <doodle/" in source or "../board/" in source or "board/src/" in source:
        violations.append(f"boundary violation: {path.relative_to(root)} includes a forbidden layer or private path")
for path in (root / "doodle").rglob("*.[ch]"):
    source = path.read_text()
    if "#include <board/" in source or "../magic/" in source or "magic/src/" in source:
        violations.append(f"boundary violation: {path.relative_to(root)} includes Board or Magic private code")
if violations:
    print("\n".join(violations))
    sys.exit(1)
print("boundary check passed")
