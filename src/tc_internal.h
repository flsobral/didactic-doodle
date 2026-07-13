/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_INTERNAL_H
#define TC_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <SDL3/SDL.h>
#include "tc_runtime/tc_app.h"

struct TcFrameScheduler { TcFrameCallback callback; void* user_data; bool running; bool requested; double last_timestamp; };
struct TcNativeWindowHandle { SDL_Window* window; };
struct TcNativeSurfaceHandle { SDL_Surface* surface; };
struct TcPlatformBackend { struct TcNativeWindowHandle window; struct TcNativeSurfaceHandle surface; struct TcFrameScheduler scheduler; bool initialized; bool redraw_requested; int width, height; };
struct TcGraphicsContext { TcGraphicsApi api; SDL_Window* window; SDL_Surface* surface; void* pixels; int width, height, pitch; float scale; };
struct TcRenderer2D { TcGraphicsContext* context; void* implementation; TcCanvas2D* canvas; };
struct TcCanvas2D { void* implementation; };
struct TcApp { TcAppCallbacks callbacks; void* user_data; TcPlatformBackend* backend; TcRenderer2D* renderer; bool running; bool shutdown_called; };

int tc_sdl_backend_create(const TcBackendConfig* config, TcPlatformBackend** out_backend);
int tc_cpu_context_create(TcNativeWindowHandle* window, TcNativeSurfaceHandle* surface, TcGraphicsContext** out_context);
void tc_cpu_context_resize(TcGraphicsContext* context, int width, int height, float scale);
void tc_cpu_context_present(TcGraphicsContext* context);
void tc_cpu_context_destroy(TcGraphicsContext* context);
void tc_sdl_backend_refresh_surface(TcPlatformBackend* backend);

#endif
