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

#if USE_X11
#include "../x11/focus.h"
#define BACKEND(F) backend_x11_##F
#else
#include "../legacy/focus.h"
#define BACKEND(F) backend_legacy_##F
#endif

#define WINDOW_ACTIVATE_EVENT "window:activate"
#define WINDOW_DEACTIVATE_EVENT "window:deactivate"

typedef struct Subscriber
{
    FocusCallback callback;
    gpointer data;
} Subscriber;

static void set_window(AtspiAccessible *accessible, gpointer focus_ptr);
static gint compare_subscriber_to_callback(gconstpointer subscriber_ptr, gconstpointer callback_ptr);

// creates a new focus focus and starts the listening
Focus *focus_new(gpointer backend)
{
    Focus *focus = g_new(Focus, 1);

    // init subscribers
    focus->subscribers = NULL;

    // add backend
    focus->accessible = NULL;
    focus->backend = BACKEND(focus_new(backend, set_window, focus));

    return focus;
}

// destroys a focus and stops the listening
void focus_destroy(Focus *focus)
{
    // free backend
    BACKEND(focus_destroy(focus->backend));

    // free subscribers
    g_list_free_full(focus->subscribers, g_free);

    // unref window
    if (focus->accessible)
        g_object_unref(focus->accessible);

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

// get the currently focused window
AtspiAccessible *focus_get_window(Focus *focus)
{
    if (focus->accessible)
        g_object_ref(focus->accessible);
    return focus->accessible;
}

// set the focused window and send it to the subscribers
static void set_window(AtspiAccessible *accessible, gpointer focus_ptr)
{
    Focus *focus = focus_ptr;

    // set the new window
    if (focus->accessible)
        g_object_unref(focus->accessible);
    if (accessible)
        g_object_ref(accessible);
    focus->accessible = accessible;

    // notify the subscribers
    for (GList *link = focus->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;
        subscriber->callback(focus->accessible, subscriber->data);
    }
}

// check if a subscriber matches a callback, returning 0 if so
static gint compare_subscriber_to_callback(gconstpointer subscriber_ptr, gconstpointer callback_ptr)
{
    return !(((Subscriber *)subscriber_ptr)->callback == callback_ptr);
}
