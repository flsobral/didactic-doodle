/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_android_backend.h"

#if defined(__ANDROID__)
#include <android/choreographer.h>
#include <android/input.h>
#include <android/native_window.h>
#include <stdlib.h>
#include <string.h>

struct TcFrameScheduler { TcFrameCallback callback; void* user_data; int running; int requested; double last_timestamp; };
struct TcAndroidNativeBackend { struct android_app* app; TcEventSink sink; void* sink_user_data; struct TcFrameScheduler scheduler; int width, height; };

static void tc_android_emit(TcAndroidNativeBackend* backend, TcEvent event) { if (backend->sink) backend->sink(backend->sink_user_data, &event); }
static void tc_android_frame(long frame_time_ns, void* user_data) {
    TcAndroidNativeBackend* backend = user_data;
    if (!backend->scheduler.running) return;
    tc_android_backend_tick(backend, (double)frame_time_ns / 1e9);
    AChoreographer_postFrameCallback(AChoreographer_getInstance(), tc_android_frame, backend);
}
static int32_t tc_android_input(struct android_app* app, AInputEvent* input) {
    TcAndroidNativeBackend* backend = app->userData;
    if (!backend || AInputEvent_getType(input) != AINPUT_EVENT_TYPE_MOTION) return 0;
    int action = AMotionEvent_getAction(input) & AMOTION_EVENT_ACTION_MASK;
    TcEvent event = { .type = TC_EVENT_POINTER_MOVE };
    if (action == AMOTION_EVENT_ACTION_DOWN) event.type = TC_EVENT_POINTER_DOWN;
    else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_CANCEL) event.type = TC_EVENT_POINTER_UP;
    else if (action != AMOTION_EVENT_ACTION_MOVE) return 0;
    event.data.pointer.x = AMotionEvent_getX(input, 0); event.data.pointer.y = AMotionEvent_getY(input, 0);
    event.data.pointer.pointer_id = (unsigned)AMotionEvent_getPointerId(input, 0); event.data.pointer.button = TC_POINTER_BUTTON_LEFT;
    tc_android_emit(backend, event); return 1;
}
static void tc_android_command(struct android_app* app, int32_t command) {
    TcAndroidNativeBackend* backend = app->userData;
    if (!backend) return;
    TcEvent event = { .type = TC_EVENT_NONE };
    switch (command) {
        case APP_CMD_INIT_WINDOW:
        case APP_CMD_WINDOW_RESIZED:
            if (!app->window) return;
            backend->width = ANativeWindow_getWidth(app->window); backend->height = ANativeWindow_getHeight(app->window);
            event.type = TC_EVENT_RESIZE; event.data.resize.width = backend->width; event.data.resize.height = backend->height; event.data.resize.scale = 1.0f; break;
        case APP_CMD_PAUSE: event.type = TC_EVENT_PAUSE; break;
        case APP_CMD_RESUME: event.type = TC_EVENT_RESUME; break;
        case APP_CMD_DESTROY: event.type = TC_EVENT_QUIT; backend->scheduler.running = 0; break;
        default: return;
    }
    tc_android_emit(backend, event);
}
int tc_android_backend_attach(struct android_app* app, TcEventSink sink, void* user_data, TcAndroidNativeBackend** out_backend) {
    if (!app || !sink || !out_backend) return TC_ERROR_INVALID_ARGUMENT;
    TcAndroidNativeBackend* backend = calloc(1, sizeof(*backend)); if (!backend) return TC_ERROR_OUT_OF_MEMORY;
    backend->app = app; backend->sink = sink; backend->sink_user_data = user_data;
    app->userData = backend; app->onAppCmd = tc_android_command; app->onInputEvent = tc_android_input; *out_backend = backend;
    return TC_OK;
}
void tc_android_backend_detach(TcAndroidNativeBackend* backend) { if (!backend) return; if (backend->app && backend->app->userData == backend) backend->app->userData = NULL; free(backend); }
TcFrameScheduler* tc_android_backend_scheduler(TcAndroidNativeBackend* backend) { return backend ? (TcFrameScheduler*)&backend->scheduler : NULL; }
int tc_android_backend_start(TcAndroidNativeBackend* backend, TcFrameCallback callback, void* user_data) {
    if (!backend || !callback) return TC_ERROR_INVALID_ARGUMENT;
    backend->scheduler.callback = callback; backend->scheduler.user_data = user_data; backend->scheduler.running = 1; backend->scheduler.last_timestamp = 0.0;
    AChoreographer_postFrameCallback(AChoreographer_getInstance(), tc_android_frame, backend);
    return TC_OK;
}
void tc_android_backend_stop(TcAndroidNativeBackend* backend) { if (backend) backend->scheduler.running = 0; }
void tc_android_backend_tick(TcAndroidNativeBackend* backend, double timestamp_seconds) {
    if (!backend || !backend->scheduler.running || !backend->scheduler.callback) return;
    double previous = backend->scheduler.last_timestamp;
    backend->scheduler.last_timestamp = timestamp_seconds;
    backend->scheduler.callback(backend->scheduler.user_data, timestamp_seconds, previous > 0.0 ? timestamp_seconds - previous : 0.0);
}
#endif
