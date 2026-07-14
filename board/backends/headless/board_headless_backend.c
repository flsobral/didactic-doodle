/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "../../src/board_internal.h"
#include <stdlib.h>
static BoardResult board_headless_map(void *data, void **pixels, uint32_t *width, uint32_t *height, uint32_t *stride, BoardPixelFormat *format, float *scale) { BoardBackend *backend = data; if (!backend || !pixels || !width || !height || !stride || !format || !scale) return BOARD_ERROR_INVALID_ARGUMENT; *pixels = backend->pixels; *width = backend->width; *height = backend->height; *stride = backend->stride; *format = BOARD_PIXEL_FORMAT_RGBA8888; *scale = backend->scale; return BOARD_OK; }
static BoardResult board_headless_present(void *data) { return data ? BOARD_OK : BOARD_ERROR_INVALID_ARGUMENT; }
static BoardResult board_headless_run(BoardBackend *backend, BoardEventSink sink, void *user_data) { (void)backend; (void)sink; (void)user_data; return BOARD_ERROR_UNAVAILABLE; }
static void board_headless_dispose(BoardBackend *backend) { free(backend->pixels); backend->pixels = NULL; }
BoardResult board_headless_backend_init(BoardBackend *backend, const BoardBackendConfig *config) { if (!backend || !config) return BOARD_ERROR_INVALID_ARGUMENT; backend->width = config->width; backend->height = config->height; backend->scale = config->scale > 0 ? config->scale : 1.0f; backend->stride = backend->width * 4; backend->pixels = calloc(backend->height, backend->stride); if (!backend->pixels) return BOARD_ERROR_OUT_OF_MEMORY; backend->surface.cpu = (BoardSurfaceCpuInterface){ sizeof(BoardSurfaceCpuInterface), BOARD_ABI_VERSION, backend, board_headless_map, board_headless_present }; backend->run = board_headless_run; backend->dispose = board_headless_dispose; return BOARD_OK; }
