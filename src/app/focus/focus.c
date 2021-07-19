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

#include "focus.h"

#include "window.h"

#define WINDOW_ACTIVATE_EVENT "window:activate"
#define WINDOW_DEACTIVATE_EVENT "window:deactivate"

typedef struct Subscriber
{
    FocusCallback callback;
    gpointer data;
} Subscriber;

static gint subscriber_matches_callback(gconstpointer subscriber, gconstpointer source);
static void activation_callback(AtspiEvent *event, gpointer focus_ptr);
static void deactivation_callback(AtspiEvent *event, gpointer focus_ptr);
static void notify_subscribers(Focus *focus);

Focus *focus_new()
{
    Focus *focus = g_new(Focus, 1);

    // initialize subscriber list
    focus->subscribers = NULL;

    // get first window
    focus->window = window_get_focused(focus);

    // register listeners
    focus->listener_activation = atspi_event_listener_new(activation_callback, focus, NULL);
    atspi_event_listener_register(focus->listener_activation, WINDOW_ACTIVATE_EVENT, NULL);
    focus->listener_deactivation = atspi_event_listener_new(deactivation_callback, focus, NULL);
    atspi_event_listener_register(focus->listener_deactivation, WINDOW_DEACTIVATE_EVENT, NULL);

    return focus;
}

void focus_destroy(Focus *focus)
{
    // deregister listeners
    atspi_event_listener_deregister(focus->listener_activation, WINDOW_ACTIVATE_EVENT, NULL);
    g_object_unref(focus->listener_activation);
    atspi_event_listener_deregister(focus->listener_deactivation, WINDOW_DEACTIVATE_EVENT, NULL);
    g_object_unref(focus->listener_deactivation);

    // unref window
    if (focus->window)
        g_object_unref(focus->window);

    // free subscriber lists
    g_list_free_full(focus->subscribers, g_free);

    g_free(focus);
}

void focus_subscribe(Focus *focus, FocusCallback callback, gpointer data)
{
    Subscriber *subscriber = g_new(Subscriber, 1);

    subscriber->callback = callback;
    subscriber->data = data;

    focus->subscribers = g_list_prepend(focus->subscribers, subscriber);
}

void focus_unsubscribe(Focus *focus, FocusCallback callback)
{
    GList *subscriber_node;
    while ((subscriber_node = g_list_find_custom(focus->subscribers, callback, subscriber_matches_callback)))
    {
        g_free(subscriber_node->data);
        focus->subscribers = g_list_remove_all(focus->subscribers, subscriber_node->data);
    }
}

static gint subscriber_matches_callback(gconstpointer subscriber, gconstpointer callback)
{
    // return 0 = match
    return !(((Subscriber *)subscriber)->callback == callback);
}

static void activation_callback(AtspiEvent *event, gpointer focus_ptr)
{
    Focus *focus = focus_ptr;

    // set focused window
    if (focus->window)
        g_object_unref(focus->window);
    focus->window = g_object_ref(event->source);

    const gchar *active_name = atspi_accessible_get_name(focus->window, NULL);
    g_debug("focus: Activated window '%s'", active_name);
    g_free((gpointer)active_name);

    // notify subscribers
    notify_subscribers(focus);

    // free event
    g_boxed_free(ATSPI_TYPE_EVENT, event);
}

static void deactivation_callback(AtspiEvent *event, gpointer focus_ptr)
{
    Focus *focus = focus_ptr;

    // check to make sure we should be removing the currently focused window
    if (focus->window != event->source)
    {
        g_debug("focus: Deactivated inactive window");
        g_boxed_free(ATSPI_TYPE_EVENT, event);
        return;
    }

    // set focused window
    if (focus->window)
        g_object_unref(focus->window);
    focus->window = NULL;

    g_debug("focus: Deactivated window");

    // notify subscribers
    notify_subscribers(focus);

    // free event
    g_boxed_free(ATSPI_TYPE_EVENT, event);
}

static void notify_subscribers(Focus *focus)
{
    for (GList *link = focus->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        if (focus->window)
            g_object_ref(focus->window);
        subscriber->callback(focus->window, subscriber->data);
    }
}

AtspiAccessible *focus_get_window(Focus *focus)
{
    if (focus->window)
        g_object_ref(focus->window);
    return focus->window;
}