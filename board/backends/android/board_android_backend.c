/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "../../src/board_internal.h"
#include <board/board_android.h>
#include <android/choreographer.h>
#include <android/input.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct BoardAndroidState {
    BoardBackend *backend;
    struct android_app *app;
    ANativeWindow *window;
} BoardAndroidState;

static uint64_t board_android_timestamp(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000000000ULL + (uint64_t)now.tv_nsec;
}

static void board_android_emit(BoardBackend *backend, BoardEventType type) {
    BoardEvent event = {sizeof(BoardEvent), BOARD_ABI_VERSION, type, board_android_timestamp(), {{0}}};
    if (backend && backend->event_sink) backend->event_sink(backend->event_data, &event);
}

static BoardResult board_android_resize(BoardAndroidState *state) {
    BoardBackend *backend;
    uint8_t *pixels;
    uint32_t width, height;
    BoardEvent event = {sizeof(BoardEvent), BOARD_ABI_VERSION, BOARD_EVENT_RESIZE, board_android_timestamp(), {{0}}};
    if (!state || !state->window || !(backend = state->backend)) return BOARD_ERROR_UNAVAILABLE;
    if (ANativeWindow_setBuffersGeometry(state->window, 0, 0, WINDOW_FORMAT_RGBA_8888) != 0) return BOARD_ERROR_PLATFORM;
    width = (uint32_t)ANativeWindow_getWidth(state->window);
    height = (uint32_t)ANativeWindow_getHeight(state->window);
    if (!width || !height) return BOARD_ERROR_UNAVAILABLE;
    if (width == backend->width && height == backend->height) return BOARD_OK;
    pixels = (uint8_t *)realloc(backend->pixels, (size_t)width * height * 4u);
    if (!pixels) return BOARD_ERROR_OUT_OF_MEMORY;
    backend->pixels = pixels;
    backend->width = width;
    backend->height = height;
    backend->stride = width * 4u;
    backend->scale = 1.0f;
    event.data.resize.width = width;
    event.data.resize.height = height;
    event.data.resize.scale = backend->scale;
    if (backend->event_sink) backend->event_sink(backend->event_data, &event);
    return BOARD_OK;
}

static BoardResult board_android_map(void *data, void **pixels, uint32_t *width, uint32_t *height, uint32_t *stride, BoardPixelFormat *format, float *scale) {
    BoardAndroidState *state = (BoardAndroidState *)data;
    BoardBackend *backend = state ? state->backend : NULL;
    if (!state || !state->window || !backend || !pixels || !width || !height || !stride || !format || !scale) return BOARD_ERROR_UNAVAILABLE;
    *pixels = backend->pixels;
    *width = backend->width;
    *height = backend->height;
    *stride = backend->stride;
    *format = BOARD_PIXEL_FORMAT_RGBA8888;
    *scale = backend->scale;
    return BOARD_OK;
}

static BoardResult board_android_present(void *data) {
    BoardAndroidState *state = (BoardAndroidState *)data;
    BoardBackend *backend = state ? state->backend : NULL;
    ANativeWindow_Buffer buffer;
    uint32_t rows, bytes, y;
    if (!state || !state->window || !backend || !backend->pixels || ANativeWindow_lock(state->window, &buffer, NULL) != 0) return BOARD_ERROR_PLATFORM;
    rows = backend->height < (uint32_t)buffer.height ? backend->height : (uint32_t)buffer.height;
    bytes = backend->stride < (uint32_t)buffer.stride * 4u ? backend->stride : (uint32_t)buffer.stride * 4u;
    for (y = 0; y < rows; ++y) memcpy((uint8_t *)buffer.bits + (size_t)y * buffer.stride * 4u, backend->pixels + (size_t)y * backend->stride, bytes);
    ANativeWindow_unlockAndPost(state->window);
    return BOARD_OK;
}

static int32_t board_android_input(struct android_app *app, AInputEvent *input) {
    BoardAndroidState *state = app ? (BoardAndroidState *)app->userData : NULL;
    BoardBackend *backend = state ? state->backend : NULL;
    BoardEvent event = {sizeof(BoardEvent), BOARD_ABI_VERSION, BOARD_EVENT_NONE, board_android_timestamp(), {{0}}};
    int32_t action;
    if (!backend || AInputEvent_getType(input) != AINPUT_EVENT_TYPE_MOTION) return 0;
    action = AMotionEvent_getAction(input) & AMOTION_EVENT_ACTION_MASK;
    if (action == AMOTION_EVENT_ACTION_DOWN) event.type = BOARD_EVENT_POINTER_DOWN;
    else if (action == AMOTION_EVENT_ACTION_MOVE) event.type = BOARD_EVENT_POINTER_MOVE;
    else if (action == AMOTION_EVENT_ACTION_UP) event.type = BOARD_EVENT_POINTER_UP;
    else if (action == AMOTION_EVENT_ACTION_CANCEL) event.type = BOARD_EVENT_POINTER_CANCEL;
    else return 0;
    event.data.pointer.x = AMotionEvent_getX(input, 0);
    event.data.pointer.y = AMotionEvent_getY(input, 0);
    event.data.pointer.pointer_id = (uint32_t)AMotionEvent_getPointerId(input, 0);
    event.data.pointer.button = BOARD_POINTER_BUTTON_LEFT;
    if (backend->event_sink) backend->event_sink(backend->event_data, &event);
    return 1;
}

static void board_android_command(struct android_app *app, int32_t command) {
    BoardAndroidState *state = app ? (BoardAndroidState *)app->userData : NULL;
    if (!state || !state->backend) return;
    switch (command) {
        case APP_CMD_INIT_WINDOW:
        case APP_CMD_WINDOW_RESIZED:
            state->window = app->window;
            (void)board_android_resize(state);
            break;
        case APP_CMD_TERM_WINDOW:
            state->window = NULL;
            board_android_emit(state->backend, BOARD_EVENT_PAUSE);
            break;
        case APP_CMD_PAUSE:
            board_android_emit(state->backend, BOARD_EVENT_PAUSE);
            break;
        case APP_CMD_RESUME:
            board_android_emit(state->backend, BOARD_EVENT_RESUME);
            break;
        case APP_CMD_DESTROY:
            board_android_emit(state->backend, BOARD_EVENT_QUIT);
            board_scheduler_stop(&state->backend->scheduler);
            break;
        default:
            break;
    }
}

static void board_android_frame(long timestamp_ns, void *data) {
    BoardAndroidState *state = (BoardAndroidState *)data;
    BoardBackend *backend = state ? state->backend : NULL;
    if (!backend || !backend->scheduler.running) return;
    if (state->window) {
        board_scheduler_request_frame(&backend->scheduler);
        board_backend_step(backend, (uint64_t)timestamp_ns);
    }
    if (backend->scheduler.running) AChoreographer_postFrameCallback(AChoreographer_getInstance(), board_android_frame, state);
}

static BoardResult board_android_start(BoardBackend *backend) {
    BoardAndroidState *state = backend ? (BoardAndroidState *)backend->implementation : NULL;
    if (!state || !state->app || !state->window) return BOARD_ERROR_UNAVAILABLE;
    AChoreographer_postFrameCallback(AChoreographer_getInstance(), board_android_frame, state);
    return BOARD_OK;
}

static BoardResult board_android_run(BoardBackend *backend, BoardEventSink sink, void *data) {
    BoardAndroidState *state = backend ? (BoardAndroidState *)backend->implementation : NULL;
    (void)sink;
    (void)data;
    if (!state || !state->app) return BOARD_ERROR_UNAVAILABLE;
    while (backend->scheduler.running && !state->app->destroyRequested) {
        int events;
        struct android_poll_source *source = NULL;
        if (ALooper_pollOnce(-1, NULL, &events, (void **)&source) >= 0 && source) source->process(state->app, source);
    }
    return BOARD_OK;
}

static void board_android_dispose(BoardBackend *backend) {
    BoardAndroidState *state = backend ? (BoardAndroidState *)backend->implementation : NULL;
    if (state && state->app && state->app->userData == state) {
        state->app->userData = NULL;
        state->app->onAppCmd = NULL;
        state->app->onInputEvent = NULL;
    }
    free(state);
    free(backend->pixels);
    backend->implementation = NULL;
    backend->pixels = NULL;
}

BoardResult board_android_backend_init(BoardBackend *backend, const BoardBackendConfig *config) {
    BoardAndroidState *state;
    if (!backend || !config) return BOARD_ERROR_INVALID_ARGUMENT;
    backend->width = config->width;
    backend->height = config->height;
    backend->stride = config->width * 4u;
    backend->scale = 1.0f;
    backend->pixels = (uint8_t *)calloc(config->height, backend->stride);
    state = (BoardAndroidState *)calloc(1, sizeof(*state));
    if (!backend->pixels || !state) { free(backend->pixels); free(state); backend->pixels = NULL; return BOARD_ERROR_OUT_OF_MEMORY; }
    state->backend = backend;
    backend->implementation = state;
    backend->surface.cpu = (BoardSurfaceCpuInterface){sizeof(BoardSurfaceCpuInterface), BOARD_ABI_VERSION, state, board_android_map, board_android_present};
    backend->start = board_android_start;
    backend->run = board_android_run;
    backend->dispose = board_android_dispose;
    return BOARD_OK;
}

BoardResult board_android_attach(BoardBackend *backend, void *native_app) {
    BoardAndroidState *state = backend ? (BoardAndroidState *)backend->implementation : NULL;
    struct android_app *app = (struct android_app *)native_app;
    if (!backend || backend->kind != BOARD_BACKEND_ANDROID || !state || !app || state->app || app->userData) return BOARD_ERROR_INVALID_ARGUMENT;
    state->app = app;
    app->userData = state;
    app->onAppCmd = board_android_command;
    app->onInputEvent = board_android_input;
    while (!app->window && !app->destroyRequested) {
        int events;
        struct android_poll_source *source = NULL;
        if (ALooper_pollOnce(-1, NULL, &events, (void **)&source) >= 0 && source) source->process(app, source);
    }
    if (app->destroyRequested || !state->window) return BOARD_ERROR_UNAVAILABLE;
    return board_android_resize(state);
}
