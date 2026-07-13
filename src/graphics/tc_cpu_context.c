/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#include <stdlib.h>

int tc_cpu_context_create(TcNativeWindowHandle* window, TcNativeSurfaceHandle* surface, TcGraphicsContext** out_context) {
    if (!window || !window->window || !surface || !out_context) return TC_ERROR_INVALID_ARGUMENT;
    surface->surface = SDL_GetWindowSurface(window->window);
    if (!surface->surface) return TC_ERROR_PLATFORM;
    TcGraphicsContext* context = calloc(1, sizeof(*context));
    if (!context) return TC_ERROR_OUT_OF_MEMORY;
    context->api = TC_GRAPHICS_CPU; context->window = window->window; context->surface = surface->surface;
    context->pixels = surface->surface->pixels; context->width = surface->surface->w; context->height = surface->surface->h; context->pitch = surface->surface->pitch; context->scale = 1.0f;
    *out_context = context;
    return TC_OK;
}

int tc_graphics_context_create(TcGraphicsApi api, TcNativeWindowHandle* window, TcNativeSurfaceHandle* surface, TcGraphicsContext** out_context) {
    if (api != TC_GRAPHICS_CPU) return TC_ERROR_UNAVAILABLE;
    return tc_cpu_context_create(window, surface, out_context);
}
void tc_cpu_context_resize(TcGraphicsContext* context, int width, int height, float scale) {
    if (!context) return;
    context->surface = SDL_GetWindowSurface(context->window);
    if (!context->surface) return;
    context->pixels = context->surface->pixels; context->width = width; context->height = height; context->pitch = context->surface->pitch; context->scale = scale;
}
void tc_graphics_context_resize(TcGraphicsContext* context, int width, int height, float scale) { if (context) tc_cpu_context_resize(context, width, height, scale); }
void tc_cpu_context_present(TcGraphicsContext* context) { if (context && context->window) SDL_UpdateWindowSurface(context->window); }
void tc_graphics_context_present(TcGraphicsContext* context) { if (context) tc_cpu_context_present(context); }
void tc_cpu_context_destroy(TcGraphicsContext* context) { free(context); }
void tc_graphics_context_destroy(TcGraphicsContext* context) { if (context) tc_cpu_context_destroy(context); }
