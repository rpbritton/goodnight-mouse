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

#ifndef FFA9B89E_680B_4943_A4D6_75EC9AA5ADB4
#define FFA9B89E_680B_4943_A4D6_75EC9AA5ADB4

#include <gdk/gdk.h>

#include "../backend/backend.h"

// used to subscribe to events emitted from a keymap
typedef struct Keymap
{
    GdkKeymap *keymap;
    guint8 hotkey_modifiers;
} Keymap;

Keymap *keymap_new();
void keymap_destroy(Keymap *keymap);
guint8 keymap_physical_modifiers(Keymap *keymap, GdkModifierType modifiers);
GdkModifierType keymap_all_modifiers(Keymap *keymap, guint8 modifiers);
guint8 keymap_hotkey_modifiers(Keymap *keymap, guint8 modifiers);
GList *keymap_get_keycodes(Keymap *keymap, guint keysym, guint8 additional_modifiers);
guint keymap_get_keysym(Keymap *keymap, BackendKeyboardEvent event, guint8 *consumed_modifiers);

#endif /* FFA9B89E_680B_4943_A4D6_75EC9AA5ADB4 */
