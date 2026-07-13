/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_INTERNAL_H
#define TC_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include "tc_runtime/tc_app.h"

struct TcFrameScheduler { TcFrameCallback callback; void* user_data; bool running; bool requested; double last_timestamp; };
struct TcNativeWindowHandle { void* value; };
struct TcNativeSurfaceHandle { void* value; };
struct TcPlatformBackend { struct TcNativeWindowHandle window; struct TcNativeSurfaceHandle surface; struct TcFrameScheduler scheduler; void* implementation; bool initialized; bool redraw_requested; int width, height; };
struct TcGraphicsContext { TcGraphicsApi api; void* window; void* surface; void* pixels; int width, height, pitch; float scale; };
struct TcRenderer2D { TcGraphicsContext* context; void* implementation; TcCanvas2D* canvas; };
struct TcCanvas2D { void* implementation; };
struct TcApp { TcAppCallbacks callbacks; void* user_data; TcPlatformBackend* backend; TcRenderer2D* renderer; bool running; bool shutdown_called; };

int tc_sdl_backend_create(const TcBackendConfig* config, TcPlatformBackend** out_backend);
int tc_backend_run(TcPlatformBackend* backend, TcEventSink sink, void* user_data);
int tc_cpu_context_create(TcNativeWindowHandle* window, TcNativeSurfaceHandle* surface, TcGraphicsContext** out_context);
void tc_cpu_context_resize(TcGraphicsContext* context, int width, int height, float scale);
void tc_cpu_context_present(TcGraphicsContext* context);
void tc_cpu_context_destroy(TcGraphicsContext* context);
void tc_sdl_backend_refresh_surface(TcPlatformBackend* backend);
int tc_android_cpu_context_create(void* native_window, TcGraphicsContext** out_context);
int tc_android_cpu_context_resize(TcGraphicsContext* context, int width, int height, float scale);
void tc_android_cpu_destroy(TcGraphicsContext* context);

#endif
