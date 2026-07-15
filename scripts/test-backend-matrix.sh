#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
combination=${1:-}
build_root=${MDB_MATRIX_BUILD_ROOT:-"$root/build/backend-matrix"}
mobile_delay=${MDB_MOBILE_DELAY_SECONDS:-3}

fail() { echo "error: $*" >&2; exit 2; }
require_directory() { [[ -d "$1" ]] || fail "missing directory: $1"; }
require_file() { [[ -f "$1" ]] || fail "missing file: $1"; }

desktop() {
  local backend=$1 build="$build_root/desktop-$backend"
  local skia_root=${MDB_DESKTOP_SKIA_ROOT:-"$root/.cache/skia-158dc9d7-r4"}
  [[ $backend != METAL || $(uname) == Darwin ]] || fail "desktop Metal requires macOS"
  require_file "$skia_root/headers/modules/skia/include/core/SkCanvas.h"
  cmake -S "$root" -B "$build" -DBOARD_BACKEND=SDL3 -DMAGIC_BACKEND="$backend" -DDOODLE_RENDERER=SKIA -DDOODLE_SKIA_ROOT="$skia_root" -DMDB_BUILD_EXAMPLES=ON -DMDB_BUILD_TESTS=ON
  cmake --build "$build" --parallel
  "$build/examples/desktop/magic_doodle_board_demo" --frames 3
}

headless() {
  local build="$build_root/headless-cpu"
  local skia_root=${MDB_DESKTOP_SKIA_ROOT:-"$root/.cache/skia-158dc9d7-r4"}
  require_file "$skia_root/headers/modules/skia/include/core/SkCanvas.h"
  cmake -S "$root" -B "$build" -DBOARD_BACKEND=HEADLESS -DMAGIC_BACKEND=CPU -DDOODLE_RENDERER=SKIA -DDOODLE_SKIA_ROOT="$skia_root" -DMDB_BUILD_TESTS=ON -DMDB_BUILD_EXAMPLES=OFF
  cmake --build "$build" --parallel
  ctest --test-dir "$build" --output-on-failure -R '^mdb_headless_skia$'
}

ios() {
  local backend=$1 build="$build_root/ios-$backend"
  local skia_root=${MDB_IOS_SKIA_ROOT:-"$root/.cache/skia-158dc9d7-r4"}
  local png_root=${MDB_IOS_PNG_ROOT:-"$root/.cache/libpng-ios-sim/libpng/ios-simulator/arm64"}
  local zlib_root=${MDB_IOS_ZLIB_ROOT:-"$root/.cache/zlib-ng-ios-sim/zlib-ng/ios-simulator/arm64"}
  [[ $(uname) == Darwin ]] || fail "iOS simulator validation requires macOS"
  require_file "$skia_root/headers/modules/skia/include/core/SkCanvas.h"
  require_directory "$png_root"
  require_directory "$zlib_root"
  xcrun simctl bootstatus booted -b
  cmake -S "$root" -B "$build" -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator -DCMAKE_OSX_ARCHITECTURES=arm64 -DBOARD_BACKEND=IOS_NATIVE -DMAGIC_BACKEND="$backend" -DDOODLE_RENDERER=SKIA -DDOODLE_SKIA_ROOT="$skia_root" -DDOODLE_IOS_PNG_ROOT="$png_root" -DDOODLE_IOS_ZLIB_ROOT="$zlib_root" -DMDB_BUILD_TESTS=OFF -DMDB_BUILD_EXAMPLES=ON
  cmake --build "$build" --parallel
  xcrun simctl install booted "$build/magic_doodle_board_ios_demo.app"
  xcrun simctl launch booted com.amalgam.magicdoodleboard.demo
  sleep "$mobile_delay"
  xcrun simctl io booted screenshot "$root/artifacts/final/ios-$(printf '%s' "$backend" | tr '[:upper:]' '[:lower:]')-simulator.png"
}

android() {
  local backend=$1
  local android_home=${ANDROID_HOME:-"$HOME/Library/Android/sdk"}
  local adb=${ADB:-"$android_home/platform-tools/adb"}
  local skia_root=${MDB_ANDROID_SKIA_ROOT:-"$root/.cache/skia-android-r4"}
  local png_root=${MDB_ANDROID_PNG_ROOT:-"$root/.cache/libpng-android/libpng/android/arm64-v8a"}
  local zlib_root=${MDB_ANDROID_ZLIB_ROOT:-"$root/.cache/zlib-ng-android/zlib-ng/android/arm64-v8a"}
  require_file "$adb"
  require_file "$skia_root/headers/modules/skia/include/core/SkCanvas.h"
  require_directory "$png_root"
  require_directory "$zlib_root"
  "$adb" wait-for-device
  [[ $("$adb" shell getprop sys.boot_completed | tr -d '\r') == 1 ]] || fail "an Android emulator must be fully booted"
  (cd "$root/android" && ANDROID_HOME="$android_home" ./gradlew :app:assembleDebug -PdoodleSkiaRoot="$skia_root" -PdoodleAndroidPngRoot="$png_root" -PdoodleAndroidZlibRoot="$zlib_root" -PmdbAndroidMagicBackend="$backend")
  "$adb" install -r "$root/android/app/build/outputs/apk/debug/app-debug.apk"
  "$adb" shell am force-stop com.amalgam.magicdoodleboard.demo
  "$adb" shell am start -n com.amalgam.magicdoodleboard.demo/android.app.NativeActivity
  sleep "$mobile_delay"
  "$adb" shell pidof com.amalgam.magicdoodleboard.demo >/dev/null || fail "Android demo exited before validation"
  "$adb" exec-out screencap -p > "$root/artifacts/final/android-$(printf '%s' "$backend" | tr '[:upper:]' '[:lower:]')-emulator.png"
}

web() {
  local build="$build_root/web-skia"
  local skia_root=${MDB_WEB_SKIA_ROOT:-"$root/.cache/skia-wasm32-r4"}
  local emcmake=${EMCMAKE:-"$root/.cache/emsdk-main/upstream/emscripten/emcmake"}
  local browser=${MDB_WEB_BROWSER:-safari}
  local timeout=${MDB_WEB_TIMEOUT_SECONDS:-8}
  local html js wasm
  if [[ ! -x "$emcmake" ]]; then emcmake=$(command -v emcmake || true); fi
  [[ -n "$emcmake" ]] || fail "Emscripten emcmake is required; set EMCMAKE or install the pinned Emscripten 2.0.6 toolchain"
  require_file "$skia_root/headers/modules/skia/include/core/SkCanvas.h"
  "$emcmake" cmake -S "$root" -B "$build" -DBOARD_BACKEND=WEB -DMAGIC_BACKEND=WEB -DDOODLE_RENDERER=SKIA -DDOODLE_SKIA_ROOT="$skia_root" -DMDB_BUILD_TESTS=OFF -DMDB_BUILD_EXAMPLES=ON
  cmake --build "$build" --parallel
  html="$build/examples/web/magic_doodle_board_web_demo.html"
  js="$build/examples/web/magic_doodle_board_web_demo.js"
  wasm="$build/examples/web/magic_doodle_board_web_demo.wasm"
  require_file "$html"; require_file "$js"; require_file "$wasm"
  command -v emrun >/dev/null || fail "emrun is required to launch the Web demo"
  mkdir -p "$root/artifacts/final"
  emrun --browser "$browser" --timeout "$timeout" --timeout-returncode 0 "$html"
  {
    printf 'web-skia browser smoke run completed with %s for %s seconds\n' "$browser" "$timeout"
    printf 'magic_doodle_board_web_demo.html: %s bytes\n' "$(wc -c < "$html")"
    printf 'magic_doodle_board_web_demo.js: %s bytes\n' "$(wc -c < "$js")"
    printf 'magic_doodle_board_web_demo.wasm: %s bytes\n' "$(wc -c < "$wasm")"
  } > "$root/artifacts/final/web-skia-artifacts.txt"
}

case "$combination" in
  headless-cpu-skia) headless ;;
  desktop-cpu-skia) desktop CPU ;;
  desktop-opengl-skia) desktop OPENGL ;;
  desktop-metal-skia) desktop METAL ;;
  ios-cpu-skia) ios CPU ;;
  ios-opengl-skia) ios OPENGL ;;
  ios-metal-skia) ios METAL ;;
  android-cpu-skia) android CPU ;;
  android-opengl-skia) android OPENGL ;;
  android-vulkan-skia) android VULKAN ;;
  web-skia) web ;;
  *) fail "usage: $0 {headless-cpu-skia|desktop-cpu-skia|desktop-opengl-skia|desktop-metal-skia|ios-cpu-skia|ios-opengl-skia|ios-metal-skia|android-cpu-skia|android-opengl-skia|android-vulkan-skia|web-skia}" ;;
esac
