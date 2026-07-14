/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <UIKit/UIKit.h>

typedef struct TcIosMetalContext {
    CAMetalLayer* layer;
    id<MTLDevice> device;
    id<MTLCommandQueue> queue;
    id<CAMetalDrawable> drawable;
} TcIosMetalContext;

static TcIosMetalContext* tc_ios_metal(TcGraphicsContext* context) {
    return context && context->api == TC_GRAPHICS_METAL ? (TcIosMetalContext*)context->surface : NULL;
}

static int tc_ios_metal_resize_drawable(TcGraphicsContext* context, int width, int height, float scale) {
    TcIosMetalContext* metal = tc_ios_metal(context);
    if (!metal || width <= 0 || height <= 0) return TC_ERROR_INVALID_ARGUMENT;
    metal->layer.contentsScale = scale;
    metal->layer.drawableSize = CGSizeMake(width, height);
    context->width = width; context->height = height; context->scale = scale;
    return TC_OK;
}

int tc_ios_metal_context_create(void* view, int width, int height, float scale, TcGraphicsContext** out_context) {
    UIView* native_view = (__bridge UIView*)view;
    if (!native_view || !out_context || width <= 0 || height <= 0 || ![native_view.layer isKindOfClass:CAMetalLayer.class]) return TC_ERROR_INVALID_ARGUMENT;
    TcGraphicsContext* context = (TcGraphicsContext*)calloc(1, sizeof(*context));
    TcIosMetalContext* metal = (TcIosMetalContext*)calloc(1, sizeof(*metal));
    if (!context || !metal) { free(context); free(metal); return TC_ERROR_OUT_OF_MEMORY; }
    metal->layer = (CAMetalLayer*)native_view.layer;
    metal->device = MTLCreateSystemDefaultDevice();
    metal->queue = [metal->device newCommandQueue];
    if (!metal->device || !metal->queue) { [metal->queue release]; [metal->device release]; free(metal); free(context); return TC_ERROR_UNAVAILABLE; }
    metal->layer.device = metal->device;
    metal->layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metal->layer.framebufferOnly = YES;
    context->api = TC_GRAPHICS_METAL; context->window = view; context->surface = metal;
    if (tc_ios_metal_resize_drawable(context, width, height, scale) != TC_OK) { [metal->queue release]; [metal->device release]; free(metal); free(context); return TC_ERROR_PLATFORM; }
    *out_context = context;
    return TC_OK;
}

int tc_ios_metal_context_resize(TcGraphicsContext* context, int width, int height, float scale) {
    return tc_ios_metal_resize_drawable(context, width, height, scale);
}

void tc_ios_metal_context_present(TcGraphicsContext* context) {
    TcIosMetalContext* metal = tc_ios_metal(context);
    if (!metal || !metal->drawable) return;
    id<MTLCommandBuffer> present = [metal->queue commandBuffer];
    [present presentDrawable:metal->drawable];
    [present commit];
    [metal->drawable release];
    metal->drawable = nil;
}

void tc_ios_metal_context_destroy(TcGraphicsContext* context) {
    if (!context) return;
    TcIosMetalContext* metal = tc_ios_metal(context);
    if (metal) { [metal->drawable release]; [metal->queue release]; [metal->device release]; free(metal); }
    free(context);
}

extern "C" int tc_metal_context_retain_handles(TcGraphicsContext* context, void** out_device, void** out_queue) {
    TcIosMetalContext* metal = tc_ios_metal(context);
    if (!metal || !out_device || !out_queue) return TC_ERROR_INVALID_ARGUMENT;
    *out_device = [metal->device retain];
    *out_queue = [metal->queue retain];
    return TC_OK;
}
extern "C" void tc_metal_context_release_handles(void* device, void* queue) { [(id<MTLCommandQueue>)queue release]; [(id<MTLDevice>)device release]; }
extern "C" void* tc_metal_context_get_layer(TcGraphicsContext* context) { TcIosMetalContext* metal = tc_ios_metal(context); return metal ? metal->layer : NULL; }
extern "C" const void** tc_metal_context_get_drawable_slot(TcGraphicsContext* context) { TcIosMetalContext* metal = tc_ios_metal(context); return metal ? (const void**)&metal->drawable : NULL; }
