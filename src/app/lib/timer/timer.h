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

#ifndef B6C2C450_A702_4F02_BE47_E49FB0D8C23D
#define B6C2C450_A702_4F02_BE47_E49FB0D8C23D

#include <glib.h>

typedef glong Timer;

Timer timer_start();
glong timer_stop(Timer timer);

#endif /* B6C2C450_A702_4F02_BE47_E49FB0D8C23D */
