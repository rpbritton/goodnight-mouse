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

    // create match rule
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
    registry->controls_fresh = g_hash_table_new_full(NULL, NULL, g_object_unref, NULL);

    return registry;
}

void registry_destroy(Registry *registry)
{
    // clean up
    registry_reset(registry);

    // free members
    g_object_unref(registry->match_interactive);

    g_hash_table_unref(registry->controls);
    g_hash_table_unref(registry->controls_fresh);

    g_free(registry);
}

void registry_reset(Registry *registry)
{
    // free window
    if (registry->window)
    {
        g_object_unref(registry->window);
        registry->window = NULL;
    }

    // remove all controls
    g_hash_table_remove_all(registry->controls);
    g_hash_table_remove_all(registry->controls_fresh);
}

void registry_watch(Registry *registry, AtspiAccessible *window)
{
    // clean up
    registry_reset(registry);

    // do nothing if no window
    if (!window)
        return;

    // set up
    registry->window = g_object_ref(window);
    // todo: add idle interval to refresh

    // fetch new controls
    registry_refresh(registry);
}

static void registry_refresh(Registry *registry)
{
    g_hash_table_remove_all(registry->controls_fresh);

    // refresh the registry
    registry_refresh_recurse(registry, registry->window);

    // remove missing controls
    GHashTableIter iter;
    gpointer accessible_ptr, control_ptr;
    g_hash_table_iter_init(&iter, registry->controls);
    while (g_hash_table_iter_next(&iter, &accessible_ptr, &control_ptr))
    {
        if (g_hash_table_contains(registry->controls_fresh, accessible_ptr))
            continue;

        registry->callback_remove(control_ptr, registry->callback_data);
        g_hash_table_remove(registry->controls, accessible_ptr);
    }
}

static void registry_refresh_recurse(Registry *registry, AtspiAccessible *accessible)
{
    // add accessible
    ControlType control_type = control_identify_type(accessible);
    if (control_type != CONTROL_TYPE_NONE)
    {
        // create if new
        if (!g_hash_table_contains(registry->controls, accessible))
        {
            Control *control = control_new(control_type, accessible);
            g_hash_table_insert(registry->controls, g_object_ref(accessible), control);
            registry->callback_add(control, registry->callback_data);
        }

        // mark as refreshed
        g_hash_table_add(registry->controls_fresh, g_object_ref(accessible));
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

guint registry_count(Registry *registry)
{
    return g_hash_table_size(registry->controls);
}