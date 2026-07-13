#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only

set -euo pipefail

# Downloads an externally published, pinned Skia build. It intentionally keeps
# generated headers and libraries outside version control.
destination=${1:-"$PWD/.cache/skia-158dc9d7"}
platform=${2:-macos-arm64}
release=https://github.com/TotalCross/totalcross-depot-tools/releases/download/skia-158dc9d7-r3

case "$platform" in
  macos-arm64|linux-x86_64|linux-aarch64|linux-armv7l|android-arm64-v8a) ;;
  *) echo "Unsupported Skia artifact: $platform" >&2; exit 2 ;;
esac

mkdir -p "$destination"
curl -L --fail --retry 3 -o "$destination/skia-dev-headers.zip" "$release/skia-dev-headers-158dc9d7.zip"
curl -L --fail --retry 3 -o "$destination/libskia-$platform.a" "$release/libskia-$platform.a"
unzip -qo "$destination/skia-dev-headers.zip" -d "$destination/headers"
echo "Skia ready at $destination"
