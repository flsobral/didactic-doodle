/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <board/board_app.h>
#include <doodle/doodle_renderer.h>
#include <magic/magic_context.h>
#include "../common/magic_doodle_board_scene.h"

typedef struct Demo { BoardApp *app; MagicContext *magic; DoodleRenderer *renderer; MagicDoodleBoardScene scene; } Demo;

static void demo_event(void *data, const BoardEvent *event) { Demo *demo = data; magic_doodle_board_scene_event(&demo->scene, event); if (event->type == BOARD_EVENT_RESIZE) magic_context_resize(demo->magic, event->data.resize.width, event->data.resize.height, event->data.resize.scale); }
static void demo_update(void *data, double delta) { magic_doodle_board_scene_update(&((Demo *)data)->scene, delta); }
static void demo_frame(void *data, uint64_t timestamp_ns, double delta_seconds) {
    Demo *demo = data; MagicFrame *frame = NULL; DoodleCanvas *canvas = NULL;
    (void)timestamp_ns; (void)delta_seconds;
    if (magic_context_begin_frame(demo->magic, &frame) != MAGIC_OK) return;
    if (doodle_renderer_begin_frame(demo->renderer, frame, &canvas) == DOODLE_OK) { magic_doodle_board_scene_draw(&demo->scene, canvas); doodle_renderer_end_frame(demo->renderer, canvas); }
    magic_context_end_frame(demo->magic, frame);
}

int main(void) {
    BoardBackend *backend = NULL; BoardApp *app = NULL; MagicContext *magic = NULL; MagicFrame *initial_frame = NULL; DoodleRenderer *renderer = NULL; Demo demo = {0}; int result = 1;
    BoardBackendConfig backend_config = {sizeof(backend_config), BOARD_ABI_VERSION, BOARD_BACKEND_WEB, "Magic Doodle Board", 960, 640, 1, 1};
    MagicConfig magic_config = {sizeof(magic_config), MAGIC_ABI_VERSION, MAGIC_BACKEND_WEB, 1};
    DoodleRendererConfig renderer_config = {sizeof(renderer_config), DOODLE_ABI_VERSION, 0};
    BoardAppCallbacks callbacks = {sizeof(callbacks), BOARD_ABI_VERSION, NULL, demo_event, demo_update, demo_frame, NULL};
    BoardAppConfig app_config;
    if (board_backend_create(&backend_config, &backend) != BOARD_OK || magic_context_create(board_backend_surface(backend), &magic_config, &magic) != MAGIC_OK || doodle_renderer_create(doodle_skia_provider(), magic, &renderer_config, &renderer) != DOODLE_OK) goto cleanup;
    demo.magic = magic; demo.renderer = renderer; magic_doodle_board_scene_init(&demo.scene, 960, 640);
    if (magic_context_begin_frame(magic, &initial_frame) == MAGIC_OK) { magic_doodle_board_scene_init(&demo.scene, (float)magic_frame_width(initial_frame), (float)magic_frame_height(initial_frame)); magic_context_end_frame(magic, initial_frame); }
    magic_doodle_board_scene_set_runtime(&demo.scene, backend, magic, renderer);
    app_config = (BoardAppConfig){sizeof(app_config), BOARD_ABI_VERSION, backend, callbacks, &demo};
    if (board_app_create(&app_config, &app) != BOARD_OK || board_app_start(app) != BOARD_OK) goto cleanup;
    demo.app = app;
    result = board_app_run(app) == BOARD_OK ? 0 : 1;
cleanup:
    board_app_destroy(app); doodle_renderer_destroy(renderer); magic_context_destroy(magic); board_backend_destroy(backend);
    return result;
}
