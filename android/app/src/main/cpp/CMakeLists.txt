# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only
cmake_minimum_required(VERSION 3.22)
project(magic_doodle_board_android_demo LANGUAGES C CXX)

set(MDB_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../..")
set(DOODLE_SKIA_ROOT "" CACHE PATH "Root created by scripts/fetch-totalcross-skia.sh for android-arm64-v8a")
set(DOODLE_ANDROID_PNG_ROOT "" CACHE PATH "Android libpng arm64-v8a prefix")
set(DOODLE_ANDROID_ZLIB_ROOT "" CACHE PATH "Android zlib-ng arm64-v8a prefix")

set(BOARD_BACKEND ANDROID_NATIVE CACHE STRING "" FORCE)
set(MAGIC_BACKEND CPU CACHE STRING "" FORCE)
set(DOODLE_RENDERER SKIA CACHE STRING "" FORCE)
set(BOARD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MAGIC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DOODLE_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory("${MDB_ROOT}/board" board)
add_subdirectory("${MDB_ROOT}/magic" magic)
add_subdirectory("${MDB_ROOT}/doodle" doodle)

add_library(magic_doodle_board_android_demo SHARED
  "${MDB_ROOT}/examples/android/magic_doodle_board_android_demo.c"
  "${MDB_ROOT}/examples/common/magic_doodle_board_scene.c")
target_include_directories(magic_doodle_board_android_demo PRIVATE
  "${MDB_ROOT}/examples/common"
  "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")
target_link_libraries(magic_doodle_board_android_demo PRIVATE Board::Android Magic::Magic Doodle::Skia)
target_link_options(magic_doodle_board_android_demo PRIVATE "-Wl,-u,ANativeActivity_onCreate")
