/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef DOODLE_CANVAS_H
#define DOODLE_CANVAS_H
#include "doodle_types.h"
#ifdef __cplusplus
extern "C" {
#endif
void doodle_canvas_save(DoodleCanvas *canvas);
void doodle_canvas_restore(DoodleCanvas *canvas);
void doodle_canvas_translate(DoodleCanvas *canvas, float x, float y);
void doodle_canvas_scale(DoodleCanvas *canvas, float x, float y);
void doodle_canvas_rotate(DoodleCanvas *canvas, float degrees);
void doodle_canvas_clip_rect(DoodleCanvas *canvas, DoodleRect rect);
void doodle_canvas_clear(DoodleCanvas *canvas, DoodleColor color);
void doodle_canvas_draw_rect(DoodleCanvas *canvas, DoodleRect rect, DoodlePaint paint);
void doodle_canvas_draw_round_rect(DoodleCanvas *canvas, DoodleRect rect, float radius, DoodlePaint paint);
void doodle_canvas_draw_line(DoodleCanvas *canvas, DoodlePoint a, DoodlePoint b, DoodlePaint paint);
void doodle_canvas_draw_circle(DoodleCanvas *canvas, DoodlePoint center, float radius, DoodlePaint paint);
void doodle_canvas_draw_text(DoodleCanvas *canvas, const char *text, float x, float y, DoodleTextStyle style);
#ifdef __cplusplus
}
#endif
#endif
