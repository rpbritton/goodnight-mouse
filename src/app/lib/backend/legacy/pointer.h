/**
 * Copyright (C) 2021 Ryan Britton
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

#ifndef CFF92E9A_91E0_4BBE_AB7D_7E3CFE7366E2
#define CFF92E9A_91E0_4BBE_AB7D_7E3CFE7366E2

#include <atspi/atspi.h>

#include "legacy.h"
#include "../pointer.h"

// backend for pointer events that uses pure atspi
typedef struct BackendLegacyPointer
{
    BackendLegacy *backend;

    BackendPointerCallback callback;
    gpointer data;

    AtspiDeviceListener *listener;
} BackendLegacyPointer;

BackendLegacyPointer *backend_legacy_pointer_new(BackendLegacy *backend, BackendPointerCallback callback, gpointer data);
void backend_legacy_pointer_destroy(BackendLegacyPointer *pointer);
void backend_legacy_pointer_grab(BackendLegacyPointer *pointer);
void backend_legacy_pointer_ungrab(BackendLegacyPointer *pointer);
void backend_legacy_pointer_grab_button(BackendLegacyPointer *pointer, guint button, BackendStateEvent state);
void backend_legacy_pointer_ungrab_button(BackendLegacyPointer *pointer, guint button, BackendStateEvent state);

#endif /* CFF92E9A_91E0_4BBE_AB7D_7E3CFE7366E2 */
