/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_RUNTIME_TYPES_H
#define TC_RUNTIME_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum TcResult { TC_OK = 0, TC_ERROR_INVALID_ARGUMENT = -1, TC_ERROR_UNAVAILABLE = -2, TC_ERROR_OUT_OF_MEMORY = -3, TC_ERROR_PLATFORM = -4, TC_ERROR_RENDERER = -5 } TcResult;
typedef struct TcPoint { float x, y; } TcPoint;
typedef struct TcSize { float width, height; } TcSize;
typedef struct TcRect { float x, y, width, height; } TcRect;
typedef struct TcColor { float r, g, b, a; } TcColor;
typedef struct TcTransform { float m11, m12, m21, m22, dx, dy; } TcTransform;
typedef enum TcPaintStyle { TC_PAINT_FILL, TC_PAINT_STROKE } TcPaintStyle;
typedef struct TcPaint { TcColor color; float stroke_width; TcPaintStyle style; } TcPaint;
typedef struct TcTextStyle { TcColor color; float size; const char* family; } TcTextStyle;

typedef struct TcApp TcApp;
typedef struct TcPlatformBackend TcPlatformBackend;
typedef struct TcGraphicsContext TcGraphicsContext;
typedef struct TcRenderer2D TcRenderer2D;
typedef struct TcCanvas2D TcCanvas2D;
typedef struct TcFrameScheduler TcFrameScheduler;
typedef struct TcNativeWindowHandle TcNativeWindowHandle;
typedef struct TcNativeSurfaceHandle TcNativeSurfaceHandle;
typedef struct TcPath TcPath;
typedef struct TcImage TcImage;

#ifdef __cplusplus
}
#endif
#endif
