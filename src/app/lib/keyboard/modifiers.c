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

#include "modifiers.h"

#include <gdk/gdk.h>

// get the current keyboard modifier state
guint keyboard_modifiers()
{
    GdkKeymap *keymap = gdk_keymap_get_for_display(gdk_display_get_default());
    guint modifiers = gdk_keymap_get_modifier_state(keymap);
    return keyboard_modifiers_map(modifiers);
}

// map all modifiers to the eight real ones
guint keyboard_modifiers_map(guint modifiers)
{
    GdkKeymap *keymap = gdk_keymap_get_for_display(gdk_display_get_default());
    gdk_keymap_map_virtual_modifiers(keymap, &modifiers);
    return modifiers & 0xFF;
}
