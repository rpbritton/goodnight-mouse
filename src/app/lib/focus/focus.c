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

static void callback_activation(AtspiEvent *event, gpointer listener_ptr);
static void callback_deactivation(AtspiEvent *event, gpointer listener_ptr);
static void notify_subscribers(Focus *focus);
static gint compare_subscriber_to_callback(gconstpointer subscriber_ptr, gconstpointer callback_ptr);

// creates a new focus focus and starts the listening
Focus *focus_new()
{
    Focus *focus = g_new(Focus, 1);

    // init subscribers
    focus->subscribers = NULL;

    // get currently focused window
    focus->window = focus_get_window_fresh();

    // register listeners
    focus->listener_activation = atspi_event_listener_new(callback_activation, focus, NULL);
    focus->listener_deactivation = atspi_event_listener_new(callback_deactivation, focus, NULL);
    g_message("focus: Registering event listener");
    atspi_event_listener_register(focus->listener_activation, WINDOW_ACTIVATE_EVENT, NULL);
    atspi_event_listener_register(focus->listener_deactivation, WINDOW_DEACTIVATE_EVENT, NULL);

    return focus;
}

// destroys a focus focus and stops the listening
void focus_destroy(Focus *focus)
{
    // deregister listeners
    g_message("focus: Deregistering event listener");
    atspi_event_listener_deregister(focus->listener_activation, WINDOW_ACTIVATE_EVENT, NULL);
    atspi_event_listener_deregister(focus->listener_deactivation, WINDOW_DEACTIVATE_EVENT, NULL);
    g_object_unref(focus->listener_activation);
    g_object_unref(focus->listener_deactivation);

    // free subscribers
    g_list_free_full(focus->subscribers, g_free);

    // unref window
    if (focus->window)
        g_object_unref(focus->window);

    g_free(focus);
}

// subscribe a callback to focus events
void focus_subscribe(Focus *focus, FocusCallback callback, gpointer data)
{
    // don't add if subscribed
    if (g_list_find_custom(focus->subscribers, callback, compare_subscriber_to_callback))
        return;

    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;

    // add subscriber
    focus->subscribers = g_list_append(focus->subscribers, subscriber);
}

// remove a callback from the subscribers
void focus_unsubscribe(Focus *focus, FocusCallback callback)
{
    // find every instance of the callback and remove
    GList *link = NULL;
    while ((link = g_list_find_custom(focus->subscribers, callback, compare_subscriber_to_callback)))
        focus->subscribers = g_list_delete_link(focus->subscribers, link);
}

// get the currently tracked window in the focus focus
AtspiAccessible *focus_get_window(Focus *focus)
{
    if (focus->window)
        g_object_ref(focus->window);
    return focus->window;
}

// handles a window activation event
static void callback_activation(AtspiEvent *event, gpointer listener_ptr)
{
    Focus *focus = listener_ptr;

    // make sure active window is different
    if (focus->window == event->source)
    {
        g_boxed_free(ATSPI_TYPE_EVENT, event);
        return;
    }

    // set focused window
    if (focus->window)
        g_object_unref(focus->window);
    focus->window = g_object_ref(event->source);

    // notify the subscribers
    notify_subscribers(focus);

    // free event
    g_boxed_free(ATSPI_TYPE_EVENT, event);
}

// handles a window deactivation event
static void callback_deactivation(AtspiEvent *event, gpointer listener_ptr)
{
    Focus *focus = listener_ptr;

    // make sure deactivating window is the same
    if (focus->window != event->source)
    {
        g_boxed_free(ATSPI_TYPE_EVENT, event);
        return;
    }

    // set focused window
    if (focus->window)
        g_object_unref(focus->window);
    focus->window = NULL;

    // notify the subscribers
    notify_subscribers(focus);

    // free event
    g_boxed_free(ATSPI_TYPE_EVENT, event);
}

// send the focused window to all subscribers
static void notify_subscribers(Focus *focus)
{
    for (GList *link = focus->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;
        subscriber->callback(focus->window, subscriber->data);
    }
}

// check if a subscriber matches a callback, returning 0 if so
static gint compare_subscriber_to_callback(gconstpointer subscriber_ptr, gconstpointer callback_ptr)
{
    return !(((Subscriber *)subscriber_ptr)->callback == callback_ptr);
}
