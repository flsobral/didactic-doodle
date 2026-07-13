/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#include <SDL3/SDL.h>
#include <stdlib.h>

static int tc_cpu_context_set_surface(TcGraphicsContext* context, SDL_Surface* surface, int width, int height, float scale) {
    TcCpuPixelFormat pixel_format;
    switch (surface->format) {
        case SDL_PIXELFORMAT_ARGB8888:
        case SDL_PIXELFORMAT_XRGB8888: pixel_format = TC_CPU_PIXEL_FORMAT_BGRA8888; break;
        case SDL_PIXELFORMAT_ABGR8888:
        case SDL_PIXELFORMAT_XBGR8888: pixel_format = TC_CPU_PIXEL_FORMAT_RGBA8888; break;
        default:
            SDL_Log("tc_runtime: unsupported CPU window-surface format %s", SDL_GetPixelFormatName(surface->format));
            return TC_ERROR_UNAVAILABLE;
    }
    context->surface = surface; context->pixels = surface->pixels; context->width = width; context->height = height;
    context->pitch = surface->pitch; context->scale = scale; context->pixel_format = pixel_format;
    return TC_OK;
}

int tc_cpu_context_create(TcNativeWindowHandle* window, TcNativeSurfaceHandle* surface, TcGraphicsContext** out_context) {
    SDL_Window* sdl_window = window ? window->value : NULL;
    if (!sdl_window || !surface || !out_context) return TC_ERROR_INVALID_ARGUMENT;
    surface->value = SDL_GetWindowSurface(sdl_window);
    SDL_Surface* sdl_surface = surface->value;
    if (!sdl_surface) return TC_ERROR_PLATFORM;
    TcGraphicsContext* context = calloc(1, sizeof(*context));
    if (!context) return TC_ERROR_OUT_OF_MEMORY;
    context->api = TC_GRAPHICS_CPU; context->window = sdl_window;
    int result = tc_cpu_context_set_surface(context, sdl_surface, sdl_surface->w, sdl_surface->h, 1.0f);
    if (result != TC_OK) { free(context); return result; }
    SDL_Log("tc_runtime: CPU window surface format %s", SDL_GetPixelFormatName(sdl_surface->format));
    *out_context = context;
    return TC_OK;
}

int tc_graphics_context_create(TcGraphicsApi api, TcNativeWindowHandle* window, TcNativeSurfaceHandle* surface, TcGraphicsContext** out_context) {
    if (api != TC_GRAPHICS_CPU) return TC_ERROR_UNAVAILABLE;
    return tc_cpu_context_create(window, surface, out_context);
}
void tc_cpu_context_resize(TcGraphicsContext* context, int width, int height, float scale) {
    if (!context) return;
    SDL_Surface* surface = SDL_GetWindowSurface((SDL_Window*)context->window);
    if (!surface) return;
    tc_cpu_context_set_surface(context, surface, width, height, scale);
}
void tc_graphics_context_resize(TcGraphicsContext* context, int width, int height, float scale) { if (context) tc_cpu_context_resize(context, width, height, scale); }
void tc_cpu_context_present(TcGraphicsContext* context) { if (context && context->window) SDL_UpdateWindowSurface((SDL_Window*)context->window); }
void tc_graphics_context_present(TcGraphicsContext* context) { if (context) tc_cpu_context_present(context); }
void tc_cpu_context_destroy(TcGraphicsContext* context) { free(context); }
void tc_graphics_context_destroy(TcGraphicsContext* context) { if (context) tc_cpu_context_destroy(context); }
