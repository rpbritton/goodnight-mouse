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

#include "overlay.h"

Overlay *overlay_new()
{
    Overlay *overlay = g_new(Overlay, 1);

    return overlay;
}

void overlay_destroy(Overlay *overlay)
{
    g_free(overlay);
}

void overlay_reset(Overlay *overlay)
{
}

void overlay_control_add(Overlay *overlay, Control *control)
{
}

void overlay_control_remove(Overlay *overlay, Control *control)
{
}
