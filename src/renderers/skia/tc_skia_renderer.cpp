/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#include "tc_skia_renderer_c_api.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#if TC_BUILD_GRAPHICS_OPENGL || TC_BUILD_GRAPHICS_METAL || TC_BUILD_GRAPHICS_VULKAN
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#endif
#if TC_BUILD_GRAPHICS_OPENGL
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#endif
#if TC_BUILD_GRAPHICS_METAL
#include "include/gpu/mtl/GrMtlTypes.h"
#endif
#if TC_BUILD_GRAPHICS_VULKAN
extern "C" void* tc_android_vk_context_get_skia_context(TcGraphicsContext*);
extern "C" void* tc_android_vk_begin_frame(TcGraphicsContext*);
extern "C" int tc_android_vk_end_frame(TcGraphicsContext*, void*);
#endif
#include <memory>
#include <new>

struct TcSkiaRenderer {
    sk_sp<SkSurface> surface;
#if TC_BUILD_GRAPHICS_OPENGL
    sk_sp<GrDirectContext> gl_context;
#endif
#if TC_BUILD_GRAPHICS_METAL
    sk_sp<GrDirectContext> metal_context;
#endif
#if TC_BUILD_GRAPHICS_VULKAN
    sk_sp<GrDirectContext> vulkan_context;
#endif
    TcCanvas2D canvas;
};
static SkColor4f color(TcColor c) { return {c.r, c.g, c.b, c.a}; }
static SkPaint paint(TcPaint value) { SkPaint p; p.setColor4f(color(value.color), nullptr); p.setStyle(value.style == TC_PAINT_STROKE ? SkPaint::kStroke_Style : SkPaint::kFill_Style); p.setStrokeWidth(value.stroke_width); p.setAntiAlias(true); return p; }
static TcSkiaRenderer* impl(TcCanvas2D* canvas) { return canvas ? static_cast<TcSkiaRenderer*>(canvas->implementation) : nullptr; }
static SkCanvas* native(TcCanvas2D* canvas) { TcSkiaRenderer* r = impl(canvas); return r && r->surface ? r->surface->getCanvas() : nullptr; }
static SkImageInfo raster_info(const TcGraphicsContext* context, int width, int height) { return SkImageInfo::Make(width, height, context->pixel_format == TC_CPU_PIXEL_FORMAT_BGRA8888 ? kBGRA_8888_SkColorType : kRGBA_8888_SkColorType, kPremul_SkAlphaType); }
#if TC_BUILD_GRAPHICS_OPENGL
static sk_sp<SkSurface> gl_surface(TcSkiaRenderer* state, TcGraphicsContext* context, int width, int height) {
    GrGLint framebuffer = context->framebuffer;
    GrGLint stencil_bits = context->stencil_bits;
    GrGLint sample_count = context->sample_count;
    state->gl_context->resetContext();
    sk_sp<const GrGLInterface> gl = GrGLMakeNativeInterface();
    if (!gl || !gl->fFunctions.fBindFramebuffer) return nullptr;
    gl->fFunctions.fBindFramebuffer(0x8D40, framebuffer);
    GrGLFramebufferInfo framebuffer_info = {(GrGLuint)framebuffer, 0x8058};
    GrBackendRenderTarget target(width, height, sample_count, stencil_bits, framebuffer_info);
    return SkSurface::MakeFromBackendRenderTarget(state->gl_context.get(), target, kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType, nullptr, nullptr);
}
#endif
#if TC_BUILD_GRAPHICS_METAL
extern "C" int tc_metal_context_retain_handles(TcGraphicsContext*, void**, void**);
extern "C" void tc_metal_context_release_handles(void*, void*);
extern "C" void* tc_metal_context_get_layer(TcGraphicsContext*);
extern "C" const void** tc_metal_context_get_drawable_slot(TcGraphicsContext*);
static sk_sp<SkSurface> metal_surface(TcSkiaRenderer* state, TcGraphicsContext* context) {
    const void** drawable_slot = tc_metal_context_get_drawable_slot(context);
    if (!drawable_slot) return nullptr;
    return SkSurface::MakeFromCAMetalLayer(state->metal_context.get(), tc_metal_context_get_layer(context), kTopLeft_GrSurfaceOrigin, 1, kBGRA_8888_SkColorType, nullptr, nullptr, drawable_slot);
}
#endif
extern "C" int tc_skia_renderer_create(TcGraphicsContext* context, TcRenderer2D** out_renderer) {
    if (!context || !out_renderer) return TC_ERROR_INVALID_ARGUMENT;
    TcRenderer2D* renderer = new (std::nothrow) TcRenderer2D{};
    TcSkiaRenderer* state = new (std::nothrow) TcSkiaRenderer{};
    if (!renderer || !state) { delete renderer; delete state; return TC_ERROR_OUT_OF_MEMORY; }
    if (context->api == TC_GRAPHICS_CPU && context->pixels) {
        SkImageInfo info = raster_info(context, context->width, context->height);
        state->surface = SkSurface::MakeRasterDirect(info, context->pixels, context->pitch);
    }
#if TC_BUILD_GRAPHICS_OPENGL
    else if (context->api == TC_GRAPHICS_OPENGL && context->surface) {
        state->gl_context = GrDirectContext::MakeGL(GrGLMakeNativeInterface());
        if (state->gl_context) state->surface = gl_surface(state, context, context->width, context->height);
    }
#endif
#if TC_BUILD_GRAPHICS_METAL
    else if (context->api == TC_GRAPHICS_METAL && context->surface) {
        void* device = nullptr; void* queue = nullptr;
        if (tc_metal_context_retain_handles(context, &device, &queue) == TC_OK) {
            state->metal_context = GrDirectContext::MakeMetal(device, queue);
            if (!state->metal_context) tc_metal_context_release_handles(device, queue);
        }
    }
#endif
#if TC_BUILD_GRAPHICS_VULKAN
    else if (context->api == TC_GRAPHICS_VULKAN && context->surface) {
        if (void* native_context = tc_android_vk_context_get_skia_context(context)) state->vulkan_context = sk_ref_sp(static_cast<GrDirectContext*>(native_context));
    }
#endif
    if (!state->surface
#if TC_BUILD_GRAPHICS_METAL
        && !(context->api == TC_GRAPHICS_METAL && state->metal_context)
#endif
#if TC_BUILD_GRAPHICS_VULKAN
        && !(context->api == TC_GRAPHICS_VULKAN && state->vulkan_context)
#endif
    ) { delete renderer; delete state; return TC_ERROR_RENDERER; }
    state->canvas.implementation = state; renderer->context = context; renderer->implementation = state; renderer->canvas = &state->canvas; *out_renderer = renderer;
    return TC_OK;
}
extern "C" int tc_renderer_create(TcRendererKind kind, TcGraphicsContext* context, TcRenderer2D** out_renderer) { return kind == TC_RENDERER_SKIA ? tc_skia_renderer_create(context, out_renderer) : TC_ERROR_UNAVAILABLE; }
extern "C" int tc_renderer_attach(TcRenderer2D* renderer, TcGraphicsContext* context) { if (!renderer || !context) return TC_ERROR_INVALID_ARGUMENT; renderer->context = context; return tc_renderer_resize(renderer, context->width, context->height, context->scale); }
extern "C" int tc_renderer_resize(TcRenderer2D* renderer, int width, int height, float scale) {
    (void)scale; if (!renderer || !renderer->context || width <= 0 || height <= 0) return TC_ERROR_INVALID_ARGUMENT;
    TcGraphicsContext* context = renderer->context; TcSkiaRenderer* state = static_cast<TcSkiaRenderer*>(renderer->implementation);
    if (context->api == TC_GRAPHICS_CPU) { SkImageInfo info = raster_info(context, width, height); state->surface = SkSurface::MakeRasterDirect(info, context->pixels, context->pitch); }
#if TC_BUILD_GRAPHICS_OPENGL
    else if (context->api == TC_GRAPHICS_OPENGL && state->gl_context) state->surface = gl_surface(state, context, width, height);
#endif
#if TC_BUILD_GRAPHICS_METAL
    else if (context->api == TC_GRAPHICS_METAL && state->metal_context) state->surface.reset();
#endif
#if TC_BUILD_GRAPHICS_VULKAN
    else if (context->api == TC_GRAPHICS_VULKAN && state->vulkan_context) state->surface.reset();
#endif
    else return TC_ERROR_UNAVAILABLE;
    return state->surface
#if TC_BUILD_GRAPHICS_METAL
        || (context->api == TC_GRAPHICS_METAL && state->metal_context)
#endif
#if TC_BUILD_GRAPHICS_VULKAN
        || (context->api == TC_GRAPHICS_VULKAN && state->vulkan_context)
#endif
        ? TC_OK : TC_ERROR_RENDERER;
}
extern "C" TcCanvas2D* tc_renderer_begin_frame(TcRenderer2D* renderer) {
    if (!renderer) return nullptr;
    TcSkiaRenderer* state = static_cast<TcSkiaRenderer*>(renderer->implementation);
#if TC_BUILD_GRAPHICS_METAL
    if (renderer->context && renderer->context->api == TC_GRAPHICS_METAL && state && state->metal_context) state->surface = metal_surface(state, renderer->context);
#endif
#if TC_BUILD_GRAPHICS_VULKAN
    if (renderer->context && renderer->context->api == TC_GRAPHICS_VULKAN && state && state->vulkan_context) state->surface = sk_ref_sp(static_cast<SkSurface*>(tc_android_vk_begin_frame(renderer->context)));
#endif
    return state && state->surface ? renderer->canvas : nullptr;
}
extern "C" int tc_renderer_end_frame(TcRenderer2D* renderer) { if (!renderer) return TC_ERROR_INVALID_ARGUMENT; TcSkiaRenderer* state = static_cast<TcSkiaRenderer*>(renderer->implementation); if (!state || !state->surface) return TC_ERROR_RENDERER;
#if TC_BUILD_GRAPHICS_VULKAN
    if (renderer->context && renderer->context->api == TC_GRAPHICS_VULKAN) return tc_android_vk_end_frame(renderer->context, state->surface.get());
#endif
    state->surface->flushAndSubmit(); tc_graphics_context_present(renderer->context); return TC_OK; }
extern "C" void tc_renderer_destroy(TcRenderer2D* renderer) { if (!renderer) return; delete static_cast<TcSkiaRenderer*>(renderer->implementation); delete renderer; }

extern "C" void tc_canvas_save(TcCanvas2D* c) { if (auto* n = native(c)) n->save(); }
extern "C" void tc_canvas_restore(TcCanvas2D* c) { if (auto* n = native(c)) n->restore(); }
extern "C" void tc_canvas_translate(TcCanvas2D* c, float x, float y) { if (auto* n = native(c)) n->translate(x, y); }
extern "C" void tc_canvas_scale(TcCanvas2D* c, float x, float y) { if (auto* n = native(c)) n->scale(x, y); }
extern "C" void tc_canvas_rotate(TcCanvas2D* c, float degrees) { if (auto* n = native(c)) n->rotate(degrees); }
extern "C" void tc_canvas_clip_rect(TcCanvas2D* c, TcRect r) { if (auto* n = native(c)) n->clipRect(SkRect::MakeXYWH(r.x, r.y, r.width, r.height)); }
extern "C" void tc_canvas_clear(TcCanvas2D* c, TcColor v) { if (auto* n = native(c)) n->clear(color(v)); }
extern "C" void tc_canvas_draw_rect(TcCanvas2D* c, TcRect r, TcPaint p) { if (auto* n = native(c)) n->drawRect(SkRect::MakeXYWH(r.x, r.y, r.width, r.height), paint(p)); }
extern "C" void tc_canvas_draw_round_rect(TcCanvas2D* c, TcRect r, float radius, TcPaint p) { if (auto* n = native(c)) n->drawRoundRect(SkRect::MakeXYWH(r.x, r.y, r.width, r.height), radius, radius, paint(p)); }
extern "C" void tc_canvas_draw_line(TcCanvas2D* c, TcPoint a, TcPoint b, TcPaint p) { if (auto* n = native(c)) n->drawLine(a.x, a.y, b.x, b.y, paint(p)); }
extern "C" void tc_canvas_draw_circle(TcCanvas2D* c, TcPoint center, float radius, TcPaint p) { if (auto* n = native(c)) n->drawCircle(center.x, center.y, radius, paint(p)); }
extern "C" void tc_canvas_draw_text(TcCanvas2D* c, const char* text, float x, float y, TcTextStyle style) { if (auto* n = native(c); n && text) { SkFont font(nullptr, style.size); SkPaint p; p.setColor4f(color(style.color), nullptr); p.setAntiAlias(true); n->drawString(text, x, y, font, p); } }
