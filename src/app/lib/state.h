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

#ifndef C5BB90B9_061D_47C6_BF57_F8CD6CB47E70
#define C5BB90B9_061D_47C6_BF57_F8CD6CB47E70

#include <gdk/gdk.h>

#include "backend/backend.h"
#include "keymap.h"

// used to subscribe to events emitted from a state
typedef struct State
{
    BackendState *backend;

    Keymap *keymap;
} State;

State *state_new(Backend *backend, Keymap *keymap);
void state_destroy(State *state);
GdkModifierType state_get_modifiers(State *state);
BackendStateEvent state_get_state(State *state);

#endif /* C5BB90B9_061D_47C6_BF57_F8CD6CB47E70 */
