#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only

"""Dependency-free fixtures for scripts/check_copyright.py."""

from __future__ import annotations

import subprocess
import sys
import tempfile
from pathlib import Path


SCRIPT = Path(__file__).with_name("check_copyright.py")
COPYRIGHT = "SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda."
LICENSE = "SPDX-License-Identifier: LGPL-2.1-only"


def c_header(copyright: str = COPYRIGHT, license: str = LICENSE) -> str:
    return f"/*\n * {copyright}\n * {license}\n */\n\n"


def hash_header(copyright: str = COPYRIGHT, license: str = LICENSE) -> str:
    return f"# {copyright}\n# {license}\n\n"


def run_fixture(files: dict[str, str]) -> subprocess.CompletedProcess[str]:
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = Path(temporary_directory)
        for relative_path, content in files.items():
            target = root / relative_path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(content, encoding="utf-8")
        subprocess.run(["git", "init", "-q"], cwd=root, check=True)
        subprocess.run(["git", "add", "-A"], cwd=root, check=True)
        return subprocess.run([sys.executable, str(SCRIPT)], cwd=root, check=False, capture_output=True, text=True)


def assert_success(files: dict[str, str]) -> None:
    result = run_fixture(files)
    assert result.returncode == 0, result.stdout + result.stderr


def assert_failure(files: dict[str, str], expected: str) -> None:
    result = run_fixture(files)
    assert result.returncode == 1, result.stdout + result.stderr
    assert expected in result.stdout, result.stdout


def test_valid_c_style_header() -> None:
    assert_success({"valid.c": c_header() + "int main(void) { return 0; }\n"})


def test_valid_hash_comment_header() -> None:
    assert_success({"valid.yml": hash_header() + "name: valid\n"})


def test_shebang_precedes_valid_header() -> None:
    assert_success({"valid.sh": "#!/usr/bin/env bash\n" + hash_header() + "exit 0\n"})


def test_missing_copyright_line() -> None:
    assert_failure({"missing.c": c_header(copyright="")}, "missing SPDX-FileCopyrightText")


def test_missing_license_line() -> None:
    assert_failure({"missing.c": c_header(license="")}, "missing SPDX-License-Identifier")


def test_misspelled_company_name() -> None:
    assert_failure({"company.c": c_header(copyright="SPDX-FileCopyrightText: 2026 Amalgam Ltda.")}, 'expected copyright holder "2026 Amalgam Solucoes em TI Ltda."')


def test_wrong_year() -> None:
    assert_failure({"year.c": c_header(copyright="SPDX-FileCopyrightText: 2025 Amalgam Solucoes em TI Ltda.")}, 'expected copyright holder "2026 Amalgam Solucoes em TI Ltda."')


def test_or_later_is_rejected() -> None:
    assert_failure({"license.c": c_header(license="SPDX-License-Identifier: LGPL-2.1-or-later")}, 'expected SPDX license "LGPL-2.1-only"')


def test_third_party_file_is_excluded() -> None:
    assert_success({"vendor/library.c": "int imported(void) { return 1; }\n"})


def test_generated_file_is_excluded() -> None:
    assert_success({"generated/output.c": "int generated(void) { return 1; }\n"})


def test_path_with_spaces() -> None:
    assert_success({"source files/valid file.c": c_header() + "int valid(void) { return 1; }\n"})


def test_diagnostics_are_sorted() -> None:
    result = run_fixture({"z.c": "int z;\n", "a.c": "int a;\n"})
    assert result.returncode == 1, result.stdout + result.stderr
    assert result.stdout.splitlines() == ["a.c: missing SPDX-FileCopyrightText; missing SPDX-License-Identifier", "z.c: missing SPDX-FileCopyrightText; missing SPDX-License-Identifier"], result.stdout


def main() -> int:
    tests = [value for name, value in sorted(globals().items()) if name.startswith("test_")]
    for test in tests:
        test()
    print(f"Copyright validator tests passed: {len(tests)} tests.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
