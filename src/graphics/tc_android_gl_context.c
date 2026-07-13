/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#if defined(__ANDROID__)
#include <EGL/egl.h>
#include <android/native_window.h>
#include <android/log.h>
#include <stdlib.h>

typedef struct TcAndroidGlContext { EGLDisplay display; EGLSurface surface; EGLContext context; EGLConfig config; } TcAndroidGlContext;

static int tc_android_gl_create_surface(TcGraphicsContext* context) {
    TcAndroidGlContext* gl = context->surface;
    EGLint attributes[] = {EGL_NONE};
    gl->surface = eglCreateWindowSurface(gl->display, gl->config, (ANativeWindow*)context->window, attributes);
    if (gl->surface == EGL_NO_SURFACE || !eglMakeCurrent(gl->display, gl->surface, gl->surface, gl->context)) return TC_ERROR_PLATFORM;
    EGLint width = 0, height = 0;
    if (!eglQuerySurface(gl->display, gl->surface, EGL_WIDTH, &width) || !eglQuerySurface(gl->display, gl->surface, EGL_HEIGHT, &height)) return TC_ERROR_PLATFORM;
    context->width = width; context->height = height; context->framebuffer = 0; context->stencil_bits = 0; context->sample_count = 0; context->scale = 1.0f;
    return TC_OK;
}

int tc_android_gl_context_create(void* native_window, TcGraphicsContext** out_context) {
    if (!native_window || !out_context) return TC_ERROR_INVALID_ARGUMENT;
    TcGraphicsContext* context = calloc(1, sizeof(*context)); TcAndroidGlContext* gl = calloc(1, sizeof(*gl));
    if (!context || !gl) { free(context); free(gl); return TC_ERROR_OUT_OF_MEMORY; }
    gl->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint major = 0, minor = 0, count = 0;
    EGLint config_attributes[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE};
    EGLint context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    if (gl->display == EGL_NO_DISPLAY || !eglInitialize(gl->display, &major, &minor) || !eglChooseConfig(gl->display, config_attributes, &gl->config, 1, &count) || count != 1) goto failure;
    gl->context = eglCreateContext(gl->display, gl->config, EGL_NO_CONTEXT, context_attributes);
    if (gl->context == EGL_NO_CONTEXT) goto failure;
    context->api = TC_GRAPHICS_OPENGL; context->window = native_window; context->surface = gl;
    if (tc_android_gl_create_surface(context) != TC_OK) goto failure;
    *out_context = context;
    __android_log_print(ANDROID_LOG_INFO, "tc_runtime", "EGL %d.%d OpenGL ES 3 context created", major, minor);
    return TC_OK;
failure:
    if (gl->display != EGL_NO_DISPLAY) { if (gl->context != EGL_NO_CONTEXT) eglDestroyContext(gl->display, gl->context); eglTerminate(gl->display); }
    free(gl); free(context); return TC_ERROR_PLATFORM;
}

int tc_android_gl_context_resize(TcGraphicsContext* context, int width, int height, float scale) {
    (void)width; (void)height; (void)scale;
    if (!context || context->api != TC_GRAPHICS_OPENGL) return TC_ERROR_INVALID_ARGUMENT;
    TcAndroidGlContext* gl = context->surface;
    if (gl->surface != EGL_NO_SURFACE) { eglMakeCurrent(gl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT); eglDestroySurface(gl->display, gl->surface); gl->surface = EGL_NO_SURFACE; }
    return tc_android_gl_create_surface(context);
}

void tc_android_gl_context_present(TcGraphicsContext* context) {
    if (!context || context->api != TC_GRAPHICS_OPENGL) return;
    TcAndroidGlContext* gl = context->surface;
    if (!eglSwapBuffers(gl->display, gl->surface)) __android_log_print(ANDROID_LOG_ERROR, "tc_runtime", "eglSwapBuffers failed: 0x%x", eglGetError());
}

void tc_android_gl_context_destroy(TcGraphicsContext* context) {
    if (!context) return;
    TcAndroidGlContext* gl = context->surface;
    if (gl) { if (gl->display != EGL_NO_DISPLAY) { eglMakeCurrent(gl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT); if (gl->surface != EGL_NO_SURFACE) eglDestroySurface(gl->display, gl->surface); if (gl->context != EGL_NO_CONTEXT) eglDestroyContext(gl->display, gl->context); eglTerminate(gl->display); } free(gl); }
    free(context);
}
#endif
