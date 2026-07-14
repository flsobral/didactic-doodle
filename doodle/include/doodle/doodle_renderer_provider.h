/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef DOODLE_RENDERER_PROVIDER_H
#define DOODLE_RENDERER_PROVIDER_H
#include <stdint.h>
#include <magic/magic_context.h>
#include "doodle_canvas.h"
#include "doodle_version.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct DoodleRendererConfig { uint32_t struct_size; uint32_t abi_version; uint32_t flags; } DoodleRendererConfig;
typedef struct DoodleRendererProvider { uint32_t struct_size; uint32_t abi_version; const char *name; DoodleResult (*create)(MagicContext *magic, const DoodleRendererConfig *config, void **out_state); void (*destroy)(void *state); DoodleResult (*begin_frame)(void *state, MagicFrame *frame, DoodleCanvas **out_canvas); DoodleResult (*end_frame)(void *state, DoodleCanvas *canvas); void (*canvas_save)(DoodleCanvas *canvas); void (*canvas_restore)(DoodleCanvas *canvas); void (*canvas_clear)(DoodleCanvas *canvas, DoodleColor color); void (*canvas_draw_rect)(DoodleCanvas *canvas, DoodleRect rect, DoodlePaint paint); } DoodleRendererProvider;
const DoodleRendererProvider *doodle_skia_provider(void);
const DoodleRendererProvider *doodle_blend2d_provider(void);
const DoodleRendererProvider *doodle_nanovg_provider(void);
const DoodleRendererProvider *doodle_vello_provider(void);
#ifdef __cplusplus
}
#endif
#endif
