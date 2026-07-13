/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_runtime/tc_event.h"

/* Event queues stay private to backends. This translation unit reserves the
 * runtime-owned queue boundary without exposing a polling model publicly. */
int tc_event_queue_translation_unit_anchor(void) { return 0; }
