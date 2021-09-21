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

#include "keymap.h"

#define HOTKEY_MODIFIERS (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_SUPER_MASK)

// creates a new keymap translator
Keymap *keymap_new()
{
    Keymap *keymap = g_new(Keymap, 1);

    // add keymap
    keymap->keymap = gdk_keymap_get_for_display(gdk_display_get_default());

    // add valid modifiers
    keymap->hotkey_modifiers = keymap_physical_modifiers(keymap, HOTKEY_MODIFIERS);

    // return
    return keymap;
}

// stops and destroys a keymap
void keymap_destroy(Keymap *keymap)
{
    // free
    g_free(keymap);
}

// map all virtual modifiers to physical modifiers
guint8 keymap_physical_modifiers(Keymap *keymap, GdkModifierType modifiers)
{
    gdk_keymap_map_virtual_modifiers(keymap->keymap, &modifiers);
    return modifiers & 0xFF;
}

// map all modifiers, physical and virtual
GdkModifierType keymap_all_modifiers(Keymap *keymap, guint8 physical_modifiers)
{
    GdkModifierType modifiers = physical_modifiers;
    gdk_keymap_add_virtual_modifiers(keymap->keymap, &modifiers);
    return modifiers;
}

// return only the modifiers used in hotkeys
guint8 keymap_hotkey_modifiers(Keymap *keymap, guint8 modifiers)
{
    return modifiers & keymap->hotkey_modifiers;
}

// get the keycode and modifiers required to generate the keysym
GList *keymap_get_keycodes(Keymap *keymap, guint keysym, guint8 additional_modifiers)
{
    // sanitize modifiers
    additional_modifiers = keymap_physical_modifiers(keymap, additional_modifiers);

    // get all keycode entries
    GdkKeymapKey *keys;
    gint n_keys;
    gdk_keymap_get_entries_for_keyval(keymap->keymap, keysym, &keys, &n_keys);

    // proccess keycode entries
    GList *recipes = NULL;
    for (gint index = 0; index < n_keys; index++)
    {
        for (guint8 modifiers = 0; modifiers < 0xFF; modifiers++)
        {
            // get keysym and consumed modifiers from state
            guint generated_keysym;
            GdkModifierType consumed_modifiers;
            gdk_keymap_translate_keyboard_state(keymap->keymap, keys[index].keycode,
                                                modifiers, keys[index].group,
                                                &generated_keysym, NULL, NULL, &consumed_modifiers);

            // ensure keysym is the same
            if (generated_keysym != keysym)
                continue;

            // ensure modifiers match
            if (additional_modifiers != (modifiers & ~consumed_modifiers & keymap->hotkey_modifiers))
                continue;

            // create the recipe
            BackendKeyboardEvent *recipe = g_new(BackendKeyboardEvent, 1);
            recipe->keycode = keys[index].keycode;
            recipe->state.modifiers = modifiers;
            recipe->state.group = keys[index].group;
            recipes = g_list_append(recipes, recipe);
        }
    }
    if (keys)
        g_free(keys);

    return recipes;
}

// lookup a keysym by key event. hotkey modifiers are modifiers relevant to hotkeys
guint keymap_get_keysym(Keymap *keymap, BackendKeyboardEvent event, guint8 *consumed_modifiers)
{
    // lookup keysym and consumed modifiers
    guint keysym;
    GdkModifierType returned_consumed_modifiers;
    gdk_keymap_translate_keyboard_state(keymap->keymap, event.keycode,
                                        event.state.modifiers, event.state.group,
                                        &keysym, NULL, NULL, &returned_consumed_modifiers);

    // return consumed modifiers
    if (consumed_modifiers)
        *consumed_modifiers = keymap_physical_modifiers(keymap, returned_consumed_modifiers);

    // return
    return keysym;
}
