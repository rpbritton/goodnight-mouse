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
static void registry_refresh_recurse(Registry *registry, AtspiAccessible *accessible, GHashTable *refreshed);
static gboolean registry_refresh_loop(gpointer registry_ptr);

static void wrap_control_destroy(gpointer control_ptr)
{
    control_destroy(control_ptr);
}

Registry *registry_new(ControlConfig *control_config)
{
    Registry *registry = g_new(Registry, 1);

    // add control config
    registry->control_config = control_config;

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
    registry->controls = g_hash_table_new_full(NULL, NULL, g_object_unref, wrap_control_destroy);

    return registry;
}

void registry_destroy(Registry *registry)
{
    // unwatch
    registry_unwatch(registry);

    // free members
    g_object_unref(registry->match_interactive);

    g_hash_table_unref(registry->controls);

    g_free(registry);
}

void registry_watch(Registry *registry, AtspiAccessible *window, RegistrySubscriber subscriber)
{
    // unwatch first
    registry_unwatch(registry);

    // do nothing if no window
    if (!window)
        return;

    // set new window
    registry->window = g_object_ref(window);

    // set subscriber
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
    gpointer accessible_ptr, control_ptr;
    g_hash_table_iter_init(&iter, registry->controls);
    while (g_hash_table_iter_next(&iter, &accessible_ptr, &control_ptr))
    {
        if (registry->subscriber.remove)
            registry->subscriber.remove(control_ptr, registry->subscriber.data);
        g_hash_table_iter_remove(&iter);
    }

    // stop the refresh loop
    g_source_remove(registry->refresh_source_id);
}

static void registry_refresh(Registry *registry)
{
    GHashTable *refreshed = g_hash_table_new_full(NULL, NULL, g_object_unref, NULL);

    // refresh the registry
    registry_refresh_recurse(registry, registry->window, refreshed);

    // remove non refreshed controls
    GHashTableIter iter;
    gpointer accessible_ptr, control_ptr;
    g_hash_table_iter_init(&iter, registry->controls);
    while (g_hash_table_iter_next(&iter, &accessible_ptr, &control_ptr))
    {
        if (!g_hash_table_contains(refreshed, accessible_ptr))
        {
            if (registry->subscriber.remove)
                registry->subscriber.remove(control_ptr, registry->subscriber.data);
            g_hash_table_iter_remove(&iter);
        }
    }
    g_hash_table_unref(refreshed);
}

static void registry_refresh_recurse(Registry *registry, AtspiAccessible *accessible, GHashTable *refreshed)
{
    // add if new
    if (!g_hash_table_contains(registry->controls, accessible))
    {
        ControlType control_type = control_identify_type(accessible);
        if (control_type != CONTROL_TYPE_NONE)
        {
            Control *control = control_new(control_type, accessible, registry->control_config);
            g_hash_table_insert(registry->controls, g_object_ref(accessible), control);
            if (registry->subscriber.add)
                registry->subscriber.add(control, registry->subscriber.data);
        }
    }

    // mark as refreshed
    g_hash_table_add(refreshed, g_object_ref(accessible));

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
        registry_refresh_recurse(registry, child, refreshed);
        g_object_unref(child);
    }
    g_array_unref(children);
    g_object_unref(collection);

    return;
}

static gboolean registry_refresh_loop(gpointer registry_ptr)
{
    Registry *registry = registry_ptr;

    // refresh the registry
    registry_refresh(registry);

    // add a source to call in a bit
    registry->refresh_source_id = g_timeout_add(REGISTRY_REFRESH_INTERVAL,
                                                registry_refresh_loop,
                                                registry);

    // remove this source
    return G_SOURCE_REMOVE;
}