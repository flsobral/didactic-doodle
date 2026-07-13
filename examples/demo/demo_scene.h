/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_DEMO_SCENE_H
#define TC_DEMO_SCENE_H
#include "tc_runtime/tc_app.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct DemoScene { int width, height; float pointer_x, pointer_y; int pointer_down; double elapsed; } DemoScene;
void demo_scene_init(DemoScene* scene, int width, int height);
void demo_scene_on_event(void* user_data, const TcEvent* event);
void demo_scene_on_update(void* user_data, double delta_seconds);
void demo_scene_on_draw(void* user_data, TcCanvas2D* canvas);
#ifdef __cplusplus
}
#endif
#endif
