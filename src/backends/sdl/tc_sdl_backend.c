/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#include <stdlib.h>

static TcPointerButton tc_sdl_button(Uint8 button) {
    if (button == SDL_BUTTON_LEFT) return TC_POINTER_BUTTON_LEFT;
    if (button == SDL_BUTTON_MIDDLE) return TC_POINTER_BUTTON_MIDDLE;
    if (button == SDL_BUTTON_RIGHT) return TC_POINTER_BUTTON_RIGHT;
    return TC_POINTER_BUTTON_NONE;
}

int tc_sdl_backend_create(const TcBackendConfig* config, TcPlatformBackend** out_backend) {
    if (!config || !out_backend) return TC_ERROR_INVALID_ARGUMENT;
    TcPlatformBackend* backend = calloc(1, sizeof(*backend));
    if (!backend) return TC_ERROR_OUT_OF_MEMORY;
    int result = tc_backend_init(backend, config);
    if (result != TC_OK) { free(backend); return result; }
    *out_backend = backend;
    return TC_OK;
}
int tc_backend_create(const TcBackendConfig* config, TcPlatformBackend** out_backend) { return tc_sdl_backend_create(config, out_backend); }
int tc_backend_init(TcPlatformBackend* backend, const TcBackendConfig* config) {
    if (!backend || !config) return TC_ERROR_INVALID_ARGUMENT;
    if (!SDL_Init(SDL_INIT_VIDEO)) return TC_ERROR_PLATFORM;
    Uint64 flags = config->resizable ? SDL_WINDOW_RESIZABLE : 0;
    backend->window.window = SDL_CreateWindow(config->title ? config->title : "tc_runtime", config->width, config->height, flags);
    if (!backend->window.window) { SDL_Quit(); return TC_ERROR_PLATFORM; }
    backend->width = config->width; backend->height = config->height; backend->initialized = true;
    backend->scheduler.running = true; backend->scheduler.requested = true;
    return TC_OK;
}
TcNativeWindowHandle* tc_backend_get_native_window(TcPlatformBackend* backend) { return backend ? &backend->window : NULL; }
TcNativeSurfaceHandle* tc_backend_get_native_surface(TcPlatformBackend* backend) { return backend ? &backend->surface : NULL; }
TcFrameScheduler* tc_backend_get_scheduler(TcPlatformBackend* backend) { return backend ? &backend->scheduler : NULL; }
void tc_sdl_backend_refresh_surface(TcPlatformBackend* backend) { if (backend && backend->window.window) backend->surface.surface = SDL_GetWindowSurface(backend->window.window); }
int tc_backend_pump_events(TcPlatformBackend* backend, TcEventSink sink, void* user_data) {
    if (!backend || !sink) return TC_ERROR_INVALID_ARGUMENT;
    SDL_Event native_event;
    while (SDL_PollEvent(&native_event)) {
        TcEvent event = { .type = TC_EVENT_NONE };
        switch (native_event.type) {
            case SDL_EVENT_QUIT: event.type = TC_EVENT_QUIT; break;
            case SDL_EVENT_WINDOW_RESIZED: case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                backend->width = native_event.window.data1; backend->height = native_event.window.data2;
                tc_sdl_backend_refresh_surface(backend);
                event.type = TC_EVENT_RESIZE; event.data.resize.width = backend->width; event.data.resize.height = backend->height; event.data.resize.scale = 1.0f; break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN: case SDL_EVENT_MOUSE_BUTTON_UP:
                event.type = native_event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? TC_EVENT_POINTER_DOWN : TC_EVENT_POINTER_UP;
                event.data.pointer.x = native_event.button.x; event.data.pointer.y = native_event.button.y; event.data.pointer.button = tc_sdl_button(native_event.button.button); event.data.pointer.pointer_id = 0; break;
            case SDL_EVENT_MOUSE_MOTION:
                event.type = TC_EVENT_POINTER_MOVE; event.data.pointer.x = native_event.motion.x; event.data.pointer.y = native_event.motion.y; event.data.pointer.button = TC_POINTER_BUTTON_NONE; event.data.pointer.pointer_id = 0; break;
            case SDL_EVENT_KEY_DOWN: case SDL_EVENT_KEY_UP:
                event.type = native_event.type == SDL_EVENT_KEY_DOWN ? TC_EVENT_KEY_DOWN : TC_EVENT_KEY_UP;
                event.data.key.key = (int)native_event.key.key; event.data.key.repeat = native_event.key.repeat; break;
            case SDL_EVENT_TEXT_INPUT:
                event.type = TC_EVENT_TEXT_INPUT; SDL_strlcpy(event.data.text.text, native_event.text.text, sizeof(event.data.text.text)); break;
            default: break;
        }
        if (event.type != TC_EVENT_NONE) sink(user_data, &event);
    }
    return TC_OK;
}
void tc_backend_request_redraw(TcPlatformBackend* backend) { if (backend) backend->redraw_requested = true; }
void tc_backend_shutdown(TcPlatformBackend* backend) { if (!backend || !backend->initialized) return; SDL_DestroyWindow(backend->window.window); backend->window.window = NULL; backend->initialized = false; SDL_Quit(); }
void tc_backend_destroy(TcPlatformBackend* backend) { if (!backend) return; tc_backend_shutdown(backend); free(backend); }
