/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#if defined(__ANDROID__)
#include <android/native_window.h>
#include <android/window.h>
#include <stdlib.h>
#include <string.h>

int tc_android_cpu_context_create(void* native_window, TcGraphicsContext** out_context) {
    ANativeWindow* window = native_window; if (!window || !out_context) return TC_ERROR_INVALID_ARGUMENT;
    if (ANativeWindow_setBuffersGeometry(window, 0, 0, WINDOW_FORMAT_RGBA_8888) != 0) return TC_ERROR_PLATFORM;
    int width = ANativeWindow_getWidth(window), height = ANativeWindow_getHeight(window);
    TcGraphicsContext* context = calloc(1, sizeof(*context)); if (!context) return TC_ERROR_OUT_OF_MEMORY;
    context->pixels = calloc((size_t)width * (size_t)height, 4); if (!context->pixels) { free(context); return TC_ERROR_OUT_OF_MEMORY; }
    context->api = TC_GRAPHICS_CPU; context->window = window; context->width = width; context->height = height; context->pitch = width * 4; context->scale = 1.0f; context->pixel_format = TC_CPU_PIXEL_FORMAT_RGBA8888; *out_context = context; return TC_OK;
}
int tc_android_cpu_context_resize(TcGraphicsContext* context, int width, int height, float scale) {
    if (!context || width <= 0 || height <= 0) return TC_ERROR_INVALID_ARGUMENT;
    void* pixels = realloc(context->pixels, (size_t)width * (size_t)height * 4); if (!pixels) return TC_ERROR_OUT_OF_MEMORY;
    context->pixels = pixels; context->width = width; context->height = height; context->pitch = width * 4; context->scale = scale;
    return TC_OK;
}
void tc_android_cpu_present(TcGraphicsContext* context) {
    ANativeWindow_Buffer buffer; if (!context || ANativeWindow_lock(context->window, &buffer, NULL) != 0) return;
    int rows = context->height < buffer.height ? context->height : buffer.height, bytes = context->pitch < buffer.stride * 4 ? context->pitch : buffer.stride * 4;
    for (int y = 0; y < rows; ++y) memcpy((char*)buffer.bits + (size_t)y * buffer.stride * 4, (char*)context->pixels + (size_t)y * context->pitch, bytes);
    ANativeWindow_unlockAndPost(context->window);
}
void tc_android_cpu_destroy(TcGraphicsContext* context) { if (context) { free(context->pixels); free(context); } }
#endif
