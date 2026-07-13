/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_RUNTIME_SCHEDULER_H
#define TC_RUNTIME_SCHEDULER_H
#include "tc_types.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef void (*TcFrameCallback)(void* user_data, double timestamp_seconds, double delta_seconds);
int tc_scheduler_start(TcFrameScheduler* scheduler, TcFrameCallback callback, void* user_data);
int tc_scheduler_request_frame(TcFrameScheduler* scheduler);
void tc_scheduler_stop(TcFrameScheduler* scheduler);
#ifdef __cplusplus
}
#endif
#endif
