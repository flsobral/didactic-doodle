/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "board_internal.h"
#include <stddef.h>
#include <stdlib.h>

BoardResult board_host_dispatch_ui(const BoardHostServices *services, BoardUiTask task, void *task_data) {
    if (!services || !task || services->struct_size < sizeof(*services) || services->abi_version != BOARD_ABI_VERSION || !services->dispatch_ui) return BOARD_ERROR_INVALID_ARGUMENT;
    return services->dispatch_ui(services->user_data, task, task_data);
}

BoardResult board_backend_host_mode(const BoardBackend *backend, BoardHostMode *out_mode) {
    if (!backend || !out_mode) return BOARD_ERROR_INVALID_ARGUMENT;
    *out_mode = backend->host_mode;
    return BOARD_OK;
}

static BoardResult board_native_view_slot_apply(BoardNativeViewSlot *slot) {
    if (!slot || !slot->backend || !slot->backend->native_slot_apply) return BOARD_ERROR_UNAVAILABLE;
    return slot->backend->native_slot_apply(slot);
}

BoardResult board_native_view_slot_create(BoardBackend *backend, const BoardNativeViewSlotConfig *config, BoardNativeViewSlot **out_slot) {
    BoardNativeViewSlot *slot;
    BoardResult result;
    if (!backend || !config || !out_slot || config->struct_size < sizeof(*config) || config->abi_version != BOARD_ABI_VERSION || !config->native_view || config->frame.width < 0 || config->frame.height < 0 || (config->clip_enabled && (config->clip.width < 0 || config->clip.height < 0))) return BOARD_ERROR_INVALID_ARGUMENT;
    if (backend->host_mode != BOARD_HOST_MODE_HYBRID_OVERLAY || !backend->native_slot_create) return BOARD_ERROR_UNAVAILABLE;
    if (config->z_order != BOARD_NATIVE_VIEW_ABOVE_RENDERER && config->z_order != BOARD_NATIVE_VIEW_BELOW_RENDERER) return BOARD_ERROR_INVALID_ARGUMENT;
    slot = (BoardNativeViewSlot *)calloc(1, sizeof(*slot));
    if (!slot) return BOARD_ERROR_OUT_OF_MEMORY;
    slot->backend = backend;
    slot->frame = config->frame;
    slot->clip = config->clip;
    slot->visible = config->visible;
    slot->clip_enabled = config->clip_enabled;
    slot->z_order = config->z_order;
    result = backend->native_slot_create(backend, config, slot);
    if (result != BOARD_OK) { if (slot->implementation && backend->native_slot_destroy) backend->native_slot_destroy(slot); free(slot); return result; }
    *out_slot = slot;
    return BOARD_OK;
}

void board_native_view_slot_destroy(BoardNativeViewSlot *slot) {
    if (!slot) return;
    if (slot->backend && slot->backend->native_slot_destroy) slot->backend->native_slot_destroy(slot);
    free(slot);
}

BoardResult board_native_view_slot_set_frame(BoardNativeViewSlot *slot, BoardRect frame) {
    BoardRect previous;
    BoardResult result;
    if (!slot || frame.width < 0 || frame.height < 0) return BOARD_ERROR_INVALID_ARGUMENT;
    previous = slot->frame;
    slot->frame = frame;
    result = board_native_view_slot_apply(slot);
    if (result != BOARD_OK) slot->frame = previous;
    return result;
}

BoardResult board_native_view_slot_set_clip(BoardNativeViewSlot *slot, const BoardRect *clip) {
    BoardRect previous;
    uint8_t previous_enabled;
    BoardResult result;
    if (!slot || (clip && (clip->width < 0 || clip->height < 0))) return BOARD_ERROR_INVALID_ARGUMENT;
    previous = slot->clip;
    previous_enabled = slot->clip_enabled;
    slot->clip_enabled = clip != NULL;
    if (clip) slot->clip = *clip;
    result = board_native_view_slot_apply(slot);
    if (result != BOARD_OK) { slot->clip = previous; slot->clip_enabled = previous_enabled; }
    return result;
}

BoardResult board_native_view_slot_set_visible(BoardNativeViewSlot *slot, uint8_t visible) {
    uint8_t previous;
    BoardResult result;
    if (!slot) return BOARD_ERROR_INVALID_ARGUMENT;
    previous = slot->visible;
    slot->visible = visible != 0;
    result = board_native_view_slot_apply(slot);
    if (result != BOARD_OK) slot->visible = previous;
    return result;
}

BoardResult board_native_view_slot_set_z_order(BoardNativeViewSlot *slot, BoardNativeViewZOrder z_order) {
    BoardNativeViewZOrder previous;
    BoardResult result;
    if (!slot || (z_order != BOARD_NATIVE_VIEW_ABOVE_RENDERER && z_order != BOARD_NATIVE_VIEW_BELOW_RENDERER)) return BOARD_ERROR_INVALID_ARGUMENT;
    previous = slot->z_order;
    slot->z_order = z_order;
    result = board_native_view_slot_apply(slot);
    if (result != BOARD_OK) slot->z_order = previous;
    return result;
}
