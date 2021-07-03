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

#include "codes.h"

typedef struct Mapping
{
    Control *control;
    GArray *code;
} Mapping;

static void mapping_free(gpointer mapping_ptr);
static gint compare_mapping_to_control(gconstpointer mapping, gconstpointer control);

static void reset_generator(Codes *codes);

static guint key_from_index(GArray *keys, guint index);
static guint key_to_index(GArray *keys, guint key);

Codes *codes_new(CodesConfig *config)
{
    Codes *codes = g_new(Codes, 1);

    // add members
    codes->keys = g_array_copy(config->keys);
    codes->code_prefix = g_array_new(FALSE, FALSE, sizeof(guint));
    codes->key_index = 0;

    codes->mappings = NULL;

    return codes;
}

void codes_destroy(Codes *codes)
{
    g_array_unref(codes->keys);
    g_array_unref(codes->code_prefix);

    g_list_free_full(codes->mappings, mapping_free);

    g_free(codes);
}

static void reset_generator(Codes *codes)
{
    codes->code_prefix = g_array_remove_range(codes->code_prefix, 0, codes->code_prefix->len);
    codes->key_index = 0;
}

void codes_control_add(Codes *codes, Control *control)
{
    // check if control already exists
    if (g_list_find_custom(codes->mappings, control, compare_mapping_to_control))
        return;

    // check for existing space
    GList *empty_link = g_list_find_custom(codes->mappings, NULL, compare_mapping_to_control);
    if (empty_link)
    {
        ((Mapping *)empty_link->data)->control = control;
        return;
    }

    // update the prefix if out of keys
    if (codes->key_index >= codes->keys->len)
    {
        // remove the first mapping
        GList *link = codes->mappings;
        Mapping *mapping = link->data;
        codes->mappings = g_list_remove_link(codes->mappings, link);

        // use the first control's code as the new prefix
        g_array_unref(codes->code_prefix);
        codes->code_prefix = mapping->code;

        // reset the key index
        codes->key_index = 0;

        // set the new code
        mapping->code = g_array_append_val(g_array_copy(codes->code_prefix),
                                           g_array_index(codes->keys, guint, codes->key_index));
        codes->key_index++;

        // re-add the mapping to the back
        codes->mappings = g_list_concat(codes->mappings, link);
    }

    // create the code mapping
    Mapping *mapping = g_new(Mapping, 1);
    mapping->control = control;
    mapping->code = g_array_append_val(g_array_copy(codes->code_prefix),
                                       g_array_index(codes->keys, guint, codes->key_index));
    codes->key_index++;

    // add the mapping
    codes->mappings = g_list_append(codes->mappings, mapping);
}

void codes_control_remove(Codes *codes, Control *control)
{
    GList *link;
    while ((link = g_list_find_custom(codes->mappings, control, compare_mapping_to_control)))
        ((Mapping *)link->data)->control = NULL;

    if (g_list_length(codes->mappings) == 0)
        reset_generator(codes);
}

void codes_key_add(Codes *codes, guint keyval)
{
    g_warning("codes: codes_key_add not implemented");
}

void codes_key_remove(Codes *codes)
{
    g_warning("codes: codes_key_remove not implemented");
}

static void mapping_free(gpointer mapping_ptr)
{
    Mapping *mapping = mapping_ptr;

    g_array_unref(mapping->code);

    g_free(mapping);
}

static gint compare_mapping_to_control(gconstpointer mapping, gconstpointer control)
{
    // return 0 if equal
    return !(((Mapping *)mapping)->control == control);
}

static guint key_from_index(GArray *keys, guint index)
{
    if (index >= keys->len)
        return -1;
    return g_array_index(keys, guint, index);
}

static guint key_to_index(GArray *keys, guint key)
{
    for (gint index = 0; index < keys->len; index++)
        if (key == g_array_index(keys, guint, index))
            return index;
    return -1;
}