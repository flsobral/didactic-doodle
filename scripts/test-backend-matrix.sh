#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
combination=${1:-}
build_root=${MDB_MATRIX_BUILD_ROOT:-"$root/build"}
mobile_delay=${MDB_MOBILE_DELAY_SECONDS:-3}
web_server_pid=
web_server_log=

fail() { echo "error: $*" >&2; exit 2; }
require_directory() { [[ -d "$1" ]] || fail "missing directory: $1"; }
require_file() { [[ -f "$1" ]] || fail "missing file: $1"; }
stop_web_server() {
  if [[ -n ${web_server_pid:-} ]]; then
    kill "$web_server_pid" 2>/dev/null || true
    wait "$web_server_pid" 2>/dev/null || true
    web_server_pid=
  fi
  if [[ -n ${web_server_log:-} ]]; then
    rm -f "$web_server_log"
    web_server_log=
  fi
}

desktop() {
  local backend=$1
  local backend_name
  backend_name=$(printf '%s' "$backend" | tr '[:upper:]' '[:lower:]')
  local build="$build_root/desktop/sdl3-$backend_name-skia"
  local skia_root=${MDB_DESKTOP_SKIA_ROOT:-"$root/.cache/skia-158dc9d7-r4"}
  local vulkan_sdk validation_log runtime_log
  [[ $backend != METAL || $(uname) == Darwin ]] || fail "desktop Metal requires macOS"
  if [[ $backend == VULKAN ]]; then
    [[ $(uname) == Darwin ]] || fail "desktop Vulkan smoke validation is currently available only on macOS; Windows and Linux still require their own runners"
    vulkan_sdk=${MDB_VULKAN_SDK:-${VULKAN_SDK:-}}
    [[ -n $vulkan_sdk ]] || fail "desktop Vulkan requires MDB_VULKAN_SDK or VULKAN_SDK"
    require_directory "$vulkan_sdk"
    require_file "$vulkan_sdk/bin/vulkaninfo"
    require_file "$vulkan_sdk/share/vulkan/icd.d/MoltenVK_icd.json"
    skia_root=${MDB_DESKTOP_VULKAN_SKIA_ROOT:-"$root/.cache/skia-158dc9d7-r5"}
    export VULKAN_SDK="$vulkan_sdk"
    export DYLD_LIBRARY_PATH="$vulkan_sdk/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}"
    export VK_ICD_FILENAMES="${MDB_VULKAN_ICD:-$vulkan_sdk/share/vulkan/icd.d/MoltenVK_icd.json}"
    export VK_LAYER_PATH="${MDB_VULKAN_LAYER_PATH:-$vulkan_sdk/share/vulkan/explicit_layer.d}"
    export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation
    "$vulkan_sdk/bin/vulkaninfo" --summary >/dev/null
  fi
  require_file "$skia_root/headers/modules/skia/include/core/SkCanvas.h"
  cmake -S "$root" -B "$build" -DBOARD_BACKEND=SDL3 -DMAGIC_BACKEND="$backend" -DDOODLE_RENDERER=SKIA -DDOODLE_SKIA_ROOT="$skia_root" -DMDB_BUILD_EXAMPLES=ON -DMDB_BUILD_TESTS=ON
  cmake --build "$build" --parallel
  [[ ${MDB_BUILD_ONLY:-0} == 1 ]] && return
  if [[ $backend == VULKAN ]]; then
    validation_log="$build/vulkan-validation.log"
    runtime_log="$build/runtime-identity.log"
    "$build/examples/desktop/magic_doodle_board_demo" --frames 3 >"$runtime_log" 2>"$validation_log"
    if rg -q 'Validation Error|\bERROR\b' "$validation_log"; then cat "$validation_log" >&2; fail "Vulkan validation reported an error"; fi
    if ! rg -q '^Board: SDL3 .+ \| Magic: Vulkan .+ \| Doodle: Skia .+$' "$runtime_log"; then cat "$runtime_log" >&2; fail "desktop Vulkan demo did not create and report the requested runtime combination"; fi
    cat "$runtime_log"
  else
    "$build/examples/desktop/magic_doodle_board_demo" --frames 3
  fi
}

headless() {
  local build="$build_root/headless/headless-cpu-skia"
  local skia_root=${MDB_DESKTOP_SKIA_ROOT:-"$root/.cache/skia-158dc9d7-r4"}
  require_file "$skia_root/headers/modules/skia/include/core/SkCanvas.h"
  cmake -S "$root" -B "$build" -DBOARD_BACKEND=HEADLESS -DMAGIC_BACKEND=CPU -DDOODLE_RENDERER=SKIA -DDOODLE_SKIA_ROOT="$skia_root" -DMDB_BUILD_TESTS=ON -DMDB_BUILD_EXAMPLES=OFF
  cmake --build "$build" --parallel
  [[ ${MDB_BUILD_ONLY:-0} == 1 ]] && return
  ctest --test-dir "$build" --output-on-failure -R '^mdb_headless_skia$'
}

ios() {
  local backend=$1
  local backend_name
  backend_name=$(printf '%s' "$backend" | tr '[:upper:]' '[:lower:]')
  local build="$build_root/ios/native-$backend_name-skia"
  local skia_root=${MDB_IOS_SKIA_ROOT:-"$root/.cache/skia-158dc9d7-r4"}
  local png_root=${MDB_IOS_PNG_ROOT:-"$root/.cache/libpng-ios-sim/libpng/ios-simulator/arm64"}
  local zlib_root=${MDB_IOS_ZLIB_ROOT:-"$root/.cache/zlib-ng-ios-sim/zlib-ng/ios-simulator/arm64"}
  [[ $(uname) == Darwin ]] || fail "iOS simulator validation requires macOS"
  require_file "$skia_root/headers/modules/skia/include/core/SkCanvas.h"
  require_directory "$png_root"
  require_directory "$zlib_root"
  if [[ ${MDB_BUILD_ONLY:-0} != 1 ]]; then xcrun simctl bootstatus booted -b; fi
  cmake -S "$root" -B "$build" -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator -DCMAKE_OSX_ARCHITECTURES=arm64 -DBOARD_BACKEND=IOS_NATIVE -DMAGIC_BACKEND="$backend" -DDOODLE_RENDERER=SKIA -DDOODLE_SKIA_ROOT="$skia_root" -DDOODLE_IOS_PNG_ROOT="$png_root" -DDOODLE_IOS_ZLIB_ROOT="$zlib_root" -DMDB_BUILD_TESTS=OFF -DMDB_BUILD_EXAMPLES=ON
  cmake --build "$build" --parallel
  [[ ${MDB_BUILD_ONLY:-0} == 1 ]] && return
  xcrun simctl install booted "$build/magic_doodle_board_ios_demo.app"
  xcrun simctl launch booted com.amalgam.magicdoodleboard.demo
  sleep "$mobile_delay"
  xcrun simctl io booted screenshot "$root/artifacts/final/ios-$(printf '%s' "$backend" | tr '[:upper:]' '[:lower:]')-simulator.png"
}

android() {
  local backend=$1
  local android_home=${ANDROID_HOME:-"$HOME/Library/Android/sdk"}
  local adb=${ADB:-"$android_home/platform-tools/adb"}
  local boot_timeout=${MDB_ANDROID_BOOT_TIMEOUT_SECONDS:-60}
  local skia_root=${MDB_ANDROID_SKIA_ROOT:-"$root/.cache/skia-android-r4"}
  local png_root=${MDB_ANDROID_PNG_ROOT:-"$root/.cache/libpng-android/libpng/android/arm64-v8a"}
  local zlib_root=${MDB_ANDROID_ZLIB_ROOT:-"$root/.cache/zlib-ng-android/zlib-ng/android/arm64-v8a"}
  local backend_name artifact_directory artifact_name
  backend_name=$(printf '%s' "$backend" | tr '[:upper:]' '[:lower:]')
  require_file "$skia_root/headers/modules/skia/include/core/SkCanvas.h"
  require_directory "$png_root"
  require_directory "$zlib_root"
  (cd "$root/android" && ANDROID_HOME="$android_home" ./gradlew :app:assembleDebug -PdoodleSkiaRoot="$skia_root" -PdoodleAndroidPngRoot="$png_root" -PdoodleAndroidZlibRoot="$zlib_root" -PmdbAndroidMagicBackend="$backend")
  artifact_directory="$build_root/android/native-$backend_name-skia"
  artifact_name="magic_doodle_board_android_${backend_name}_demo.apk"
  mkdir -p "$artifact_directory"
  cp "$root/android/app/build/outputs/apk/debug/app-debug.apk" "$artifact_directory/$artifact_name"
  printf 'Android %s APK: %s\n' "$backend" "$artifact_directory/$artifact_name"
  [[ ${MDB_BUILD_ONLY:-0} == 1 ]] && return
  require_file "$adb"
  [[ $boot_timeout =~ ^[0-9]+$ ]] && ((boot_timeout > 0)) || fail "MDB_ANDROID_BOOT_TIMEOUT_SECONDS must be a positive number of seconds"
  local deadline=$(( $(date +%s) + boot_timeout ))
  while [[ $(date +%s) -lt $deadline ]]; do
    if [[ $("$adb" get-state 2>/dev/null || true) == device ]] && [[ $("$adb" shell getprop sys.boot_completed 2>/dev/null | tr -d '\r') == 1 ]]; then break; fi
    sleep 1
  done
  [[ $("$adb" get-state 2>/dev/null || true) == device ]] && [[ $("$adb" shell getprop sys.boot_completed 2>/dev/null | tr -d '\r') == 1 ]] || fail "no fully booted Android emulator was detected within ${boot_timeout}s; start an AVD or set ADB"
  "$adb" install -r "$root/android/app/build/outputs/apk/debug/app-debug.apk"
  "$adb" shell am force-stop com.amalgam.magicdoodleboard.demo
  "$adb" shell am start -n com.amalgam.magicdoodleboard.demo/.MagicDoodleBoardActivity
  sleep "$mobile_delay"
  "$adb" shell pidof com.amalgam.magicdoodleboard.demo >/dev/null || fail "Android demo exited before validation"
  "$adb" exec-out screencap -p > "$root/artifacts/final/android-$(printf '%s' "$backend" | tr '[:upper:]' '[:lower:]')-emulator.png"
}

web() {
  local build="$build_root/web/web-web-skia"
  local skia_root=${MDB_WEB_SKIA_ROOT:-"$root/.cache/skia-wasm32-r4"}
  local emcmake=${EMCMAKE:-"$root/.cache/emsdk-main/upstream/emscripten/emcmake"}
  local browser=${MDB_WEB_BROWSER:-safari}
  local timeout=${MDB_WEB_TIMEOUT_SECONDS:-8}
  local port=${MDB_WEB_PORT:-}
  local html js wasm data font web_directory url
  if [[ ! -x "$emcmake" ]]; then emcmake=$(command -v emcmake || true); fi
  [[ -n "$emcmake" ]] || fail "Emscripten emcmake is required; set EMCMAKE or install the pinned Emscripten 2.0.6 toolchain"
  require_file "$skia_root/headers/modules/skia/include/core/SkCanvas.h"
  "$emcmake" cmake -S "$root" -B "$build" -DBOARD_BACKEND=WEB -DMAGIC_BACKEND=WEB -DDOODLE_RENDERER=SKIA -DDOODLE_SKIA_ROOT="$skia_root" -DMDB_BUILD_TESTS=OFF -DMDB_BUILD_EXAMPLES=ON
  cmake --build "$build" --parallel
  html="$build/examples/web/magic_doodle_board_web_demo.html"
  js="$build/examples/web/magic_doodle_board_web_demo.js"
  wasm="$build/examples/web/magic_doodle_board_web_demo.wasm"
  data="$build/examples/web/magic_doodle_board_web_demo.data"
  font="$skia_root/Roboto-Regular.ttf"
  require_file "$html"; require_file "$js"; require_file "$wasm"; require_file "$data"; require_file "$font"
  [[ ${MDB_BUILD_ONLY:-0} == 1 ]] && return
  command -v python3 >/dev/null || fail "python3 is required to serve the Web demo"
  command -v curl >/dev/null || fail "curl is required to verify the local Web server"
  if [[ -n $port ]]; then
    [[ $port =~ ^[0-9]+$ ]] && ((port > 0 && port < 65536)) || fail "MDB_WEB_PORT must be a TCP port from 1 to 65535"
  else
    port=$(python3 -c 'import socket; listener = socket.socket(); listener.bind(("127.0.0.1", 0)); print(listener.getsockname()[1]); listener.close()')
  fi
  mkdir -p "$root/artifacts/final"
  web_directory=$(dirname "$html")
  url="http://127.0.0.1:$port/$(basename "$html")"
  web_server_log=$(mktemp "${TMPDIR:-/tmp}/magic-doodle-board-web-http.XXXXXX.log")
  python3 -m http.server "$port" --bind 127.0.0.1 --directory "$web_directory" >"$web_server_log" 2>&1 &
  web_server_pid=$!
  trap stop_web_server EXIT
  if ! kill -0 "$web_server_pid" 2>/dev/null; then
    cat "$web_server_log" >&2
    fail "local Web server could not start on 127.0.0.1:$port"
  fi
  if ! curl --retry 20 --retry-connrefused --retry-delay 0 --silent --fail "$url" >/dev/null; then
    cat "$web_server_log" >&2
    fail "local Web server did not serve $url"
  fi
  if [[ $(uname) == Darwin ]]; then
    command -v open >/dev/null || fail "macOS open command is required to launch the Web demo"
    open -a "$browser" "$url"
  else
    command -v "$browser" >/dev/null || fail "browser executable '$browser' was not found"
    "$browser" "$url" >/dev/null 2>&1 &
  fi
  sleep "$timeout"
  {
    printf 'web-skia browser smoke run completed with %s over HTTP for %s seconds\n' "$browser" "$timeout"
    printf 'magic_doodle_board_web_demo.html: %s bytes\n' "$(wc -c < "$html")"
    printf 'magic_doodle_board_web_demo.js: %s bytes\n' "$(wc -c < "$js")"
    printf 'magic_doodle_board_web_demo.wasm: %s bytes\n' "$(wc -c < "$wasm")"
    printf 'magic_doodle_board_web_demo.data: %s bytes\n' "$(wc -c < "$data")"
  } > "$root/artifacts/final/web-skia-artifacts.txt"
}

build_all() {
  local supported=(headless-cpu-skia desktop-cpu-skia desktop-opengl-skia desktop-metal-skia desktop-vulkan-skia ios-cpu-skia ios-opengl-skia ios-metal-skia android-cpu-skia android-opengl-skia android-vulkan-skia web-skia)
  local combination
  for combination in "${supported[@]}"; do
    printf '==> building %s\n' "$combination"
    MDB_BUILD_ONLY=1 "$root/scripts/test-backend-matrix.sh" "$combination"
  done
}

case "$combination" in
  headless-cpu-skia) headless ;;
  desktop-cpu-skia) desktop CPU ;;
  desktop-opengl-skia) desktop OPENGL ;;
  desktop-metal-skia) desktop METAL ;;
  desktop-vulkan-skia) desktop VULKAN ;;
  ios-cpu-skia) ios CPU ;;
  ios-opengl-skia) ios OPENGL ;;
  ios-metal-skia) ios METAL ;;
  android-cpu-skia) android CPU ;;
  android-opengl-skia) android OPENGL ;;
  android-vulkan-skia) android VULKAN ;;
  web-skia) web ;;
  build-all) build_all ;;
  *) fail "usage: $0 {headless-cpu-skia|desktop-cpu-skia|desktop-opengl-skia|desktop-metal-skia|desktop-vulkan-skia|ios-cpu-skia|ios-opengl-skia|ios-metal-skia|android-cpu-skia|android-opengl-skia|android-vulkan-skia|web-skia|build-all}" ;;
esac
