/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <doodle/doodle_renderer_provider.h>

int main(void) {
    return doodle_blend2d_provider() || doodle_nanovg_provider() || doodle_vello_provider();
}
