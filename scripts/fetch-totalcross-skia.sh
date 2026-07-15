#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only

set -euo pipefail

# Downloads an externally published, pinned Skia build. It intentionally keeps
# generated headers and libraries outside version control.
destination=${1:-"$PWD/.cache/skia-158dc9d7-r5"}
platform=${2:-macos-arm64}
release=https://github.com/TotalCross/totalcross-depot-tools/releases/download/skia-158dc9d7-r5
font=https://raw.githubusercontent.com/google/skia/158dc9d7d4c/resources/fonts/Roboto-Regular.ttf

case "$platform" in
  macos-arm64|linux-x86_64|linux-aarch64|linux-armv7l|android-arm64-v8a|ios-arm64|ios-simulator-arm64|wasm32|windows-x64|windows-arm64|windows-x86) ;;
  *) echo "Unsupported Skia artifact: $platform" >&2; exit 2 ;;
esac

mkdir -p "$destination"
curl -L --fail --retry 3 -o "$destination/skia-dev-headers.zip" "$release/skia-dev-headers-158dc9d7.zip"
extension=a
if [[ "$platform" == windows-* ]]; then extension=lib; fi
curl -L --fail --retry 3 -o "$destination/libskia-$platform.$extension" "$release/libskia-$platform.$extension"
if [[ "$platform" == wasm32 ]]; then curl -L --fail --retry 3 -o "$destination/Roboto-Regular.ttf" "$font"; fi
unzip -qo "$destination/skia-dev-headers.zip" -d "$destination/headers"
echo "Skia ready at $destination"
