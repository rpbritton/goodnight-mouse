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

#define REGISTRY_REFRESH_INTERVAL (250)
#define REGISTRY_REFRESH_PAUSE_COUNT (16)

static void registry_refresh(Registry *registry, gboolean quickly);
static gboolean registry_refresh_loop(gpointer registry_ptr);

static gboolean registry_check_children(Registry *registry, ControlType control_type);
static GList *registry_get_children(Registry *registry, AtspiAccessible *accessible);
static GList *registry_get_children_fallback(Registry *registry, AtspiAccessible *accessible);

static void registry_remove_missing_accessibles(Registry *registry, GHashTable *accessibles_to_keep);
static void registry_add_accessibles(Registry *registry, GArray *accessibles_to_add);

static const AtspiStateType INTERACTIVE_STATES[] = {
    ATSPI_STATE_SHOWING,
    ATSPI_STATE_VISIBLE,
    ATSPI_STATE_ENABLED,
    ATSPI_STATE_SENSITIVE,
};

#define NUM_INTERACTIVE_STATES (sizeof(INTERACTIVE_STATES) / sizeof(INTERACTIVE_STATES[0]))

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

    // refresh the registry
    registry_refresh(registry, TRUE);

    // start the refresh loop
    registry->refresh_source_id = g_timeout_add(REGISTRY_REFRESH_INTERVAL,
                                                registry_refresh_loop,
                                                registry);
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

static void registry_refresh(Registry *registry, gboolean quickly)
{
    GHashTable *accessibles_to_keep = g_hash_table_new_full(NULL, NULL, g_object_unref, NULL);
    GArray *accessibles_to_add = g_array_new(FALSE, FALSE, sizeof(AtspiAccessible *));

    // loop through queued accessibles
    GList *accessible_queue = g_list_append(NULL, g_object_ref(registry->window));
    guint counter = 0;
    while (accessible_queue)
    {
        // check to pause to process other glib events
        if (++counter % REGISTRY_REFRESH_PAUSE_COUNT == 0 || !quickly)
            while (g_main_context_iteration(NULL, FALSE))
                continue;

        // pop first accessible to check
        AtspiAccessible *accessible = accessible_queue->data;
        accessible_queue = g_list_delete_link(accessible_queue, accessible_queue);

        // mark to keep, stealing the reference
        g_hash_table_add(accessibles_to_keep, accessible);

        // identify the accessible
        ControlType control_type = identify_control(accessible);

        // add the children to the front
        if (registry_check_children(registry, control_type))
            accessible_queue = g_list_concat(registry_get_children(registry, accessible), accessible_queue);

        // don't do anything if the control type is none
        if (control_type == CONTROL_TYPE_NONE)
            continue;

        // mark to add if does yet not exist
        if (!g_hash_table_contains(registry->accessibles, accessible))
            accessibles_to_add = g_array_append_val(accessibles_to_add, accessible);
    }

    // remove accessibles that were not found in the refresh
    registry_remove_missing_accessibles(registry, accessibles_to_keep);

    // add accessibles that do not exist yet
    registry_add_accessibles(registry, accessibles_to_add);

    // free
    g_array_unref(accessibles_to_add);
    g_hash_table_unref(accessibles_to_keep);
}

static gboolean registry_refresh_loop(gpointer registry_ptr)
{
    Registry *registry = registry_ptr;

    // refresh the registry
    registry_refresh(registry, FALSE);

    // add a source to call after interval
    registry->refresh_source_id = g_timeout_add(REGISTRY_REFRESH_INTERVAL,
                                                registry_refresh_loop,
                                                registry);

    // remove this source
    return G_SOURCE_REMOVE;
}

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

        // don't add child if not interactive
        if (!is_interactive)
        {
            g_object_unref(child);
            continue;
        }

        // add the child
        children = g_list_append(children, child);
    }

    return children;
}

// remove all accessibles not found in the given hash table
static void registry_remove_missing_accessibles(Registry *registry, GHashTable *accessibles_to_keep)
{
    // check all existing accessibles
    GHashTableIter iter;
    gpointer accessible_ptr, null_ptr;
    g_hash_table_iter_init(&iter, registry->accessibles);
    while (g_hash_table_iter_next(&iter, &accessible_ptr, &null_ptr))
    {
        // do nothing if found
        if (g_hash_table_contains(accessibles_to_keep, accessible_ptr))
            continue;

        // remove if not found
        if (registry->subscriber.remove)
            registry->subscriber.remove(accessible_ptr, registry->subscriber.data);
        g_hash_table_iter_remove(&iter);
    }
}

// add accessibles to the registry and callback in a quasi-random manner
static void registry_add_accessibles(Registry *registry, GArray *accessibles_to_add)
{
    // create a quasi-random number generator
    gsl_qrng *generator = gsl_qrng_alloc(gsl_qrng_halton, 1);
    // the lowest power of 2 greater than the number of accessibles multiplied
    // by the generated value will be a unique, whole integer that can be used
    // as an index, with the exception of 0 being 0.5
    gint multiplier = 1;
    while (multiplier < accessibles_to_add->len)
        multiplier *= 2;

    // iterate through all the indexes
    for (gint count = 0; count < multiplier; count++)
    {
        double value;
        gsl_qrng_get(generator, &value);

        // get index from generator
        gint index = (gint)(value * multiplier);
        if (index >= accessibles_to_add->len)
            continue;

        // get accessible
        AtspiAccessible *accessible = g_array_index(accessibles_to_add, AtspiAccessible *, index);
        if (g_hash_table_contains(registry->accessibles, accessible))
            continue;

        // add accessible
        g_hash_table_add(registry->accessibles, g_object_ref(accessible));
        if (registry->subscriber.add)
            registry->subscriber.add(accessible, registry->subscriber.data);
    }

    // free generator
    gsl_qrng_free(generator);
}
