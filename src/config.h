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

#ifndef E72C461F_B3E0_4080_8624_9A8F366A3903
#define E72C461F_B3E0_4080_8624_9A8F366A3903

#include <glib.h>

#include "app/app.h"

typedef struct Config
{
    gboolean log_verbose;
    gboolean run_once;

    AppConfig app;
} Config;

Config *config_parse(int argc, char **argv);
void config_destroy(Config *config);

#endif /* E72C461F_B3E0_4080_8624_9A8F366A3903 */
