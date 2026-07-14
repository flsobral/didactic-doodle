/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef BOARD_IOS_H
#define BOARD_IOS_H
#include "board_backend.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Returns the reusable native Board view as an opaque pointer. */
BoardResult board_ios_view_get(BoardBackend *backend, void **out_view);

#ifdef __cplusplus
}
#endif
#endif
