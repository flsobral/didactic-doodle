/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "board_internal.h"
#include <stdlib.h>
#include <string.h>

BoardResult board_surface_query_interface(BoardNativeSurface *surface, BoardSurfaceInterfaceId id, uint32_t version, void *out, size_t size) { if (!surface || !out) return BOARD_ERROR_INVALID_ARGUMENT; if (version != BOARD_ABI_VERSION) return BOARD_ERROR_VERSION; if (id == BOARD_SURFACE_INTERFACE_CPU) { if (size < sizeof(BoardSurfaceCpuInterface)) return BOARD_ERROR_INVALID_ARGUMENT; memcpy(out, &surface->cpu, sizeof(surface->cpu)); return BOARD_OK; } if (id == BOARD_SURFACE_INTERFACE_OPENGL && surface->opengl.create_context) { if (size < sizeof(BoardSurfaceOpenGLInterface)) return BOARD_ERROR_INVALID_ARGUMENT; memcpy(out, &surface->opengl, sizeof(surface->opengl)); return BOARD_OK; } return BOARD_ERROR_UNAVAILABLE; }
BoardResult board_scheduler_start(BoardFrameScheduler *scheduler, BoardFrameCallback callback, void *user_data) { if (!scheduler || !callback) return BOARD_ERROR_INVALID_ARGUMENT; scheduler->callback = callback; scheduler->user_data = user_data; scheduler->previous_timestamp_ns = 0; scheduler->running = 1; scheduler->requested = 1; return BOARD_OK; }
BoardResult board_scheduler_request_frame(BoardFrameScheduler *scheduler) { if (!scheduler || !scheduler->running) return BOARD_ERROR_INVALID_ARGUMENT; scheduler->requested = 1; return BOARD_OK; }
void board_scheduler_stop(BoardFrameScheduler *scheduler) { if (scheduler) scheduler->running = 0; }
BoardResult board_scheduler_step(BoardFrameScheduler *scheduler, uint64_t timestamp_ns) { double delta; if (!scheduler || !scheduler->running) return BOARD_ERROR_INVALID_ARGUMENT; if (!scheduler->requested) return BOARD_OK; delta = scheduler->previous_timestamp_ns ? (double)(timestamp_ns - scheduler->previous_timestamp_ns) / 1000000000.0 : 0.0; scheduler->previous_timestamp_ns = timestamp_ns; scheduler->requested = 0; scheduler->callback(scheduler->user_data, timestamp_ns, delta); return BOARD_OK; }
BoardResult board_backend_create(const BoardBackendConfig *config, BoardBackend **out_backend) {
    BoardBackend *backend; BoardResult result;
    if (!config || !out_backend || config->struct_size < sizeof(*config) || config->abi_version != BOARD_ABI_VERSION || !config->width || !config->height) return BOARD_ERROR_INVALID_ARGUMENT;
    backend = calloc(1, sizeof(*backend)); if (!backend) return BOARD_ERROR_OUT_OF_MEMORY; backend->kind = config->kind;
    if (config->kind == BOARD_BACKEND_HEADLESS) result = board_headless_backend_init(backend, config);
#if BOARD_BUILD_SDL3
    else if (config->kind == BOARD_BACKEND_SDL3) result = board_sdl3_backend_init(backend, config);
#endif
    else result = BOARD_ERROR_UNAVAILABLE;
    if (result != BOARD_OK) { free(backend); return result; } *out_backend = backend; return BOARD_OK;
}
void board_backend_destroy(BoardBackend *backend) { if (backend) { if (backend->dispose) backend->dispose(backend); free(backend); } }
BoardNativeSurface *board_backend_surface(BoardBackend *backend) { return backend ? &backend->surface : NULL; }
BoardFrameScheduler *board_backend_scheduler(BoardBackend *backend) { return backend ? &backend->scheduler : NULL; }
BoardResult board_backend_post_event(BoardBackend *backend, const BoardEvent *event) { if (!backend || !event || event->struct_size < sizeof(*event) || event->abi_version != BOARD_ABI_VERSION || backend->event_count == 32) return BOARD_ERROR_INVALID_ARGUMENT; backend->events[backend->event_count++] = *event; return BOARD_OK; }
BoardResult board_backend_dispatch_events(BoardBackend *backend, BoardEventSink sink, void *data) { uint32_t i; if (!backend || !sink) return BOARD_ERROR_INVALID_ARGUMENT; for (i = 0; i < backend->event_count; ++i) sink(data, &backend->events[i]); backend->event_count = 0; return BOARD_OK; }
BoardResult board_backend_step(BoardBackend *backend, uint64_t timestamp_ns) { return backend ? board_scheduler_step(&backend->scheduler, timestamp_ns) : BOARD_ERROR_INVALID_ARGUMENT; }
BoardResult board_backend_run_internal(BoardBackend *backend, BoardEventSink sink, void *data) { return !backend || !sink || !backend->run ? BOARD_ERROR_UNAVAILABLE : backend->run(backend, sink, data); }
static void board_app_event(void *data, const BoardEvent *event) { BoardApp *app = data; if (event->type == BOARD_EVENT_QUIT) board_app_request_quit(app); if (app->callbacks.on_event) app->callbacks.on_event(app->user_data, event); }
static void board_app_frame(void *data, uint64_t timestamp_ns, double delta) { BoardApp *app = data; if (!app->running) return; if (app->callbacks.on_update) app->callbacks.on_update(app->user_data, delta); if (app->callbacks.on_frame) app->callbacks.on_frame(app->user_data, timestamp_ns, delta); }
BoardResult board_app_create(const BoardAppConfig *config, BoardApp **out_app) { BoardApp *app; if (!config || !out_app || !config->backend || config->struct_size < sizeof(*config) || config->abi_version != BOARD_ABI_VERSION || config->callbacks.struct_size < sizeof(BoardAppCallbacks) || config->callbacks.abi_version != BOARD_ABI_VERSION) return BOARD_ERROR_INVALID_ARGUMENT; app = calloc(1, sizeof(*app)); if (!app) return BOARD_ERROR_OUT_OF_MEMORY; app->backend = config->backend; app->callbacks = config->callbacks; app->user_data = config->user_data; *out_app = app; return BOARD_OK; }
void board_app_destroy(BoardApp *app) { if (!app) return; if (app->started && app->callbacks.on_shutdown) app->callbacks.on_shutdown(app->user_data); free(app); }
BoardResult board_app_start(BoardApp *app) { if (!app) return BOARD_ERROR_INVALID_ARGUMENT; app->running = 1; app->started = 1; if (app->callbacks.on_start) app->callbacks.on_start(app->user_data); return board_scheduler_start(&app->backend->scheduler, board_app_frame, app); }
void board_app_request_quit(BoardApp *app) { if (app) { app->running = 0; board_scheduler_stop(&app->backend->scheduler); } }
BoardResult board_app_step(BoardApp *app, uint64_t timestamp_ns) { if (!app || !app->started) return BOARD_ERROR_INVALID_ARGUMENT; board_backend_dispatch_events(app->backend, board_app_event, app); return board_backend_step(app->backend, timestamp_ns); }
BoardResult board_app_run(BoardApp *app) { if (!app || !app->started) return BOARD_ERROR_INVALID_ARGUMENT; return board_backend_run_internal(app->backend, board_app_event, app); }
