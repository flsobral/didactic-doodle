/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#import <UIKit/UIKit.h>

int tc_ios_cpu_context_create(void* view, int width, int height, float scale, TcGraphicsContext** out_context) {
    if (!view || !out_context || width <= 0 || height <= 0) return TC_ERROR_INVALID_ARGUMENT;
    TcGraphicsContext* context = (TcGraphicsContext*)calloc(1, sizeof(*context));
    if (!context) return TC_ERROR_OUT_OF_MEMORY;
    context->pixels = calloc((size_t)width * (size_t)height, 4); if (!context->pixels) { free(context); return TC_ERROR_OUT_OF_MEMORY; }
    context->api = TC_GRAPHICS_CPU; context->window = view; context->width = width; context->height = height; context->pitch = width * 4; context->scale = scale; *out_context = context; return TC_OK;
}
int tc_ios_cpu_context_resize(TcGraphicsContext* context, int width, int height, float scale) {
    if (!context || width <= 0 || height <= 0) return TC_ERROR_INVALID_ARGUMENT;
    void* pixels = realloc(context->pixels, (size_t)width * (size_t)height * 4); if (!pixels) return TC_ERROR_OUT_OF_MEMORY;
    context->pixels = pixels; context->width = width; context->height = height; context->pitch = width * 4; context->scale = scale; return TC_OK;
}
void tc_ios_cpu_context_present(TcGraphicsContext* context) { if (context && context->window) [(__bridge UIView*)context->window setNeedsDisplay]; }
void tc_ios_cpu_context_destroy(TcGraphicsContext* context) { if (context) { free(context->pixels); free(context); } }
