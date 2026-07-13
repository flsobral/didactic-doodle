/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#if defined(__ANDROID__)
void tc_android_cpu_present(TcGraphicsContext* context);
void tc_android_cpu_destroy(TcGraphicsContext* context);
#if TC_ANDROID_GRAPHICS_OPENGL
int tc_android_gl_context_create(void* native_window, TcGraphicsContext** out_context);
int tc_android_gl_context_resize(TcGraphicsContext* context, int width, int height, float scale);
void tc_android_gl_context_present(TcGraphicsContext* context);
void tc_android_gl_context_destroy(TcGraphicsContext* context);
#endif
int tc_graphics_context_create(TcGraphicsApi api, TcNativeWindowHandle* window, TcNativeSurfaceHandle* surface, TcGraphicsContext** out_context) {
    (void)surface;
    if (!window) return TC_ERROR_INVALID_ARGUMENT;
    if (api == TC_GRAPHICS_CPU) return tc_android_cpu_context_create(window->value, out_context);
#if TC_ANDROID_GRAPHICS_OPENGL
    if (api == TC_GRAPHICS_OPENGL) return tc_android_gl_context_create(window->value, out_context);
#endif
    return TC_ERROR_UNAVAILABLE;
}
void tc_graphics_context_resize(TcGraphicsContext* context, int width, int height, float scale) {
    if (!context) return;
    if (context->api == TC_GRAPHICS_CPU) tc_android_cpu_context_resize(context, width, height, scale);
#if TC_ANDROID_GRAPHICS_OPENGL
    else if (context->api == TC_GRAPHICS_OPENGL) tc_android_gl_context_resize(context, width, height, scale);
#endif
}
void tc_graphics_context_present(TcGraphicsContext* context) {
    if (!context) return;
    if (context->api == TC_GRAPHICS_CPU) tc_android_cpu_present(context);
#if TC_ANDROID_GRAPHICS_OPENGL
    else if (context->api == TC_GRAPHICS_OPENGL) tc_android_gl_context_present(context);
#endif
}
void tc_graphics_context_destroy(TcGraphicsContext* context) {
    if (!context) return;
    if (context->api == TC_GRAPHICS_CPU) tc_android_cpu_destroy(context);
#if TC_ANDROID_GRAPHICS_OPENGL
    else if (context->api == TC_GRAPHICS_OPENGL) tc_android_gl_context_destroy(context);
#endif
}
#endif
