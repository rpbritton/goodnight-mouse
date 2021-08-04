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

#include "emulation.h"

gboolean keyboard_set_modifiers(Keyboard *keyboard, guint modifiers, gboolean lock)
{
    // unregister keyboard
    //gboolean registered = keyboard_is_registered(keyboard);
    //if (registered)
    //    keyboard_deregister(keyboard);

    // set modifiers
    AtspiKeySynthType synth_type = lock ? ATSPI_KEY_LOCKMODIFIERS : ATSPI_KEY_UNLOCKMODIFIERS;
    gboolean success = atspi_generate_keyboard_event(modifiers, NULL, synth_type, NULL);

    // reregister keyboard
    //if (registered)
    //    keyboard_register(keyboard);

    // return success
    return success;
}

gboolean keyboard_press_key(Keyboard *keyboard, guint key)
{
    // unregister keyboard
    //gboolean registered = keyboard_is_registered(keyboard);
    //if (registered)
    //    keyboard_deregister(keyboard);

    // press key
    gboolean success = atspi_generate_keyboard_event(key, NULL, ATSPI_KEY_SYM, NULL);

    // reregister keyboard
    //if (registered)
    //    keyboard_register(keyboard);

    // return success
    return success;
}
