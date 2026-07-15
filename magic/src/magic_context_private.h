/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef MAGIC_CONTEXT_PRIVATE_H
#define MAGIC_CONTEXT_PRIVATE_H
#include <magic/magic_context.h>
#include <magic/magic_interop.h>

typedef struct MagicBackendOps {
    MagicResult (*create)(MagicContext *context, BoardNativeSurface *surface);
    void (*destroy)(MagicContext *context);
    MagicResult (*resize)(MagicContext *context, uint32_t width, uint32_t height, float scale);
    MagicResult (*begin_frame)(MagicContext *context, MagicFrame *frame);
    MagicResult (*end_frame)(MagicContext *context, MagicFrame *frame);
} MagicBackendOps;

struct MagicContext { MagicBackend backend; const MagicBackendOps *ops; const char *backend_name; char backend_version[128]; void *backend_data; MagicFrame *active; uint64_t sequence; };
struct MagicFrame { MagicContext *context; MagicCpuInterop cpu; MagicOpenGLInterop opengl; MagicMetalInterop metal; MagicVulkanInterop vulkan; MagicWebInterop web; uint64_t sequence; unsigned valid : 1; };

MagicResult magic_cpu_backend_create(MagicContext *context, BoardNativeSurface *surface);
void magic_cpu_backend_destroy(MagicContext *context);
MagicResult magic_cpu_backend_resize(MagicContext *context, uint32_t width, uint32_t height, float scale);
MagicResult magic_cpu_backend_begin_frame(MagicContext *context, MagicFrame *frame);
MagicResult magic_cpu_backend_end_frame(MagicContext *context, MagicFrame *frame);
#if MAGIC_BUILD_OPENGL
MagicResult magic_opengl_backend_create(MagicContext *context, BoardNativeSurface *surface);
void magic_opengl_backend_destroy(MagicContext *context);
MagicResult magic_opengl_backend_resize(MagicContext *context, uint32_t width, uint32_t height, float scale);
MagicResult magic_opengl_backend_begin_frame(MagicContext *context, MagicFrame *frame);
MagicResult magic_opengl_backend_end_frame(MagicContext *context, MagicFrame *frame);
const char *magic_opengl_backend_version(const MagicContext *context);
#endif
#if MAGIC_BUILD_WEB
MagicResult magic_web_backend_create(MagicContext *context, BoardNativeSurface *surface);
void magic_web_backend_destroy(MagicContext *context);
MagicResult magic_web_backend_resize(MagicContext *context, uint32_t width, uint32_t height, float scale);
MagicResult magic_web_backend_begin_frame(MagicContext *context, MagicFrame *frame);
MagicResult magic_web_backend_end_frame(MagicContext *context, MagicFrame *frame);
const char *magic_web_backend_version(const MagicContext *context);
#endif
#if MAGIC_BUILD_VULKAN
#ifdef __cplusplus
extern "C" {
#endif
MagicResult magic_vulkan_backend_create(MagicContext *context, BoardNativeSurface *surface);
void magic_vulkan_backend_destroy(MagicContext *context);
MagicResult magic_vulkan_backend_resize(MagicContext *context, uint32_t width, uint32_t height, float scale);
MagicResult magic_vulkan_backend_begin_frame(MagicContext *context, MagicFrame *frame);
MagicResult magic_vulkan_backend_end_frame(MagicContext *context, MagicFrame *frame);
const char *magic_vulkan_backend_version(const MagicContext *context);
#ifdef __cplusplus
}
#endif
#endif
#if MAGIC_BUILD_METAL
#ifdef __cplusplus
extern "C" {
#endif
MagicResult magic_metal_backend_create(MagicContext *context, BoardNativeSurface *surface);
void magic_metal_backend_destroy(MagicContext *context);
MagicResult magic_metal_backend_resize(MagicContext *context, uint32_t width, uint32_t height, float scale);
MagicResult magic_metal_backend_begin_frame(MagicContext *context, MagicFrame *frame);
MagicResult magic_metal_backend_end_frame(MagicContext *context, MagicFrame *frame);
const char *magic_metal_backend_version(const MagicContext *context);
#ifdef __cplusplus
}
#endif
#endif
#endif
