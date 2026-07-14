/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <doodle/doodle_renderer.h>
#include <stdlib.h>
struct DoodleRenderer { const DoodleRendererProvider *provider; void *state; DoodleCanvas *active; };
DoodleResult doodle_renderer_create(const DoodleRendererProvider *provider, MagicContext *magic, const DoodleRendererConfig *config, DoodleRenderer **out_renderer) {
 DoodleRenderer *renderer; if (!provider || !magic || !config || !out_renderer || provider->struct_size < sizeof(*provider) || provider->abi_version != DOODLE_ABI_VERSION || config->struct_size < sizeof(*config) || config->abi_version != DOODLE_ABI_VERSION || !provider->create || !provider->begin_frame || !provider->end_frame) return DOODLE_ERROR_INVALID_ARGUMENT;
 renderer = calloc(1, sizeof(*renderer)); if (!renderer) return DOODLE_ERROR_OUT_OF_MEMORY; renderer->provider = provider; if (provider->create(magic, config, &renderer->state) != DOODLE_OK) { free(renderer); return DOODLE_ERROR_RENDERER; } *out_renderer = renderer; return DOODLE_OK;
}
void doodle_renderer_destroy(DoodleRenderer *renderer) { if (renderer) { if (renderer->provider->destroy) renderer->provider->destroy(renderer->state); free(renderer); } }
DoodleResult doodle_renderer_begin_frame(DoodleRenderer *renderer, MagicFrame *frame, DoodleCanvas **out_canvas) { DoodleResult result; if (!renderer || !frame || !out_canvas || renderer->active) return DOODLE_ERROR_INVALID_ARGUMENT; result = renderer->provider->begin_frame(renderer->state, frame, out_canvas); if (result == DOODLE_OK) renderer->active = *out_canvas; return result; }
DoodleResult doodle_renderer_end_frame(DoodleRenderer *renderer, DoodleCanvas *canvas) { DoodleResult result; if (!renderer || !canvas || renderer->active != canvas) return DOODLE_ERROR_INVALID_ARGUMENT; result = renderer->provider->end_frame(renderer->state, canvas); renderer->active = 0; return result; }
void doodle_canvas_save(DoodleCanvas *canvas) { (void)canvas; }
void doodle_canvas_restore(DoodleCanvas *canvas) { (void)canvas; }
void doodle_canvas_translate(DoodleCanvas *canvas, float x, float y) { (void)canvas; (void)x; (void)y; }
void doodle_canvas_scale(DoodleCanvas *canvas, float x, float y) { (void)canvas; (void)x; (void)y; }
void doodle_canvas_rotate(DoodleCanvas *canvas, float degrees) { (void)canvas; (void)degrees; }
void doodle_canvas_clip_rect(DoodleCanvas *canvas, DoodleRect rect) { (void)canvas; (void)rect; }
void doodle_canvas_clear(DoodleCanvas *canvas, DoodleColor color) { (void)canvas; (void)color; }
void doodle_canvas_draw_rect(DoodleCanvas *canvas, DoodleRect rect, DoodlePaint paint) { (void)canvas; (void)rect; (void)paint; }
void doodle_canvas_draw_round_rect(DoodleCanvas *canvas, DoodleRect rect, float radius, DoodlePaint paint) { (void)canvas; (void)rect; (void)radius; (void)paint; }
void doodle_canvas_draw_line(DoodleCanvas *canvas, DoodlePoint a, DoodlePoint b, DoodlePaint paint) { (void)canvas; (void)a; (void)b; (void)paint; }
void doodle_canvas_draw_circle(DoodleCanvas *canvas, DoodlePoint center, float radius, DoodlePaint paint) { (void)canvas; (void)center; (void)radius; (void)paint; }
void doodle_canvas_draw_text(DoodleCanvas *canvas, const char *text, float x, float y, DoodleTextStyle style) { (void)canvas; (void)text; (void)x; (void)y; (void)style; }
