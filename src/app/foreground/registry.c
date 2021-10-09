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

#include <gsl/gsl_qrng.h>

#include "identify.h"

#define REGISTRY_REFRESH_INTERVAL (200)

static gboolean registry_refresh_source_start(gpointer registry_ptr);
static gboolean registry_refresh_source_run(gpointer registry_ptr);
static gboolean registry_refresh_iterate(Registry *registry);
static void registry_refresh_finish(Registry *registry);

static gboolean registry_check_children(Registry *registry, ControlType control_type);
static GList *registry_get_children(Registry *registry, AtspiAccessible *accessible);
static GList *registry_get_children_fallback(Registry *registry, AtspiAccessible *accessible);

static const AtspiStateType INTERACTIVE_STATES[] = {
    ATSPI_STATE_SHOWING,
    ATSPI_STATE_VISIBLE,
    ATSPI_STATE_ENABLED,
    ATSPI_STATE_SENSITIVE,
};

#define NUM_INTERACTIVE_STATES (sizeof(INTERACTIVE_STATES) / sizeof(INTERACTIVE_STATES[0]))

// create a new registry
Registry *registry_new()
{
    Registry *registry = g_new(Registry, 1);

    // init members
    registry->accessibles = g_hash_table_new_full(NULL, NULL, g_object_unref, NULL);

    // create match rule
    AtspiStateSet *interactive_states = atspi_state_set_new(NULL);
    for (gint index = 0; index < NUM_INTERACTIVE_STATES; index++)
        atspi_state_set_add(interactive_states, INTERACTIVE_STATES[index]);
    registry->match_interactive = atspi_match_rule_new(interactive_states, ATSPI_Collection_MATCH_ALL,
                                                       NULL, ATSPI_Collection_MATCH_NONE,
                                                       NULL, ATSPI_Collection_MATCH_NONE,
                                                       NULL, ATSPI_Collection_MATCH_NONE,
                                                       FALSE);
    g_object_unref(interactive_states);

    // set not watching
    registry->window = NULL;

    // init refresh iterator
    registry->accessibles_to_process = NULL;
    registry->accessibles_to_keep = g_hash_table_new_full(NULL, NULL, g_object_unref, NULL);
    registry->accessibles_to_add = g_ptr_array_new_with_free_func(g_object_unref);

    return registry;
}

// destroy the registry
void registry_destroy(Registry *registry)
{
    // unwatch
    registry_unwatch(registry);

    // free members
    g_hash_table_unref(registry->accessibles);
    g_object_unref(registry->match_interactive);

    // free refresh iterator
    g_list_free_full(registry->accessibles_to_process, g_object_unref);
    registry->accessibles_to_process = NULL;
    g_hash_table_unref(registry->accessibles_to_keep);
    g_ptr_array_unref(registry->accessibles_to_add);

    // free registry
    g_free(registry);
}

// watch a specific window
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
    registry_refresh_source_start(registry);
}

// stop the registry from watching anything
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

    // stop the refresh iterator
    g_source_remove(registry->refresh_source_id);
    g_list_free_full(registry->accessibles_to_process, g_object_unref);
    registry->accessibles_to_process = NULL;
    g_hash_table_remove_all(registry->accessibles_to_keep);
    g_ptr_array_remove_range(registry->accessibles_to_add, 0, registry->accessibles_to_add->len);
}

// start a refresh loop
static gboolean registry_refresh_source_start(gpointer registry_ptr)
{
    Registry *registry = registry_ptr;

    // add the refresh source
    registry->refresh_source_id = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
                                                  registry_refresh_source_run,
                                                  registry,
                                                  NULL);

    // remove this source
    return G_SOURCE_REMOVE;
}

// run a refresh loop
static gboolean registry_refresh_source_run(gpointer registry_ptr)
{
    Registry *registry = registry_ptr;

    // start with the window if there are no accessibles to process
    if (registry->accessibles_to_process == NULL)
        registry->accessibles_to_process = g_list_append(registry->accessibles_to_process, g_object_ref(registry->window));

    // run an iteration, continue if there is more processing required
    if (registry_refresh_iterate(registry))
        return G_SOURCE_CONTINUE;

    // finalize this refresh
    registry_refresh_finish(registry);

    // add the timeout source
    registry->refresh_source_id = g_timeout_add(REGISTRY_REFRESH_INTERVAL, registry_refresh_source_start, registry);

    // remove this source
    return G_SOURCE_REMOVE;
}

// run a single iteration of the refresh loop
static gboolean registry_refresh_iterate(Registry *registry)
{
    // ensure there are accessibles to process
    if (registry->accessibles_to_process == NULL)
        return FALSE;

    // pop first accessible to check
    AtspiAccessible *accessible = registry->accessibles_to_process->data;
    registry->accessibles_to_process = g_list_delete_link(registry->accessibles_to_process, registry->accessibles_to_process);

    // mark as processed (steals the reference) and don't process again
    if (!g_hash_table_add(registry->accessibles_to_keep, accessible))
        return (registry->accessibles_to_process != NULL);

    // identify the accessible
    ControlType control_type = identify_control(accessible);

    // add the children to the front
    if (registry_check_children(registry, control_type))
        registry->accessibles_to_process = g_list_concat(registry_get_children(registry, accessible), registry->accessibles_to_process);

    // mark to add if it is a valid control and does not already exist
    if (control_type != CONTROL_TYPE_NONE && !g_hash_table_contains(registry->accessibles, accessible))
        g_ptr_array_add(registry->accessibles_to_add, g_object_ref(accessible));

    // return whether there are more iterations to be done
    return (registry->accessibles_to_process != NULL);
}

// get whether to check the child accessibles of this control type
// todo: is this too specific (would be cleaner without?)
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

// get all the children of an accessible
static GList *registry_get_children(Registry *registry, AtspiAccessible *accessible)
{
    GList *children = NULL;

    // get collection
    AtspiCollection *collection = atspi_accessible_get_collection_iface(accessible);
    if (!collection)
        return registry_get_children_fallback(registry, accessible);

    // get interactive children
    GArray *array = atspi_collection_get_matches(collection,
                                                 registry->match_interactive,
                                                 ATSPI_Collection_SORT_ORDER_CANONICAL,
                                                 0, FALSE, NULL);
    if (!array)
        return children;

    // convert to linked list
    for (gint index = 0; index < array->len; index++)
        children = g_list_append(children, g_array_index(array, AtspiAccessible *, index));

    // clean up
    g_array_unref(array);
    g_object_unref(collection);

    // return
    return children;
}

// get all the children of an accessible by iteration, not collections
static GList *registry_get_children_fallback(Registry *registry, AtspiAccessible *accessible)
{
    GList *children = NULL;

    // check all the children manually
    for (gint index = 0; index < atspi_accessible_get_child_count(accessible, NULL); index++)
    {
        AtspiAccessible *child = atspi_accessible_get_child_at_index(accessible, index, NULL);
        if (!child)
            continue;

        // check if is interactive
        AtspiStateSet *state_set = atspi_accessible_get_state_set(child);
        gboolean is_interactive = TRUE;
        for (gint index = 0; index < NUM_INTERACTIVE_STATES; index++)
            is_interactive &= atspi_state_set_contains(state_set, INTERACTIVE_STATES[index]);
        g_object_unref(state_set);

        // add the child if it is interactive
        if (is_interactive)
            children = g_list_append(children, child);
        else
            g_object_unref(child);
    }

    return children;
}

// finalize the results of a refresh
static void registry_refresh_finish(Registry *registry)
{
    // remove any accessibles not found
    GHashTableIter iter;
    gpointer accessible_ptr, null_ptr;
    g_hash_table_iter_init(&iter, registry->accessibles);
    while (g_hash_table_iter_next(&iter, &accessible_ptr, &null_ptr))
    {
        // do nothing if found
        if (g_hash_table_contains(registry->accessibles_to_keep, accessible_ptr))
            continue;

        // remove if not found
        if (registry->subscriber.remove)
            registry->subscriber.remove(accessible_ptr, registry->subscriber.data);
        g_hash_table_iter_remove(&iter);
    }
    g_hash_table_remove_all(registry->accessibles_to_keep);

    // add all the new accessibles
    // create a quasi-random number generator
    gsl_qrng *generator = gsl_qrng_alloc(gsl_qrng_halton, 1);
    // the lowest power of 2 greater than the number of accessibles multiplied
    // by the generated value will be a unique, whole integer that can be used
    // as an index, with the exception of 0 being 0.5
    gint multiplier = 1;
    while (multiplier < registry->accessibles_to_add->len)
        multiplier *= 2;

    // iterate through all the indexes
    for (gint count = 0; count < multiplier; count++)
    {
        double value;
        gsl_qrng_get(generator, &value);

        // get index from generator
        gint index = (gint)(value * multiplier);
        if (index >= registry->accessibles_to_add->len)
            continue;

        // get accessible
        AtspiAccessible *accessible = g_ptr_array_index(registry->accessibles_to_add, index);

        // add accessible
        g_hash_table_add(registry->accessibles, g_object_ref(accessible));
        if (registry->subscriber.add)
            registry->subscriber.add(accessible, registry->subscriber.data);
    }
    g_ptr_array_remove_range(registry->accessibles_to_add, 0, registry->accessibles_to_add->len);

    // free generator
    gsl_qrng_free(generator);
}
