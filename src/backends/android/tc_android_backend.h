/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_ANDROID_BACKEND_H
#define TC_ANDROID_BACKEND_H

/* This is deliberately private: Android NDK types must not escape public
 * tc_runtime headers. It requires API level 24 or newer because frames use
 * AChoreographer directly. Applications bind the generic TcEvent sink here
 * from the Android launcher glue. */
#if defined(__ANDROID__)
#include <android_native_app_glue.h>
#include "tc_runtime/tc_event.h"
#include "tc_runtime/tc_scheduler.h"

typedef struct TcAndroidNativeBackend TcAndroidNativeBackend;
int tc_android_backend_attach(struct android_app* app, TcEventSink sink, void* user_data, TcAndroidNativeBackend** out_backend);
void tc_android_backend_detach(TcAndroidNativeBackend* backend);
TcFrameScheduler* tc_android_backend_scheduler(TcAndroidNativeBackend* backend);
void tc_android_backend_tick(TcAndroidNativeBackend* backend, double timestamp_seconds);
#endif
#endif
