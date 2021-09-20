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

#ifndef FC683DC4_C90D_49B9_B1AE_EDA3E41B2841
#define FC683DC4_C90D_49B9_B1AE_EDA3E41B2841

#include "../backend/backend.h"
#include "../keymap/keymap.h"
#include "../state/state.h"

// used to subscribe to events emitted from a emulator
typedef struct Emulator
{
    BackendEmulator *backend;

    Keymap *keymap;
    State *state;
} Emulator;

Emulator *emulator_new(Backend *backend, Keymap *keymap, State *state);
void emulator_destroy(Emulator *emulator);
gboolean emulator_reset(Emulator *emulator);
gboolean emulator_modifiers(Emulator *emulator, GdkModifierType modifiers);
gboolean emulator_key(Emulator *emulator, guint keysym, GdkModifierType modifiers);
gboolean emulator_move(Emulator *emulator, gint x, gint y);
gboolean emulator_button(Emulator *emulator, guint button, GdkModifierType modifiers, gint x, gint y);

#endif /* FC683DC4_C90D_49B9_B1AE_EDA3E41B2841 */
