/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef BOARD_NATIVE_VIEW_H
#define BOARD_NATIVE_VIEW_H

#include "board_host.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BoardBackend BoardBackend;
typedef struct BoardNativeViewSlot BoardNativeViewSlot;

typedef enum BoardNativeViewZOrder {
    BOARD_NATIVE_VIEW_ABOVE_RENDERER,
    BOARD_NATIVE_VIEW_BELOW_RENDERER
} BoardNativeViewZOrder;

typedef struct BoardNativeViewSlotConfig {
    uint32_t struct_size;
    uint32_t abi_version;
    void *native_view;
    BoardRect frame;
    BoardRect clip;
    uint8_t visible;
    uint8_t clip_enabled;
    BoardNativeViewZOrder z_order;
} BoardNativeViewSlotConfig;

/*
 * Slots are available only to a BOARD_HOST_MODE_HYBRID_OVERLAY host. Frames
 * and clips use logical host coordinates. Board does not own native_view; the
 * platform host attaches it to the slot while the slot exists. Implemented
 * hosts currently support ABOVE_RENDERER only. All slot operations must run
 * on the platform UI thread.
 */
BoardResult board_native_view_slot_create(BoardBackend *backend, const BoardNativeViewSlotConfig *config, BoardNativeViewSlot **out_slot);
void board_native_view_slot_destroy(BoardNativeViewSlot *slot);
BoardResult board_native_view_slot_set_frame(BoardNativeViewSlot *slot, BoardRect frame);
BoardResult board_native_view_slot_set_clip(BoardNativeViewSlot *slot, const BoardRect *clip);
BoardResult board_native_view_slot_set_visible(BoardNativeViewSlot *slot, uint8_t visible);
BoardResult board_native_view_slot_set_z_order(BoardNativeViewSlot *slot, BoardNativeViewZOrder z_order);

#ifdef __cplusplus
}
#endif
#endif
