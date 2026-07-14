/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef DOODLE_RENDERER_H
#define DOODLE_RENDERER_H
#include "doodle_renderer_provider.h"
#ifdef __cplusplus
extern "C" {
#endif
DoodleResult doodle_renderer_create(const DoodleRendererProvider *provider, MagicContext *magic, const DoodleRendererConfig *config, DoodleRenderer **out_renderer);
void doodle_renderer_destroy(DoodleRenderer *renderer);
DoodleResult doodle_renderer_begin_frame(DoodleRenderer *renderer, MagicFrame *frame, DoodleCanvas **out_canvas);
DoodleResult doodle_renderer_end_frame(DoodleRenderer *renderer, DoodleCanvas *canvas);
#ifdef __cplusplus
}
#endif
#endif
