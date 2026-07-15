# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only

cmake_minimum_required(VERSION 3.22)
project(MagicDoodleBoardIosDemo LANGUAGES C CXX OBJC OBJCXX)

# This is a convenience source directory for iOS-only consumers.  The actual
# application composition remains in the root project so it always uses the
# public Board -> Magic -> Doodle boundary, like every other platform entry.
set(MDB_IOS_MAGIC_BACKEND "CPU" CACHE STRING "iOS Magic backend: CPU, OPENGL, or METAL")
set_property(CACHE MDB_IOS_MAGIC_BACKEND PROPERTY STRINGS CPU OPENGL METAL)
set(MDB_IOS_SKIA_ROOT "" CACHE PATH "Root produced by scripts/fetch-totalcross-skia.sh for iOS")
set(MDB_IOS_PNG_ROOT "" CACHE PATH "iOS libpng prefix containing include/png.h and lib/libpng16.a")
set(MDB_IOS_ZLIB_ROOT "" CACHE PATH "iOS zlib prefix containing include/zlib.h and lib/libz.a")

if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
  message(FATAL_ERROR "The ios source directory requires an iOS CMake toolchain (CMAKE_SYSTEM_NAME=iOS).")
endif()
if(NOT MDB_IOS_MAGIC_BACKEND STREQUAL "CPU" AND NOT MDB_IOS_MAGIC_BACKEND STREQUAL "OPENGL" AND NOT MDB_IOS_MAGIC_BACKEND STREQUAL "METAL")
  message(FATAL_ERROR "MDB_IOS_MAGIC_BACKEND must be CPU, OPENGL, or METAL.")
endif()

set(BOARD_BACKEND IOS_NATIVE CACHE STRING "" FORCE)
set(MAGIC_BACKEND "${MDB_IOS_MAGIC_BACKEND}" CACHE STRING "" FORCE)
set(DOODLE_RENDERER SKIA CACHE STRING "" FORCE)
set(DOODLE_SKIA_ROOT "${MDB_IOS_SKIA_ROOT}" CACHE PATH "" FORCE)
set(DOODLE_IOS_PNG_ROOT "${MDB_IOS_PNG_ROOT}" CACHE PATH "" FORCE)
set(DOODLE_IOS_ZLIB_ROOT "${MDB_IOS_ZLIB_ROOT}" CACHE PATH "" FORCE)
set(MDB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MDB_BUILD_EXAMPLES ON CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.." magic_doodle_board)
