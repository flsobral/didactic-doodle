/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "../../src/magic_context_private.h"
#if defined(MAGIC_BUILD_IOS_OPENGL)
#include <OpenGLES/ES3/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif
#include <stdlib.h>

typedef struct MagicOpenGLBackend { BoardSurfaceOpenGLInterface surface; void *context; } MagicOpenGLBackend;
static MagicResult magic_opengl_board_result(BoardResult result) { return result == BOARD_OK ? MAGIC_OK : result == BOARD_ERROR_VERSION ? MAGIC_ERROR_VERSION : result == BOARD_ERROR_UNAVAILABLE ? MAGIC_ERROR_UNAVAILABLE : MAGIC_ERROR_SURFACE; }
MagicResult magic_opengl_backend_create(MagicContext *context, BoardNativeSurface *surface) { MagicOpenGLBackend *backend; BoardResult result; if (!context || !surface) return MAGIC_ERROR_INVALID_ARGUMENT; backend = calloc(1, sizeof(*backend)); if (!backend) return MAGIC_ERROR_OUT_OF_MEMORY; result = board_surface_query_interface(surface, BOARD_SURFACE_INTERFACE_OPENGL, BOARD_ABI_VERSION, &backend->surface, sizeof(backend->surface)); if (result != BOARD_OK) { free(backend); return magic_opengl_board_result(result); } result = backend->surface.create_context(backend->surface.user_data, &backend->context); if (result != BOARD_OK) { free(backend); return magic_opengl_board_result(result); } context->backend_data = backend; return MAGIC_OK; }
void magic_opengl_backend_destroy(MagicContext *context) { MagicOpenGLBackend *backend = context ? context->backend_data : NULL; if (backend) { backend->surface.destroy_context(backend->surface.user_data, backend->context); free(backend); context->backend_data = NULL; } }
MagicResult magic_opengl_backend_resize(MagicContext *context, uint32_t width, uint32_t height, float scale) { (void)width; (void)height; (void)scale; return context && context->backend_data ? MAGIC_OK : MAGIC_ERROR_INVALID_ARGUMENT; }
MagicResult magic_opengl_backend_begin_frame(MagicContext *context, MagicFrame *frame) { MagicOpenGLBackend *backend; uint32_t width, height; float scale; GLint framebuffer = 0; BoardResult result; if (!context || !frame || !context->backend_data) return MAGIC_ERROR_INVALID_ARGUMENT; backend = context->backend_data; result = backend->surface.make_current(backend->surface.user_data, backend->context); if (result != BOARD_OK) return magic_opengl_board_result(result); result = backend->surface.drawable_size(backend->surface.user_data, &width, &height, &scale); if (result != BOARD_OK) return magic_opengl_board_result(result); glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer); glViewport(0, 0, (GLsizei)width, (GLsizei)height); frame->opengl = (MagicOpenGLInterop){ sizeof(MagicOpenGLInterop), MAGIC_ABI_VERSION, backend->context, (uint32_t)framebuffer, width, height, scale }; return MAGIC_OK; }
MagicResult magic_opengl_backend_end_frame(MagicContext *context, MagicFrame *frame) { MagicOpenGLBackend *backend; (void)frame; if (!context || !context->backend_data) return MAGIC_ERROR_INVALID_ARGUMENT; backend = context->backend_data; return magic_opengl_board_result(backend->surface.swap_buffers(backend->surface.user_data)); }
