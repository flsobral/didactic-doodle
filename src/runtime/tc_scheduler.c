/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"

int tc_scheduler_start(TcFrameScheduler* scheduler, TcFrameCallback callback, void* user_data) {
    if (!scheduler || !callback) return TC_ERROR_INVALID_ARGUMENT;
    scheduler->callback = callback;
    scheduler->user_data = user_data;
    scheduler->running = true;
    scheduler->requested = true;
    scheduler->last_timestamp = 0.0;
    return TC_OK;
}

int tc_scheduler_request_frame(TcFrameScheduler* scheduler) {
    if (!scheduler || !scheduler->running) return TC_ERROR_INVALID_ARGUMENT;
    scheduler->requested = true;
    return TC_OK;
}

void tc_scheduler_stop(TcFrameScheduler* scheduler) { if (scheduler) scheduler->running = false; }
