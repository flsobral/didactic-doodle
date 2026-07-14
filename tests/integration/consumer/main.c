/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <board/board_backend.h>
#include <magic/magic_context.h>
#include <doodle/doodle_renderer.h>
int main(void) { return BOARD_ABI_VERSION == MAGIC_ABI_VERSION && MAGIC_ABI_VERSION == DOODLE_ABI_VERSION ? 0 : 1; }
