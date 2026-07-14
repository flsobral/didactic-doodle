/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <stdlib.h>

typedef struct TcWebBackend {
    TcEventSink sink;
    void* user_data;
    double previous_seconds;
    float scale;
} TcWebBackend;

static TcWebBackend* tc_web(TcPlatformBackend* backend) {
    return backend ? (TcWebBackend*)backend->implementation : NULL;
}

static TcPointerButton tc_web_button(short button) {
    if (button == 0) return TC_POINTER_BUTTON_LEFT;
    if (button == 1) return TC_POINTER_BUTTON_MIDDLE;
    if (button == 2) return TC_POINTER_BUTTON_RIGHT;
    return TC_POINTER_BUTTON_NONE;
}

static void tc_web_emit(TcPlatformBackend* backend, TcEvent event) {
    TcWebBackend* state = tc_web(backend);
    if (state && state->sink) state->sink(state->user_data, &event);
}

static void tc_web_resize_canvas(TcPlatformBackend* backend, double css_width, double css_height) {
    TcWebBackend* state = tc_web(backend);
    if (!state || css_width <= 0 || css_height <= 0) return;
    const double scale = emscripten_get_device_pixel_ratio();
    const int width = (int)(css_width * scale + 0.5);
    const int height = (int)(css_height * scale + 0.5);
    if (width <= 0 || height <= 0) return;
    emscripten_set_canvas_element_size("#canvas", width, height);
    if (backend->width == width && backend->height == height && state->scale == (float)scale) return;
    backend->width = width;
    backend->height = height;
    state->scale = (float)scale;
    TcEvent event = {.type = TC_EVENT_RESIZE};
    event.data.resize.width = width;
    event.data.resize.height = height;
    event.data.resize.scale = state->scale;
    tc_web_emit(backend, event);
}

static EM_BOOL tc_web_resize_callback(int event_type, const EmscriptenUiEvent* event, void* user_data) {
    (void)event_type;
    (void)event;
    double width = 0.0, height = 0.0;
    emscripten_get_element_css_size("#canvas", &width, &height);
    tc_web_resize_canvas((TcPlatformBackend*)user_data, width, height);
    return EM_TRUE;
}

static EM_BOOL tc_web_mouse_callback(int event_type, const EmscriptenMouseEvent* event, void* user_data) {
    TcPlatformBackend* backend = user_data;
    TcWebBackend* state = tc_web(backend);
    if (!state) return EM_FALSE;
    TcEvent output = {.type = TC_EVENT_POINTER_MOVE};
    if (event_type == EMSCRIPTEN_EVENT_MOUSEDOWN) output.type = TC_EVENT_POINTER_DOWN;
    if (event_type == EMSCRIPTEN_EVENT_MOUSEUP) output.type = TC_EVENT_POINTER_UP;
    output.data.pointer.x = (float)event->targetX * state->scale;
    output.data.pointer.y = (float)event->targetY * state->scale;
    output.data.pointer.button = tc_web_button(event->button);
    output.data.pointer.pointer_id = 0;
    tc_web_emit(backend, output);
    return EM_TRUE;
}

static EM_BOOL tc_web_touch_callback(int event_type, const EmscriptenTouchEvent* event, void* user_data) {
    TcPlatformBackend* backend = user_data;
    TcWebBackend* state = tc_web(backend);
    if (!state || event->numTouches == 0) return EM_FALSE;
    const EmscriptenTouchPoint* touch = &event->touches[0];
    for (int i = 0; i < event->numTouches; ++i) {
        if (event->touches[i].isChanged) { touch = &event->touches[i]; break; }
    }
    TcEvent output = {.type = event_type == EMSCRIPTEN_EVENT_TOUCHEND ? TC_EVENT_POINTER_UP : event_type == EMSCRIPTEN_EVENT_TOUCHSTART ? TC_EVENT_POINTER_DOWN : TC_EVENT_POINTER_MOVE};
    output.data.pointer.x = (float)touch->targetX * state->scale;
    output.data.pointer.y = (float)touch->targetY * state->scale;
    output.data.pointer.button = TC_POINTER_BUTTON_LEFT;
    output.data.pointer.pointer_id = (unsigned int)touch->identifier;
    tc_web_emit(backend, output);
    return EM_TRUE;
}

static EM_BOOL tc_web_key_callback(int event_type, const EmscriptenKeyboardEvent* event, void* user_data) {
    TcEvent output = {.type = event_type == EMSCRIPTEN_EVENT_KEYDOWN ? TC_EVENT_KEY_DOWN : TC_EVENT_KEY_UP};
    output.data.key.key = event->keyCode;
    output.data.key.repeat = event->repeat;
    tc_web_emit((TcPlatformBackend*)user_data, output);
    return EM_FALSE;
}

static void tc_web_frame(void* user_data) {
    TcPlatformBackend* backend = user_data;
    TcWebBackend* state = tc_web(backend);
    if (!backend || !state || !backend->scheduler.running) {
        emscripten_cancel_main_loop();
        return;
    }
    const double now = emscripten_get_now() / 1000.0;
    const double delta = state->previous_seconds > 0.0 ? now - state->previous_seconds : 0.0;
    state->previous_seconds = now;
    if (backend->scheduler.callback) backend->scheduler.callback(backend->scheduler.user_data, now, delta);
}

int tc_backend_init(TcPlatformBackend* backend, const TcBackendConfig* config) {
    if (!backend || !config) return TC_ERROR_INVALID_ARGUMENT;
    TcWebBackend* state = calloc(1, sizeof(*state));
    if (!state) return TC_ERROR_OUT_OF_MEMORY;
    backend->implementation = state;
    backend->window.value = backend;
    backend->initialized = true;
    backend->scheduler.running = true;
    backend->scheduler.requested = true;
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, backend, EM_TRUE, tc_web_resize_callback);
    emscripten_set_mousedown_callback("#canvas", backend, EM_TRUE, tc_web_mouse_callback);
    emscripten_set_mousemove_callback("#canvas", backend, EM_TRUE, tc_web_mouse_callback);
    emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, backend, EM_TRUE, tc_web_mouse_callback);
    emscripten_set_touchstart_callback("#canvas", backend, EM_TRUE, tc_web_touch_callback);
    emscripten_set_touchmove_callback("#canvas", backend, EM_TRUE, tc_web_touch_callback);
    emscripten_set_touchend_callback("#canvas", backend, EM_TRUE, tc_web_touch_callback);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, backend, EM_TRUE, tc_web_key_callback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, backend, EM_TRUE, tc_web_key_callback);
    double width = config->width, height = config->height;
    emscripten_get_element_css_size("#canvas", &width, &height);
    tc_web_resize_canvas(backend, width, height);
    return TC_OK;
}

int tc_backend_create(const TcBackendConfig* config, TcPlatformBackend** out_backend) {
    if (!config || !out_backend) return TC_ERROR_INVALID_ARGUMENT;
    TcPlatformBackend* backend = calloc(1, sizeof(*backend));
    if (!backend) return TC_ERROR_OUT_OF_MEMORY;
    const int result = tc_backend_init(backend, config);
    if (result != TC_OK) { free(backend); return result; }
    *out_backend = backend;
    return TC_OK;
}

TcNativeWindowHandle* tc_backend_get_native_window(TcPlatformBackend* backend) { return backend ? &backend->window : NULL; }
TcNativeSurfaceHandle* tc_backend_get_native_surface(TcPlatformBackend* backend) { return backend ? &backend->surface : NULL; }
TcFrameScheduler* tc_backend_get_scheduler(TcPlatformBackend* backend) { return backend ? &backend->scheduler : NULL; }
int tc_backend_pump_events(TcPlatformBackend* backend, TcEventSink sink, void* user_data) { (void)backend; (void)sink; (void)user_data; return TC_OK; }
int tc_backend_run(TcPlatformBackend* backend, TcEventSink sink, void* user_data) {
    TcWebBackend* state = tc_web(backend);
    if (!state || !sink || !backend->scheduler.callback) return TC_ERROR_INVALID_ARGUMENT;
    state->sink = sink;
    state->user_data = user_data;
    emscripten_set_main_loop_arg(tc_web_frame, backend, 0, EM_TRUE);
    return TC_OK;
}
void tc_backend_request_redraw(TcPlatformBackend* backend) { if (backend) backend->redraw_requested = true; }
void tc_backend_shutdown(TcPlatformBackend* backend) {
    if (!backend || !backend->initialized) return;
    emscripten_cancel_main_loop();
    free(backend->implementation);
    backend->implementation = NULL;
    backend->initialized = false;
}
void tc_backend_destroy(TcPlatformBackend* backend) { if (!backend) return; tc_backend_shutdown(backend); free(backend); }
#endif
