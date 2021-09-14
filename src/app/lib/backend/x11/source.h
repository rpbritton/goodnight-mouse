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

#ifndef F8B6723E_EF60_47AB_A1E9_AE5D77A7896E
#define F8B6723E_EF60_47AB_A1E9_AE5D77A7896E

#if USE_X11

#include <glib.h>
#include <X11/Xlib.h>

GSource *x11_source_new(Display *display);

#endif /* USE_X11 */

#endif /* F8B6723E_EF60_47AB_A1E9_AE5D77A7896E */
