/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#include <SDL3/SDL.h>
#include <stdlib.h>

int tc_graphics_context_create(TcGraphicsApi api, TcNativeWindowHandle* window, TcNativeSurfaceHandle* surface, TcGraphicsContext** out_context) {
    (void)surface;
    SDL_Window* sdl_window = window ? window->value : NULL;
    if (api != TC_GRAPHICS_OPENGL || !sdl_window || !out_context) return TC_ERROR_INVALID_ARGUMENT;
    SDL_GLContext gl_context = SDL_GL_CreateContext(sdl_window);
    if (!gl_context || !SDL_GL_MakeCurrent(sdl_window, gl_context)) { if (gl_context) SDL_GL_DestroyContext(gl_context); return TC_ERROR_PLATFORM; }
    SDL_GL_SetSwapInterval(1);
    int width = 0, height = 0;
    if (!SDL_GetWindowSizeInPixels(sdl_window, &width, &height) || width <= 0 || height <= 0) { SDL_GL_DestroyContext(gl_context); return TC_ERROR_PLATFORM; }
    TcGraphicsContext* context = calloc(1, sizeof(*context));
    if (!context) { SDL_GL_DestroyContext(gl_context); return TC_ERROR_OUT_OF_MEMORY; }
    context->api = TC_GRAPHICS_OPENGL; context->window = sdl_window; context->surface = gl_context; context->width = width; context->height = height; context->scale = 1.0f;
    *out_context = context;
    SDL_Log("tc_runtime: OpenGL 3.2 core context created");
    return TC_OK;
}

void tc_graphics_context_resize(TcGraphicsContext* context, int width, int height, float scale) {
    if (!context || context->api != TC_GRAPHICS_OPENGL) return;
    context->width = width; context->height = height; context->scale = scale;
}

void tc_graphics_context_present(TcGraphicsContext* context) {
    if (!context || context->api != TC_GRAPHICS_OPENGL) return;
    if (!SDL_GL_SwapWindow((SDL_Window*)context->window)) SDL_Log("tc_runtime: OpenGL swap failed: %s", SDL_GetError());
}

void tc_graphics_context_destroy(TcGraphicsContext* context) {
    if (!context) return;
    if (context->surface) SDL_GL_DestroyContext((SDL_GLContext)context->surface);
    free(context);
}
