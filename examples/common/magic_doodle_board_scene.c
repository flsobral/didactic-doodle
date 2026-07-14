/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "magic_doodle_board_scene.h"
#include <math.h>

static DoodleColor scene_color(float r, float g, float b, float a) { return (DoodleColor){r, g, b, a}; }
static DoodlePaint scene_fill(DoodleColor color) { return (DoodlePaint){color, 1, DOODLE_PAINT_FILL}; }
static DoodlePaint scene_stroke(DoodleColor color, float width) { return (DoodlePaint){color, width, DOODLE_PAINT_STROKE}; }
void magic_doodle_board_scene_init(MagicDoodleBoardScene *scene, float width, float height) { if (scene) *scene = (MagicDoodleBoardScene){width, height, 0, 0, 0, 0}; }
void magic_doodle_board_scene_event(MagicDoodleBoardScene *scene, const BoardEvent *event) {
    if (!scene || !event) return;
    if (event->type == BOARD_EVENT_RESIZE) { scene->width = event->data.resize.width; scene->height = event->data.resize.height; }
    if (event->type == BOARD_EVENT_POINTER_DOWN || event->type == BOARD_EVENT_POINTER_MOVE || event->type == BOARD_EVENT_POINTER_UP || event->type == BOARD_EVENT_POINTER_CANCEL) { scene->pointer_x = event->data.pointer.x; scene->pointer_y = event->data.pointer.y; scene->pointer_down = event->type != BOARD_EVENT_POINTER_UP && event->type != BOARD_EVENT_POINTER_CANCEL; }
}
void magic_doodle_board_scene_update(MagicDoodleBoardScene *scene, double delta_seconds) { if (scene) scene->elapsed += delta_seconds; }
void magic_doodle_board_scene_draw(MagicDoodleBoardScene *scene, DoodleCanvas *canvas) {
    float x;
    if (!scene || !canvas) return;
    doodle_canvas_clear(canvas, scene_color(.055f, .075f, .12f, 1));
    doodle_canvas_draw_rect(canvas, (DoodleRect){32, 32, scene->width - 64, scene->height - 64}, scene_fill(scene_color(.10f, .15f, .25f, 1)));
    doodle_canvas_draw_round_rect(canvas, (DoodleRect){56, 82, 250, 94}, 16, scene_fill(scene_color(.16f, .55f, .86f, 1)));
    doodle_canvas_draw_line(canvas, (DoodlePoint){56, 210}, (DoodlePoint){scene->width - 56, 210}, scene_stroke(scene_color(.95f, .73f, .28f, 1), 5));
    doodle_canvas_draw_text(canvas, "Magic Doodle Board", 56, 65, (DoodleTextStyle){scene_color(.92f, .95f, 1, 1), 24, NULL});
    x = scene->width * .5f + cosf((float)scene->elapsed * 2.f) * (scene->width * .25f);
    doodle_canvas_save(canvas);
    doodle_canvas_translate(canvas, x, scene->height * .56f);
    doodle_canvas_rotate(canvas, (float)scene->elapsed * 55.f);
    doodle_canvas_draw_round_rect(canvas, (DoodleRect){-36, -36, 72, 72}, 12, scene_fill(scene_color(.97f, .31f, .42f, 1)));
    doodle_canvas_restore(canvas);
    if (scene->pointer_down) doodle_canvas_draw_circle(canvas, (DoodlePoint){scene->pointer_x, scene->pointer_y}, 18, scene_fill(scene_color(.3f, .95f, .66f, .85f)));
}
