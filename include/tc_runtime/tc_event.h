/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_RUNTIME_EVENT_H
#define TC_RUNTIME_EVENT_H
#include "tc_types.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum TcEventType { TC_EVENT_NONE = 0, TC_EVENT_QUIT, TC_EVENT_RESIZE, TC_EVENT_REDRAW_REQUESTED, TC_EVENT_POINTER_DOWN, TC_EVENT_POINTER_MOVE, TC_EVENT_POINTER_UP, TC_EVENT_KEY_DOWN, TC_EVENT_KEY_UP, TC_EVENT_TEXT_INPUT, TC_EVENT_PAUSE, TC_EVENT_RESUME } TcEventType;
typedef enum TcPointerButton { TC_POINTER_BUTTON_NONE, TC_POINTER_BUTTON_LEFT, TC_POINTER_BUTTON_MIDDLE, TC_POINTER_BUTTON_RIGHT } TcPointerButton;
typedef struct TcEvent { TcEventType type; union { struct { int width, height; float scale; } resize; struct { float x, y; TcPointerButton button; unsigned int pointer_id; } pointer; struct { int key; int repeat; } key; struct { char text[32]; } text; } data; } TcEvent;
typedef void (*TcEventSink)(void* user_data, const TcEvent* event);
#ifdef __cplusplus
}
#endif
#endif
