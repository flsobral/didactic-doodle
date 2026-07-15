/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "../../src/board_internal.h"
#include <board/board_android.h>
#include <android/choreographer.h>
#include <android/input.h>
#include <android/native_window.h>
#include <jni.h>
#if BOARD_BUILD_ANDROID_OPENGL
#include <EGL/egl.h>
#endif
#if BOARD_BUILD_ANDROID_VULKAN
#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h>
#endif
#include <android_native_app_glue.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct BoardAndroidState {
    BoardBackend *backend;
    struct android_app *app;
    ANativeWindow *window;
    uint8_t owns_window;
    JavaVM *java_vm;
    jobject host_view;
#if BOARD_BUILD_ANDROID_OPENGL
    void *active_gl;
#endif
} BoardAndroidState;

static JNIEnv *board_android_jni(BoardAndroidState *state, int *detach) {
    JNIEnv *environment = NULL;
    if (!state || !state->java_vm || !detach) return NULL;
    *detach = 0;
    if ((*state->java_vm)->GetEnv(state->java_vm, (void **)&environment, JNI_VERSION_1_6) != JNI_OK) {
        if ((*state->java_vm)->AttachCurrentThread(state->java_vm, &environment, NULL) != JNI_OK) return NULL;
        *detach = 1;
    }
    return environment;
}

static BoardResult board_android_slot_apply(BoardNativeViewSlot *slot) {
    BoardAndroidState *state = slot && slot->backend ? (BoardAndroidState *)slot->backend->implementation : NULL;
    JNIEnv *environment;
    jclass host_class;
    jmethodID update;
    int detach;
    if (!state || !state->host_view || !slot->implementation || slot->z_order != BOARD_NATIVE_VIEW_ABOVE_RENDERER) return BOARD_ERROR_UNAVAILABLE;
    environment = board_android_jni(state, &detach); if (!environment) return BOARD_ERROR_PLATFORM;
    host_class = (*environment)->GetObjectClass(environment, state->host_view);
    update = host_class ? (*environment)->GetMethodID(environment, host_class, "updateNativeOverlay", "(Landroid/view/View;FFFFFFFFZZ)V") : NULL;
    if (!update) { if (detach) (*state->java_vm)->DetachCurrentThread(state->java_vm); return BOARD_ERROR_PLATFORM; }
    (*environment)->CallVoidMethod(environment, state->host_view, update, (jobject)slot->implementation, slot->frame.x, slot->frame.y, slot->frame.width, slot->frame.height, slot->clip.x, slot->clip.y, slot->clip.width, slot->clip.height, slot->clip_enabled ? JNI_TRUE : JNI_FALSE, slot->visible ? JNI_TRUE : JNI_FALSE);
    if ((*environment)->ExceptionCheck(environment)) { (*environment)->ExceptionClear(environment); if (detach) (*state->java_vm)->DetachCurrentThread(state->java_vm); return BOARD_ERROR_PLATFORM; }
    if (detach) (*state->java_vm)->DetachCurrentThread(state->java_vm);
    return BOARD_OK;
}

static BoardResult board_android_slot_create(BoardBackend *backend, const BoardNativeViewSlotConfig *config, BoardNativeViewSlot *slot) {
    BoardAndroidState *state = backend ? (BoardAndroidState *)backend->implementation : NULL;
    JNIEnv *environment;
    jclass host_class;
    jmethodID add;
    int detach;
    if (!state || !state->host_view || !config || !slot || config->z_order != BOARD_NATIVE_VIEW_ABOVE_RENDERER) return BOARD_ERROR_UNAVAILABLE;
    environment = board_android_jni(state, &detach); if (!environment) return BOARD_ERROR_PLATFORM;
    host_class = (*environment)->GetObjectClass(environment, state->host_view);
    add = host_class ? (*environment)->GetMethodID(environment, host_class, "addNativeOverlay", "(Landroid/view/View;)V") : NULL;
    if (!add) { if (detach) (*state->java_vm)->DetachCurrentThread(state->java_vm); return BOARD_ERROR_PLATFORM; }
    (*environment)->CallVoidMethod(environment, state->host_view, add, (jobject)config->native_view);
    if ((*environment)->ExceptionCheck(environment)) { (*environment)->ExceptionClear(environment); if (detach) (*state->java_vm)->DetachCurrentThread(state->java_vm); return BOARD_ERROR_PLATFORM; }
    slot->implementation = config->native_view;
    if (detach) (*state->java_vm)->DetachCurrentThread(state->java_vm);
    return board_android_slot_apply(slot);
}

static void board_android_slot_destroy(BoardNativeViewSlot *slot) {
    BoardAndroidState *state = slot && slot->backend ? (BoardAndroidState *)slot->backend->implementation : NULL;
    JNIEnv *environment;
    jclass host_class;
    jmethodID remove;
    int detach;
    if (!state || !slot || !slot->implementation) return;
    environment = board_android_jni(state, &detach);
    if (environment && state->host_view) { host_class = (*environment)->GetObjectClass(environment, state->host_view); remove = host_class ? (*environment)->GetMethodID(environment, host_class, "removeNativeOverlay", "(Landroid/view/View;)V") : NULL; if (remove) (*environment)->CallVoidMethod(environment, state->host_view, remove, (jobject)slot->implementation); (*environment)->DeleteGlobalRef(environment, (jobject)slot->implementation); }
    if (detach) (*state->java_vm)->DetachCurrentThread(state->java_vm);
    slot->implementation = NULL;
}

#if BOARD_BUILD_ANDROID_VULKAN
static const char *const *board_android_vk_extensions(void *data, uint32_t *count) {
    static const char *const extensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME };
    (void)data;
    if (!count) return NULL;
    *count = (uint32_t)(sizeof(extensions) / sizeof(extensions[0]));
    return extensions;
}

static BoardResult board_android_vk_create_surface(void *data, void *instance, uint64_t *out_surface) {
    BoardAndroidState *state = (BoardAndroidState *)data;
    VkAndroidSurfaceCreateInfoKHR info = { VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR };
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (!state || !state->window || !instance || !out_surface) return BOARD_ERROR_UNAVAILABLE;
    info.window = state->window;
    if (vkCreateAndroidSurfaceKHR((VkInstance)instance, &info, NULL, &surface) != VK_SUCCESS) return BOARD_ERROR_PLATFORM;
    *out_surface = (uint64_t)(uintptr_t)surface;
    return BOARD_OK;
}

static void board_android_vk_destroy_surface(void *data, void *instance, uint64_t surface) {
    (void)data;
    if (instance && surface) vkDestroySurfaceKHR((VkInstance)instance, (VkSurfaceKHR)(uintptr_t)surface, NULL);
}
#endif

#if BOARD_BUILD_ANDROID_OPENGL
typedef struct BoardAndroidOpenGLContext { EGLDisplay display; EGLConfig config; EGLContext context; EGLSurface surface; uint32_t width, height; float scale; } BoardAndroidOpenGLContext;
static BoardResult board_android_gl_rebuild(BoardAndroidState *state, BoardAndroidOpenGLContext *gl);
static void board_android_gl_release_surface(BoardAndroidOpenGLContext *gl);
#endif

static uint64_t board_android_timestamp(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000000000ULL + (uint64_t)now.tv_nsec;
}

static void board_android_emit(BoardBackend *backend, BoardEventType type) {
    BoardEvent event = {sizeof(BoardEvent), BOARD_ABI_VERSION, type, board_android_timestamp(), {{0}}};
    if (backend && backend->event_sink) backend->event_sink(backend->event_data, &event);
}

static BoardResult board_android_resize(BoardAndroidState *state) {
    BoardBackend *backend;
    uint8_t *pixels;
    uint32_t width, height, previous_width, previous_height;
    BoardResult result;
    BoardEvent event = {sizeof(BoardEvent), BOARD_ABI_VERSION, BOARD_EVENT_RESIZE, board_android_timestamp(), {{0}}};
    if (!state || !state->window || !(backend = state->backend)) return BOARD_ERROR_UNAVAILABLE;
    previous_width = backend->width;
    previous_height = backend->height;
#if BOARD_BUILD_ANDROID_OPENGL
    if (state->active_gl) {
        result = board_android_gl_rebuild(state, (BoardAndroidOpenGLContext *)state->active_gl);
        if (result != BOARD_OK) return result;
        width = backend->width;
        height = backend->height;
    } else
#endif
    {
        if (ANativeWindow_setBuffersGeometry(state->window, 0, 0, WINDOW_FORMAT_RGBA_8888) != 0) return BOARD_ERROR_PLATFORM;
        width = (uint32_t)ANativeWindow_getWidth(state->window);
        height = (uint32_t)ANativeWindow_getHeight(state->window);
    }
    if (!width || !height) return BOARD_ERROR_UNAVAILABLE;
    if (width != previous_width || height != previous_height) {
        pixels = (uint8_t *)realloc(backend->pixels, (size_t)width * height * 4u);
        if (!pixels) return BOARD_ERROR_OUT_OF_MEMORY;
        backend->pixels = pixels;
    }
    if (width == previous_width && height == previous_height) return BOARD_OK;
    backend->width = width;
    backend->height = height;
    backend->stride = width * 4u;
    backend->scale = 1.0f;
    event.data.resize.width = width;
    event.data.resize.height = height;
    event.data.resize.scale = backend->scale;
    if (backend->event_sink) backend->event_sink(backend->event_data, &event);
    return BOARD_OK;
}

#if BOARD_BUILD_ANDROID_OPENGL
static void board_android_gl_release_surface(BoardAndroidOpenGLContext *gl) {
    if (!gl || gl->display == EGL_NO_DISPLAY || gl->surface == EGL_NO_SURFACE) return;
    eglMakeCurrent(gl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(gl->display, gl->surface);
    gl->surface = EGL_NO_SURFACE;
}

static BoardResult board_android_gl_rebuild(BoardAndroidState *state, BoardAndroidOpenGLContext *gl) {
    EGLint width = 0, height = 0;
    if (!state || !state->window || !gl || gl->display == EGL_NO_DISPLAY || gl->context == EGL_NO_CONTEXT) return BOARD_ERROR_UNAVAILABLE;
    board_android_gl_release_surface(gl);
    gl->surface = eglCreateWindowSurface(gl->display, gl->config, state->window, NULL);
    if (gl->surface == EGL_NO_SURFACE || !eglMakeCurrent(gl->display, gl->surface, gl->surface, gl->context)) return BOARD_ERROR_PLATFORM;
    eglSwapInterval(gl->display, 1);
    if (!eglQuerySurface(gl->display, gl->surface, EGL_WIDTH, &width) || !eglQuerySurface(gl->display, gl->surface, EGL_HEIGHT, &height) || width <= 0 || height <= 0) return BOARD_ERROR_PLATFORM;
    gl->width = (uint32_t)width;
    gl->height = (uint32_t)height;
    gl->scale = 1.0f;
    state->backend->width = gl->width;
    state->backend->height = gl->height;
    state->backend->stride = gl->width * 4u;
    state->backend->scale = gl->scale;
    return BOARD_OK;
}

static BoardResult board_android_gl_create(void *data, void **out_context) {
    BoardAndroidState *state = (BoardAndroidState *)data;
    BoardAndroidOpenGLContext *gl;
    EGLint count = 0;
    EGLint config_attributes[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE};
    EGLint context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    if (!state || !state->window || !out_context || state->active_gl) return BOARD_ERROR_INVALID_ARGUMENT;
    gl = (BoardAndroidOpenGLContext *)calloc(1, sizeof(*gl));
    if (!gl) return BOARD_ERROR_OUT_OF_MEMORY;
    gl->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (gl->display == EGL_NO_DISPLAY || !eglInitialize(gl->display, NULL, NULL) || !eglBindAPI(EGL_OPENGL_ES_API) || !eglChooseConfig(gl->display, config_attributes, &gl->config, 1, &count) || count < 1) goto failure;
    gl->context = eglCreateContext(gl->display, gl->config, EGL_NO_CONTEXT, context_attributes);
    if (gl->context == EGL_NO_CONTEXT) goto failure;
    state->active_gl = gl;
    if (board_android_gl_rebuild(state, gl) != BOARD_OK) goto failure;
    *out_context = gl;
    return BOARD_OK;
failure:
    board_android_gl_release_surface(gl);
    if (gl->display != EGL_NO_DISPLAY) { if (gl->context != EGL_NO_CONTEXT) eglDestroyContext(gl->display, gl->context); eglTerminate(gl->display); }
    if (state->active_gl == gl) state->active_gl = NULL;
    free(gl);
    return BOARD_ERROR_PLATFORM;
}

static void board_android_gl_destroy(void *data, void *context) {
    BoardAndroidState *state = (BoardAndroidState *)data;
    BoardAndroidOpenGLContext *gl = (BoardAndroidOpenGLContext *)context;
    if (!gl) return;
    board_android_gl_release_surface(gl);
    if (gl->display != EGL_NO_DISPLAY) { if (gl->context != EGL_NO_CONTEXT) eglDestroyContext(gl->display, gl->context); eglTerminate(gl->display); }
    if (state && state->active_gl == gl) state->active_gl = NULL;
    free(gl);
}

static BoardResult board_android_gl_make_current(void *data, void *context) {
    BoardAndroidState *state = (BoardAndroidState *)data;
    BoardAndroidOpenGLContext *gl = (BoardAndroidOpenGLContext *)context;
    if (!state || !gl || state->active_gl != gl || gl->surface == EGL_NO_SURFACE) return BOARD_ERROR_UNAVAILABLE;
    return eglMakeCurrent(gl->display, gl->surface, gl->surface, gl->context) ? BOARD_OK : BOARD_ERROR_PLATFORM;
}

static void *board_android_gl_get_proc(void *data, const char *name) { (void)data; return name ? (void *)eglGetProcAddress(name) : NULL; }

static BoardResult board_android_gl_drawable_size(void *data, uint32_t *width, uint32_t *height, float *scale) {
    BoardAndroidState *state = (BoardAndroidState *)data;
    BoardAndroidOpenGLContext *gl = state ? (BoardAndroidOpenGLContext *)state->active_gl : NULL;
    if (!gl || !width || !height || !scale) return BOARD_ERROR_INVALID_ARGUMENT;
    *width = gl->width;
    *height = gl->height;
    *scale = gl->scale;
    return BOARD_OK;
}

static BoardResult board_android_gl_swap(void *data) {
    BoardAndroidState *state = (BoardAndroidState *)data;
    BoardAndroidOpenGLContext *gl = state ? (BoardAndroidOpenGLContext *)state->active_gl : NULL;
    if (!gl || gl->surface == EGL_NO_SURFACE) return BOARD_ERROR_UNAVAILABLE;
    return eglSwapBuffers(gl->display, gl->surface) ? BOARD_OK : BOARD_ERROR_PLATFORM;
}
#endif

static BoardResult board_android_map(void *data, void **pixels, uint32_t *width, uint32_t *height, uint32_t *stride, BoardPixelFormat *format, float *scale) {
    BoardAndroidState *state = (BoardAndroidState *)data;
    BoardBackend *backend = state ? state->backend : NULL;
    if (!state || !state->window || !backend || !pixels || !width || !height || !stride || !format || !scale) return BOARD_ERROR_UNAVAILABLE;
    *pixels = backend->pixels;
    *width = backend->width;
    *height = backend->height;
    *stride = backend->stride;
    *format = BOARD_PIXEL_FORMAT_RGBA8888;
    *scale = backend->scale;
    return BOARD_OK;
}

static BoardResult board_android_present(void *data) {
    BoardAndroidState *state = (BoardAndroidState *)data;
    BoardBackend *backend = state ? state->backend : NULL;
    ANativeWindow_Buffer buffer;
    uint32_t rows, bytes, y;
    if (!state || !state->window || !backend || !backend->pixels || ANativeWindow_lock(state->window, &buffer, NULL) != 0) return BOARD_ERROR_PLATFORM;
    rows = backend->height < (uint32_t)buffer.height ? backend->height : (uint32_t)buffer.height;
    bytes = backend->stride < (uint32_t)buffer.stride * 4u ? backend->stride : (uint32_t)buffer.stride * 4u;
    for (y = 0; y < rows; ++y) memcpy((uint8_t *)buffer.bits + (size_t)y * buffer.stride * 4u, backend->pixels + (size_t)y * backend->stride, bytes);
    ANativeWindow_unlockAndPost(state->window);
    return BOARD_OK;
}

static int32_t board_android_input(struct android_app *app, AInputEvent *input) {
    BoardAndroidState *state = app ? (BoardAndroidState *)app->userData : NULL;
    BoardBackend *backend = state ? state->backend : NULL;
    BoardEvent event = {sizeof(BoardEvent), BOARD_ABI_VERSION, BOARD_EVENT_NONE, board_android_timestamp(), {{0}}};
    int32_t action;
    if (!backend || AInputEvent_getType(input) != AINPUT_EVENT_TYPE_MOTION) return 0;
    action = AMotionEvent_getAction(input) & AMOTION_EVENT_ACTION_MASK;
    if (action == AMOTION_EVENT_ACTION_DOWN) event.type = BOARD_EVENT_POINTER_DOWN;
    else if (action == AMOTION_EVENT_ACTION_MOVE) event.type = BOARD_EVENT_POINTER_MOVE;
    else if (action == AMOTION_EVENT_ACTION_UP) event.type = BOARD_EVENT_POINTER_UP;
    else if (action == AMOTION_EVENT_ACTION_CANCEL) event.type = BOARD_EVENT_POINTER_CANCEL;
    else return 0;
    event.data.pointer.x = AMotionEvent_getX(input, 0);
    event.data.pointer.y = AMotionEvent_getY(input, 0);
    event.data.pointer.pointer_id = (uint32_t)AMotionEvent_getPointerId(input, 0);
    event.data.pointer.button = BOARD_POINTER_BUTTON_LEFT;
    if (backend->event_sink) backend->event_sink(backend->event_data, &event);
    return 1;
}

static void board_android_command(struct android_app *app, int32_t command) {
    BoardAndroidState *state = app ? (BoardAndroidState *)app->userData : NULL;
    if (!state || !state->backend) return;
    switch (command) {
        case APP_CMD_INIT_WINDOW:
        case APP_CMD_WINDOW_RESIZED:
            state->window = app->window;
            (void)board_android_resize(state);
            break;
        case APP_CMD_TERM_WINDOW:
#if BOARD_BUILD_ANDROID_OPENGL
            if (state->active_gl) board_android_gl_release_surface((BoardAndroidOpenGLContext *)state->active_gl);
#endif
            state->window = NULL;
            board_android_emit(state->backend, BOARD_EVENT_PAUSE);
            break;
        case APP_CMD_PAUSE:
            board_android_emit(state->backend, BOARD_EVENT_PAUSE);
            break;
        case APP_CMD_RESUME:
            board_android_emit(state->backend, BOARD_EVENT_RESUME);
            break;
        case APP_CMD_DESTROY:
            board_android_emit(state->backend, BOARD_EVENT_QUIT);
            board_scheduler_stop(&state->backend->scheduler);
            break;
        default:
            break;
    }
}

static void board_android_frame(long timestamp_ns, void *data) {
    BoardAndroidState *state = (BoardAndroidState *)data;
    BoardBackend *backend = state ? state->backend : NULL;
    if (!backend || !backend->scheduler.running) return;
    if (state->window) {
        board_scheduler_request_frame(&backend->scheduler);
        board_backend_step(backend, (uint64_t)timestamp_ns);
    }
    if (backend->scheduler.running) AChoreographer_postFrameCallback(AChoreographer_getInstance(), board_android_frame, state);
}

static BoardResult board_android_start(BoardBackend *backend) {
    BoardAndroidState *state = backend ? (BoardAndroidState *)backend->implementation : NULL;
    if (!state || !state->window) return BOARD_ERROR_UNAVAILABLE;
    AChoreographer_postFrameCallback(AChoreographer_getInstance(), board_android_frame, state);
    return BOARD_OK;
}

static BoardResult board_android_run(BoardBackend *backend, BoardEventSink sink, void *data) {
    BoardAndroidState *state = backend ? (BoardAndroidState *)backend->implementation : NULL;
    (void)sink;
    (void)data;
    if (!state || !state->app) return BOARD_ERROR_UNAVAILABLE;
    while (backend->scheduler.running && !state->app->destroyRequested) {
        int events;
        struct android_poll_source *source = NULL;
        if (ALooper_pollOnce(-1, NULL, &events, (void **)&source) >= 0 && source) source->process(state->app, source);
    }
    return BOARD_OK;
}

static void board_android_dispose(BoardBackend *backend) {
    BoardAndroidState *state = backend ? (BoardAndroidState *)backend->implementation : NULL;
#if BOARD_BUILD_ANDROID_OPENGL
    if (state && state->active_gl) board_android_gl_destroy(state, state->active_gl);
#endif
    if (state && state->app && state->app->userData == state) {
        state->app->userData = NULL;
        state->app->onAppCmd = NULL;
        state->app->onInputEvent = NULL;
    }
    if (state && state->owns_window && state->window) ANativeWindow_release(state->window);
    if (state && state->host_view) { int detach; JNIEnv *environment = board_android_jni(state, &detach); if (environment) (*environment)->DeleteGlobalRef(environment, state->host_view); if (detach) (*state->java_vm)->DetachCurrentThread(state->java_vm); }
    free(state);
    free(backend->pixels);
    backend->implementation = NULL;
    backend->pixels = NULL;
}

BoardResult board_android_backend_init(BoardBackend *backend, const BoardBackendConfig *config) {
    BoardAndroidState *state;
    if (!backend || !config) return BOARD_ERROR_INVALID_ARGUMENT;
    backend->width = config->width;
    backend->height = config->height;
    backend->stride = config->width * 4u;
    backend->scale = 1.0f;
    backend->pixels = (uint8_t *)calloc(config->height, backend->stride);
    state = (BoardAndroidState *)calloc(1, sizeof(*state));
    if (!backend->pixels || !state) { free(backend->pixels); free(state); backend->pixels = NULL; return BOARD_ERROR_OUT_OF_MEMORY; }
    state->backend = backend;
    backend->implementation = state;
    backend->native_slot_create = board_android_slot_create;
    backend->native_slot_destroy = board_android_slot_destroy;
    backend->native_slot_apply = board_android_slot_apply;
    backend->surface.cpu = (BoardSurfaceCpuInterface){sizeof(BoardSurfaceCpuInterface), BOARD_ABI_VERSION, state, board_android_map, board_android_present};
#if BOARD_BUILD_ANDROID_OPENGL
    backend->surface.opengl = (BoardSurfaceOpenGLInterface){sizeof(BoardSurfaceOpenGLInterface), BOARD_ABI_VERSION, state, board_android_gl_create, board_android_gl_destroy, board_android_gl_make_current, board_android_gl_get_proc, board_android_gl_drawable_size, board_android_gl_swap};
#endif
#if BOARD_BUILD_ANDROID_VULKAN
    backend->surface.vulkan = (BoardSurfaceVulkanInterface){sizeof(BoardSurfaceVulkanInterface), BOARD_ABI_VERSION, state, board_android_vk_extensions, board_android_vk_create_surface, board_android_vk_destroy_surface};
#endif
    backend->start = board_android_start;
    backend->run = board_android_run;
    backend->dispose = board_android_dispose;
    return BOARD_OK;
}

BoardResult board_android_attach(BoardBackend *backend, void *native_app) {
    BoardAndroidState *state = backend ? (BoardAndroidState *)backend->implementation : NULL;
    struct android_app *app = (struct android_app *)native_app;
    if (!backend || backend->kind != BOARD_BACKEND_ANDROID || !state || !app || state->app || app->userData) return BOARD_ERROR_INVALID_ARGUMENT;
    state->app = app;
    app->userData = state;
    app->onAppCmd = board_android_command;
    app->onInputEvent = board_android_input;
    while (!app->window && !app->destroyRequested) {
        int events;
        struct android_poll_source *source = NULL;
        if (ALooper_pollOnce(-1, NULL, &events, (void **)&source) >= 0 && source) source->process(app, source);
    }
    if (app->destroyRequested || !state->window) return BOARD_ERROR_UNAVAILABLE;
    return board_android_resize(state);
}

BoardResult board_android_attach_window(BoardBackend *backend, void *native_window) {
    BoardAndroidState *state = backend ? (BoardAndroidState *)backend->implementation : NULL;
    if (!backend || backend->kind != BOARD_BACKEND_ANDROID || !state || !native_window || state->window || state->app) return BOARD_ERROR_INVALID_ARGUMENT;
    state->window = (ANativeWindow *)native_window;
    state->owns_window = 1;
    return board_android_resize(state);
}

BoardResult board_android_resize_window(BoardBackend *backend) {
    BoardAndroidState *state = backend ? (BoardAndroidState *)backend->implementation : NULL;
    if (!backend || backend->kind != BOARD_BACKEND_ANDROID || !state || !state->window) return BOARD_ERROR_INVALID_ARGUMENT;
    return board_android_resize(state);
}

BoardResult board_android_set_host_view(BoardBackend *backend, void *jni_environment, void *native_view) {
    BoardAndroidState *state = backend ? (BoardAndroidState *)backend->implementation : NULL;
    JNIEnv *environment = (JNIEnv *)jni_environment;
    if (!backend || backend->kind != BOARD_BACKEND_ANDROID || !state || !environment || !native_view || state->host_view || (*environment)->GetJavaVM(environment, &state->java_vm) != JNI_OK) return BOARD_ERROR_INVALID_ARGUMENT;
    state->host_view = (*environment)->NewGlobalRef(environment, (jobject)native_view);
    return state->host_view ? BOARD_OK : BOARD_ERROR_OUT_OF_MEMORY;
}
