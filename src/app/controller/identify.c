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

static gboolean accessible_is_interactive(AtspiAccessible *accessible);

ControlType control_identify_type(AtspiAccessible *accessible)
{
    if (!accessible)
        return CONTROL_TYPE_UNKNOWN;

    // check interactivity
    if (!accessible_is_interactive(accessible))
        return CONTROL_TYPE_NONINTERACTIVE;

    // demultiplex role
    AtspiRole role = atspi_accessible_get_role(accessible, NULL);
    switch (role)
    {
    case ATSPI_ROLE_PUSH_BUTTON:
    case ATSPI_ROLE_TOGGLE_BUTTON:
    case ATSPI_ROLE_COMBO_BOX:
    case ATSPI_ROLE_RADIO_BUTTON:
    case ATSPI_ROLE_SPIN_BUTTON:
    case ATSPI_ROLE_CHECK_BOX:
    case ATSPI_ROLE_CHECK_MENU_ITEM:
    case ATSPI_ROLE_LINK:
    case ATSPI_ROLE_MENU:
    case ATSPI_ROLE_MENU_ITEM:
        return CONTROL_TYPE_PRESS;
    case ATSPI_ROLE_TEXT:
    case ATSPI_ROLE_ENTRY:
    case ATSPI_ROLE_PASSWORD_TEXT:
    case ATSPI_ROLE_SLIDER:
    case ATSPI_ROLE_DIAL:
    case ATSPI_ROLE_DATE_EDITOR:
        // todo: check if already active
        return CONTROL_TYPE_FOCUS;
    default:
        return CONTROL_TYPE_UNKNOWN;
    }
}

static gboolean accessible_is_interactive(AtspiAccessible *accessible)
{
    AtspiStateSet *state_set = atspi_accessible_get_state_set(accessible);

    gboolean is_interactive = atspi_state_set_contains(state_set, ATSPI_STATE_SHOWING) &&
                              atspi_state_set_contains(state_set, ATSPI_STATE_VISIBLE) &&
                              atspi_state_set_contains(state_set, ATSPI_STATE_ENABLED) &&
                              atspi_state_set_contains(state_set, ATSPI_STATE_SENSITIVE);

    g_object_unref(state_set);

    return is_interactive;
}