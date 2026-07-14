/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <magic/magic_context.h>
#include <magic/magic_interop.h>
#include <string.h>
#include <stdlib.h>
struct MagicContext { MagicBackend backend; BoardSurfaceCpuInterface cpu; MagicFrame *active; uint64_t sequence; };
struct MagicFrame { MagicContext *context; MagicCpuInterop cpu; uint64_t sequence; unsigned valid : 1; };
static MagicResult board_result(BoardResult result) { return result == BOARD_OK ? MAGIC_OK : result == BOARD_ERROR_VERSION ? MAGIC_ERROR_VERSION : result == BOARD_ERROR_UNAVAILABLE ? MAGIC_ERROR_UNAVAILABLE : MAGIC_ERROR_SURFACE; }
MagicResult magic_context_create(BoardNativeSurface *surface, const MagicConfig *config, MagicContext **out_context) {
    MagicContext *context; BoardResult result;
    if (!surface || !config || !out_context || config->struct_size < sizeof(*config) || config->abi_version != MAGIC_ABI_VERSION || (config->backend != MAGIC_BACKEND_CPU && config->backend != MAGIC_BACKEND_AUTO)) return MAGIC_ERROR_INVALID_ARGUMENT;
    context = calloc(1, sizeof(*context)); if (!context) return MAGIC_ERROR_OUT_OF_MEMORY;
    result = board_surface_query_interface(surface, BOARD_SURFACE_INTERFACE_CPU, BOARD_ABI_VERSION, &context->cpu, sizeof(context->cpu));
    if (result != BOARD_OK) { free(context); return board_result(result); } context->backend = MAGIC_BACKEND_CPU; *out_context = context; return MAGIC_OK;
}
void magic_context_destroy(MagicContext *context) { if (context) { free(context->active); free(context); } }
MagicResult magic_context_resize(MagicContext *context, uint32_t width, uint32_t height, float scale) { (void)width; (void)height; (void)scale; return context ? MAGIC_OK : MAGIC_ERROR_INVALID_ARGUMENT; }
MagicResult magic_context_begin_frame(MagicContext *context, MagicFrame **out_frame) {
    MagicFrame *frame; void *pixels; uint32_t width, height, stride; BoardPixelFormat format; float scale; BoardResult result;
    if (!context || !out_frame || context->active) return MAGIC_ERROR_INVALID_ARGUMENT;
    result = context->cpu.map_pixels(context->cpu.user_data, &pixels, &width, &height, &stride, &format, &scale); if (result != BOARD_OK) return board_result(result);
    frame = calloc(1, sizeof(*frame)); if (!frame) return MAGIC_ERROR_OUT_OF_MEMORY;
    frame->context = context; frame->sequence = ++context->sequence; frame->valid = 1; frame->cpu = (MagicCpuInterop){ sizeof(MagicCpuInterop), MAGIC_ABI_VERSION, pixels, width, height, stride, format == BOARD_PIXEL_FORMAT_BGRA8888 ? MAGIC_PIXEL_FORMAT_BGRA8888 : MAGIC_PIXEL_FORMAT_RGBA8888, scale };
    context->active = frame; *out_frame = frame; return MAGIC_OK;
}
MagicResult magic_context_end_frame(MagicContext *context, MagicFrame *frame) { BoardResult result; if (!context || !frame || context->active != frame || !frame->valid) return MAGIC_ERROR_INVALID_ARGUMENT; result = context->cpu.present(context->cpu.user_data); frame->valid = 0; free(frame); context->active = 0; return board_result(result); }
MagicBackend magic_context_backend(const MagicContext *context) { return context ? context->backend : MAGIC_BACKEND_AUTO; }
int magic_context_is_device_lost(const MagicContext *context) { (void)context; return 0; }
uint32_t magic_frame_width(const MagicFrame *frame) { return frame ? frame->cpu.width : 0; }
uint32_t magic_frame_height(const MagicFrame *frame) { return frame ? frame->cpu.height : 0; }
uint32_t magic_frame_stride(const MagicFrame *frame) { return frame ? frame->cpu.stride : 0; }
float magic_frame_scale(const MagicFrame *frame) { return frame ? frame->cpu.scale : 0; }
uint64_t magic_frame_sequence(const MagicFrame *frame) { return frame ? frame->sequence : 0; }
int magic_frame_is_valid(const MagicFrame *frame) { return frame && frame->valid; }
MagicResult magic_frame_query_interop(MagicFrame *frame, MagicInteropId id, uint32_t version, void *out, size_t size) { if (!frame || !out || !frame->valid) return MAGIC_ERROR_INVALID_ARGUMENT; if (version != MAGIC_ABI_VERSION) return MAGIC_ERROR_VERSION; if (id != MAGIC_INTEROP_CPU) return MAGIC_ERROR_UNAVAILABLE; if (size < sizeof(MagicCpuInterop)) return MAGIC_ERROR_INVALID_ARGUMENT; memcpy(out, &frame->cpu, sizeof(frame->cpu)); return MAGIC_OK; }
