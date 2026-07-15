/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef MAGIC_CONTEXT_H
#define MAGIC_CONTEXT_H
#include <board/board_surface.h>
#include "magic_frame.h"
#include "magic_version.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct MagicConfig { uint32_t struct_size; uint32_t abi_version; MagicBackend backend; uint32_t sample_count; } MagicConfig;
MagicResult magic_context_create(BoardNativeSurface *surface, const MagicConfig *config, MagicContext **out_context);
void magic_context_destroy(MagicContext *context);
MagicResult magic_context_resize(MagicContext *context, uint32_t width, uint32_t height, float scale);
MagicResult magic_context_begin_frame(MagicContext *context, MagicFrame **out_frame);
MagicResult magic_context_end_frame(MagicContext *context, MagicFrame *frame);
MagicBackend magic_context_backend(const MagicContext *context);
const char *magic_context_backend_name(const MagicContext *context);
const char *magic_context_backend_version(const MagicContext *context);
int magic_context_is_device_lost(const MagicContext *context);
#ifdef __cplusplus
}
#endif
#endif
