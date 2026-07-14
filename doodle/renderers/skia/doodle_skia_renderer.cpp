/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <doodle/doodle_renderer.h>
#include <magic/magic_interop.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include <memory>
#include <new>

struct DoodleCanvas { SkCanvas *native; };
struct DoodleSkiaState { sk_sp<SkSurface> surface; DoodleCanvas canvas; };
static SkColor4f skia_color(DoodleColor color) { return { color.r, color.g, color.b, color.a }; }
static SkPaint skia_paint(DoodlePaint paint) { SkPaint result; result.setColor4f(skia_color(paint.color), nullptr); result.setStyle(paint.style == DOODLE_PAINT_STROKE ? SkPaint::kStroke_Style : SkPaint::kFill_Style); result.setStrokeWidth(paint.stroke_width); result.setAntiAlias(true); return result; }
static DoodleResult skia_create(MagicContext *, const DoodleRendererConfig *, void **out_state) { DoodleSkiaState *state = new (std::nothrow) DoodleSkiaState{}; if (!state) return DOODLE_ERROR_OUT_OF_MEMORY; *out_state = state; return DOODLE_OK; }
static void skia_destroy(void *state) { delete static_cast<DoodleSkiaState *>(state); }
static DoodleResult skia_begin(void *value, MagicFrame *frame, DoodleCanvas **out_canvas) {
  MagicCpuInterop pixels{}; DoodleSkiaState *state = static_cast<DoodleSkiaState *>(value);
  if (!state || magic_frame_query_interop(frame, MAGIC_INTEROP_CPU, MAGIC_ABI_VERSION, &pixels, sizeof(pixels)) != MAGIC_OK) return DOODLE_ERROR_UNAVAILABLE;
  SkColorType format = pixels.format == MAGIC_PIXEL_FORMAT_BGRA8888 ? kBGRA_8888_SkColorType : kRGBA_8888_SkColorType;
  state->surface = SkSurface::MakeRasterDirect(SkImageInfo::Make((int)pixels.width, (int)pixels.height, format, kPremul_SkAlphaType), pixels.pixels, pixels.stride);
  if (!state->surface) return DOODLE_ERROR_RENDERER;
  state->canvas.native = state->surface->getCanvas(); *out_canvas = &state->canvas; return DOODLE_OK;
}
static DoodleResult skia_end(void *value, DoodleCanvas *canvas) { DoodleSkiaState *state = static_cast<DoodleSkiaState *>(value); if (!state || canvas != &state->canvas || !state->surface) return DOODLE_ERROR_INVALID_ARGUMENT; state->surface->flushAndSubmit(); state->canvas.native = nullptr; state->surface.reset(); return DOODLE_OK; }
static const DoodleRendererProvider provider = { sizeof(DoodleRendererProvider), DOODLE_ABI_VERSION, "Skia", skia_create, skia_destroy, skia_begin, skia_end, nullptr, nullptr, nullptr, nullptr };
extern "C" const DoodleRendererProvider *doodle_skia_provider(void) { return &provider; }
extern "C" void doodle_canvas_save(DoodleCanvas *canvas) { if (canvas && canvas->native) canvas->native->save(); }
extern "C" void doodle_canvas_restore(DoodleCanvas *canvas) { if (canvas && canvas->native) canvas->native->restore(); }
extern "C" void doodle_canvas_translate(DoodleCanvas *canvas, float x, float y) { if (canvas && canvas->native) canvas->native->translate(x, y); }
extern "C" void doodle_canvas_scale(DoodleCanvas *canvas, float x, float y) { if (canvas && canvas->native) canvas->native->scale(x, y); }
extern "C" void doodle_canvas_rotate(DoodleCanvas *canvas, float degrees) { if (canvas && canvas->native) canvas->native->rotate(degrees); }
extern "C" void doodle_canvas_clip_rect(DoodleCanvas *canvas, DoodleRect rect) { if (canvas && canvas->native) canvas->native->clipRect(SkRect::MakeXYWH(rect.x, rect.y, rect.width, rect.height)); }
extern "C" void doodle_canvas_clear(DoodleCanvas *canvas, DoodleColor color) { if (canvas && canvas->native) canvas->native->clear(skia_color(color)); }
extern "C" void doodle_canvas_draw_rect(DoodleCanvas *canvas, DoodleRect rect, DoodlePaint paint) { if (canvas && canvas->native) canvas->native->drawRect(SkRect::MakeXYWH(rect.x, rect.y, rect.width, rect.height), skia_paint(paint)); }
extern "C" void doodle_canvas_draw_round_rect(DoodleCanvas *canvas, DoodleRect rect, float radius, DoodlePaint paint) { if (canvas && canvas->native) canvas->native->drawRoundRect(SkRect::MakeXYWH(rect.x, rect.y, rect.width, rect.height), radius, radius, skia_paint(paint)); }
extern "C" void doodle_canvas_draw_line(DoodleCanvas *canvas, DoodlePoint a, DoodlePoint b, DoodlePaint paint) { if (canvas && canvas->native) canvas->native->drawLine(a.x, a.y, b.x, b.y, skia_paint(paint)); }
extern "C" void doodle_canvas_draw_circle(DoodleCanvas *canvas, DoodlePoint point, float radius, DoodlePaint paint) { if (canvas && canvas->native) canvas->native->drawCircle(point.x, point.y, radius, skia_paint(paint)); }
extern "C" void doodle_canvas_draw_text(DoodleCanvas *canvas, const char *text, float x, float y, DoodleTextStyle style) { if (canvas && canvas->native && text) { SkFont font(nullptr, style.size); SkPaint paint; paint.setColor4f(skia_color(style.color), nullptr); paint.setAntiAlias(true); canvas->native->drawString(text, x, y, font, paint); } }
