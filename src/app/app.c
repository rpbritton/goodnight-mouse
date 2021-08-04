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

#include "app.h"

#include <glib-unix.h>
#include <gtk/gtk.h>
#include <atspi/atspi.h>

static gboolean signal_quit(gpointer app_ptr);

// create a new app from the configuration
App *app_new(AppConfig *config)
{
    App *app = g_new(App, 1);

    // initialize common libraries
    gtk_init(NULL, NULL);
    atspi_init();

    // add signal subscription
    app->signal_sighup = g_unix_signal_add(SIGHUP, signal_quit, app);
    app->signal_sigint = g_unix_signal_add(SIGINT, signal_quit, app);
    app->signal_sigterm = g_unix_signal_add(SIGTERM, signal_quit, app);

    // create listeners
    app->mouse_listener = mouse_listener_new();
    app->focus_listener = focus_listener_new();

    // create managers
    app->foreground = foreground_new(config->foreground, app->mouse_listener, app->focus_listener);
    app->background = background_new(config->background, app->foreground, app->focus_listener);

    return app;
}

// destroy and free an app
void app_destroy(App *app)
{
    // free managers
    background_destroy(app->background);
    foreground_destroy(app->foreground);

    // free listeners
    mouse_listener_destroy(app->mouse_listener);
    focus_listener_destroy(app->focus_listener);

    // remove signal subscription
    g_source_remove(app->signal_sighup);
    g_source_remove(app->signal_sigint);
    g_source_remove(app->signal_sigterm);

    // exit common libraries
    atspi_exit();

    g_free(app);
}

// run an app's background
void app_run(App *app)
{
    g_debug("app: Running loop");
    background_run(app->background);
}

// run the app's foreground
void app_run_once(App *app)
{
    g_debug("app: Running once");
    foreground_run(app->foreground);
}

// returns whether the app is currently running
gboolean app_is_running(App *app)
{
    return background_is_running(app->background) ||
           foreground_is_running(app->foreground);
}

// quit the app if running
void app_quit(App *app)
{
    background_quit(app->background);
    foreground_quit(app->foreground);
}

// callback for a signal to quit has been received
static gboolean signal_quit(gpointer app_ptr)
{
    App *app = (App *)app_ptr;

    app_quit(app);

    return G_SOURCE_CONTINUE;
}
