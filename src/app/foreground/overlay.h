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

#ifndef CD11B90E_5DEF_4A3C_9FA5_7626C9545641
#define CD11B90E_5DEF_4A3C_9FA5_7626C9545641

#include <glib.h>
#include <gdk/gdk.h>

#include "control.h"

typedef struct OverlayConfig
{
} OverlayConfig;

typedef struct Overlay
{
} Overlay;

Overlay *overlay_new();
void overlay_destroy(Overlay *overlay);
void overlay_add_control(Overlay *overlay, Control *control);
void overlay_remove_control(Overlay *overlay, Control *control);

#endif /* CD11B90E_5DEF_4A3C_9FA5_7626C9545641 */
