/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_RUNTIME_GRAPHICS_H
#define TC_RUNTIME_GRAPHICS_H
#include "tc_types.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum TcGraphicsApi { TC_GRAPHICS_CPU = 0, TC_GRAPHICS_OPENGL, TC_GRAPHICS_METAL, TC_GRAPHICS_VULKAN } TcGraphicsApi;
int tc_graphics_context_create(TcGraphicsApi api, TcNativeWindowHandle* window, TcNativeSurfaceHandle* surface, TcGraphicsContext** out_context);
void tc_graphics_context_resize(TcGraphicsContext* context, int width, int height, float scale);
void tc_graphics_context_present(TcGraphicsContext* context);
void tc_graphics_context_destroy(TcGraphicsContext* context);
#ifdef __cplusplus
}
#endif
#endif
