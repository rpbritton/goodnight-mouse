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

#ifndef E6235E1E_3D35_4FBB_8900_4B67D274702D
#define E6235E1E_3D35_4FBB_8900_4B67D274702D

#include <atspi/atspi.h>

#include "tag.h"

typedef enum ControlType
{
    CONTROL_TYPE_NONE,
    CONTROL_TYPE_PRESS,
    CONTROL_TYPE_FOCUS,
} ControlType;

typedef struct Control
{
    ControlType type;
    AtspiAccessible *accessible;

    Tag *tag;
} Control;

Control *control_new(ControlType type, AtspiAccessible *accessible); // todo: add argument for config
void control_destroy(gpointer control_ptr);
void control_execute(Control *control);
void control_set_tag(Control *control, Tag *tag);

#endif /* E6235E1E_3D35_4FBB_8900_4B67D274702D */
