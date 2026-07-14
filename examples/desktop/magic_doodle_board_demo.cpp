/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <board/board_app.h>
#include <doodle/doodle_renderer.h>
#include <magic/magic_context.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <string_view>

struct Demo { BoardApp *app; MagicContext *magic; DoodleRenderer *renderer; float width, height, pointer_x, pointer_y; bool pointer_down; double elapsed; unsigned frames, frame_limit; };
static DoodleColor color(float r, float g, float b, float a) { return {r, g, b, a}; }
static DoodlePaint fill(DoodleColor value) { return {value, 1, DOODLE_PAINT_FILL}; }
static DoodlePaint stroke(DoodleColor value, float width) { return {value, width, DOODLE_PAINT_STROKE}; }
static void demo_event(void *data, const BoardEvent *event) { Demo *demo = static_cast<Demo *>(data); if (event->type == BOARD_EVENT_RESIZE) { demo->width = (float)event->data.resize.width; demo->height = (float)event->data.resize.height; magic_context_resize(demo->magic, event->data.resize.width, event->data.resize.height, event->data.resize.scale); } if (event->type == BOARD_EVENT_POINTER_DOWN || event->type == BOARD_EVENT_POINTER_MOVE || event->type == BOARD_EVENT_POINTER_UP) { demo->pointer_x = event->data.pointer.x; demo->pointer_y = event->data.pointer.y; demo->pointer_down = event->type != BOARD_EVENT_POINTER_UP; } }
static void demo_update(void *data, double delta) { static_cast<Demo *>(data)->elapsed += delta; }
static void demo_frame(void *data, uint64_t, double) {
    Demo *demo = static_cast<Demo *>(data); MagicFrame *frame = nullptr; DoodleCanvas *canvas = nullptr;
    if (magic_context_begin_frame(demo->magic, &frame) != MAGIC_OK) return;
    if (doodle_renderer_begin_frame(demo->renderer, frame, &canvas) == DOODLE_OK) {
        float w = demo->width, h = demo->height;
        doodle_canvas_clear(canvas, color(.055f, .075f, .12f, 1));
        doodle_canvas_draw_rect(canvas, {32, 32, w - 64, h - 64}, fill(color(.10f, .15f, .25f, 1)));
        doodle_canvas_draw_round_rect(canvas, {56, 82, 250, 94}, 16, fill(color(.16f, .55f, .86f, 1)));
        doodle_canvas_draw_line(canvas, {56, 210}, {w - 56, 210}, stroke(color(.95f, .73f, .28f, 1), 5));
        doodle_canvas_draw_text(canvas, "Magic Doodle Board: SDL3 + CPU + Skia", 56, 65, {color(.92f, .95f, 1, 1), 24, nullptr});
        float x = w * .5f + std::cos((float)demo->elapsed * 2.f) * (w * .25f);
        doodle_canvas_save(canvas); doodle_canvas_translate(canvas, x, h * .56f); doodle_canvas_rotate(canvas, (float)demo->elapsed * 55.f); doodle_canvas_draw_round_rect(canvas, {-36, -36, 72, 72}, 12, fill(color(.97f, .31f, .42f, 1))); doodle_canvas_restore(canvas);
        if (demo->pointer_down) doodle_canvas_draw_circle(canvas, {demo->pointer_x, demo->pointer_y}, 18, fill(color(.3f, .95f, .66f, .85f)));
        doodle_renderer_end_frame(demo->renderer, canvas);
    }
    magic_context_end_frame(demo->magic, frame);
    if (demo->frame_limit && ++demo->frames >= demo->frame_limit) board_app_request_quit(demo->app);
}
int main(int argc, char **argv) {
    BoardBackend *backend = nullptr; BoardApp *app = nullptr; MagicContext *magic = nullptr; DoodleRenderer *renderer = nullptr; int result = 1;
    BoardBackendConfig backend_config = {sizeof(backend_config), BOARD_ABI_VERSION, BOARD_BACKEND_SDL3, "Magic Doodle Board", 960, 640, 1, 1};
#if MDB_DEMO_OPENGL
    MagicConfig magic_config = {sizeof(magic_config), MAGIC_ABI_VERSION, MAGIC_BACKEND_OPENGL, 1};
#else
    MagicConfig magic_config = {sizeof(magic_config), MAGIC_ABI_VERSION, MAGIC_BACKEND_CPU, 1};
#endif
    DoodleRendererConfig renderer_config = {sizeof(renderer_config), DOODLE_ABI_VERSION, 0};
    if (board_backend_create(&backend_config, &backend) != BOARD_OK || magic_context_create(board_backend_surface(backend), &magic_config, &magic) != MAGIC_OK || doodle_renderer_create(doodle_skia_provider(), magic, &renderer_config, &renderer) != DOODLE_OK) goto cleanup;
    { unsigned frame_limit = 0; if (argc == 3 && std::string_view(argv[1]) == "--frames") frame_limit = (unsigned)std::strtoul(argv[2], nullptr, 10); Demo demo = {nullptr, magic, renderer, 960, 640, 0, 0, false, 0, 0, frame_limit}; BoardAppCallbacks callbacks = {sizeof(callbacks), BOARD_ABI_VERSION, nullptr, demo_event, demo_update, demo_frame, nullptr}; BoardAppConfig app_config = {sizeof(app_config), BOARD_ABI_VERSION, backend, callbacks, &demo}; if (board_app_create(&app_config, &app) != BOARD_OK || board_app_start(app) != BOARD_OK) goto cleanup; demo.app = app; std::puts("magic_doodle_board_demo: running; close the window to exit"); result = board_app_run(app) == BOARD_OK ? 0 : 1; }
cleanup:
    board_app_destroy(app); doodle_renderer_destroy(renderer); magic_context_destroy(magic); board_backend_destroy(backend); return result;
}
