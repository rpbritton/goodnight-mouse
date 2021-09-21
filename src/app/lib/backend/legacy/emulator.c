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

#include "emulator.h"

// create a new legacy emulator listener
BackendLegacyEmulator *backend_legacy_emulator_new(BackendLegacy *backend)
{
    BackendLegacyEmulator *emulator = g_new(BackendLegacyEmulator, 1);

    // add backend
    emulator->backend = backend;

    // set emulated modifiers
    emulator->emulated_modifiers = 0;

    // return
    return emulator;
}

// destroy the emulator listener
void backend_legacy_emulator_destroy(BackendLegacyEmulator *emulator)
{
    // free
    g_free(emulator);
}

// reset any emulated elements
gboolean backend_legacy_emulator_reset(BackendLegacyEmulator *emulator)
{
    gboolean success = TRUE;

    // reset modifiers
    if (emulator->emulated_modifiers != 0)
    {
        success &= atspi_generate_keyboard_event(emulator->emulated_modifiers, NULL, ATSPI_KEY_UNLOCKMODIFIERS, NULL);
        emulator->emulated_modifiers = 0;
    }

    // return whether success
    return success;
}

// set the state
gboolean backend_legacy_emulator_state(BackendLegacyEmulator *emulator, BackendStateEvent state)
{
    // set modifiers
    emulator->emulated_modifiers |= state.modifiers;
    g_message("locking modifiers: %d", emulator->emulated_modifiers);
    if (!atspi_generate_keyboard_event(emulator->emulated_modifiers, NULL, ATSPI_KEY_LOCKMODIFIERS, NULL))
        return FALSE;

    // set pointer position
    if (!atspi_generate_mouse_event(state.pointer_x, state.pointer_y, "abs", NULL))
        return FALSE;

    // return success
    return TRUE;
}

// set the key event
gboolean backend_legacy_emulator_key(BackendLegacyEmulator *emulator, BackendKeyboardEvent event)
{
    // set the state
    if (!backend_legacy_emulator_state(emulator, event.state))
        return FALSE;

    // press the key
    if (!atspi_generate_keyboard_event(event.keycode, NULL, (event.pressed) ? ATSPI_KEY_PRESS : ATSPI_KEY_RELEASE, NULL))
        return FALSE;

    // return success
    return TRUE;
}

// set the button event
gboolean backend_legacy_emulator_button(BackendLegacyEmulator *emulator, BackendPointerEvent event)
{
    // set the state
    if (!backend_legacy_emulator_state(emulator, event.state))
        return FALSE;

    // move and press the mouse
    gchar *event_name = g_strdup_printf("b%d%c", event.button, (event.pressed) ? 'p' : 'r');
    gboolean success = atspi_generate_mouse_event(event.state.pointer_x, event.state.pointer_y, event_name, NULL);
    g_free(event_name);
    if (!success)
        return FALSE;

    // return success
    return TRUE;
}
