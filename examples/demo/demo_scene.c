/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "demo_scene.h"
#include <math.h>
#include <stddef.h>

static TcColor color(float r, float g, float b, float a) { return (TcColor){r, g, b, a}; }
static TcPaint fill(TcColor c) { return (TcPaint){c, 1.0f, TC_PAINT_FILL}; }
static TcPaint stroke(TcColor c, float width) { return (TcPaint){c, width, TC_PAINT_STROKE}; }
void demo_scene_init(DemoScene* scene, int width, int height) { *scene = (DemoScene){.width = width, .height = height}; }
void demo_scene_on_event(void* data, const TcEvent* event) {
    DemoScene* scene = data;
    if (event->type == TC_EVENT_RESIZE) { scene->width = event->data.resize.width; scene->height = event->data.resize.height; }
    if (event->type == TC_EVENT_POINTER_DOWN || event->type == TC_EVENT_POINTER_MOVE || event->type == TC_EVENT_POINTER_UP) {
        scene->pointer_x = event->data.pointer.x; scene->pointer_y = event->data.pointer.y; scene->pointer_down = event->type != TC_EVENT_POINTER_UP;
    }
}
void demo_scene_on_update(void* data, double delta_seconds) { ((DemoScene*)data)->elapsed += delta_seconds; }
void demo_scene_on_draw(void* data, TcCanvas2D* canvas) {
    DemoScene* s = data; float w = (float)s->width, h = (float)s->height;
    tc_canvas_clear(canvas, color(0.055f, 0.075f, 0.12f, 1));
    tc_canvas_draw_rect(canvas, (TcRect){32, 32, w - 64, h - 64}, fill(color(.10f, .15f, .25f, 1)));
    tc_canvas_draw_round_rect(canvas, (TcRect){56, 82, 190, 94}, 16, fill(color(.16f, .55f, .86f, 1)));
    tc_canvas_draw_line(canvas, (TcPoint){56, 210}, (TcPoint){w - 56, 210}, stroke(color(.95f, .73f, .28f, 1), 5));
    tc_canvas_draw_text(canvas, "tc_runtime: SDL3 + Skia", 56, 65, (TcTextStyle){color(.92f, .95f, 1, 1), 24, NULL});
    float x = w * .5f + cosf((float)s->elapsed * 2.f) * (w * .25f);
    tc_canvas_save(canvas); tc_canvas_translate(canvas, x, h * .56f); tc_canvas_rotate(canvas, (float)s->elapsed * 55.f);
    tc_canvas_draw_round_rect(canvas, (TcRect){-36, -36, 72, 72}, 12, fill(color(.97f, .31f, .42f, 1))); tc_canvas_restore(canvas);
    if (s->pointer_down) tc_canvas_draw_circle(canvas, (TcPoint){s->pointer_x, s->pointer_y}, 18, fill(color(.3f, .95f, .66f, .85f)));
}
