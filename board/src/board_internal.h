/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef BOARD_INTERNAL_H
#define BOARD_INTERNAL_H
#include <board/board_app.h>

struct BoardFrameScheduler { BoardFrameCallback callback; void *user_data; uint64_t previous_timestamp_ns; unsigned running : 1; unsigned requested : 1; };
struct BoardNativeSurface { BoardSurfaceCpuInterface cpu; BoardSurfaceOpenGLInterface opengl; };
typedef BoardResult (*BoardBackendRun)(BoardBackend *backend, BoardEventSink sink, void *user_data);
typedef void (*BoardBackendDispose)(BoardBackend *backend);
struct BoardBackend { BoardNativeSurface surface; BoardFrameScheduler scheduler; BoardBackendKind kind; void *implementation; BoardBackendRun run; BoardBackendDispose dispose; uint8_t *pixels; uint32_t width, height, stride; float scale; BoardEvent events[32]; uint32_t event_count; };
struct BoardApp { BoardBackend *backend; BoardAppCallbacks callbacks; void *user_data; unsigned running : 1; unsigned started : 1; };
BoardResult board_headless_backend_init(BoardBackend *backend, const BoardBackendConfig *config);
#if BOARD_BUILD_SDL3
BoardResult board_sdl3_backend_init(BoardBackend *backend, const BoardBackendConfig *config);
#endif
BoardResult board_backend_run_internal(BoardBackend *backend, BoardEventSink sink, void *user_data);
#endif
