/**
 * Copyright (C) 2021 ryan
 *
 * This file is part of Goodnight Mouse.
 *
 * Goodnight Mouse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Goodnight Mouse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Goodnight Mouse.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef A9931399_E0BF_4011_A403_586AD57B1A31
#define A9931399_E0BF_4011_A403_586AD57B1A31

#include <atspi/atspi.h>

#include "backend.h"
#include "../common/focus.h"

// backend for focus that uses pure atspi
typedef struct BackendLegacyFocus
{
    BackendLegacy *backend;

    BackendFocusCallback callback;
    gpointer data;

    AtspiEventListener *listener;
} BackendLegacyFocus;

BackendLegacyFocus *backend_legacy_focus_new(BackendLegacy *backend, BackendFocusCallback callback, gpointer data);
void backend_legacy_focus_destroy(BackendLegacyFocus *focus);
AtspiAccessible *backend_legacy_focus_get_window(BackendLegacyFocus *focus);

#endif /* A9931399_E0BF_4011_A403_586AD57B1A31 */
