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

struct MagicContext { MagicBackend backend; const MagicBackendOps *ops; void *backend_data; MagicFrame *active; uint64_t sequence; };
struct MagicFrame { MagicContext *context; MagicCpuInterop cpu; uint64_t sequence; unsigned valid : 1; };

MagicResult magic_cpu_backend_create(MagicContext *context, BoardNativeSurface *surface);
void magic_cpu_backend_destroy(MagicContext *context);
MagicResult magic_cpu_backend_resize(MagicContext *context, uint32_t width, uint32_t height, float scale);
MagicResult magic_cpu_backend_begin_frame(MagicContext *context, MagicFrame *frame);
MagicResult magic_cpu_backend_end_frame(MagicContext *context, MagicFrame *frame);
#endif
