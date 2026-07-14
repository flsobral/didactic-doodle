/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef BOARD_TYPES_H
#define BOARD_TYPES_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum BoardResult { BOARD_OK = 0, BOARD_ERROR_INVALID_ARGUMENT = -1, BOARD_ERROR_UNAVAILABLE = -2, BOARD_ERROR_OUT_OF_MEMORY = -3, BOARD_ERROR_PLATFORM = -4, BOARD_ERROR_VERSION = -5 } BoardResult;
typedef struct BoardSize { uint32_t width, height; } BoardSize;
typedef struct BoardRect { float x, y, width, height; } BoardRect;
#ifdef __cplusplus
}
#endif
#endif
