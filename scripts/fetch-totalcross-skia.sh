#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only

set -euo pipefail

# Downloads an externally published, pinned Skia build through the pinned
# TotalCross depot. It keeps generated headers and libraries outside Git.
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
destination=${1:-"$root/.cache/skia-158dc9d7-r5"}
target=${2:-macos-arm64}
depot_fetch="$root/deps/fetch-depot-tools.sh"
depot_dir=${TOTALCROSS_DEPOT_TOOLS_DIR:-"$root/deps/totalcross-depot-tools"}
font=https://raw.githubusercontent.com/google/skia/158dc9d7d4c/resources/fonts/Roboto-Regular.ttf

case "$target" in
  macos-arm64) platform=macos; arch=arm64; artifact=macos/arm64/libskia.a; extension=a ;;
  linux-x86_64) platform=linux; arch=x86_64; artifact=linux/x86_64/libskia.a; extension=a ;;
  linux-aarch64) platform=linux; arch=aarch64; artifact=linux/aarch64/libskia.a; extension=a ;;
  linux-armv7l) platform=linux; arch=armv7l; artifact=linux/armv7l/libskia.a; extension=a ;;
  android-arm64-v8a) platform=android; arch=arm64-v8a; artifact=android/arm64-v8a/libskia.a; extension=a ;;
  ios-arm64) platform=ios; arch=arm64; artifact=ios/arm64/libskia.a; extension=a ;;
  ios-simulator-arm64) platform=ios-simulator; arch=arm64; artifact=ios-simulator/arm64/libskia.a; extension=a ;;
  wasm32) platform=wasm; arch=wasm32; artifact=wasm/wasm32/libskia.a; extension=a ;;
  windows-x64) platform=windows; arch=x64; artifact=windows/x64/libskia.lib; extension=lib ;;
  windows-arm64) platform=windows; arch=arm64; artifact=windows/arm64/libskia.lib; extension=lib ;;
  windows-x86) platform=windows; arch=x86; artifact=windows/x86/libskia.lib; extension=lib ;;
  *) echo "Unsupported Skia artifact: $target" >&2; exit 2 ;;
esac

[[ -f "$depot_fetch" ]] || { echo "Missing depot bootstrap script: $depot_fetch" >&2; exit 1; }
bash "$depot_fetch" >/dev/null
skia="$depot_dir/skia"
"$skia/fetch.sh" --platform "$platform" --arch "$arch" --install-dev

source_root="$skia/local"
source_library="$source_root/out/Release/$artifact"
[[ -f "$source_root/include/core/SkCanvas.h" && -f "$source_library" ]] || { echo "Incomplete Skia depot artifact for $target" >&2; exit 1; }
rm -rf "$destination"
mkdir -p "$destination/headers/modules/skia"
cp -R "$source_root/include" "$destination/headers/modules/skia/include"
cp -R "$source_root/src" "$destination/headers/modules/skia/src"
cp "$source_library" "$destination/libskia-$target.$extension"
if [[ "$target" == wasm32 ]]; then curl -L --fail --retry 3 -o "$destination/Roboto-Regular.ttf" "$font"; fi
echo "Skia ready at $destination"
