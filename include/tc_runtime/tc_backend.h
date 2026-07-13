/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_RUNTIME_BACKEND_H
#define TC_RUNTIME_BACKEND_H
#include "tc_event.h"
#include "tc_scheduler.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct TcBackendConfig { const char* title; int width, height; int resizable; } TcBackendConfig;
int tc_backend_create(const TcBackendConfig* config, TcPlatformBackend** out_backend);
int tc_backend_init(TcPlatformBackend* backend, const TcBackendConfig* config);
TcNativeWindowHandle* tc_backend_get_native_window(TcPlatformBackend* backend);
TcNativeSurfaceHandle* tc_backend_get_native_surface(TcPlatformBackend* backend);
TcFrameScheduler* tc_backend_get_scheduler(TcPlatformBackend* backend);
int tc_backend_pump_events(TcPlatformBackend* backend, TcEventSink sink, void* user_data);
void tc_backend_request_redraw(TcPlatformBackend* backend);
void tc_backend_shutdown(TcPlatformBackend* backend);
void tc_backend_destroy(TcPlatformBackend* backend);
#ifdef __cplusplus
}
#endif
#endif
