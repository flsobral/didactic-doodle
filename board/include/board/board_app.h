/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef BOARD_APP_H
#define BOARD_APP_H
#include "board_backend.h"
#include "board_version.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct BoardApp BoardApp;
typedef struct BoardAppCallbacks { uint32_t struct_size; uint32_t abi_version; void (*on_start)(void *user_data); void (*on_event)(void *user_data, const BoardEvent *event); void (*on_update)(void *user_data, double delta_seconds); void (*on_frame)(void *user_data, uint64_t timestamp_ns, double delta_seconds); void (*on_shutdown)(void *user_data); } BoardAppCallbacks;
typedef struct BoardAppConfig { uint32_t struct_size; uint32_t abi_version; BoardBackend *backend; BoardAppCallbacks callbacks; void *user_data; } BoardAppConfig;
BoardResult board_app_create(const BoardAppConfig *config, BoardApp **out_app);
void board_app_destroy(BoardApp *app);
BoardResult board_app_start(BoardApp *app);
BoardResult board_app_run(BoardApp *app);
void board_app_request_quit(BoardApp *app);
BoardResult board_app_step(BoardApp *app, uint64_t timestamp_ns);
#ifdef __cplusplus
}
#endif
#endif
