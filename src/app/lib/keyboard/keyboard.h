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

#ifndef D102CB85_DF5A_44CB_80DC_B281855A12AB
#define D102CB85_DF5A_44CB_80DC_B281855A12AB

#include <gdk/gdk.h>

#include "../backend/backend.h"

// callback type used to notify on subscribed keyboard event
typedef void (*KeyboardCallback)(KeyboardEvent event, gpointer data);

// used to subscribe to events emitted from a keyboard
typedef struct Keyboard
{
    BackendKeyboard *backend;

    GdkKeymap *keymap;

    GList *subscribers;
} Keyboard;

Keyboard *keyboard_new(Backend *backend);
void keyboard_destroy(Keyboard *keyboard);
void keyboard_subscribe(Keyboard *keyboard, KeyboardCallback callback, gpointer data);
void keyboard_unsubscribe(Keyboard *keyboard, KeyboardCallback callback, gpointer data);
void keyboard_subscribe_key(Keyboard *keyboard, KeyboardEvent event, KeyboardCallback callback, gpointer data);
void keyboard_unsubscribe_key(Keyboard *keyboard, KeyboardEvent event, KeyboardCallback callback, gpointer data);
Modifiers keyboard_get_modifiers(Keyboard *keyboard);
Modifiers keyboard_map_modifiers(Keyboard *keyboard, GdkModifierType mods);

#endif /* D102CB85_DF5A_44CB_80DC_B281855A12AB */
