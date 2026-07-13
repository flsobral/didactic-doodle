/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_SKIA_RENDERER_C_API_H
#define TC_SKIA_RENDERER_C_API_H
#include "tc_runtime/tc_renderer.h"
#ifdef __cplusplus
extern "C" {
#endif
int tc_skia_renderer_create(TcGraphicsContext* context, TcRenderer2D** out_renderer);
#ifdef __cplusplus
}
#endif
#endif
