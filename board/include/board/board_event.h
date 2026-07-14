/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef BOARD_EVENT_H
#define BOARD_EVENT_H
#include <stdint.h>
#include "board_types.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum BoardEventType { BOARD_EVENT_NONE, BOARD_EVENT_QUIT, BOARD_EVENT_RESIZE, BOARD_EVENT_SCALE_CHANGED, BOARD_EVENT_POINTER_DOWN, BOARD_EVENT_POINTER_MOVE, BOARD_EVENT_POINTER_UP, BOARD_EVENT_POINTER_CANCEL, BOARD_EVENT_SCROLL, BOARD_EVENT_KEY_DOWN, BOARD_EVENT_KEY_UP, BOARD_EVENT_TEXT_INPUT, BOARD_EVENT_COMPOSITION, BOARD_EVENT_FOCUS, BOARD_EVENT_PAUSE, BOARD_EVENT_RESUME, BOARD_EVENT_LOW_MEMORY } BoardEventType;
typedef enum BoardPointerButton { BOARD_POINTER_BUTTON_NONE, BOARD_POINTER_BUTTON_LEFT, BOARD_POINTER_BUTTON_MIDDLE, BOARD_POINTER_BUTTON_RIGHT } BoardPointerButton;
typedef struct BoardEvent { uint32_t struct_size; uint32_t abi_version; BoardEventType type; uint64_t timestamp_ns; union { struct { uint32_t width, height; float scale; } resize; struct { float x, y; BoardPointerButton button; uint32_t pointer_id; } pointer; struct { float x, y; } scroll; struct { int32_t key; uint8_t repeat; } key; struct { char text[64]; } text; } data; } BoardEvent;
typedef void (*BoardEventSink)(void *user_data, const BoardEvent *event);
#ifdef __cplusplus
}
#endif
#endif
