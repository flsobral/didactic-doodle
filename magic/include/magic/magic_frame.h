/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef MAGIC_FRAME_H
#define MAGIC_FRAME_H
#include <stdint.h>
#include "magic_types.h"
#ifdef __cplusplus
extern "C" {
#endif
uint32_t magic_frame_width(const MagicFrame *frame);
uint32_t magic_frame_height(const MagicFrame *frame);
uint32_t magic_frame_stride(const MagicFrame *frame);
float magic_frame_scale(const MagicFrame *frame);
uint64_t magic_frame_sequence(const MagicFrame *frame);
int magic_frame_is_valid(const MagicFrame *frame);
#ifdef __cplusplus
}
#endif
#endif
