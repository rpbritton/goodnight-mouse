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

#ifndef C7CBA95A_8C7B_4B0A_B36D_BF177EB4CC54
#define C7CBA95A_8C7B_4B0A_B36D_BF177EB4CC54

#include <atspi/atspi.h>

#include "legacy.h"
#include "../keyboard.h"
#include "../pointer.h"
#include "../state.h"

// backend for getting emulator
typedef struct BackendLegacyEmulator
{
    BackendLegacy *backend;

    guint8 emulated_modifiers;
} BackendLegacyEmulator;

BackendLegacyEmulator *backend_legacy_emulator_new(BackendLegacy *backend);
void backend_legacy_emulator_destroy(BackendLegacyEmulator *emulator);
gboolean backend_legacy_emulator_reset(BackendLegacyEmulator *emulator);
gboolean backend_legacy_emulator_state(BackendLegacyEmulator *emulator, BackendStateEvent state);
gboolean backend_legacy_emulator_key(BackendLegacyEmulator *emulator, BackendKeyboardEvent event);
gboolean backend_legacy_emulator_button(BackendLegacyEmulator *emulator, BackendPointerEvent event);

#endif /* C7CBA95A_8C7B_4B0A_B36D_BF177EB4CC54 */
