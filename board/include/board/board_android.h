/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef BOARD_ANDROID_H
#define BOARD_ANDROID_H
#include "board_backend.h"
#ifdef __cplusplus
extern "C" {
#endif

/* native_app is an opaque Android NativeActivity handle supplied by the
 * platform entry point. Board owns all Android-specific interaction with it. */
BoardResult board_android_attach(BoardBackend *backend, void *native_app);

/* The caller transfers one ANativeWindow reference as an opaque pointer. */
BoardResult board_android_attach_window(BoardBackend *backend, void *native_window);
BoardResult board_android_resize_window(BoardBackend *backend);
BoardResult board_android_set_host_view(BoardBackend *backend, void *jni_environment, void *native_view);

#ifdef __cplusplus
}
#endif
#endif
