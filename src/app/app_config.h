/**
 * Copyright (C) 2021 Ryan Britton
 *
 * This file is part of goodnight_mouse.
 *
 * goodnight_mouse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * goodnight_mouse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with goodnight_mouse.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BE799D04_6948_4D6A_86DC_2285A84C1F50
#define BE799D04_6948_4D6A_86DC_2285A84C1F50

#include <glib.h>

#include "background/background_config.h"
#include "foreground/foreground_config.h"

// configuration for creating a new app
typedef struct AppConfig
{
    BackgroundConfig *background;
    ForegroundConfig *foreground;
} AppConfig;

AppConfig *app_new_config(GKeyFile *key_file);
void app_destroy_config(AppConfig *config);

#endif /* BE799D04_6948_4D6A_86DC_2285A84C1F50 */
