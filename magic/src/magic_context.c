/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "magic_context_private.h"
#include <stdlib.h>
#include <string.h>

static const MagicBackendOps magic_cpu_ops = { magic_cpu_backend_create, magic_cpu_backend_destroy, magic_cpu_backend_resize, magic_cpu_backend_begin_frame, magic_cpu_backend_end_frame };
MagicResult magic_context_create(BoardNativeSurface *surface, const MagicConfig *config, MagicContext **out_context) {
    MagicContext *context;
    if (!surface || !config || !out_context || config->struct_size < sizeof(*config) || config->abi_version != MAGIC_ABI_VERSION || (config->backend != MAGIC_BACKEND_CPU && config->backend != MAGIC_BACKEND_AUTO)) return MAGIC_ERROR_INVALID_ARGUMENT;
    context = calloc(1, sizeof(*context)); if (!context) return MAGIC_ERROR_OUT_OF_MEMORY;
    context->backend = MAGIC_BACKEND_CPU; context->ops = &magic_cpu_ops;
    if (context->ops->create(context, surface) != MAGIC_OK) { free(context); return MAGIC_ERROR_SURFACE; }
    *out_context = context; return MAGIC_OK;
}
void magic_context_destroy(MagicContext *context) { if (context) { if (context->active) magic_context_end_frame(context, context->active); context->ops->destroy(context); free(context); } }
MagicResult magic_context_resize(MagicContext *context, uint32_t width, uint32_t height, float scale) { return context ? context->ops->resize(context, width, height, scale) : MAGIC_ERROR_INVALID_ARGUMENT; }
MagicResult magic_context_begin_frame(MagicContext *context, MagicFrame **out_frame) {
    MagicFrame *frame; MagicResult result;
    if (!context || !out_frame || context->active) return MAGIC_ERROR_INVALID_ARGUMENT;
    frame = calloc(1, sizeof(*frame)); if (!frame) return MAGIC_ERROR_OUT_OF_MEMORY;
    frame->context = context; frame->sequence = ++context->sequence; frame->valid = 1;
    result = context->ops->begin_frame(context, frame); if (result != MAGIC_OK) { free(frame); return result; }
    context->active = frame; *out_frame = frame; return MAGIC_OK;
}
MagicResult magic_context_end_frame(MagicContext *context, MagicFrame *frame) { MagicResult result; if (!context || !frame || context->active != frame || !frame->valid) return MAGIC_ERROR_INVALID_ARGUMENT; result = context->ops->end_frame(context, frame); frame->valid = 0; free(frame); context->active = 0; return result; }
MagicBackend magic_context_backend(const MagicContext *context) { return context ? context->backend : MAGIC_BACKEND_AUTO; }
int magic_context_is_device_lost(const MagicContext *context) { (void)context; return 0; }
uint32_t magic_frame_width(const MagicFrame *frame) { return frame ? frame->cpu.width : 0; }
uint32_t magic_frame_height(const MagicFrame *frame) { return frame ? frame->cpu.height : 0; }
uint32_t magic_frame_stride(const MagicFrame *frame) { return frame ? frame->cpu.stride : 0; }
float magic_frame_scale(const MagicFrame *frame) { return frame ? frame->cpu.scale : 0; }
uint64_t magic_frame_sequence(const MagicFrame *frame) { return frame ? frame->sequence : 0; }
int magic_frame_is_valid(const MagicFrame *frame) { return frame && frame->valid; }
MagicResult magic_frame_query_interop(MagicFrame *frame, MagicInteropId id, uint32_t version, void *out, size_t size) { if (!frame || !out || !frame->valid) return MAGIC_ERROR_INVALID_ARGUMENT; if (version != MAGIC_ABI_VERSION) return MAGIC_ERROR_VERSION; if (id != MAGIC_INTEROP_CPU) return MAGIC_ERROR_UNAVAILABLE; if (size < sizeof(MagicCpuInterop)) return MAGIC_ERROR_INVALID_ARGUMENT; memcpy(out, &frame->cpu, sizeof(frame->cpu)); return MAGIC_OK; }
