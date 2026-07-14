#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only

"""Validate canonical SPDX headers in tracked first-party source files."""

from __future__ import annotations

import subprocess
import sys
from pathlib import PurePosixPath


COPYRIGHT = "SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda."
LICENSE = "SPDX-License-Identifier: LGPL-2.1-only"
SOURCE_SUFFIXES = {".c", ".cpp", ".gradle", ".h", ".html", ".mm", ".py", ".sh", ".xml", ".yml"}
EXCLUDED_DIRECTORY_NAMES = {"build", "dist", "external", "gen", "generated", "node_modules", "out", "target", "third-party", "third_party", "vendor"}
EXCLUDED_PREFIXES = ("android/gradle/wrapper/",)


def tracked_paths() -> list[str]:
    result = subprocess.run(["git", "ls-files", "-z"], check=True, capture_output=True)
    return sorted(path.decode("utf-8", "surrogateescape") for path in result.stdout.split(b"\0") if path)


def is_excluded(path: str) -> bool:
    return path.startswith(EXCLUDED_PREFIXES) or bool(set(PurePosixPath(path).parts) & EXCLUDED_DIRECTORY_NAMES)


def is_candidate(path: str) -> bool:
    if is_excluded(path):
        return False
    name = PurePosixPath(path).name
    return name == "CMakeLists.txt" or path == "android/gradle.properties.example" or PurePosixPath(path).suffix in SOURCE_SUFFIXES


def diagnose(path: str) -> str | None:
    try:
        with open(path, "r", encoding="utf-8", errors="replace", newline="") as source:
            header = "".join(source.readlines()[:20])
    except OSError as error:
        return f"{path}: cannot read file: {error.strerror}"

    issues: list[str] = []
    copyright_lines = [line for line in header.splitlines() if "SPDX-FileCopyrightText:" in line]
    if not any(COPYRIGHT in line for line in copyright_lines):
        if copyright_lines or "Copyright" in header:
            issues.append('expected copyright holder "2026 Amalgam Solucoes em TI Ltda."')
        else:
            issues.append("missing SPDX-FileCopyrightText")
    license_lines = [line for line in header.splitlines() if "SPDX-License-Identifier:" in line]
    if not any(LICENSE in line for line in license_lines):
        if license_lines:
            issues.append('expected SPDX license "LGPL-2.1-only"')
        else:
            issues.append("missing SPDX-License-Identifier")
    return f"{path}: {'; '.join(issues)}" if issues else None


def main() -> int:
    paths = tracked_paths()
    failures = [message for path in paths if is_candidate(path) if (message := diagnose(path)) is not None]
    if failures:
        print("\n".join(failures))
        return 1
    print(f"Copyright validation passed: {sum(is_candidate(path) for path in paths)} applicable files checked.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
