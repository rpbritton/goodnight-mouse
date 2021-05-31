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

#include "timeout.h"

#include <stdio.h>
#include <atspi/atspi.h>

// default timeouts, found in atspi-misc.c
#define METHOD_CALL_TIMEOUT 800
#define APP_STARTUP_TIME 15000

// timeout log message, found in dbus-pending-call.c
#define DBUS_TIMEOUT_MESSAGE "Did not receive a reply."

gboolean timeout_enabled = TRUE;
guint log_handler_id = 0;

void log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);

void timeout_enable()
{
    if (timeout_enabled)
        return;

    // fetch an accessible to call an event from
    AtspiAccessible *desktop = atspi_get_desktop(0);

    // unhide log timeout warnings
    g_log_remove_handler("dbind", log_handler_id);

    // set the default timeouts
    atspi_set_timeout(METHOD_CALL_TIMEOUT, APP_STARTUP_TIME);

    // make a call that will actually force the timeout setting
    atspi_accessible_get_id(desktop, NULL);
}

void timeout_disable()
{
    if (!timeout_enabled)
        return;

    // fetch an accessible to call an event from
    AtspiAccessible *desktop = atspi_get_desktop(0);

    // hide log timeout warnings
    log_handler_id = g_log_set_handler("dbind", G_LOG_LEVEL_WARNING, log_handler, NULL);

    // set the timeout to zero
    atspi_set_timeout(0, 0);

    // make a call that will actually force the timeout setting
    atspi_accessible_get_id(desktop, NULL);
}

void log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
    if (g_strrstr(message, DBUS_TIMEOUT_MESSAGE))
        return;

    g_log_default_handler(log_domain, log_level, message, user_data);
}