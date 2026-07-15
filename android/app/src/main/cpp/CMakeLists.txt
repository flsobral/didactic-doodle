# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only
cmake_minimum_required(VERSION 3.22)
project(magic_doodle_board_android_demo LANGUAGES C CXX)

set(MDB_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../..")
set(DOODLE_SKIA_ROOT "" CACHE PATH "Root created by scripts/fetch-totalcross-skia.sh for android-arm64-v8a")
set(DOODLE_ANDROID_PNG_ROOT "" CACHE PATH "Android libpng arm64-v8a prefix")
set(DOODLE_ANDROID_ZLIB_ROOT "" CACHE PATH "Android zlib-ng arm64-v8a prefix")
set(MDB_ANDROID_MAGIC_BACKEND "CPU" CACHE STRING "Android Magic backend: CPU, OPENGL, or VULKAN")
set_property(CACHE MDB_ANDROID_MAGIC_BACKEND PROPERTY STRINGS CPU OPENGL VULKAN)
if(NOT MDB_ANDROID_MAGIC_BACKEND STREQUAL "CPU" AND NOT MDB_ANDROID_MAGIC_BACKEND STREQUAL "OPENGL" AND NOT MDB_ANDROID_MAGIC_BACKEND STREQUAL "VULKAN")
  message(FATAL_ERROR "MDB_ANDROID_MAGIC_BACKEND must be CPU, OPENGL, or VULKAN.")
endif()

set(BOARD_BACKEND ANDROID_NATIVE CACHE STRING "" FORCE)
set(MAGIC_BACKEND "${MDB_ANDROID_MAGIC_BACKEND}" CACHE STRING "" FORCE)
set(DOODLE_RENDERER SKIA CACHE STRING "" FORCE)
if(MDB_ANDROID_MAGIC_BACKEND STREQUAL "OPENGL")
  set(BOARD_ANDROID_ENABLE_OPENGL ON CACHE BOOL "" FORCE)
else()
  set(BOARD_ANDROID_ENABLE_OPENGL OFF CACHE BOOL "" FORCE)
endif()
if(MDB_ANDROID_MAGIC_BACKEND STREQUAL "VULKAN")
  set(BOARD_ANDROID_ENABLE_VULKAN ON CACHE BOOL "" FORCE)
else()
  set(BOARD_ANDROID_ENABLE_VULKAN OFF CACHE BOOL "" FORCE)
endif()
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
target_compile_definitions(magic_doodle_board_android_demo PRIVATE MDB_ANDROID_OPENGL=$<STREQUAL:${MDB_ANDROID_MAGIC_BACKEND},OPENGL> MDB_ANDROID_VULKAN=$<STREQUAL:${MDB_ANDROID_MAGIC_BACKEND},VULKAN>)
