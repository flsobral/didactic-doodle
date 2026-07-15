/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef MAGIC_INTEROP_H
#define MAGIC_INTEROP_H
#include <stddef.h>
#include <stdint.h>
#include "magic_types.h"
#include "magic_version.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum MagicInteropId { MAGIC_INTEROP_CPU = 1, MAGIC_INTEROP_OPENGL = 2, MAGIC_INTEROP_METAL = 3, MAGIC_INTEROP_VULKAN = 4, MAGIC_INTEROP_WEB = 5 } MagicInteropId;
typedef struct MagicCpuInterop { uint32_t struct_size; uint32_t abi_version; void *pixels; uint32_t width, height, stride; MagicPixelFormat format; float scale; } MagicCpuInterop;
typedef struct MagicOpenGLInterop { uint32_t struct_size; uint32_t abi_version; void *context; uint32_t framebuffer; uint32_t width, height; float scale; } MagicOpenGLInterop;
typedef struct MagicMetalInterop { uint32_t struct_size; uint32_t abi_version; void *layer; void *device; void *command_queue; const void **drawable_slot; uint32_t width, height; float scale; } MagicMetalInterop;
typedef struct MagicVulkanInterop { uint32_t struct_size; uint32_t abi_version; uint64_t instance; uint64_t physical_device; uint64_t device; uint64_t queue; uint64_t image; uint64_t acquire_semaphore; uint64_t render_complete_semaphore; void *device_features; uint32_t queue_family; uint32_t image_format; uint32_t image_usage; uint32_t width, height; float scale; } MagicVulkanInterop;
typedef struct MagicWebInterop { uint32_t struct_size; uint32_t abi_version; void *context; } MagicWebInterop;
MagicResult magic_frame_query_interop(MagicFrame *frame, MagicInteropId id, uint32_t abi_version, void *out_interop, size_t out_interop_size);
#ifdef __cplusplus
}
#endif
#endif
