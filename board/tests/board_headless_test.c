/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <board/board_app.h>
static unsigned frames;
static void on_frame(void *data, uint64_t timestamp, double delta) { (void)data; (void)timestamp; (void)delta; ++frames; }
int main(void) {
    BoardBackend *backend = 0; BoardApp *app = 0;
    BoardBackendConfig backend_config = { sizeof(BoardBackendConfig), BOARD_ABI_VERSION, BOARD_BACKEND_HEADLESS, "test", 8, 8, 1.0f, 0 };
    BoardAppCallbacks callbacks = { sizeof(BoardAppCallbacks), BOARD_ABI_VERSION, 0, 0, 0, on_frame, 0 };
    BoardAppConfig app_config = { sizeof(BoardAppConfig), BOARD_ABI_VERSION, 0, {0}, 0 };
    if (board_backend_create(&backend_config, &backend) != BOARD_OK) return 1;
    app_config.backend = backend; app_config.callbacks = callbacks;
    if (board_app_create(&app_config, &app) != BOARD_OK || board_app_start(app) != BOARD_OK || board_app_step(app, 1) != BOARD_OK || frames != 1) return 2;
    if (board_surface_query_interface(board_backend_surface(backend), BOARD_SURFACE_INTERFACE_CPU, 2, &callbacks, sizeof(callbacks)) != BOARD_ERROR_VERSION) return 3;
    board_app_destroy(app); board_backend_destroy(backend); return 0;
}
