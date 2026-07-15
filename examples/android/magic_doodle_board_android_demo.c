/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <board/board_android.h>
#include <board/board_app.h>
#include <doodle/doodle_renderer.h>
#include <magic/magic_context.h>
#include "../common/magic_doodle_board_scene.h"
#include <android/native_window_jni.h>
#include <jni.h>
#include <stdlib.h>

typedef struct AndroidDemo {
    BoardBackend *backend;
    BoardApp *app;
    MagicContext *magic;
    DoodleRenderer *renderer;
    BoardNativeViewSlot *overlay_slot;
    MagicDoodleBoardScene scene;
} AndroidDemo;

static void android_demo_event(void *data, const BoardEvent *event) {
    AndroidDemo *demo = (AndroidDemo *)data;
    magic_doodle_board_scene_event(&demo->scene, event);
    if (event->type == BOARD_EVENT_RESIZE) (void)magic_context_resize(demo->magic, event->data.resize.width, event->data.resize.height, event->data.resize.scale);
}
static void android_demo_update(void *data, double delta_seconds) { magic_doodle_board_scene_update(&((AndroidDemo *)data)->scene, delta_seconds); }
static void android_demo_frame(void *data, uint64_t timestamp_ns, double delta_seconds) {
    AndroidDemo *demo = (AndroidDemo *)data;
    MagicFrame *frame = NULL;
    DoodleCanvas *canvas = NULL;
    (void)timestamp_ns;
    if (magic_context_begin_frame(demo->magic, &frame) != MAGIC_OK) return;
    if (doodle_renderer_begin_frame(demo->renderer, frame, &canvas) == DOODLE_OK) {
        magic_doodle_board_scene_draw(&demo->scene, canvas);
        doodle_renderer_end_frame(demo->renderer, canvas);
    }
    (void)magic_context_end_frame(demo->magic, frame);
    (void)delta_seconds;
}

static void android_demo_destroy(AndroidDemo *demo) {
    if (!demo) return;
    board_app_request_quit(demo->app);
    board_native_view_slot_destroy(demo->overlay_slot);
    board_app_destroy(demo->app);
    doodle_renderer_destroy(demo->renderer);
    magic_context_destroy(demo->magic);
    board_backend_destroy(demo->backend);
    free(demo);
}

JNIEXPORT jlong JNICALL Java_org_magicdoodle_board_BoardView_nativeCreate(JNIEnv *environment, jclass type, jobject host_view, jobject surface, jint width, jint height) {
    AndroidDemo *demo = (AndroidDemo *)calloc(1, sizeof(*demo));
    ANativeWindow *window = NULL;
    BoardBackendConfig backend_config = {sizeof(BoardBackendConfig), BOARD_ABI_VERSION, BOARD_BACKEND_ANDROID, "Magic Doodle Board", (uint32_t)width, (uint32_t)height, 1.0f, 0, BOARD_HOST_MODE_EMBEDDED};
#if MDB_ANDROID_OPENGL
    MagicConfig magic_config = {sizeof(MagicConfig), MAGIC_ABI_VERSION, MAGIC_BACKEND_OPENGL, 1};
#elif MDB_ANDROID_VULKAN
    MagicConfig magic_config = {sizeof(MagicConfig), MAGIC_ABI_VERSION, MAGIC_BACKEND_VULKAN, 1};
#else
    MagicConfig magic_config = {sizeof(MagicConfig), MAGIC_ABI_VERSION, MAGIC_BACKEND_CPU, 1};
#endif
    DoodleRendererConfig renderer_config = {sizeof(DoodleRendererConfig), DOODLE_ABI_VERSION, 0};
    BoardAppCallbacks callbacks = {sizeof(BoardAppCallbacks), BOARD_ABI_VERSION, NULL, android_demo_event, android_demo_update, android_demo_frame, NULL};
    BoardAppConfig app_config;
    BoardSurfaceCpuInterface pixels;
    void *memory;
    uint32_t scene_width, scene_height, stride;
    BoardPixelFormat format;
    float scale;
    (void)type;
    if (!demo || !surface || width <= 0 || height <= 0) goto failure;
    window = ANativeWindow_fromSurface(environment, surface);
    if (!window || board_backend_create(&backend_config, &demo->backend) != BOARD_OK || board_android_attach_window(demo->backend, window) != BOARD_OK || board_android_set_host_view(demo->backend, environment, host_view) != BOARD_OK) goto failure;
    window = NULL;
    if (magic_context_create(board_backend_surface(demo->backend), &magic_config, &demo->magic) != MAGIC_OK || doodle_renderer_create(doodle_skia_provider(), demo->magic, &renderer_config, &demo->renderer) != DOODLE_OK) goto failure;
    if (board_surface_query_interface(board_backend_surface(demo->backend), BOARD_SURFACE_INTERFACE_CPU, BOARD_ABI_VERSION, &pixels, sizeof(pixels)) != BOARD_OK || pixels.map_pixels(pixels.user_data, &memory, &scene_width, &scene_height, &stride, &format, &scale) != BOARD_OK) goto failure;
    (void)memory; (void)stride; (void)format; (void)scale;
    magic_doodle_board_scene_init(&demo->scene, (float)scene_width, (float)scene_height);
    app_config = (BoardAppConfig){sizeof(BoardAppConfig), BOARD_ABI_VERSION, demo->backend, callbacks, demo};
    if (board_app_create(&app_config, &demo->app) != BOARD_OK) goto failure;
    return (jlong)(uintptr_t)demo;
failure:
    if (window) ANativeWindow_release(window);
    android_demo_destroy(demo);
    return 0;
}

JNIEXPORT void JNICALL Java_org_magicdoodle_board_BoardView_nativeStart(JNIEnv *environment, jclass type, jlong handle) {
    AndroidDemo *demo = (AndroidDemo *)(uintptr_t)handle;
    (void)environment; (void)type;
    if (demo && demo->app) (void)board_app_start(demo->app);
}

JNIEXPORT void JNICALL Java_org_magicdoodle_board_BoardView_nativeResize(JNIEnv *environment, jclass type, jlong handle) {
    AndroidDemo *demo = (AndroidDemo *)(uintptr_t)handle;
    (void)environment; (void)type;
    if (demo) (void)board_android_resize_window(demo->backend);
}

JNIEXPORT void JNICALL Java_org_magicdoodle_board_BoardView_nativeAttachOverlay(JNIEnv *environment, jclass type, jlong handle, jobject overlay, jfloat x, jfloat y, jfloat width, jfloat height) {
    AndroidDemo *demo = (AndroidDemo *)(uintptr_t)handle;
    jobject reference;
    BoardNativeViewSlotConfig config;
    (void)type;
    if (!demo || !overlay) return;
    board_native_view_slot_destroy(demo->overlay_slot);
    demo->overlay_slot = NULL;
    reference = (*environment)->NewGlobalRef(environment, overlay);
    if (!reference) return;
    config = (BoardNativeViewSlotConfig){sizeof(BoardNativeViewSlotConfig), BOARD_ABI_VERSION, reference, {x, y, width, height}, {0, 0, width + x, height + y}, 1, 0, BOARD_NATIVE_VIEW_ABOVE_RENDERER};
    if (board_native_view_slot_create(demo->backend, &config, &demo->overlay_slot) != BOARD_OK) (*environment)->DeleteGlobalRef(environment, reference);
}

JNIEXPORT void JNICALL Java_org_magicdoodle_board_BoardView_nativeDestroy(JNIEnv *environment, jclass type, jlong handle) {
    (void)environment; (void)type;
    android_demo_destroy((AndroidDemo *)(uintptr_t)handle);
}
