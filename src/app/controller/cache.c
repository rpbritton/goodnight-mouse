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

#include "cache.h"

#include "control.h"
#include "identify.h"

static void window_cache_free(gpointer window_cache_ptr);
static AtspiAccessible *accessible_get_window(AtspiAccessible *accessible);

Cache *cache_new()
{
    Cache *cache = g_new(Cache, 1);

    // init window cache
    cache->windows = g_hash_table_new_full(NULL, NULL, g_object_unref, window_cache_free);

    // create match rule
    GArray *roles = control_identify_list_roles();
    cache->match_rule = atspi_match_rule_new(NULL, ATSPI_Collection_MATCH_NONE,
                                             NULL, ATSPI_Collection_MATCH_NONE,
                                             roles, ATSPI_Collection_MATCH_ANY,
                                             NULL, ATSPI_Collection_MATCH_NONE,
                                             FALSE);
    g_array_unref(roles);

    return cache;
}

static void window_cache_free(gpointer window_cache_ptr)
{
    GHashTable *window_cache = (GHashTable *)window_cache_ptr;
    g_hash_table_remove_all(window_cache);
    g_hash_table_unref(window_cache);
}

void cache_destroy(Cache *cache)
{
    // free window caches
    g_hash_table_remove_all(cache->windows);
    g_hash_table_unref(cache->windows);

    // free match rule
    g_object_unref(cache->match_rule);

    g_free(cache);
}

GArray *cache_list(Cache *cache, AtspiAccessible *window)
{
    GHashTable *window_cache = g_hash_table_lookup(cache->windows, window);
    if (!window_cache)
        return NULL;

    GArray *cache_list = g_array_sized_new(FALSE, FALSE, sizeof(Control *), g_hash_table_size(window_cache));

    // todo: check if showing (alternative, maintain interactivity in cache)
    GHashTableIter iter;
    g_hash_table_iter_init(&iter, window_cache);
    gpointer accessible_ptr, null_ptr;
    while (g_hash_table_iter_next(&iter, &accessible_ptr, &null_ptr))
    {
        Control *control = control_new((AtspiAccessible *)accessible_ptr);
        g_array_append_val(cache_list, control);
    }

    return cache_list;
}

void cache_add(Cache *cache, AtspiAccessible *accessible)
{
    // get the current window cache
    AtspiAccessible *window = accessible_get_window(accessible);
    if (!window)
        return;
    GHashTable *window_cache = g_hash_table_lookup(cache->windows, window);
    g_object_unref(window);
    if (!window_cache)
        return;

    // check the accessible itself
    if (control_identify_type(accessible) != CONTROL_TYPE_NONE)
        g_hash_table_add(window_cache, g_object_ref(accessible));

    // get collection interface for children
    AtspiCollection *collection = atspi_accessible_get_collection_iface(accessible);
    if (!collection)
    {
        g_warning("controller: Collection is NULL");
        return;
    }

    // search for matching accessibles
    GArray *accessibles = atspi_collection_get_matches(collection,
                                                       cache->match_rule,
                                                       ATSPI_Collection_SORT_ORDER_CANONICAL,
                                                       0, TRUE, NULL);
    g_object_unref(collection);
    if (!accessibles)
    {
        g_warning("controller: Accessibles is NULL");
        return;
    }

    // add returned accessibles
    for (gint accessible_index = 0; accessible_index < accessibles->len; accessible_index++)
    {
        AtspiAccessible *child_accessible = g_array_index(accessibles, AtspiAccessible *, accessible_index);

        if (!g_hash_table_contains(window_cache, child_accessible))
            g_hash_table_add(window_cache, child_accessible);
        else
            g_object_unref(child_accessible);
    }

    g_array_unref(accessibles);
}

void cache_add_window(Cache *cache, AtspiAccessible *window)
{
    if (g_hash_table_contains(cache->windows, window))
        return;

    GHashTable *window_cache = g_hash_table_new_full(NULL, NULL, g_object_unref, NULL);
    g_hash_table_insert(cache->windows, g_object_ref(window), window_cache);
    cache_add(cache, window);
}

void cache_remove(Cache *cache, AtspiAccessible *accessible)
{
    // todo: remove children? Maybe just check them all
    g_message("I need to remove something... but what?");
}

static AtspiAccessible *accessible_get_window(AtspiAccessible *accessible)
{
    // check if passed accessible is root
    AtspiAccessible *desktop = atspi_get_desktop(0);
    if (accessible == desktop)
    {
        g_object_unref(desktop);
        return NULL;
    }

    // check if passed accessible is an application
    AtspiAccessible *application = atspi_accessible_get_parent(accessible, NULL);
    if (application == desktop)
    {
        g_object_unref(desktop);
        g_object_unref(application);
        return NULL;
    }
    else if (!application)
    {
        g_object_unref(desktop);
        return NULL;
    }

    AtspiAccessible *window = g_object_ref(accessible);
    while (TRUE)
    {
        // check if application parent is the desktop
        AtspiAccessible *application_parent = atspi_accessible_get_parent(application, NULL);
        if (application_parent == desktop)
        {
            g_object_unref(application_parent);
            break;
        }
        else if (!application_parent)
        {
            g_object_unref(window);
            window = NULL;
            break;
        }

        // move references up a parent
        g_object_unref(window);
        window = application;
        application = application_parent;
    }

    g_object_unref(desktop);
    g_object_unref(application);

    return window;
}