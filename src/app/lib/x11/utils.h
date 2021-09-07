/**
 * Copyright (C) 2021 ryan
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

#ifndef F92D88D4_DF91_4479_A6AE_EAF82551118E
#define F92D88D4_DF91_4479_A6AE_EAF82551118E

#if USE_X11

#include <X11/Xlib.h>
#include <X11/Xatom.h>

unsigned char *get_window_property(Display *display, Window window,
                                   const char *window_property, Atom req_type);

#endif /* USE_X11 */

#endif /* F92D88D4_DF91_4479_A6AE_EAF82551118E */
