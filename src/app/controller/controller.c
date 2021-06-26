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

#include "controller.h"

#include "identify.h"

#define EVENT_CHILD_ADD "object:children-changed:add"
#define EVENT_CHILD_REMOVE "object:children-changed:remove"

static GList *recursively_get_controls(Controller *controller, AtspiAccessible *accessible);

Controller *controller_new(Focus *focus)
{
    Controller *controller = g_new(Controller, 1);

    controller->focus = focus;

    // create match rules
    AtspiStateSet *interactive_states = atspi_state_set_new(NULL);
    atspi_state_set_add(interactive_states, ATSPI_STATE_SHOWING);
    atspi_state_set_add(interactive_states, ATSPI_STATE_VISIBLE);
    atspi_state_set_add(interactive_states, ATSPI_STATE_ENABLED);
    atspi_state_set_add(interactive_states, ATSPI_STATE_SENSITIVE);
    controller->match_interactive = atspi_match_rule_new(interactive_states, ATSPI_Collection_MATCH_ALL,
                                                         NULL, ATSPI_Collection_MATCH_NONE,
                                                         NULL, ATSPI_Collection_MATCH_NONE,
                                                         NULL, ATSPI_Collection_MATCH_NONE,
                                                         FALSE);
    g_object_unref(interactive_states);

    return controller;
}

void controller_destroy(Controller *controller)
{
    // free match rules
    g_object_unref(controller->match_interactive);

    g_free(controller);
}

GList *controller_list(Controller *controller)
{
    AtspiAccessible *window = focus_window(controller->focus);
    if (!window)
        return NULL;

    GList *list = recursively_get_controls(controller, window);

    g_object_unref(window);

    return list;
}

static GList *recursively_get_controls(Controller *controller, AtspiAccessible *accessible)
{
    GList *list = NULL;

    // check if accessible is control
    ControlType control_type = control_identify_type(accessible);
    if (control_type != CONTROL_TYPE_NONE)
        list = g_list_append(list, accessible);

    // get collection
    AtspiCollection *collection = atspi_accessible_get_collection_iface(accessible);
    if (!collection)
    {
        g_warning("controller: Collection is NULL");
        return list;
    }

    // process children that are interactive
    GArray *children_interactive = atspi_collection_get_matches(collection,
                                                                controller->match_interactive,
                                                                ATSPI_Collection_SORT_ORDER_CANONICAL,
                                                                0, FALSE, NULL);
    for (gint index = 0; index < children_interactive->len; index++)
    {
        AtspiAccessible *child = g_array_index(children_interactive, AtspiAccessible *, index);

        list = g_list_concat(list, recursively_get_controls(controller, child));

        g_object_unref(child);
    }
    g_array_unref(children_interactive);
    g_object_unref(collection);

    return list;
}