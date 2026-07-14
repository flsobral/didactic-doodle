/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_metal.h>

typedef struct TcMetalContext {
    SDL_MetalView view;
    CAMetalLayer* layer;
    id<MTLDevice> device;
    id<MTLCommandQueue> queue;
    id<CAMetalDrawable> drawable;
} TcMetalContext;

static TcMetalContext* tc_metal(TcGraphicsContext* context) {
    return context && context->api == TC_GRAPHICS_METAL ? (TcMetalContext*)context->surface : NULL;
}

static void tc_metal_resize_drawable(TcGraphicsContext* context, int width, int height, float scale) {
    TcMetalContext* metal = tc_metal(context);
    if (!metal) return;
    metal->layer.drawableSize = CGSizeMake(width, height);
    context->width = width; context->height = height; context->scale = scale;
}

int tc_graphics_context_create(TcGraphicsApi api, TcNativeWindowHandle* window, TcNativeSurfaceHandle* surface, TcGraphicsContext** out_context) {
    (void)surface;
    SDL_Window* sdl_window = window ? (SDL_Window*)window->value : NULL;
    if (api != TC_GRAPHICS_METAL || !sdl_window || !out_context) return TC_ERROR_INVALID_ARGUMENT;
    TcGraphicsContext* context = (TcGraphicsContext*)calloc(1, sizeof(*context));
    TcMetalContext* metal = (TcMetalContext*)calloc(1, sizeof(*metal));
    if (!context || !metal) { free(context); free(metal); return TC_ERROR_OUT_OF_MEMORY; }
    metal->view = SDL_Metal_CreateView(sdl_window);
    metal->layer = (CAMetalLayer*)SDL_Metal_GetLayer(metal->view);
    metal->device = MTLCreateSystemDefaultDevice();
    metal->queue = [metal->device newCommandQueue];
    int width = 0, height = 0;
    if (!metal->view || !metal->layer || !metal->device || !metal->queue || !SDL_GetWindowSizeInPixels(sdl_window, &width, &height) || width <= 0 || height <= 0) {
        [metal->queue release]; [metal->device release]; if (metal->view) SDL_Metal_DestroyView(metal->view); free(metal); free(context); return TC_ERROR_PLATFORM;
    }
    metal->layer.device = metal->device;
    metal->layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metal->layer.framebufferOnly = YES;
    context->api = TC_GRAPHICS_METAL; context->window = sdl_window; context->surface = metal;
    tc_metal_resize_drawable(context, width, height, 1.0f);
    *out_context = context;
    SDL_Log("tc_runtime: Metal context created");
    return TC_OK;
}

void tc_graphics_context_resize(TcGraphicsContext* context, int width, int height, float scale) {
    if (!context || context->api != TC_GRAPHICS_METAL || width <= 0 || height <= 0) return;
    tc_metal_resize_drawable(context, width, height, scale);
}

void tc_graphics_context_present(TcGraphicsContext* context) {
    TcMetalContext* metal = tc_metal(context);
    if (!metal || !metal->drawable) return;
    /* Queue presentation after Skia's submitted work on the same Metal queue. */
    id<MTLCommandBuffer> present = [metal->queue commandBuffer];
    [present presentDrawable:metal->drawable];
    [present commit];
    [metal->drawable release];
    metal->drawable = nil;
}

void tc_graphics_context_destroy(TcGraphicsContext* context) {
    if (!context) return;
    TcMetalContext* metal = tc_metal(context);
    if (metal) { [metal->drawable release]; [metal->queue release]; [metal->device release]; if (metal->view) SDL_Metal_DestroyView(metal->view); free(metal); }
    free(context);
}

extern "C" int tc_metal_context_retain_handles(TcGraphicsContext* context, void** out_device, void** out_queue) {
    TcMetalContext* metal = tc_metal(context);
    if (!metal || !out_device || !out_queue) return TC_ERROR_INVALID_ARGUMENT;
    *out_device = [metal->device retain]; *out_queue = [metal->queue retain];
    return TC_OK;
}
extern "C" void tc_metal_context_release_handles(void* device, void* queue) { [(id<MTLCommandQueue>)queue release]; [(id<MTLDevice>)device release]; }
extern "C" void* tc_metal_context_get_layer(TcGraphicsContext* context) { TcMetalContext* metal = tc_metal(context); return metal ? metal->layer : NULL; }
extern "C" int tc_metal_context_set_drawable(TcGraphicsContext* context, void* drawable) { TcMetalContext* metal = tc_metal(context); if (!metal || !drawable) return TC_ERROR_INVALID_ARGUMENT; [metal->drawable release]; metal->drawable = [(id<CAMetalDrawable>)drawable retain]; return TC_OK; }
