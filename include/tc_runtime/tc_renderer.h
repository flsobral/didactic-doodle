/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_RUNTIME_RENDERER_H
#define TC_RUNTIME_RENDERER_H
#include "tc_graphics.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum TcRendererKind { TC_RENDERER_SKIA = 0, TC_RENDERER_NANOVG, TC_RENDERER_BLEND2D, TC_RENDERER_VELLO } TcRendererKind;
int tc_renderer_create(TcRendererKind kind, TcGraphicsContext* context, TcRenderer2D** out_renderer);
int tc_renderer_attach(TcRenderer2D* renderer, TcGraphicsContext* context);
int tc_renderer_resize(TcRenderer2D* renderer, int width, int height, float scale);
TcCanvas2D* tc_renderer_begin_frame(TcRenderer2D* renderer);
int tc_renderer_end_frame(TcRenderer2D* renderer);
void tc_renderer_destroy(TcRenderer2D* renderer);
#ifdef __cplusplus
}
#endif
#endif
