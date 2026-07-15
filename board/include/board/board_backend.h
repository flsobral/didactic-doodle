/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef BOARD_BACKEND_H
#define BOARD_BACKEND_H
#include "board_event.h"
#include "board_host.h"
#include "board_native_view.h"
#include "board_scheduler.h"
#include "board_surface.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct BoardBackend BoardBackend;
typedef enum BoardBackendKind { BOARD_BACKEND_HEADLESS, BOARD_BACKEND_SDL3, BOARD_BACKEND_GLFW, BOARD_BACKEND_WEB, BOARD_BACKEND_WINIT, BOARD_BACKEND_ANDROID, BOARD_BACKEND_IOS } BoardBackendKind;
typedef struct BoardBackendConfig { uint32_t struct_size; uint32_t abi_version; BoardBackendKind kind; const char *title; uint32_t width, height; float scale; uint8_t resizable; BoardHostMode host_mode; } BoardBackendConfig;
BoardResult board_backend_create(const BoardBackendConfig *config, BoardBackend **out_backend);
void board_backend_destroy(BoardBackend *backend);
BoardNativeSurface *board_backend_surface(BoardBackend *backend);
BoardFrameScheduler *board_backend_scheduler(BoardBackend *backend);
BoardResult board_backend_post_event(BoardBackend *backend, const BoardEvent *event);
BoardResult board_backend_dispatch_events(BoardBackend *backend, BoardEventSink sink, void *user_data);
BoardResult board_backend_step(BoardBackend *backend, uint64_t timestamp_ns);
BoardResult board_backend_host_mode(const BoardBackend *backend, BoardHostMode *out_mode);
#ifdef __cplusplus
}
#endif
#endif
