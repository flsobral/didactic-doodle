/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "../../src/board_internal.h"
#include <SDL3/SDL.h>
#include <stdlib.h>

typedef struct BoardSdl3Backend { SDL_Window *window; } BoardSdl3Backend;
static BoardSdl3Backend *board_sdl3(BoardBackend *backend) { return backend ? backend->implementation : NULL; }
static BoardPointerButton board_sdl3_button(Uint8 button) { if (button == SDL_BUTTON_LEFT) return BOARD_POINTER_BUTTON_LEFT; if (button == SDL_BUTTON_MIDDLE) return BOARD_POINTER_BUTTON_MIDDLE; if (button == SDL_BUTTON_RIGHT) return BOARD_POINTER_BUTTON_RIGHT; return BOARD_POINTER_BUTTON_NONE; }
static BoardResult board_sdl3_map(void *data, void **pixels, uint32_t *width, uint32_t *height, uint32_t *stride, BoardPixelFormat *format, float *scale) {
    BoardBackend *backend = data; BoardSdl3Backend *state = board_sdl3(backend); SDL_Surface *surface;
    if (!backend || !state || !pixels || !width || !height || !stride || !format || !scale) return BOARD_ERROR_INVALID_ARGUMENT;
    surface = SDL_GetWindowSurface(state->window); if (!surface) return BOARD_ERROR_PLATFORM;
    if (surface->format == SDL_PIXELFORMAT_ARGB8888 || surface->format == SDL_PIXELFORMAT_XRGB8888) *format = BOARD_PIXEL_FORMAT_BGRA8888;
    else if (surface->format == SDL_PIXELFORMAT_ABGR8888 || surface->format == SDL_PIXELFORMAT_XBGR8888) *format = BOARD_PIXEL_FORMAT_RGBA8888;
    else return BOARD_ERROR_UNAVAILABLE;
    backend->width = (uint32_t)surface->w; backend->height = (uint32_t)surface->h; backend->stride = (uint32_t)surface->pitch; *pixels = surface->pixels; *width = backend->width; *height = backend->height; *stride = backend->stride; *scale = backend->scale; return BOARD_OK;
}
static BoardResult board_sdl3_present(void *data) { BoardBackend *backend = data; BoardSdl3Backend *state = board_sdl3(backend); return state && SDL_UpdateWindowSurface(state->window) ? BOARD_OK : BOARD_ERROR_PLATFORM; }
static BoardResult board_sdl3_gl_create(void *data, void **out_context) { BoardBackend *backend = data; BoardSdl3Backend *state = board_sdl3(backend); SDL_GLContext context; if (!state || !out_context) return BOARD_ERROR_INVALID_ARGUMENT; context = SDL_GL_CreateContext(state->window); if (!context || !SDL_GL_MakeCurrent(state->window, context)) { if (context) SDL_GL_DestroyContext(context); return BOARD_ERROR_PLATFORM; } SDL_GL_SetSwapInterval(1); *out_context = context; return BOARD_OK; }
static void board_sdl3_gl_destroy(void *data, void *context) { (void)data; if (context) SDL_GL_DestroyContext((SDL_GLContext)context); }
static BoardResult board_sdl3_gl_make_current(void *data, void *context) { BoardBackend *backend = data; BoardSdl3Backend *state = board_sdl3(backend); return state && context && SDL_GL_MakeCurrent(state->window, (SDL_GLContext)context) ? BOARD_OK : BOARD_ERROR_PLATFORM; }
static void *board_sdl3_gl_get_proc(void *data, const char *name) { (void)data; return name ? SDL_GL_GetProcAddress(name) : NULL; }
static BoardResult board_sdl3_gl_drawable_size(void *data, uint32_t *width, uint32_t *height, float *scale) { BoardBackend *backend = data; BoardSdl3Backend *state = board_sdl3(backend); int drawable_width = 0, drawable_height = 0; if (!state || !width || !height || !scale || !SDL_GetWindowSizeInPixels(state->window, &drawable_width, &drawable_height) || drawable_width <= 0 || drawable_height <= 0) return BOARD_ERROR_PLATFORM; backend->width = (uint32_t)drawable_width; backend->height = (uint32_t)drawable_height; *width = backend->width; *height = backend->height; *scale = backend->scale; return BOARD_OK; }
static BoardResult board_sdl3_gl_swap(void *data) { BoardBackend *backend = data; BoardSdl3Backend *state = board_sdl3(backend); return state && SDL_GL_SwapWindow(state->window) ? BOARD_OK : BOARD_ERROR_PLATFORM; }
static void board_sdl3_event(BoardBackend *backend, const SDL_Event *native, BoardEventSink sink, void *data) {
    BoardEvent event = { sizeof(BoardEvent), BOARD_ABI_VERSION, BOARD_EVENT_NONE, SDL_GetTicksNS(), {{0}} };
    switch (native->type) {
        case SDL_EVENT_QUIT: event.type = BOARD_EVENT_QUIT; break;
        case SDL_EVENT_WINDOW_RESIZED: case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: event.type = BOARD_EVENT_RESIZE; event.data.resize.width = (uint32_t)native->window.data1; event.data.resize.height = (uint32_t)native->window.data2; event.data.resize.scale = backend->scale; break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN: case SDL_EVENT_MOUSE_BUTTON_UP: event.type = native->type == SDL_EVENT_MOUSE_BUTTON_DOWN ? BOARD_EVENT_POINTER_DOWN : BOARD_EVENT_POINTER_UP; event.data.pointer.x = native->button.x; event.data.pointer.y = native->button.y; event.data.pointer.button = board_sdl3_button(native->button.button); break;
        case SDL_EVENT_MOUSE_MOTION: event.type = BOARD_EVENT_POINTER_MOVE; event.data.pointer.x = native->motion.x; event.data.pointer.y = native->motion.y; break;
        case SDL_EVENT_KEY_DOWN: case SDL_EVENT_KEY_UP: event.type = native->type == SDL_EVENT_KEY_DOWN ? BOARD_EVENT_KEY_DOWN : BOARD_EVENT_KEY_UP; event.data.key.key = (int32_t)native->key.key; event.data.key.repeat = native->key.repeat; break;
        case SDL_EVENT_TEXT_INPUT: event.type = BOARD_EVENT_TEXT_INPUT; SDL_strlcpy(event.data.text.text, native->text.text, sizeof(event.data.text.text)); break;
        default: break;
    }
    if (event.type != BOARD_EVENT_NONE) sink(data, &event);
}
static BoardResult board_sdl3_run(BoardBackend *backend, BoardEventSink sink, void *data) { SDL_Event event; uint64_t last = SDL_GetTicksNS(); if (!backend || !sink) return BOARD_ERROR_INVALID_ARGUMENT; while (backend->scheduler.running) { while (SDL_PollEvent(&event)) board_sdl3_event(backend, &event, sink, data); uint64_t now = SDL_GetTicksNS(); if (now - last >= 16666667ULL) { board_scheduler_request_frame(&backend->scheduler); board_scheduler_step(&backend->scheduler, now); last = now; } else SDL_Delay(1); } return BOARD_OK; }
static void board_sdl3_dispose(BoardBackend *backend) { BoardSdl3Backend *state = board_sdl3(backend); if (state) { SDL_DestroyWindow(state->window); free(state); } backend->implementation = NULL; SDL_Quit(); }
BoardResult board_sdl3_backend_init(BoardBackend *backend, const BoardBackendConfig *config) { BoardSdl3Backend *state; Uint64 flags = config->resizable ? SDL_WINDOW_RESIZABLE : 0; if (!backend || !config || !SDL_Init(SDL_INIT_VIDEO)) return BOARD_ERROR_PLATFORM;
#if BOARD_BUILD_SDL3_OPENGL
    flags |= SDL_WINDOW_OPENGL;
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) || !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2) || !SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) || !SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) || !SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8) || !SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8) || !SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8) || !SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8)) { SDL_Quit(); return BOARD_ERROR_PLATFORM; }
#endif
    state = calloc(1, sizeof(*state)); if (!state) { SDL_Quit(); return BOARD_ERROR_OUT_OF_MEMORY; } state->window = SDL_CreateWindow(config->title ? config->title : "Magic Doodle Board", (int)config->width, (int)config->height, flags); if (!state->window) { free(state); SDL_Quit(); return BOARD_ERROR_PLATFORM; } backend->implementation = state; backend->width = config->width; backend->height = config->height; backend->scale = config->scale > 0 ? config->scale : 1.0f; backend->surface.cpu = (BoardSurfaceCpuInterface){ sizeof(BoardSurfaceCpuInterface), BOARD_ABI_VERSION, backend, board_sdl3_map, board_sdl3_present }; backend->surface.opengl = (BoardSurfaceOpenGLInterface){ sizeof(BoardSurfaceOpenGLInterface), BOARD_ABI_VERSION, backend, board_sdl3_gl_create, board_sdl3_gl_destroy, board_sdl3_gl_make_current, board_sdl3_gl_get_proc, board_sdl3_gl_drawable_size, board_sdl3_gl_swap }; backend->run = board_sdl3_run; backend->dispose = board_sdl3_dispose; return BOARD_OK; }
