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

#include "app/app.h"
#include "config.h"

// passes through debug logs
static void enable_verbose_logging()
{
    const gchar *current_domains = g_getenv("G_MESSAGES_DEBUG");
    if (current_domains)
    {
        const gchar *all_domains = g_strdup_printf("%s,%s", current_domains, G_LOG_DOMAIN);
        g_setenv("G_MESSAGES_DEBUG", all_domains, FALSE);
        g_free((gpointer)all_domains);
    }
    else
        g_setenv("G_MESSAGES_DEBUG", G_LOG_DOMAIN, FALSE);
}

// main function of the executable
int main(int argc, char **argv)
{
    Config *config = config_new(argc, argv);
    if (!config)
        exit(1);

    if (config->verbose)
        enable_verbose_logging();

    App *app = app_new(config->app);

    if (config->once)
        app_run_once(app);
    else
        app_run(app);

    app_destroy(app);
    config_destroy(config);
}