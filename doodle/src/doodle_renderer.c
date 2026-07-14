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
