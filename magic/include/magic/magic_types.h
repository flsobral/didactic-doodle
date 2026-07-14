/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef MAGIC_TYPES_H
#define MAGIC_TYPES_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum MagicResult { MAGIC_OK = 0, MAGIC_ERROR_INVALID_ARGUMENT = -1, MAGIC_ERROR_UNAVAILABLE = -2, MAGIC_ERROR_OUT_OF_MEMORY = -3, MAGIC_ERROR_SURFACE = -4, MAGIC_ERROR_VERSION = -5, MAGIC_ERROR_DEVICE_LOST = -6 } MagicResult;
typedef enum MagicBackend { MAGIC_BACKEND_AUTO, MAGIC_BACKEND_CPU, MAGIC_BACKEND_OPENGL, MAGIC_BACKEND_METAL, MAGIC_BACKEND_VULKAN, MAGIC_BACKEND_WEB } MagicBackend;
typedef enum MagicPixelFormat { MAGIC_PIXEL_FORMAT_RGBA8888, MAGIC_PIXEL_FORMAT_BGRA8888 } MagicPixelFormat;
typedef struct MagicContext MagicContext;
typedef struct MagicFrame MagicFrame;
#ifdef __cplusplus
}
#endif
#endif
