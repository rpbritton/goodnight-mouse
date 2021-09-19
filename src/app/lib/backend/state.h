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

#ifndef E9963C50_E610_424C_B8CC_3D12B7365404
#define E9963C50_E610_424C_B8CC_3D12B7365404

#include <glib.h>

// state of the modifiers and group
typedef struct BackendStateEvent
{
    guint8 modifiers;
    guint8 group;
} BackendStateEvent;

#endif /* E9963C50_E610_424C_B8CC_3D12B7365404 */
