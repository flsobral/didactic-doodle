/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <board/board_android.h>
#include <board/board_app.h>
#include <doodle/doodle_renderer.h>
#include <magic/magic_context.h>
#include "../common/magic_doodle_board_scene.h"
#include <android_native_app_glue.h>

typedef struct AndroidDemo {
    BoardBackend *backend;
    BoardApp *app;
    MagicContext *magic;
    DoodleRenderer *renderer;
    MagicDoodleBoardScene scene;
} AndroidDemo;

static void android_demo_event(void *data, const BoardEvent *event) {
    AndroidDemo *demo = (AndroidDemo *)data;
    magic_doodle_board_scene_event(&demo->scene, event);
    if (event->type == BOARD_EVENT_RESIZE) (void)magic_context_resize(demo->magic, event->data.resize.width, event->data.resize.height, event->data.resize.scale);
}

static void android_demo_update(void *data, double delta_seconds) {
    magic_doodle_board_scene_update(&((AndroidDemo *)data)->scene, delta_seconds);
}

static void android_demo_frame(void *data, uint64_t timestamp_ns, double delta_seconds) {
    AndroidDemo *demo = (AndroidDemo *)data;
    MagicFrame *frame = NULL;
    DoodleCanvas *canvas = NULL;
    (void)timestamp_ns;
    (void)delta_seconds;
    if (magic_context_begin_frame(demo->magic, &frame) != MAGIC_OK) return;
    if (doodle_renderer_begin_frame(demo->renderer, frame, &canvas) == DOODLE_OK) {
        magic_doodle_board_scene_draw(&demo->scene, canvas);
        doodle_renderer_end_frame(demo->renderer, canvas);
    }
    (void)magic_context_end_frame(demo->magic, frame);
}

void android_main(struct android_app *native_app) {
    AndroidDemo demo = {0};
    BoardBackendConfig backend_config = {sizeof(BoardBackendConfig), BOARD_ABI_VERSION, BOARD_BACKEND_ANDROID, "Magic Doodle Board", 1280, 720, 1.0f, 0};
#if MDB_ANDROID_OPENGL
    MagicConfig magic_config = {sizeof(MagicConfig), MAGIC_ABI_VERSION, MAGIC_BACKEND_OPENGL, 1};
#else
    MagicConfig magic_config = {sizeof(MagicConfig), MAGIC_ABI_VERSION, MAGIC_BACKEND_CPU, 1};
#endif
    DoodleRendererConfig renderer_config = {sizeof(DoodleRendererConfig), DOODLE_ABI_VERSION, 0};
    BoardAppCallbacks callbacks = {sizeof(BoardAppCallbacks), BOARD_ABI_VERSION, NULL, android_demo_event, android_demo_update, android_demo_frame, NULL};
    BoardAppConfig app_config;
    BoardSurfaceCpuInterface surface;
    void *pixels;
    uint32_t width, height, stride;
    BoardPixelFormat format;
    float scale;

    if (board_backend_create(&backend_config, &demo.backend) != BOARD_OK ||
        board_android_attach(demo.backend, native_app) != BOARD_OK ||
        magic_context_create(board_backend_surface(demo.backend), &magic_config, &demo.magic) != MAGIC_OK ||
        doodle_renderer_create(doodle_skia_provider(), demo.magic, &renderer_config, &demo.renderer) != DOODLE_OK) goto cleanup;
    if (board_surface_query_interface(board_backend_surface(demo.backend), BOARD_SURFACE_INTERFACE_CPU, BOARD_ABI_VERSION, &surface, sizeof(surface)) != BOARD_OK ||
        surface.map_pixels(surface.user_data, &pixels, &width, &height, &stride, &format, &scale) != BOARD_OK) goto cleanup;
    (void)pixels;
    (void)stride;
    (void)format;
    magic_doodle_board_scene_init(&demo.scene, (float)width, (float)height);
    app_config = (BoardAppConfig){sizeof(BoardAppConfig), BOARD_ABI_VERSION, demo.backend, callbacks, &demo};
    if (board_app_create(&app_config, &demo.app) != BOARD_OK || board_app_start(demo.app) != BOARD_OK) goto cleanup;
    (void)board_app_run(demo.app);

cleanup:
    board_app_destroy(demo.app);
    doodle_renderer_destroy(demo.renderer);
    magic_context_destroy(demo.magic);
    board_backend_destroy(demo.backend);
}
