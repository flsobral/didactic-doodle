/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "../../src/magic_context_private.h"
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct MagicWebBackend { BoardSurfaceWebInterface surface; EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context; } MagicWebBackend;

static MagicResult magic_web_board_result(BoardResult result) { return result == BOARD_OK ? MAGIC_OK : result == BOARD_ERROR_VERSION ? MAGIC_ERROR_VERSION : result == BOARD_ERROR_UNAVAILABLE ? MAGIC_ERROR_UNAVAILABLE : MAGIC_ERROR_SURFACE; }

MagicResult magic_web_backend_create(MagicContext *context, BoardNativeSurface *surface) {
    MagicWebBackend *backend;
    BoardResult result;
    EmscriptenWebGLContextAttributes attributes;
    if (!context || !surface) return MAGIC_ERROR_INVALID_ARGUMENT;
    backend = calloc(1, sizeof(*backend)); if (!backend) return MAGIC_ERROR_OUT_OF_MEMORY;
    result = board_surface_query_interface(surface, BOARD_SURFACE_INTERFACE_WEB, BOARD_ABI_VERSION, &backend->surface, sizeof(backend->surface));
    if (result != BOARD_OK) { free(backend); return magic_web_board_result(result); }
    emscripten_webgl_init_context_attributes(&attributes);
    attributes.alpha = EM_TRUE; attributes.antialias = EM_TRUE; attributes.explicitSwapControl = EM_FALSE; attributes.majorVersion = 2;
    backend->context = emscripten_webgl_create_context(backend->surface.canvas_selector, &attributes);
    if (backend->context <= 0) { free(backend); return MAGIC_ERROR_SURFACE; }
    if (emscripten_webgl_make_context_current(backend->context) != EMSCRIPTEN_RESULT_SUCCESS) { emscripten_webgl_destroy_context(backend->context); free(backend); return MAGIC_ERROR_SURFACE; }
    context->backend_data = backend;
    return MAGIC_OK;
}

void magic_web_backend_destroy(MagicContext *context) {
    MagicWebBackend *backend = context ? context->backend_data : NULL;
    if (backend) { emscripten_webgl_destroy_context(backend->context); free(backend); context->backend_data = NULL; }
}

MagicResult magic_web_backend_resize(MagicContext *context, uint32_t width, uint32_t height, float scale) { (void)width; (void)height; (void)scale; return context && context->backend_data ? MAGIC_OK : MAGIC_ERROR_INVALID_ARGUMENT; }

MagicResult magic_web_backend_begin_frame(MagicContext *context, MagicFrame *frame) {
    MagicWebBackend *backend;
    GLint framebuffer = 0;
    int width = 0, height = 0;
    if (!context || !frame || !context->backend_data) return MAGIC_ERROR_INVALID_ARGUMENT;
    backend = context->backend_data;
    if (emscripten_webgl_make_context_current(backend->context) != EMSCRIPTEN_RESULT_SUCCESS) return MAGIC_ERROR_SURFACE;
    emscripten_webgl_get_drawing_buffer_size(backend->context, &width, &height);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);
    glViewport(0, 0, width, height);
    frame->web = (MagicWebInterop){ sizeof(MagicWebInterop), MAGIC_ABI_VERSION, (void *)(intptr_t)backend->context, (uint32_t)framebuffer, (uint32_t)width, (uint32_t)height, (float)emscripten_get_device_pixel_ratio() };
    return MAGIC_OK;
}

MagicResult magic_web_backend_end_frame(MagicContext *context, MagicFrame *frame) { (void)frame; return context && context->backend_data ? MAGIC_OK : MAGIC_ERROR_INVALID_ARGUMENT; }
