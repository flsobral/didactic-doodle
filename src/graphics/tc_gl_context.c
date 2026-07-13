/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/* OpenGL requires a Skia build with Ganesh GL enabled. The pinned macOS
 * archive does not include it, so CMake rejects TC_GRAPHICS=OPENGL. */
int tc_gl_context_translation_unit_anchor(void) { return 0; }
