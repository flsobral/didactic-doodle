/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <magic/magic_context.h>
#include <magic/magic_interop.h>
#include <board/board_backend.h>
#include <string.h>
int main(void) {
 BoardBackend *backend = 0; MagicContext *context = 0; MagicFrame *frame = 0; MagicCpuInterop pixels = {0};
 BoardBackendConfig board = {sizeof(board), BOARD_ABI_VERSION, BOARD_BACKEND_HEADLESS, "", 4, 4, 1, 0}; MagicConfig magic = {sizeof(magic), MAGIC_ABI_VERSION, MAGIC_BACKEND_CPU, 1};
 if (board_backend_create(&board, &backend) || magic_context_create(board_backend_surface(backend), &magic, &context) || magic_context_begin_frame(context, &frame)) return 1;
 if (strcmp(magic_context_backend_name(context), "CPU") || !magic_context_backend_version(context)[0]) return 6;
 if (magic_frame_query_interop(frame, MAGIC_INTEROP_CPU, MAGIC_ABI_VERSION + 1, &pixels, sizeof(pixels)) != MAGIC_ERROR_VERSION) return 2;
 if (magic_frame_query_interop(frame, MAGIC_INTEROP_WEB, MAGIC_ABI_VERSION, &pixels, sizeof(pixels)) != MAGIC_ERROR_UNAVAILABLE) return 3;
 if (magic_frame_query_interop(frame, MAGIC_INTEROP_CPU, MAGIC_ABI_VERSION, &pixels, sizeof(pixels)) || !pixels.pixels || pixels.width != 4) return 4;
 ((unsigned char *)pixels.pixels)[0] = 7;
 if (magic_context_end_frame(context, frame)) return 5; magic_context_destroy(context); board_backend_destroy(backend); return 0;
}
