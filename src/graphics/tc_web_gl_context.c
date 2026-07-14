/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <stdint.h>
#include <stdlib.h>

int tc_graphics_context_create(TcGraphicsApi api, TcNativeWindowHandle* window, TcNativeSurfaceHandle* surface, TcGraphicsContext** out_context) {
    (void)window;
    (void)surface;
    if (api != TC_GRAPHICS_OPENGL || !out_context) return TC_ERROR_INVALID_ARGUMENT;
    EmscriptenWebGLContextAttributes attributes;
    emscripten_webgl_init_context_attributes(&attributes);
    attributes.alpha = EM_TRUE;
    attributes.antialias = EM_TRUE;
    attributes.explicitSwapControl = EM_TRUE;
    attributes.majorVersion = 2;
    const EMSCRIPTEN_WEBGL_CONTEXT_HANDLE handle = emscripten_webgl_create_context("#canvas", &attributes);
    if (handle <= 0 || emscripten_webgl_make_context_current(handle) != EMSCRIPTEN_RESULT_SUCCESS) return TC_ERROR_PLATFORM;
    int width = 0, height = 0;
    emscripten_webgl_get_drawing_buffer_size(handle, &width, &height);
    TcGraphicsContext* context = calloc(1, sizeof(*context));
    if (!context) { emscripten_webgl_destroy_context(handle); return TC_ERROR_OUT_OF_MEMORY; }
    context->api = TC_GRAPHICS_OPENGL;
    context->surface = (void*)(intptr_t)handle;
    context->width = width;
    context->height = height;
    context->scale = (float)emscripten_get_device_pixel_ratio();
    *out_context = context;
    return TC_OK;
}

void tc_graphics_context_resize(TcGraphicsContext* context, int width, int height, float scale) {
    if (!context || context->api != TC_GRAPHICS_OPENGL) return;
    emscripten_set_canvas_element_size("#canvas", width, height);
    context->width = width;
    context->height = height;
    context->scale = scale;
}

void tc_graphics_context_present(TcGraphicsContext* context) {
    if (context && context->api == TC_GRAPHICS_OPENGL) emscripten_webgl_commit_frame();
}

void tc_graphics_context_destroy(TcGraphicsContext* context) {
    if (!context) return;
    if (context->surface) emscripten_webgl_destroy_context((EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)(intptr_t)context->surface);
    free(context);
}
#endif
