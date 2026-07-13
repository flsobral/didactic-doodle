/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TC_SDL_BACKEND_H
#define TC_SDL_BACKEND_H
#include "tc_runtime/tc_backend.h"
int tc_sdl_backend_create(const TcBackendConfig* config, TcPlatformBackend** out_backend);
#endif
