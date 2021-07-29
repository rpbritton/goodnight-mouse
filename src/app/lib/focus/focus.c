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

static void callback_activation(AtspiEvent *event, gpointer listener_ptr);
static void callback_deactivation(AtspiEvent *event, gpointer listener_ptr);

// creates a new focus listener and starts the listening
FocusListener *focus_listener_new(FocusCallback callback, gpointer data)
{
    FocusListener *listener = g_new(FocusListener, 1);

    // add callback
    listener->callback = callback;
    listener->callback_data = data;

    // get currently focused window
    listener->window = focus_get_window();

    // register listeners
    listener->listener_activation = atspi_event_listener_new(callback_activation, listener, NULL);
    atspi_event_listener_register(listener->listener_activation, WINDOW_ACTIVATE_EVENT, NULL);
    listener->listener_deactivation = atspi_event_listener_new(callback_deactivation, listener, NULL);
    atspi_event_listener_register(listener->listener_deactivation, WINDOW_DEACTIVATE_EVENT, NULL);

    return listener;
}

// destroys a focus listener and stops the listening
void focus_listener_destroy(FocusListener *listener)
{
    // deregister listeners
    atspi_event_listener_deregister(listener->listener_activation, WINDOW_ACTIVATE_EVENT, NULL);
    g_object_unref(listener->listener_activation);
    atspi_event_listener_deregister(listener->listener_deactivation, WINDOW_DEACTIVATE_EVENT, NULL);
    g_object_unref(listener->listener_deactivation);

    // unref window
    if (listener->window)
        g_object_unref(listener->window);

    g_free(listener);
}

// handles a window activation event
static void callback_activation(AtspiEvent *event, gpointer listener_ptr)
{
    FocusListener *listener = listener_ptr;

    // make sure active window is different
    if (listener->window == event->source)
    {
        g_boxed_free(ATSPI_TYPE_EVENT, event);
        return;
    }

    // set focused window
    if (listener->window)
        g_object_unref(listener->window);
    listener->window = g_object_ref(event->source);

    // callback with updated window
    listener->callback(listener->window, listener->callback_data);

    // free event
    g_boxed_free(ATSPI_TYPE_EVENT, event);
}

// handles a window deactivation event
static void callback_deactivation(AtspiEvent *event, gpointer listener_ptr)
{
    FocusListener *listener = listener_ptr;

    // make sure deactivating window is the same
    if (listener->window != event->source)
    {
        g_boxed_free(ATSPI_TYPE_EVENT, event);
        return;
    }

    // set focused window
    if (listener->window)
        g_object_unref(listener->window);
    listener->window = NULL;

    // callback with updated window
    listener->callback(listener->window, listener->callback_data);

    // free event
    g_boxed_free(ATSPI_TYPE_EVENT, event);
}
