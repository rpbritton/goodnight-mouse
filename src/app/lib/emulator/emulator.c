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

// creates a new emulator translator
Emulator *emulator_new(Backend *backend, Keymap *keymap, State *state)
{
    Emulator *emulator = g_new(Emulator, 1);

    // add emulator backend
    emulator->backend = backend_emulator_new(backend);

    // add dependencies
    emulator->keymap = keymap;
    emulator->state = state;

    // return
    return emulator;
}

// stops and destroys a emulator
void emulator_destroy(Emulator *emulator)
{
    // free the backend
    backend_emulator_destroy(emulator->backend);

    // free
    g_free(emulator);
}

// reset any emulated keys
gboolean emulator_reset(Emulator *emulator)
{
    return backend_emulator_reset(emulator->backend);
}

// emulate the given modifiers
gboolean emulator_modifiers(Emulator *emulator, GdkModifierType modifiers)
{
    // sanitize modifiers
    modifiers = keymap_physical_modifiers(emulator->keymap, modifiers);

    // get the state
    BackendStateEvent state = state_get_state(emulator->state);
    state.modifiers = modifiers;

    // set the state
    return backend_emulator_state(emulator->backend, state);
}

// emulate a keypress with the given modifiers
gboolean emulator_key(Emulator *emulator, guint keysym, GdkModifierType modifiers)
{
    // sanitize modifiers
    modifiers = keymap_physical_modifiers(emulator->keymap, modifiers);

    // get valid a backend key event to generate the keysym
    GList *recipes = keymap_get_keycodes(emulator->keymap, keysym, modifiers);

    // ensure keycode was found
    if (!recipes)
    {
        g_warning("emulator: Failed to generate keysym (%d), no keycodes found", keysym);
        return FALSE;
    }

    // use the first recipe
    BackendKeyboardEvent event = *((BackendKeyboardEvent *)recipes->data);
    g_list_free_full(recipes, g_free);

    // send a key press
    event.pressed = TRUE;
    if (!backend_emulator_key(emulator->backend, event))
    {
        emulator_reset(emulator);
        return FALSE;
    }

    // send key release
    event.pressed = FALSE;
    if (!backend_emulator_key(emulator->backend, event))
    {
        emulator_reset(emulator);
        return FALSE;
    }

    // reset the state
    return emulator_reset(emulator);
}

gboolean emulator_move(Emulator *emulator, gint x, gint y)
{
    // get the current state
    BackendStateEvent state = state_get_state(emulator->state);

    // set the parameters
    state.pointer_x = x;
    state.pointer_y = y;

    // set the state
    if (!backend_emulator_state(emulator->backend, state))
        return FALSE;

    // reset the state
    return emulator_reset(emulator);
}

gboolean emulator_button(Emulator *emulator, guint button, GdkModifierType modifiers, gint x, gint y)
{
    // sanitize modifiers
    modifiers = keymap_physical_modifiers(emulator->keymap, modifiers);

    // get the current state
    BackendStateEvent previous_state = state_get_state(emulator->state);

    // create the pressed state
    BackendStateEvent pressed_state = previous_state;
    pressed_state.modifiers = modifiers;
    pressed_state.pointer_x = x;
    pressed_state.pointer_y = y;

    // create the pointer event
    BackendPointerEvent event = {
        .button = button,
        .state = pressed_state,
    };

    // send a button press
    event.pressed = TRUE;
    if (!backend_emulator_button(emulator->backend, event))
    {
        emulator_reset(emulator);
        return FALSE;
    }

    // send a button release
    event.pressed = FALSE;
    if (!backend_emulator_button(emulator->backend, event))
    {
        emulator_reset(emulator);
        return FALSE;
    }

    // move the pointer back
    return emulator_move(emulator, previous_state.pointer_x, previous_state.pointer_y);
}
