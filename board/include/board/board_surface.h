/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef BOARD_SURFACE_H
#define BOARD_SURFACE_H
#include <stddef.h>
#include <stdint.h>
#include "board_types.h"
#include "board_version.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct BoardNativeSurface BoardNativeSurface;
typedef enum BoardSurfaceInterfaceId { BOARD_SURFACE_INTERFACE_CPU = 1, BOARD_SURFACE_INTERFACE_OPENGL = 2, BOARD_SURFACE_INTERFACE_METAL = 3, BOARD_SURFACE_INTERFACE_VULKAN = 4, BOARD_SURFACE_INTERFACE_WEB = 5 } BoardSurfaceInterfaceId;
typedef enum BoardPixelFormat { BOARD_PIXEL_FORMAT_RGBA8888, BOARD_PIXEL_FORMAT_BGRA8888 } BoardPixelFormat;
typedef struct BoardSurfaceCpuInterface { uint32_t struct_size; uint32_t abi_version; void *user_data; BoardResult (*map_pixels)(void *user_data, void **pixels, uint32_t *width, uint32_t *height, uint32_t *stride, BoardPixelFormat *format, float *scale); BoardResult (*present)(void *user_data); } BoardSurfaceCpuInterface;
typedef struct BoardSurfaceOpenGLInterface { uint32_t struct_size; uint32_t abi_version; void *user_data; BoardResult (*create_context)(void *user_data, void **out_context); void (*destroy_context)(void *user_data, void *context); BoardResult (*make_current)(void *user_data, void *context); void *(*get_proc_address)(void *user_data, const char *name); BoardResult (*drawable_size)(void *user_data, uint32_t *width, uint32_t *height, float *scale); BoardResult (*swap_buffers)(void *user_data); } BoardSurfaceOpenGLInterface;
typedef struct BoardSurfaceMetalInterface { uint32_t struct_size; uint32_t abi_version; void *layer; uint32_t width, height; float scale; } BoardSurfaceMetalInterface;
typedef struct BoardSurfaceVulkanInterface { uint32_t struct_size; uint32_t abi_version; void *user_data; const char *const *(*required_instance_extensions)(void *user_data, uint32_t *count); BoardResult (*create_surface)(void *user_data, void *instance, uint64_t *out_surface); void (*destroy_surface)(void *user_data, void *instance, uint64_t surface); } BoardSurfaceVulkanInterface;
typedef struct BoardSurfaceWebInterface { uint32_t struct_size; uint32_t abi_version; const char *canvas_selector; void *user_data; } BoardSurfaceWebInterface;
BoardResult board_surface_query_interface(BoardNativeSurface *surface, BoardSurfaceInterfaceId id, uint32_t abi_version, void *out_interface, size_t out_interface_size);
#ifdef __cplusplus
}
#endif
#endif
