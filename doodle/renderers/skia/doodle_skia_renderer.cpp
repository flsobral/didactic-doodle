/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <doodle/doodle_renderer.h>
#include <magic/magic_interop.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#if defined(SK_VULKAN)
#include <vulkan/vulkan.h>
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#endif
#if defined(SK_METAL)
#include "include/gpu/mtl/GrMtlTypes.h"
#endif
#include <memory>
#include <new>
#include <unordered_map>

struct DoodleCanvas { SkCanvas *native; };
struct DoodleSkiaState { sk_sp<SkSurface> surface; sk_sp<GrDirectContext> opengl;
#if defined(SK_METAL)
  sk_sp<GrDirectContext> metal;
#endif
#if defined(SK_VULKAN)
  sk_sp<GrDirectContext> vulkan; GrVkExtensions vulkan_extensions; std::unordered_map<uint64_t, sk_sp<SkSurface>> vulkan_surfaces; uint64_t vulkan_generation = 0; MagicVulkanInterop vulkan_frame{};
#endif
  DoodleCanvas canvas; };
static SkColor4f skia_color(DoodleColor color) { return { color.r, color.g, color.b, color.a }; }
static SkPaint skia_paint(DoodlePaint paint) { SkPaint result; result.setColor4f(skia_color(paint.color), nullptr); result.setStyle(paint.style == DOODLE_PAINT_STROKE ? SkPaint::kStroke_Style : SkPaint::kFill_Style); result.setStrokeWidth(paint.stroke_width); result.setAntiAlias(true); return result; }
static DoodleResult skia_create(MagicContext *, const DoodleRendererConfig *, void **out_state) { DoodleSkiaState *state = new (std::nothrow) DoodleSkiaState{}; if (!state) return DOODLE_ERROR_OUT_OF_MEMORY; *out_state = state; return DOODLE_OK; }
static void skia_destroy(void *state) { delete static_cast<DoodleSkiaState *>(state); }
static sk_sp<SkSurface> skia_gl_surface(DoodleSkiaState *state, uint32_t framebuffer, uint32_t width, uint32_t height) {
  GrGLFramebufferInfo info;
  GrBackendRenderTarget target;
  if (!state->opengl) state->opengl = GrDirectContext::MakeGL(GrGLMakeNativeInterface());
  if (!state->opengl) return nullptr;
  state->opengl->resetContext();
  info = { (GrGLuint)framebuffer, 0x8058 };
  target = GrBackendRenderTarget((int)width, (int)height, 0, 0, info);
  return SkSurface::MakeFromBackendRenderTarget(state->opengl.get(), target, kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType, nullptr, nullptr);
}
#if defined(SK_VULKAN)
static PFN_vkVoidFunction skia_vk_get_proc(const char *name, VkInstance instance, VkDevice device) { return device != VK_NULL_HANDLE ? vkGetDeviceProcAddr(device, name) : vkGetInstanceProcAddr(instance, name); }
#endif
static DoodleResult skia_begin(void *value, MagicFrame *frame, DoodleCanvas **out_canvas) {
  MagicCpuInterop pixels{}; MagicOpenGLInterop opengl{}; MagicMetalInterop metal{}; MagicVulkanInterop vulkan{}; MagicWebInterop web{}; DoodleSkiaState *state = static_cast<DoodleSkiaState *>(value);
  if (!state) return DOODLE_ERROR_INVALID_ARGUMENT;
  if (magic_frame_query_interop(frame, MAGIC_INTEROP_CPU, MAGIC_ABI_VERSION, &pixels, sizeof(pixels)) == MAGIC_OK) {
    SkColorType format = pixels.format == MAGIC_PIXEL_FORMAT_BGRA8888 ? kBGRA_8888_SkColorType : kRGBA_8888_SkColorType;
    state->surface = SkSurface::MakeRasterDirect(SkImageInfo::Make((int)pixels.width, (int)pixels.height, format, kPremul_SkAlphaType), pixels.pixels, pixels.stride);
  } else if (magic_frame_query_interop(frame, MAGIC_INTEROP_OPENGL, MAGIC_ABI_VERSION, &opengl, sizeof(opengl)) == MAGIC_OK) {
    state->surface = skia_gl_surface(state, opengl.framebuffer, opengl.width, opengl.height);
  } else if (magic_frame_query_interop(frame, MAGIC_INTEROP_WEB, MAGIC_ABI_VERSION, &web, sizeof(web)) == MAGIC_OK) {
    state->surface = skia_gl_surface(state, web.framebuffer, web.width, web.height);
  } else if (magic_frame_query_interop(frame, MAGIC_INTEROP_METAL, MAGIC_ABI_VERSION, &metal, sizeof(metal)) == MAGIC_OK) {
#if defined(SK_METAL)
    if (!state->metal) state->metal = GrDirectContext::MakeMetal(metal.device, metal.command_queue);
    if (state->metal) state->surface = SkSurface::MakeFromCAMetalLayer(state->metal.get(), metal.layer, kTopLeft_GrSurfaceOrigin, 1, kBGRA_8888_SkColorType, nullptr, nullptr, metal.drawable_slot);
#else
    return DOODLE_ERROR_UNAVAILABLE;
#endif
  } else if (magic_frame_query_interop(frame, MAGIC_INTEROP_VULKAN, MAGIC_ABI_VERSION, &vulkan, sizeof(vulkan)) == MAGIC_OK) {
#if defined(SK_VULKAN)
    VkInstance instance = (VkInstance)(uintptr_t)vulkan.instance; VkPhysicalDevice physical_device = (VkPhysicalDevice)(uintptr_t)vulkan.physical_device; VkDevice device = (VkDevice)(uintptr_t)vulkan.device;
    if (!state->vulkan) {
      const char *instance_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME}; const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
      if (!vulkan.device_features) return DOODLE_ERROR_UNAVAILABLE;
      state->vulkan_extensions.init(skia_vk_get_proc, instance, physical_device, 2, instance_extensions, 1, device_extensions);
      GrVkBackendContext backend{}; backend.fInstance = instance; backend.fPhysicalDevice = physical_device; backend.fDevice = device; backend.fQueue = (VkQueue)(uintptr_t)vulkan.queue; backend.fGraphicsQueueIndex = vulkan.queue_family; backend.fMaxAPIVersion = VK_API_VERSION_1_1; backend.fVkExtensions = &state->vulkan_extensions; backend.fDeviceFeatures2 = static_cast<VkPhysicalDeviceFeatures2 *>(vulkan.device_features); backend.fGetProc = skia_vk_get_proc;
      state->vulkan = GrDirectContext::MakeVulkan(backend);
    }
    if (state->vulkan) {
      if (state->vulkan_generation != vulkan.surface_generation) { state->vulkan_surfaces.clear(); state->vulkan_generation = vulkan.surface_generation; }
      auto found = state->vulkan_surfaces.find(vulkan.image);
      if (found == state->vulkan_surfaces.end()) { GrVkImageInfo image{}; image.fImage = (VkImage)vulkan.image; image.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED; image.fImageTiling = VK_IMAGE_TILING_OPTIMAL; image.fFormat = (VkFormat)vulkan.image_format; image.fImageUsageFlags = vulkan.image_usage; image.fSampleCount = 1; image.fLevelCount = 1; image.fCurrentQueueFamily = vulkan.queue_family;
        GrBackendRenderTarget target((int)vulkan.width, (int)vulkan.height, image); SkColorType color_type = (vulkan.image_format == VK_FORMAT_B8G8R8A8_UNORM || vulkan.image_format == VK_FORMAT_B8G8R8A8_SRGB) ? kBGRA_8888_SkColorType : kRGBA_8888_SkColorType;
        found = state->vulkan_surfaces.emplace(vulkan.image, SkSurface::MakeFromBackendRenderTarget(state->vulkan.get(), target, kTopLeft_GrSurfaceOrigin, color_type, nullptr, nullptr)).first; }
      state->surface = found->second;
      if (state->surface) { GrBackendSemaphore wait; wait.initVulkan((VkSemaphore)vulkan.acquire_semaphore); if (!state->surface->wait(1, &wait)) state->surface.reset(); }
      state->vulkan_frame = vulkan;
    }
#else
    return DOODLE_ERROR_UNAVAILABLE;
#endif
  } else return DOODLE_ERROR_UNAVAILABLE;
  if (!state->surface) return DOODLE_ERROR_RENDERER;
  state->canvas.native = state->surface->getCanvas(); *out_canvas = &state->canvas; return DOODLE_OK;
}
static DoodleResult skia_end(void *value, DoodleCanvas *canvas) { DoodleSkiaState *state = static_cast<DoodleSkiaState *>(value); if (!state || canvas != &state->canvas || !state->surface) return DOODLE_ERROR_INVALID_ARGUMENT;
#if defined(SK_VULKAN)
  if (state->vulkan_frame.render_complete_semaphore) { GrBackendSemaphore signal; GrFlushInfo info{}; signal.initVulkan((VkSemaphore)state->vulkan_frame.render_complete_semaphore); info.fNumSemaphores = 1; info.fSignalSemaphores = &signal; state->surface->flush(SkSurface::BackendSurfaceAccess::kPresent, info); if (!state->vulkan || !state->vulkan->submit()) return DOODLE_ERROR_RENDERER; state->vulkan_frame = {}; }
  else
#endif
  state->surface->flushAndSubmit(); state->canvas.native = nullptr; state->surface.reset(); return DOODLE_OK; }
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
