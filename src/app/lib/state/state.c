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

#include "state.h"

// creates a new state translator
State *state_new(Backend *backend, Keymap *keymap)
{
    State *state = g_new(State, 1);

    // add state backend
    state->backend = backend_state_new(backend);

    // add keymap
    state->keymap = keymap;

    // return
    return state;
}

// stops and destroys a state
void state_destroy(State *state)
{
    // free the backend
    backend_state_destroy(state->backend);

    // free
    g_free(state);
}

// get the current modifiers state
GdkModifierType state_get_modifiers(State *state)
{
    // get state
    BackendStateEvent event = backend_state_current(state->backend);

    // return mapped modifiers
    return keymap_all_modifiers(state->keymap, event.modifiers);
}

// get the current keyboard state
BackendStateEvent state_get_state(State *state)
{
    // return state
    return backend_state_current(state->backend);
}
