/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <board/board_app.h>
#include <board/board_android.h>
#include <board/board_backend.h>
#include <board/board_event.h>
#include <board/board_host.h>
#include <board/board_ios.h>
#include <board/board_native_view.h>
#include <board/board_scheduler.h>
#include <board/board_surface.h>
#include <magic/magic_context.h>
#include <magic/magic_frame.h>
#include <magic/magic_interop.h>
#include <doodle/doodle_canvas.h>
#include <doodle/doodle_renderer.h>
int main(void) { return (int)(BOARD_ABI_VERSION + MAGIC_ABI_VERSION + DOODLE_ABI_VERSION - 3u); }
