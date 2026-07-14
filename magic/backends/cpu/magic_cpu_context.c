/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "../../src/magic_context_private.h"
#include <stdlib.h>

typedef struct MagicCpuBackend { BoardSurfaceCpuInterface surface; } MagicCpuBackend;
static MagicResult magic_cpu_board_result(BoardResult result) { return result == BOARD_OK ? MAGIC_OK : result == BOARD_ERROR_VERSION ? MAGIC_ERROR_VERSION : result == BOARD_ERROR_UNAVAILABLE ? MAGIC_ERROR_UNAVAILABLE : MAGIC_ERROR_SURFACE; }
MagicResult magic_cpu_backend_create(MagicContext *context, BoardNativeSurface *surface) {
    MagicCpuBackend *backend; BoardResult result;
    if (!context || !surface) return MAGIC_ERROR_INVALID_ARGUMENT;
    backend = calloc(1, sizeof(*backend)); if (!backend) return MAGIC_ERROR_OUT_OF_MEMORY;
    result = board_surface_query_interface(surface, BOARD_SURFACE_INTERFACE_CPU, BOARD_ABI_VERSION, &backend->surface, sizeof(backend->surface));
    if (result != BOARD_OK) { free(backend); return magic_cpu_board_result(result); }
    context->backend_data = backend; return MAGIC_OK;
}
void magic_cpu_backend_destroy(MagicContext *context) { if (context) { free(context->backend_data); context->backend_data = NULL; } }
MagicResult magic_cpu_backend_resize(MagicContext *context, uint32_t width, uint32_t height, float scale) { (void)width; (void)height; (void)scale; return context && context->backend_data ? MAGIC_OK : MAGIC_ERROR_INVALID_ARGUMENT; }
MagicResult magic_cpu_backend_begin_frame(MagicContext *context, MagicFrame *frame) {
    MagicCpuBackend *backend; void *pixels; uint32_t width, height, stride; BoardPixelFormat format; float scale; BoardResult result;
    if (!context || !frame || !context->backend_data) return MAGIC_ERROR_INVALID_ARGUMENT;
    backend = context->backend_data; result = backend->surface.map_pixels(backend->surface.user_data, &pixels, &width, &height, &stride, &format, &scale); if (result != BOARD_OK) return magic_cpu_board_result(result);
    frame->cpu = (MagicCpuInterop){ sizeof(MagicCpuInterop), MAGIC_ABI_VERSION, pixels, width, height, stride, format == BOARD_PIXEL_FORMAT_BGRA8888 ? MAGIC_PIXEL_FORMAT_BGRA8888 : MAGIC_PIXEL_FORMAT_RGBA8888, scale };
    return MAGIC_OK;
}
MagicResult magic_cpu_backend_end_frame(MagicContext *context, MagicFrame *frame) { MagicCpuBackend *backend; (void)frame; if (!context || !context->backend_data) return MAGIC_ERROR_INVALID_ARGUMENT; backend = context->backend_data; return magic_cpu_board_result(backend->surface.present(backend->surface.user_data)); }
