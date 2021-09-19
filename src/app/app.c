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

    // create the backend
    app->backend = backend_new();

    // create libraries
    app->keymap = keymap_new();
    app->state = state_new(app->backend, app->keymap);
    app->emulator = emulator_new(app->backend, app->keymap, app->state);
    app->keyboard = keyboard_new(app->backend, app->keymap);
    app->pointer = pointer_new(app->backend, app->keymap);
    app->focus = focus_new(app->backend);

    // create managers
    app->foreground = foreground_new(config->foreground, app->state, app->emulator,
                                     app->keyboard, app->pointer, app->focus);
    app->background = background_new(config->background, app->foreground,
                                     app->keyboard, app->focus);

    return app;
}

// destroy and free an app
void app_destroy(App *app)
{
    // free managers
    background_destroy(app->background);
    foreground_destroy(app->foreground);

    // free libraries
    focus_destroy(app->focus);
    pointer_destroy(app->pointer);
    keyboard_destroy(app->keyboard);
    emulator_destroy(app->emulator);
    state_destroy(app->state);
    keymap_destroy(app->keymap);

    // free backend
    backend_destroy(app->backend);

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
