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

#include "identify.h"

// from an accessible find the control type
ControlType identify_control(AtspiAccessible *accessible)
{
    // none if no accessible
    if (!accessible)
        return CONTROL_TYPE_NONE;

    // get control type from role
    ControlType control_type = CONTROL_TYPE_NONE;
    switch (atspi_accessible_get_role(accessible, NULL))
    {
    case ATSPI_ROLE_PAGE_TAB:
        control_type = CONTROL_TYPE_TAB;
        break;

    case ATSPI_ROLE_LINK:
        control_type = CONTROL_TYPE_LINK;
        break;

    default:
        // check if accessible of unknown role contains a noteworthy state
        AtspiStateSet *states = atspi_accessible_get_state_set(accessible);
        if (atspi_state_set_contains(states, ATSPI_STATE_FOCUSABLE))
            control_type = CONTROL_TYPE_FOCUSABLE;
        g_object_unref(states);
        break;
    }

    // return
    return control_type;
}
