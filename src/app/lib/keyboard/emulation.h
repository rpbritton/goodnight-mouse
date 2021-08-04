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

#ifndef E7A7B34D_5F98_4087_B797_6D0FA73394FC
#define E7A7B34D_5F98_4087_B797_6D0FA73394FC

#include "keyboard.h"

gboolean keyboard_set_modifiers(Keyboard *keyboard, guint modifiers, gboolean lock);
gboolean keyboard_press_key(Keyboard *keyboard, guint key);

#endif /* E7A7B34D_5F98_4087_B797_6D0FA73394FC */
