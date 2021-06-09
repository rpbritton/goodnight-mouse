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

#ifndef CCEACF73_FB4F_4D02_AC64_ADE6E0BB919A
#define CCEACF73_FB4F_4D02_AC64_ADE6E0BB919A

#include <glib.h>

typedef struct LogConfig
{
    gboolean debug;
} LogConfig;

void log_setup(LogConfig config);

#endif /* CCEACF73_FB4F_4D02_AC64_ADE6E0BB919A */
