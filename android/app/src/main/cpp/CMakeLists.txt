# SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
# SPDX-License-Identifier: LGPL-2.1-only

cmake_minimum_required(VERSION 3.22)
project(tc_demo_android LANGUAGES C CXX)
set(TC_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../..")
set(TC_SKIA_ROOT "" CACHE PATH "Unpacked TotalCross Android Skia release")
set(TC_LIBPNG_ROOT "" CACHE PATH "Unpacked TotalCross Android libpng release")
set(TC_ZLIB_NG_ROOT "" CACHE PATH "Unpacked TotalCross Android zlib-ng release")
set(TC_ANDROID_GRAPHICS "CPU" CACHE STRING "Android graphics API: CPU, OPENGL, or VULKAN")
set_property(CACHE TC_ANDROID_GRAPHICS PROPERTY STRINGS CPU OPENGL VULKAN)
if(NOT TC_ANDROID_GRAPHICS STREQUAL "CPU" AND NOT TC_ANDROID_GRAPHICS STREQUAL "OPENGL" AND NOT TC_ANDROID_GRAPHICS STREQUAL "VULKAN")
  message(FATAL_ERROR "TC_ANDROID_GRAPHICS must be CPU, OPENGL, or VULKAN")
endif()
if(NOT EXISTS "${TC_SKIA_ROOT}/libskia-android-arm64-v8a.a")
  message(FATAL_ERROR "Set TC_SKIA_ROOT to the directory created by scripts/fetch-totalcross-skia.sh ... android-arm64-v8a")
endif()
if(NOT EXISTS "${TC_LIBPNG_ROOT}/libpng/android/arm64-v8a/lib/libpng16.a")
  message(FATAL_ERROR "Set TC_LIBPNG_ROOT to the unpacked TotalCross libpng-android-arm64-v8a release required by the selected Skia archive")
endif()
if(NOT EXISTS "${TC_ZLIB_NG_ROOT}/zlib-ng/android/arm64-v8a/lib/libz.a")
  message(FATAL_ERROR "Set TC_ZLIB_NG_ROOT to the unpacked TotalCross zlib-ng-android-arm64-v8a release required by libpng")
endif()
add_library(skia STATIC IMPORTED)
set_target_properties(skia PROPERTIES IMPORTED_LOCATION "${TC_SKIA_ROOT}/libskia-android-arm64-v8a.a" INTERFACE_INCLUDE_DIRECTORIES "${TC_SKIA_ROOT}/headers/modules/skia")
add_library(png STATIC IMPORTED)
set_target_properties(png PROPERTIES IMPORTED_LOCATION "${TC_LIBPNG_ROOT}/libpng/android/arm64-v8a/lib/libpng16.a" INTERFACE_INCLUDE_DIRECTORIES "${TC_LIBPNG_ROOT}/libpng/android/arm64-v8a/include")
add_library(zlib_ng STATIC IMPORTED)
set_target_properties(zlib_ng PROPERTIES IMPORTED_LOCATION "${TC_ZLIB_NG_ROOT}/zlib-ng/android/arm64-v8a/lib/libz.a" INTERFACE_INCLUDE_DIRECTORIES "${TC_ZLIB_NG_ROOT}/zlib-ng/android/arm64-v8a/include")
set(TC_DEMO_ANDROID_SOURCES
  ${TC_ROOT}/examples/demo_android/demo_android_main.c ${TC_ROOT}/examples/demo/demo_scene.c
  ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
  ${TC_ROOT}/src/backends/android/tc_android_backend.c ${TC_ROOT}/src/graphics/tc_android_cpu_context.c ${TC_ROOT}/src/graphics/tc_android_graphics.c
  ${TC_ROOT}/src/renderers/skia/tc_skia_renderer.cpp)
if(TC_ANDROID_GRAPHICS STREQUAL "OPENGL")
  list(APPEND TC_DEMO_ANDROID_SOURCES ${TC_ROOT}/src/graphics/tc_android_gl_context.c)
elseif(TC_ANDROID_GRAPHICS STREQUAL "VULKAN")
  list(APPEND TC_DEMO_ANDROID_SOURCES ${TC_ROOT}/src/graphics/tc_android_vulkan_context.cpp)
endif()
add_library(tc_demo_android SHARED ${TC_DEMO_ANDROID_SOURCES})
target_include_directories(tc_demo_android PRIVATE ${TC_ROOT}/include ${TC_ROOT}/src ${TC_ROOT}/src/backends/android ${TC_ROOT}/examples/demo "${ANDROID_NDK}/sources/android/native_app_glue")
target_compile_features(tc_demo_android PRIVATE c_std_11 cxx_std_17)
target_compile_definitions(tc_demo_android PRIVATE
  TC_BUILD_GRAPHICS_OPENGL=$<STREQUAL:${TC_ANDROID_GRAPHICS},OPENGL>
  TC_BUILD_GRAPHICS_VULKAN=$<STREQUAL:${TC_ANDROID_GRAPHICS},VULKAN>
  TC_ANDROID_GRAPHICS_OPENGL=$<STREQUAL:${TC_ANDROID_GRAPHICS},OPENGL>
  TC_ANDROID_GRAPHICS_VULKAN=$<STREQUAL:${TC_ANDROID_GRAPHICS},VULKAN>
  SK_GL=1
  SK_VULKAN=1)
target_link_libraries(tc_demo_android PRIVATE skia png zlib_ng android log EGL GLESv2 vulkan)
