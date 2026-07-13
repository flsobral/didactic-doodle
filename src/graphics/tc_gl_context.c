/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/* OpenGL is deliberately isolated behind TcGraphicsContext. The first GPU
 * implementation lands once the selected Skia package advertises Ganesh GL
 * support; configuration keeps CPU usable on every SDL3 + Skia installation. */
#include "tc_internal.h"
int tc_gl_context_translation_unit_anchor(void) { return 0; }
