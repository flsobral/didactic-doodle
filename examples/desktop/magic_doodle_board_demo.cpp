/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <board/board_app.h>
#include <doodle/doodle_renderer.h>
#include <magic/magic_context.h>
#include "../common/magic_doodle_board_scene.h"
#include <cstdlib>
#include <cstdio>
#include <string_view>

struct Demo { BoardApp *app; MagicContext *magic; DoodleRenderer *renderer; MagicDoodleBoardScene scene; unsigned frames, frame_limit; };
static void demo_event(void *data, const BoardEvent *event) { Demo *demo = static_cast<Demo *>(data); magic_doodle_board_scene_event(&demo->scene, event); if (event->type == BOARD_EVENT_RESIZE) magic_context_resize(demo->magic, event->data.resize.width, event->data.resize.height, event->data.resize.scale); }
static void demo_update(void *data, double delta) { magic_doodle_board_scene_update(&static_cast<Demo *>(data)->scene, delta); }
static void demo_frame(void *data, uint64_t, double) {
    Demo *demo = static_cast<Demo *>(data); MagicFrame *frame = nullptr; DoodleCanvas *canvas = nullptr;
    if (magic_context_begin_frame(demo->magic, &frame) != MAGIC_OK) return;
    if (doodle_renderer_begin_frame(demo->renderer, frame, &canvas) == DOODLE_OK) {
        magic_doodle_board_scene_draw(&demo->scene, canvas);
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
#elif MDB_DEMO_METAL
    MagicConfig magic_config = {sizeof(magic_config), MAGIC_ABI_VERSION, MAGIC_BACKEND_METAL, 1};
#elif MDB_DEMO_VULKAN
    MagicConfig magic_config = {sizeof(magic_config), MAGIC_ABI_VERSION, MAGIC_BACKEND_VULKAN, 1};
#else
    MagicConfig magic_config = {sizeof(magic_config), MAGIC_ABI_VERSION, MAGIC_BACKEND_CPU, 1};
#endif
    DoodleRendererConfig renderer_config = {sizeof(renderer_config), DOODLE_ABI_VERSION, 0};
    if (board_backend_create(&backend_config, &backend) != BOARD_OK || magic_context_create(board_backend_surface(backend), &magic_config, &magic) != MAGIC_OK || doodle_renderer_create(doodle_skia_provider(), magic, &renderer_config, &renderer) != DOODLE_OK) goto cleanup;
    { unsigned frame_limit = 0; if (argc == 3 && std::string_view(argv[1]) == "--frames") frame_limit = (unsigned)std::strtoul(argv[2], nullptr, 10); Demo demo = {nullptr, magic, renderer, {}, 0, frame_limit}; magic_doodle_board_scene_init(&demo.scene, 960, 640); magic_doodle_board_scene_set_runtime(&demo.scene, backend, magic, renderer); BoardAppCallbacks callbacks = {sizeof(callbacks), BOARD_ABI_VERSION, nullptr, demo_event, demo_update, demo_frame, nullptr}; BoardAppConfig app_config = {sizeof(app_config), BOARD_ABI_VERSION, backend, callbacks, &demo}; if (board_app_create(&app_config, &app) != BOARD_OK || board_app_start(app) != BOARD_OK) goto cleanup; demo.app = app; std::printf("%s | %s | %s\n", demo.scene.board_backend, demo.scene.magic_backend, demo.scene.renderer); if (frame_limit) std::printf("magic_doodle_board_demo: running for %u frames\n", frame_limit); else std::puts("magic_doodle_board_demo: running; close the window to exit"); result = board_app_run(app) == BOARD_OK ? 0 : 1; }
cleanup:
    board_app_destroy(app); doodle_renderer_destroy(renderer); magic_context_destroy(magic); board_backend_destroy(backend); return result;
}
