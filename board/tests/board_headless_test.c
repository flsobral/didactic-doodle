/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <board/board_app.h>
#include <string.h>
static unsigned frames;
static void on_frame(void *data, uint64_t timestamp, double delta) { (void)data; (void)timestamp; (void)delta; ++frames; }
static void on_ui_task(void *data) { *(unsigned *)data = 1; }
static BoardResult dispatch_ui(void *data, BoardUiTask task, void *task_data) { (void)data; task(task_data); return BOARD_OK; }
int main(void) {
    BoardBackend *backend = 0; BoardApp *app = 0; BoardNativeViewSlot *slot = 0; BoardHostMode mode; unsigned ran = 0;
    BoardBackendConfig backend_config = { sizeof(BoardBackendConfig), BOARD_ABI_VERSION, BOARD_BACKEND_HEADLESS, "test", 8, 8, 1.0f, 0, BOARD_HOST_MODE_HYBRID_OVERLAY };
    BoardAppCallbacks callbacks = { sizeof(BoardAppCallbacks), BOARD_ABI_VERSION, 0, 0, 0, on_frame, 0 };
    BoardAppConfig app_config = { sizeof(BoardAppConfig), BOARD_ABI_VERSION, 0, {0}, 0 };
    BoardHostServices services = { sizeof(BoardHostServices), BOARD_ABI_VERSION, 0, dispatch_ui };
    BoardNativeViewSlotConfig slot_config = { sizeof(BoardNativeViewSlotConfig), BOARD_ABI_VERSION, (void *)1, {0, 0, 1, 1}, {0, 0, 1, 1}, 1, 0, BOARD_NATIVE_VIEW_ABOVE_RENDERER };
    if (board_backend_create(&backend_config, &backend) != BOARD_OK) return 1;
    if (strcmp(board_backend_name(backend), "Headless") || !board_backend_version(backend)[0]) return 7;
    if (board_backend_host_mode(backend, &mode) != BOARD_OK || mode != BOARD_HOST_MODE_HYBRID_OVERLAY) return 4;
    if (board_host_dispatch_ui(&services, on_ui_task, &ran) != BOARD_OK || ran != 1) return 5;
    if (board_native_view_slot_create(backend, &slot_config, &slot) != BOARD_ERROR_UNAVAILABLE) return 6;
    app_config.backend = backend; app_config.callbacks = callbacks;
    if (board_app_create(&app_config, &app) != BOARD_OK || board_app_start(app) != BOARD_OK || board_app_step(app, 1) != BOARD_OK || frames != 1) return 2;
    if (board_surface_query_interface(board_backend_surface(backend), BOARD_SURFACE_INTERFACE_CPU, BOARD_ABI_VERSION + 1, &callbacks, sizeof(callbacks)) != BOARD_ERROR_VERSION) return 3;
    board_app_destroy(app); board_backend_destroy(backend); return 0;
}
