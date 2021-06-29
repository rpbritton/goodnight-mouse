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

#include "registry.h"

#include "identify.h"

static void registry_refresh(Registry *registry);
static void registry_refresh_recurse(Registry *registry, AtspiAccessible *accessible);

Registry *registry_new(RegistryCallback add, RegistryCallback remove, gpointer data)
{
    Registry *registry = g_new(Registry, 1);

    // add callbacks
    registry->callback_add = add;
    registry->callback_remove = remove;
    registry->callback_data = data;

    // create match rules
    AtspiStateSet *interactive_states = atspi_state_set_new(NULL);
    atspi_state_set_add(interactive_states, ATSPI_STATE_SHOWING);
    atspi_state_set_add(interactive_states, ATSPI_STATE_VISIBLE);
    atspi_state_set_add(interactive_states, ATSPI_STATE_ENABLED);
    atspi_state_set_add(interactive_states, ATSPI_STATE_SENSITIVE);
    registry->match_interactive = atspi_match_rule_new(interactive_states, ATSPI_Collection_MATCH_ALL,
                                                       NULL, ATSPI_Collection_MATCH_NONE,
                                                       NULL, ATSPI_Collection_MATCH_NONE,
                                                       NULL, ATSPI_Collection_MATCH_NONE,
                                                       FALSE);
    g_object_unref(interactive_states);

    // intitialize members
    registry->window = NULL;
    registry->controls = g_hash_table_new_full(NULL, NULL, g_object_unref, control_free);

    return registry;
}

void registry_destroy(Registry *registry)
{
    // clean up watching
    registry_watch(registry, NULL);

    // free members
    g_hash_table_unref(registry->controls);

    // free match rules
    g_object_unref(registry->match_interactive);

    g_free(registry);
}

void registry_watch(Registry *registry, AtspiAccessible *window)
{
    // do nothing if same
    if (registry->window == window)
        return;

    // clean up
    if (registry->window)
        g_object_unref(registry->window);
    g_hash_table_remove_all(registry->controls);

    if (!window)
    {
        registry->window = NULL;
        return;
    }

    // set up
    registry->window = g_object_ref(window);
    // todo: add idle interval

    // fetch new controls
    registry_refresh(registry);

    g_message("length: %d", g_hash_table_size(registry->controls));
}

static void registry_refresh(Registry *registry)
{
    // mark existing controls
    GHashTableIter iter;
    gpointer accessible_ptr, control_ptr;
    g_hash_table_iter_init(&iter, registry->controls);
    while (g_hash_table_iter_next(&iter, &accessible_ptr, &control_ptr))
    {
        Control *control = control_ptr;
        control->state = CONTROL_STATE_REMOVE;
    }

    // refresh the registry
    registry_refresh_recurse(registry, registry->window);

    // remove controls
    g_hash_table_iter_init(&iter, registry->controls);
    while (g_hash_table_iter_next(&iter, &accessible_ptr, &control_ptr))
    {
        Control *control = control_ptr;
        if (control->state == CONTROL_STATE_REMOVE)
        {
            registry->callback_remove(control, registry->callback_data);
            g_hash_table_remove(registry->controls, accessible_ptr);
        }
    }

    // add controls
    g_hash_table_iter_init(&iter, registry->controls);
    while (g_hash_table_iter_next(&iter, &accessible_ptr, &control_ptr))
    {
        Control *control = control_ptr;
        if (control->state == CONTROL_STATE_ADD)
        {
            registry->callback_add(control, registry->callback_data);
            control->state = CONTROL_STATE_EXISTS;
        }
    }
}

static void registry_refresh_recurse(Registry *registry, AtspiAccessible *accessible)
{
    // add accessible
    ControlType control_type = control_identify_type(accessible);
    if (control_type != CONTROL_TYPE_NONE)
    {
        // check if exists already
        Control *existing_control = g_hash_table_lookup(registry->controls, accessible);
        if (!existing_control)
            g_hash_table_insert(registry->controls,
                                g_object_ref(accessible),
                                control_new(control_type, accessible));
        else
            existing_control->state = CONTROL_STATE_EXISTS;
    }

    // get collection
    AtspiCollection *collection = atspi_accessible_get_collection_iface(accessible);
    if (!collection)
    {
        g_warning("registry: Collection is NULL");
        return;
    }

    // process children that are interactive
    GArray *children = atspi_collection_get_matches(collection,
                                                    registry->match_interactive,
                                                    ATSPI_Collection_SORT_ORDER_CANONICAL,
                                                    0, FALSE, NULL);
    for (gint index = 0; index < children->len; index++)
    {
        AtspiAccessible *child = g_array_index(children, AtspiAccessible *, index);

        registry_refresh_recurse(registry, child);

        g_object_unref(child);
    }
    g_array_unref(children);
    g_object_unref(collection);

    return;
}