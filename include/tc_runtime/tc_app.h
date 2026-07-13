/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_RUNTIME_APP_H
#define TC_RUNTIME_APP_H
#include "tc_backend.h"
#include "tc_canvas.h"
#include "tc_renderer.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct TcAppCallbacks { void (*on_event)(void* user_data, const TcEvent* event); void (*on_update)(void* user_data, double delta_seconds); void (*on_draw)(void* user_data, TcCanvas2D* canvas); void (*on_shutdown)(void* user_data); } TcAppCallbacks;
typedef struct TcAppConfig { TcAppCallbacks callbacks; void* user_data; TcPlatformBackend* backend; TcRenderer2D* renderer; } TcAppConfig;
int tc_app_create(const TcAppConfig* config, TcApp** out_app); void tc_app_dispatch_event(TcApp* app, const TcEvent* event); void tc_app_frame(TcApp* app, double timestamp_seconds, double delta_seconds); int tc_app_run(TcApp* app); void tc_app_request_quit(TcApp* app); void tc_app_destroy(TcApp* app);
#ifdef __cplusplus
}
#endif
#endif
