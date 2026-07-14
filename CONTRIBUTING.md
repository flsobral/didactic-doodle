# Contributing

## Copyright headers

New first-party source and build files must include these SPDX metadata lines:

    SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
    SPDX-License-Identifier: LGPL-2.1-only

Use the comment syntax appropriate for the file type. Preserve shebangs, XML
declarations, encoding declarations, and other required first-line constructs.

Do not replace or modify copyright and license notices in third-party,
vendored, generated, or separately licensed files. The Gradle wrapper under
`android/gradle/wrapper/` is one such excluded dependency.

Before submitting a pull request, run:

    python3 scripts/check_copyright.py
    python3 scripts/test_check_copyright.py
