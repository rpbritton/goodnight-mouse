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

typedef struct Subscriber
{
    ControllerCallback callback;
    gpointer data;
} Subscriber;

static gint subscriber_matches_callback(gconstpointer subscriber, gconstpointer source);

static void window_cache_free(gpointer window_cache_ptr);
static GHashTable *window_cache_get(Controller *controller, AtspiAccessible *window);
static GArray *window_cache_list(Controller *controller, AtspiAccessible *window);
static void window_cache_add(Controller *controller, AtspiAccessible *accessible);
static void window_cache_remove(Controller *controller, AtspiAccessible *accessible);

static AtspiAccessible *accessible_get_window(AtspiAccessible *accessible);

static void focus_callback(AtspiAccessible *window, gpointer controller_ptr);

Controller *controller_new(Focus *focus)
{
    Controller *controller = g_new(Controller, 1);

    controller->focus = focus;

    // register listeners
    focus_subscribe(controller->focus, focus_callback, controller);

    // init subscriber list
    controller->subscribers = NULL;

    // init window cache
    controller->windows_cache = g_hash_table_new_full(NULL, NULL, g_object_unref, window_cache_free);

    // create match rule
    GArray *roles = control_identify_list_roles();
    controller->match_rule = atspi_match_rule_new(NULL, ATSPI_Collection_MATCH_NONE,
                                                  NULL, ATSPI_Collection_MATCH_NONE,
                                                  roles, ATSPI_Collection_MATCH_ANY,
                                                  NULL, ATSPI_Collection_MATCH_NONE,
                                                  FALSE);
    g_array_unref(roles);

    return controller;
}

void controller_destroy(Controller *controller)
{
    // deregister listeners
    focus_unsubscribe(controller->focus, focus_callback);

    // free subscriber lists
    g_slist_free_full(controller->subscribers, g_free);

    // free window caches
    g_hash_table_remove_all(controller->windows_cache);
    g_hash_table_unref(controller->windows_cache);

    g_free(controller);
}

void controller_subscribe(Controller *controller, ControllerCallback callback, gpointer data)
{
    // todo: implement
    g_warning("Controller subscription not implemented!");
    return;

    Subscriber *subscriber = g_new(Subscriber, 1);

    *subscriber = (Subscriber){
        .callback = callback,
        .data = data,
    };

    controller->subscribers = g_slist_prepend(controller->subscribers, subscriber);
}

void controller_unsubscribe(Controller *controller, ControllerCallback callback)
{
    GSList *subscriber_node;
    while ((subscriber_node = g_slist_find_custom(controller->subscribers, callback, subscriber_matches_callback)))
    {
        g_free(subscriber_node->data);
        controller->subscribers = g_slist_remove_all(controller->subscribers, subscriber_node->data);
    }
}

static gint subscriber_matches_callback(gconstpointer subscriber, gconstpointer callback)
{
    // return 0 = match
    return !(((Subscriber *)subscriber)->callback == callback);
}

GArray *controller_list(Controller *controller)
{
    AtspiAccessible *window = focus_window(controller->focus);
    if (!window)
        return NULL;

    GArray *list = window_cache_list(controller, window);

    g_object_unref(window);

    return list;
}

static void window_cache_free(gpointer window_cache_ptr)
{
    GHashTable *window_cache = (GHashTable *)window_cache_ptr;

    g_hash_table_remove_all(window_cache);
    g_hash_table_unref(window_cache);
}

static GHashTable *window_cache_get(Controller *controller, AtspiAccessible *window)
{
    gpointer window_cache_ptr = g_hash_table_lookup(controller->windows_cache, window);
    if (!window_cache_ptr)
    {
        window_cache_ptr = g_hash_table_new_full(NULL, NULL, g_object_unref, NULL);
        g_hash_table_insert(controller->windows_cache, g_object_ref(window), window_cache_ptr);
        window_cache_add(controller, window);
    }

    return g_hash_table_ref((GHashTable *)window_cache_ptr);
}

static GArray *window_cache_list(Controller *controller, AtspiAccessible *window)
{
    GHashTable *window_cache = window_cache_get(controller, window);

    GArray *window_cache_list = g_array_sized_new(FALSE, FALSE, sizeof(Control *), g_hash_table_size(window_cache));

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, window_cache);

    // todo: check if showing (alternative, maintain interactivity in cache)
    gpointer accessible_ptr, null_ptr;
    while (g_hash_table_iter_next(&iter, &accessible_ptr, &null_ptr))
    {
        Control *control = control_new((AtspiAccessible *)accessible_ptr);
        g_array_append_val(window_cache_list, control);
    }

    g_hash_table_unref(window_cache);
    return window_cache_list;
}

static void window_cache_add(Controller *controller, AtspiAccessible *accessible)
{
    // get collection interface
    AtspiCollection *collection = atspi_accessible_get_collection_iface(accessible);
    if (!collection)
    {
        g_warning("controller: Collection is NULL");
        return;
    }

    // search for matching accessibles
    GArray *accessibles = atspi_collection_get_matches(collection,
                                                       controller->match_rule,
                                                       ATSPI_Collection_SORT_ORDER_CANONICAL,
                                                       0, TRUE, NULL);
    g_object_unref(collection);
    if (!accessibles)
    {
        g_warning("controller: Accessibles is NULL");
        return;
    }

    // get reference to list of current window cache
    AtspiAccessible *window = accessible_get_window(accessible);
    GHashTable *window_cache = window_cache_get(controller, window);

    // add returned accessibles
    for (gint accessible_index = 0; accessible_index < accessibles->len; accessible_index++)
    {
        AtspiAccessible *child_accessible = g_array_index(accessibles, AtspiAccessible *, accessible_index);

        if (!g_hash_table_contains(window_cache, child_accessible))
            g_hash_table_add(window_cache, child_accessible);
        else
            g_object_unref(child_accessible);
    }

    g_object_unref(window);
    g_hash_table_unref(window_cache);
    g_array_unref(accessibles);
}

static void window_cache_remove(Controller *controller, AtspiAccessible *accessible)
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

        // move references up a parent
        g_object_unref(window);
        window = application;
        application = application_parent;
    }

    g_object_unref(desktop);
    g_object_unref(application);

    return window;
}

static void focus_callback(AtspiAccessible *window, gpointer controller_ptr)
{
    if (!window)
        return;

    Controller *controller = (Controller *)controller_ptr;

    if (!g_hash_table_contains(controller->windows_cache, window))
        window_cache_add(controller, window);

    g_object_unref(window);
}