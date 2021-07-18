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

#define REGISTRY_REFRESH_INTERVAL 500

static void registry_refresh(Registry *registry);
static gboolean registry_refresh_loop(gpointer registry_ptr);

static gboolean registry_check_children(Registry *registry, ControlType control_type);
static GList *registry_get_children(Registry *registry, AtspiAccessible *accessible);

Registry *registry_new()
{
    Registry *registry = g_new(Registry, 1);

    // init members
    registry->accessibles = g_hash_table_new_full(NULL, NULL, g_object_unref, NULL);

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

    // set not watching
    registry->window = NULL;

    return registry;
}

void registry_destroy(Registry *registry)
{
    // unwatch
    registry_unwatch(registry);

    // free members
    g_hash_table_unref(registry->accessibles);
    g_object_unref(registry->match_interactive);

    // free registry
    g_free(registry);
}

void registry_watch(Registry *registry, AtspiAccessible *window, RegistrySubscriber subscriber)
{
    // unwatch first
    registry_unwatch(registry);

    // do nothing if no window
    if (!window)
        return;

    // set watching members
    registry->window = g_object_ref(window);
    registry->subscriber = subscriber;

    // start the refresh loop
    registry_refresh_loop(registry);
}

void registry_unwatch(Registry *registry)
{
    // skip if not watching
    if (!registry->window)
        return;

    // dereference window
    g_object_unref(registry->window);
    registry->window = NULL;

    // remove all controls
    GHashTableIter iter;
    gpointer accessible_ptr, null_ptr;
    g_hash_table_iter_init(&iter, registry->accessibles);
    while (g_hash_table_iter_next(&iter, &accessible_ptr, &null_ptr))
    {
        if (registry->subscriber.remove)
            registry->subscriber.remove(accessible_ptr, registry->subscriber.data);
        g_hash_table_iter_remove(&iter);
    }

    // stop the refresh loop
    g_source_remove(registry->refresh_source_id);
}

static void registry_refresh(Registry *registry)
{
    GHashTable *refreshed_accessibles = g_hash_table_new_full(NULL, NULL, g_object_unref, NULL);

    // loop through queued accessibles
    GList *accessible_queue = g_list_append(NULL, g_object_ref(registry->window));
    while (accessible_queue)
    {
        // pop first accessible to check
        AtspiAccessible *accessible = accessible_queue->data;
        accessible_queue = g_list_delete_link(accessible_queue, accessible_queue);

        // add accessible as refreshed
        g_hash_table_add(refreshed_accessibles, accessible);

        // identify the accessible
        ControlType control_type = identify_control(accessible);

        // add child accessibles to the front
        if (registry_check_children(registry, control_type))
            accessible_queue = g_list_concat(registry_get_children(registry, accessible), accessible_queue);

        // add accessible
        if (control_type != CONTROL_TYPE_NONE && !g_hash_table_contains(registry->accessibles, accessible))
        {
            g_hash_table_add(registry->accessibles, g_object_ref(accessible));
            if (registry->subscriber.add)
                registry->subscriber.add(accessible, registry->subscriber.data);
        }
    }

    // remove non refreshed controls
    // todo: callback remove before callback add to better utilize codes
    GHashTableIter iter;
    gpointer accessible_ptr, null_ptr;
    g_hash_table_iter_init(&iter, registry->accessibles);
    while (g_hash_table_iter_next(&iter, &accessible_ptr, &null_ptr))
    {
        if (g_hash_table_contains(refreshed_accessibles, accessible_ptr))
            continue;

        if (registry->subscriber.remove)
            registry->subscriber.remove(accessible_ptr, registry->subscriber.data);
        g_hash_table_iter_remove(&iter);
    }
    g_hash_table_unref(refreshed_accessibles);
}

static gboolean registry_refresh_loop(gpointer registry_ptr)
{
    Registry *registry = registry_ptr;

    // refresh the registry
    registry_refresh(registry);

    // add a source to call after interval
    registry->refresh_source_id = g_timeout_add(REGISTRY_REFRESH_INTERVAL,
                                                registry_refresh_loop,
                                                registry);

    // remove this source
    return G_SOURCE_REMOVE;
}

static gboolean registry_check_children(Registry *registry, ControlType control_type)
{
    switch (control_type)
    {
    case CONTROL_TYPE_TAB:
        return FALSE;
    default:
        return TRUE;
    }
}

static GList *registry_get_children(Registry *registry, AtspiAccessible *accessible)
{
    GList *children = NULL;

    // get collection
    AtspiCollection *collection = atspi_accessible_get_collection_iface(accessible);
    if (!collection)
    {
        g_warning("registry: Collection is NULL");
        return children;
    }

    // get interactive children
    GArray *array = atspi_collection_get_matches(collection,
                                                 registry->match_interactive,
                                                 ATSPI_Collection_SORT_ORDER_CANONICAL,
                                                 0, FALSE, NULL);

    // convert to linked list
    for (gint index = 0; index < array->len; index++)
        children = g_list_append(children, g_array_index(array, AtspiAccessible *, index));

    // cleanup
    g_array_unref(array);
    g_object_unref(collection);

    // return
    return children;
}