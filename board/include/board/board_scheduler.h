/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef BOARD_SCHEDULER_H
#define BOARD_SCHEDULER_H
#include <stdint.h>
#include "board_types.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct BoardFrameScheduler BoardFrameScheduler;
typedef void (*BoardFrameCallback)(void *user_data, uint64_t timestamp_ns, double delta_seconds);
BoardResult board_scheduler_start(BoardFrameScheduler *scheduler, BoardFrameCallback callback, void *user_data);
BoardResult board_scheduler_request_frame(BoardFrameScheduler *scheduler);
void board_scheduler_stop(BoardFrameScheduler *scheduler);
BoardResult board_scheduler_step(BoardFrameScheduler *scheduler, uint64_t timestamp_ns);
#ifdef __cplusplus
}
#endif
#endif
