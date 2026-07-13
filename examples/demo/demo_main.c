/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "demo_scene.h"
#include "tc_runtime/tc_graphics.h"
#include <stdio.h>

int main(void) {
    TcPlatformBackend* backend = NULL; TcGraphicsContext* graphics = NULL; TcRenderer2D* renderer = NULL; TcApp* app = NULL;
    TcBackendConfig backend_config = {"tc_demo", 960, 640, 1};
    DemoScene scene; demo_scene_init(&scene, backend_config.width, backend_config.height);
#if TC_DEMO_GRAPHICS_OPENGL
    const TcGraphicsApi graphics_api = TC_GRAPHICS_OPENGL; const char* graphics_name = "opengl";
#else
    const TcGraphicsApi graphics_api = TC_GRAPHICS_CPU; const char* graphics_name = "cpu";
#endif
    printf("tc_demo: backend=sdl\ntc_demo: renderer=skia\ntc_demo: graphics=%s\n", graphics_name);
    if (tc_backend_create(&backend_config, &backend) || tc_graphics_context_create(graphics_api, tc_backend_get_native_window(backend), tc_backend_get_native_surface(backend), &graphics) || tc_renderer_create(TC_RENDERER_SKIA, graphics, &renderer)) {
        fprintf(stderr, "tc_demo: initialization failed\n"); goto cleanup;
    }
    TcAppConfig config = { { demo_scene_on_event, demo_scene_on_update, demo_scene_on_draw, NULL }, &scene, backend, renderer };
    if (tc_app_create(&config, &app) == TC_OK) { printf("tc_demo: window created\ntc_demo: running\n"); tc_app_run(app); printf("tc_demo: shutdown\n"); }
cleanup:
    tc_app_destroy(app); tc_renderer_destroy(renderer); tc_graphics_context_destroy(graphics); tc_backend_destroy(backend);
    return app ? 0 : 1;
}
