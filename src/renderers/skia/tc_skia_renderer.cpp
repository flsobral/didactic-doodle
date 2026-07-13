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
#include <memory>
#include <new>

struct TcSkiaRenderer { sk_sp<SkSurface> surface; TcCanvas2D canvas; };
static SkColor4f color(TcColor c) { return {c.r, c.g, c.b, c.a}; }
static SkPaint paint(TcPaint value) { SkPaint p; p.setColor4f(color(value.color), nullptr); p.setStyle(value.style == TC_PAINT_STROKE ? SkPaint::kStroke_Style : SkPaint::kFill_Style); p.setStrokeWidth(value.stroke_width); p.setAntiAlias(true); return p; }
static TcSkiaRenderer* impl(TcCanvas2D* canvas) { return canvas ? static_cast<TcSkiaRenderer*>(canvas->implementation) : nullptr; }
static SkCanvas* native(TcCanvas2D* canvas) { TcSkiaRenderer* r = impl(canvas); return r && r->surface ? r->surface->getCanvas() : nullptr; }
static SkImageInfo raster_info(const TcGraphicsContext* context, int width, int height) { return SkImageInfo::Make(width, height, context->pixel_format == TC_CPU_PIXEL_FORMAT_BGRA8888 ? kBGRA_8888_SkColorType : kRGBA_8888_SkColorType, kPremul_SkAlphaType); }

extern "C" int tc_skia_renderer_create(TcGraphicsContext* context, TcRenderer2D** out_renderer) {
    if (!context || !out_renderer || context->api != TC_GRAPHICS_CPU || !context->pixels) return TC_ERROR_INVALID_ARGUMENT;
    TcRenderer2D* renderer = new (std::nothrow) TcRenderer2D{};
    TcSkiaRenderer* state = new (std::nothrow) TcSkiaRenderer{};
    if (!renderer || !state) { delete renderer; delete state; return TC_ERROR_OUT_OF_MEMORY; }
    SkImageInfo info = raster_info(context, context->width, context->height);
    state->surface = SkSurface::MakeRasterDirect(info, context->pixels, context->pitch);
    if (!state->surface) { delete renderer; delete state; return TC_ERROR_RENDERER; }
    state->canvas.implementation = state; renderer->context = context; renderer->implementation = state; renderer->canvas = &state->canvas; *out_renderer = renderer;
    return TC_OK;
}
extern "C" int tc_renderer_create(TcRendererKind kind, TcGraphicsContext* context, TcRenderer2D** out_renderer) { return kind == TC_RENDERER_SKIA ? tc_skia_renderer_create(context, out_renderer) : TC_ERROR_UNAVAILABLE; }
extern "C" int tc_renderer_attach(TcRenderer2D* renderer, TcGraphicsContext* context) { if (!renderer || !context) return TC_ERROR_INVALID_ARGUMENT; renderer->context = context; return tc_renderer_resize(renderer, context->width, context->height, context->scale); }
extern "C" int tc_renderer_resize(TcRenderer2D* renderer, int width, int height, float scale) {
    (void)scale; if (!renderer || !renderer->context || width <= 0 || height <= 0) return TC_ERROR_INVALID_ARGUMENT;
    TcGraphicsContext* context = renderer->context; TcSkiaRenderer* state = static_cast<TcSkiaRenderer*>(renderer->implementation);
    SkImageInfo info = raster_info(context, width, height); state->surface = SkSurface::MakeRasterDirect(info, context->pixels, context->pitch);
    return state->surface ? TC_OK : TC_ERROR_RENDERER;
}
extern "C" TcCanvas2D* tc_renderer_begin_frame(TcRenderer2D* renderer) { return renderer ? renderer->canvas : nullptr; }
extern "C" int tc_renderer_end_frame(TcRenderer2D* renderer) { if (!renderer) return TC_ERROR_INVALID_ARGUMENT; TcSkiaRenderer* state = static_cast<TcSkiaRenderer*>(renderer->implementation); if (!state || !state->surface) return TC_ERROR_RENDERER; state->surface->flushAndSubmit(); tc_graphics_context_present(renderer->context); return TC_OK; }
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
