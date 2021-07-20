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

#ifndef A9067501_8E65_41AA_929E_32B0E41DBC2A
#define A9067501_8E65_41AA_929E_32B0E41DBC2A

#include <glib.h>

#include "tag_config.h"

// the configuration used to create a new code manager
typedef struct CodesConfig
{
    GArray *keys;
    gboolean consecutive_keys;
    TagConfig *tag;
} CodesConfig;

CodesConfig *codes_new_config(GKeyFile *key_file);
void codes_destroy_config(CodesConfig *config);

#endif /* A9067501_8E65_41AA_929E_32B0E41DBC2A */
