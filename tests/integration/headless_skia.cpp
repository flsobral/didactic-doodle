/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <board/board_app.h>
#include <doodle/doodle_renderer.h>
#include <magic/magic_context.h>
#include <magic/magic_interop.h>
#include <cstdio>

struct App { MagicContext *magic; DoodleRenderer *renderer; };
static void draw(void *data, uint64_t, double) {
    App *app = static_cast<App *>(data); MagicFrame *frame = nullptr; DoodleCanvas *canvas = nullptr;
    if (magic_context_begin_frame(app->magic, &frame) != MAGIC_OK) return;
    if (doodle_renderer_begin_frame(app->renderer, frame, &canvas) == DOODLE_OK) {
        doodle_canvas_clear(canvas, {0.04f, 0.08f, 0.16f, 1.0f});
        doodle_canvas_draw_rect(canvas, {4, 5, 18, 10}, {{0.2f, 0.8f, 0.4f, 1.0f}, 1, DOODLE_PAINT_FILL});
        doodle_canvas_draw_circle(canvas, {24, 20}, 6, {{0.95f, 0.4f, 0.2f, 1.0f}, 1, DOODLE_PAINT_FILL});
        doodle_renderer_end_frame(app->renderer, canvas);
    }
    magic_context_end_frame(app->magic, frame);
}
int main() {
    BoardBackend *backend = nullptr; BoardApp *board = nullptr; MagicContext *magic = nullptr; DoodleRenderer *renderer = nullptr;
    BoardBackendConfig backend_config = {sizeof(backend_config), BOARD_ABI_VERSION, BOARD_BACKEND_HEADLESS, "Skia", 32, 24, 1, 0};
    MagicConfig magic_config = {sizeof(magic_config), MAGIC_ABI_VERSION, MAGIC_BACKEND_CPU, 1};
    DoodleRendererConfig renderer_config = {sizeof(renderer_config), DOODLE_ABI_VERSION, 0};
    if (board_backend_create(&backend_config, &backend) != BOARD_OK || magic_context_create(board_backend_surface(backend), &magic_config, &magic) != MAGIC_OK || doodle_renderer_create(doodle_skia_provider(), magic, &renderer_config, &renderer) != DOODLE_OK) return 1;
    App app = {magic, renderer}; BoardAppCallbacks callbacks = {sizeof(callbacks), BOARD_ABI_VERSION, nullptr, nullptr, nullptr, draw, nullptr}; BoardAppConfig app_config = {sizeof(app_config), BOARD_ABI_VERSION, backend, callbacks, &app};
    if (board_app_create(&app_config, &board) != BOARD_OK || board_app_start(board) != BOARD_OK || board_app_step(board, 1000000) != BOARD_OK) return 2;
    BoardSurfaceCpuInterface surface = {}; void *pixels = nullptr; uint32_t width = 0, height = 0, stride = 0; BoardPixelFormat format; float scale;
    if (board_surface_query_interface(board_backend_surface(backend), BOARD_SURFACE_INTERFACE_CPU, BOARD_ABI_VERSION, &surface, sizeof(surface)) != BOARD_OK || surface.map_pixels(surface.user_data, &pixels, &width, &height, &stride, &format, &scale) != BOARD_OK) return 3;
    uint64_t hash = 1469598103934665603ULL; for (uint32_t row = 0; row < height; ++row) for (uint32_t column = 0; column < stride; ++column) { hash ^= static_cast<unsigned char *>(pixels)[row * stride + column]; hash *= 1099511628211ULL; }
    std::printf("headless-skia hash: %016llx\n", static_cast<unsigned long long>(hash));
    board_app_destroy(board); doodle_renderer_destroy(renderer); magic_context_destroy(magic); board_backend_destroy(backend);
    return hash == 0xbff7964c10eaa55fULL ? 0 : 4;
}
