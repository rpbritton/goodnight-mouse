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

#include "control.h"

#include "identify.h"
#include "execution.h"

Control *control_new(ControlType type, AtspiAccessible *accessible)
{
    Control *control = g_new(Control, 1);

    control->type = type;
    control->accessible = g_object_ref(accessible);

    control->tag = NULL;

    return control;
}

void control_destroy(gpointer control_ptr)
{
    Control *control = (Control *)control_ptr;

    g_object_unref(control->accessible);

    g_free(control);
}

void control_execute(Control *control)
{
    switch (control->type)
    {
    case CONTROL_TYPE_PRESS:
        control_execution_press(control);
        break;
    case CONTROL_TYPE_FOCUS:
        control_execution_focus(control);
        break;
    default:
        break;
    }
}

void control_set_tag(Control *control, Tag *tag)
{
    control->tag = tag;

    // todo: tag set position
}