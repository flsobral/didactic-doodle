/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef DOODLE_TYPES_H
#define DOODLE_TYPES_H
#ifdef __cplusplus
extern "C" {
#endif
typedef enum DoodleResult { DOODLE_OK = 0, DOODLE_ERROR_INVALID_ARGUMENT = -1, DOODLE_ERROR_UNAVAILABLE = -2, DOODLE_ERROR_OUT_OF_MEMORY = -3, DOODLE_ERROR_RENDERER = -4, DOODLE_ERROR_VERSION = -5 } DoodleResult;
typedef struct DoodleCanvas DoodleCanvas;
typedef struct DoodleRenderer DoodleRenderer;
typedef struct DoodlePath DoodlePath;
typedef struct DoodleImage DoodleImage;
typedef struct DoodleFont DoodleFont;
typedef struct DoodlePoint { float x, y; } DoodlePoint;
typedef struct DoodleRect { float x, y, width, height; } DoodleRect;
typedef struct DoodleColor { float r, g, b, a; } DoodleColor;
typedef enum DoodlePaintStyle { DOODLE_PAINT_FILL, DOODLE_PAINT_STROKE } DoodlePaintStyle;
typedef struct DoodlePaint { DoodleColor color; float stroke_width; DoodlePaintStyle style; } DoodlePaint;
typedef struct DoodleTextStyle { DoodleColor color; float size; const char *family; } DoodleTextStyle;
#ifdef __cplusplus
}
#endif
#endif
