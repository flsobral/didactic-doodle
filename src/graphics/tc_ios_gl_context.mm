/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#import <QuartzCore/CAEAGLLayer.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES3/gl.h>

typedef struct TcIosGlContext { EAGLContext* eagl; GLuint framebuffer; GLuint colorbuffer; } TcIosGlContext;

static int tc_ios_gl_rebuild_drawable(TcGraphicsContext* context, int width, int height, float scale) {
    TcIosGlContext* gl = (TcIosGlContext*)context->surface;
    UIView* view = (__bridge UIView*)context->window; CAEAGLLayer* layer = (CAEAGLLayer*)view.layer;
    if (![EAGLContext setCurrentContext:gl->eagl]) return TC_ERROR_PLATFORM;
    layer.contentsScale = scale;
    if (gl->framebuffer) glDeleteFramebuffers(1, &gl->framebuffer);
    if (gl->colorbuffer) glDeleteRenderbuffers(1, &gl->colorbuffer);
    glGenFramebuffers(1, &gl->framebuffer); glBindFramebuffer(GL_FRAMEBUFFER, gl->framebuffer);
    glGenRenderbuffers(1, &gl->colorbuffer); glBindRenderbuffer(GL_RENDERBUFFER, gl->colorbuffer);
    if (![gl->eagl renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer]) return TC_ERROR_PLATFORM;
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, gl->colorbuffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return TC_ERROR_PLATFORM;
    GLint drawable_width = 0, drawable_height = 0;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &drawable_width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &drawable_height);
    context->width = drawable_width > 0 ? drawable_width : width; context->height = drawable_height > 0 ? drawable_height : height;
    context->framebuffer = (int)gl->framebuffer; context->stencil_bits = 0; context->sample_count = 0; context->scale = scale;
    glViewport(0, 0, context->width, context->height);
    return TC_OK;
}

int tc_ios_gl_context_create(void* view, int width, int height, float scale, TcGraphicsContext** out_context) {
    if (!view || !out_context || width <= 0 || height <= 0) return TC_ERROR_INVALID_ARGUMENT;
    TcGraphicsContext* context = (TcGraphicsContext*)calloc(1, sizeof(*context)); TcIosGlContext* gl = (TcIosGlContext*)calloc(1, sizeof(*gl));
    if (!context || !gl) { free(context); free(gl); return TC_ERROR_OUT_OF_MEMORY; }
    gl->eagl = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (!gl->eagl) { free(gl); free(context); return TC_ERROR_UNAVAILABLE; }
    context->api = TC_GRAPHICS_OPENGL; context->window = view; context->surface = gl;
    if (tc_ios_gl_rebuild_drawable(context, width, height, scale) != TC_OK) { [gl->eagl release]; free(gl); free(context); return TC_ERROR_PLATFORM; }
    *out_context = context; return TC_OK;
}

int tc_ios_gl_context_resize(TcGraphicsContext* context, int width, int height, float scale) { return !context || context->api != TC_GRAPHICS_OPENGL ? TC_ERROR_INVALID_ARGUMENT : tc_ios_gl_rebuild_drawable(context, width, height, scale); }
void tc_ios_gl_context_present(TcGraphicsContext* context) { if (!context || context->api != TC_GRAPHICS_OPENGL) return; TcIosGlContext* gl = (TcIosGlContext*)context->surface; [EAGLContext setCurrentContext:gl->eagl]; glBindRenderbuffer(GL_RENDERBUFFER, gl->colorbuffer); [gl->eagl presentRenderbuffer:GL_RENDERBUFFER]; }
void tc_ios_gl_context_destroy(TcGraphicsContext* context) { if (!context) return; TcIosGlContext* gl = (TcIosGlContext*)context->surface; if (gl) { [EAGLContext setCurrentContext:gl->eagl]; if (gl->framebuffer) glDeleteFramebuffers(1, &gl->framebuffer); if (gl->colorbuffer) glDeleteRenderbuffers(1, &gl->colorbuffer); [EAGLContext setCurrentContext:nil]; [gl->eagl release]; free(gl); } free(context); }
