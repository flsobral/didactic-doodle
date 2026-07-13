/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#include "tc_android_backend.h"
#include "demo_scene.h"
#include <android_native_app_glue.h>

typedef struct DemoAndroid { DemoScene scene; TcGraphicsContext* graphics; TcRenderer2D* renderer; } DemoAndroid;
static void on_event(void* data, const TcEvent* event) { DemoAndroid* demo = data; demo_scene_on_event(&demo->scene, event); if (event->type == TC_EVENT_RESIZE) { tc_renderer_resize(demo->renderer, event->data.resize.width, event->data.resize.height, event->data.resize.scale); } }
static void on_frame(void* data, double time, double delta) { (void)time; DemoAndroid* demo = data; demo_scene_on_update(&demo->scene, delta); TcCanvas2D* canvas = tc_renderer_begin_frame(demo->renderer); demo_scene_on_draw(&demo->scene, canvas); tc_renderer_end_frame(demo->renderer); }
void android_main(struct android_app* app) {
    TcAndroidNativeBackend* backend = NULL; DemoAndroid demo = {0};
    if (tc_android_backend_attach(app, on_event, &demo, &backend) != TC_OK) return;
    while (!app->window && !app->destroyRequested) { int events; struct android_poll_source* source; if (ALooper_pollOnce(-1, NULL, &events, (void**)&source) >= 0 && source) source->process(app, source); }
    if (app->destroyRequested || tc_android_cpu_context_create(app->window, &demo.graphics) != TC_OK || tc_renderer_create(TC_RENDERER_SKIA, demo.graphics, &demo.renderer) != TC_OK) goto cleanup;
    demo_scene_init(&demo.scene, demo.graphics->width, demo.graphics->height); tc_android_backend_start(backend, on_frame, &demo);
    while (!app->destroyRequested) { int events; struct android_poll_source* source; while (ALooper_pollOnce(0, NULL, &events, (void**)&source) >= 0 && source) source->process(app, source); }
cleanup:
    tc_android_backend_stop(backend); tc_renderer_destroy(demo.renderer); tc_android_cpu_destroy(demo.graphics); tc_android_backend_detach(backend);
}
