/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef BOARD_HOST_H
#define BOARD_HOST_H

#include "board_types.h"
#include "board_version.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum BoardHostMode {
    BOARD_HOST_MODE_FULLSCREEN_OWNED,
    BOARD_HOST_MODE_EMBEDDED,
    BOARD_HOST_MODE_HYBRID_OVERLAY
} BoardHostMode;

typedef enum BoardLifecycleState {
    BOARD_LIFECYCLE_STATE_CREATED,
    BOARD_LIFECYCLE_STATE_ACTIVE,
    BOARD_LIFECYCLE_STATE_PAUSED,
    BOARD_LIFECYCLE_STATE_STOPPED,
    BOARD_LIFECYCLE_STATE_DESTROYED
} BoardLifecycleState;

typedef void (*BoardUiTask)(void *user_data);
typedef struct BoardHostServices {
    uint32_t struct_size;
    uint32_t abi_version;
    void *user_data;
    BoardResult (*dispatch_ui)(void *user_data, BoardUiTask task, void *task_data);
} BoardHostServices;

/* Invokes a host-provided UI-thread dispatcher. The caller owns task_data. */
BoardResult board_host_dispatch_ui(const BoardHostServices *services, BoardUiTask task, void *task_data);

#ifdef __cplusplus
}
#endif
#endif
