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

#ifndef EBBD1A8E_931A_4028_8044_D10EFEDD3A27
#define EBBD1A8E_931A_4028_8044_D10EFEDD3A27

#include <gdk/gdk.h>

// background configuration
typedef struct BackgroundConfig
{
    guint keysym;
    GdkModifierType modifiers;
} BackgroundConfig;

BackgroundConfig *background_new_config(GKeyFile *key_file);
void background_destroy_config(BackgroundConfig *config);

#endif /* EBBD1A8E_931A_4028_8044_D10EFEDD3A27 */
