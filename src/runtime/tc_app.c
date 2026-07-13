/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#include <stdlib.h>

int tc_app_create(const TcAppConfig* config, TcApp** out_app) {
    if (!config || !out_app || !config->backend || !config->renderer || !config->callbacks.on_draw) return TC_ERROR_INVALID_ARGUMENT;
    TcApp* app = calloc(1, sizeof(*app));
    if (!app) return TC_ERROR_OUT_OF_MEMORY;
    app->callbacks = config->callbacks; app->user_data = config->user_data;
    app->backend = config->backend; app->renderer = config->renderer; app->running = true;
    *out_app = app;
    return TC_OK;
}

void tc_app_dispatch_event(TcApp* app, const TcEvent* event) {
    if (!app || !event) return;
    if (event->type == TC_EVENT_QUIT) { app->running = false; tc_scheduler_stop(tc_backend_get_scheduler(app->backend)); }
    if (event->type == TC_EVENT_RESIZE) {
        tc_graphics_context_resize(app->renderer->context, event->data.resize.width, event->data.resize.height, event->data.resize.scale);
        tc_renderer_resize(app->renderer, event->data.resize.width, event->data.resize.height, event->data.resize.scale);
    }
    if (app->callbacks.on_event) app->callbacks.on_event(app->user_data, event);
}

void tc_app_frame(TcApp* app, double timestamp_seconds, double delta_seconds) {
    (void)timestamp_seconds;
    if (!app || !app->running) return;
    if (app->callbacks.on_update) app->callbacks.on_update(app->user_data, delta_seconds);
    TcCanvas2D* canvas = tc_renderer_begin_frame(app->renderer);
    if (!canvas) { app->running = false; return; }
    app->callbacks.on_draw(app->user_data, canvas);
    if (tc_renderer_end_frame(app->renderer) != TC_OK) app->running = false;
}

static void tc_app_event_sink(void* user_data, const TcEvent* event) { tc_app_dispatch_event((TcApp*)user_data, event); }
static void tc_app_frame_callback(void* user_data, double timestamp_seconds, double delta_seconds) { tc_app_frame((TcApp*)user_data, timestamp_seconds, delta_seconds); }

int tc_app_run(TcApp* app) {
    if (!app) return TC_ERROR_INVALID_ARGUMENT;
    TcFrameScheduler* scheduler = tc_backend_get_scheduler(app->backend);
    if (!scheduler) return TC_ERROR_PLATFORM;
    if (tc_scheduler_start(scheduler, tc_app_frame_callback, app) != TC_OK) return TC_ERROR_PLATFORM;
    int result = tc_backend_run(app->backend, tc_app_event_sink, app);
    tc_scheduler_stop(scheduler);
    return result;
}

void tc_app_request_quit(TcApp* app) { if (app) { app->running = false; tc_scheduler_stop(tc_backend_get_scheduler(app->backend)); } }
void tc_app_destroy(TcApp* app) {
    if (!app) return;
    if (!app->shutdown_called && app->callbacks.on_shutdown) app->callbacks.on_shutdown(app->user_data);
    app->shutdown_called = true;
    free(app);
}
