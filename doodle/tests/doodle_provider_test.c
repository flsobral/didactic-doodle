/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <doodle/doodle_renderer.h>
typedef struct State { unsigned begins, ends; } State;
static DoodleResult create(MagicContext *magic, const DoodleRendererConfig *config, void **out) { static State state; (void)magic; (void)config; *out = &state; return DOODLE_OK; }
static DoodleResult begin(void *state, MagicFrame *frame, DoodleCanvas **canvas) { (void)frame; ++((State *)state)->begins; *canvas = (DoodleCanvas *)state; return DOODLE_OK; }
static DoodleResult end(void *state, DoodleCanvas *canvas) { (void)canvas; ++((State *)state)->ends; return DOODLE_OK; }
int main(void) {
 MagicContext *magic = (MagicContext *)1; MagicFrame *frame = (MagicFrame *)1; DoodleRenderer *renderer = 0; DoodleCanvas *canvas = 0;
 DoodleRendererConfig dc = {sizeof(dc), DOODLE_ABI_VERSION, 0};
 DoodleRendererProvider provider = {sizeof(provider), DOODLE_ABI_VERSION, "fake", create, 0, begin, end, 0, 0, 0, 0};
 if (doodle_renderer_create(&provider, magic, &dc, &renderer) || doodle_renderer_begin_frame(renderer, frame, &canvas) || doodle_renderer_end_frame(renderer, canvas)) return 1;
 doodle_renderer_destroy(renderer); return 0;
}
