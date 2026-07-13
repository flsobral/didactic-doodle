/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#if defined(__ANDROID__)
void tc_android_cpu_present(TcGraphicsContext* context);
void tc_android_cpu_destroy(TcGraphicsContext* context);
int tc_graphics_context_create(TcGraphicsApi api, TcNativeWindowHandle* window, TcNativeSurfaceHandle* surface, TcGraphicsContext** out_context) { (void)surface; return api == TC_GRAPHICS_CPU && window ? tc_android_cpu_context_create(window->value, out_context) : TC_ERROR_UNAVAILABLE; }
void tc_graphics_context_resize(TcGraphicsContext* context, int width, int height, float scale) { (void)context; (void)width; (void)height; (void)scale; }
void tc_graphics_context_present(TcGraphicsContext* context) { tc_android_cpu_present(context); }
void tc_graphics_context_destroy(TcGraphicsContext* context) { tc_android_cpu_destroy(context); }
#endif
