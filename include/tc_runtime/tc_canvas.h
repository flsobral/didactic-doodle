/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_RUNTIME_CANVAS_H
#define TC_RUNTIME_CANVAS_H
#include "tc_types.h"
#ifdef __cplusplus
extern "C" {
#endif
void tc_canvas_save(TcCanvas2D* canvas); void tc_canvas_restore(TcCanvas2D* canvas);
void tc_canvas_translate(TcCanvas2D* canvas, float x, float y); void tc_canvas_scale(TcCanvas2D* canvas, float x, float y); void tc_canvas_rotate(TcCanvas2D* canvas, float degrees);
void tc_canvas_clip_rect(TcCanvas2D* canvas, TcRect rect); void tc_canvas_clear(TcCanvas2D* canvas, TcColor color);
void tc_canvas_draw_rect(TcCanvas2D* canvas, TcRect rect, TcPaint paint); void tc_canvas_draw_round_rect(TcCanvas2D* canvas, TcRect rect, float radius, TcPaint paint); void tc_canvas_draw_line(TcCanvas2D* canvas, TcPoint a, TcPoint b, TcPaint paint); void tc_canvas_draw_circle(TcCanvas2D* canvas, TcPoint center, float radius, TcPaint paint); void tc_canvas_draw_text(TcCanvas2D* canvas, const char* text, float x, float y, TcTextStyle style);
#ifdef __cplusplus
}
#endif
#endif
