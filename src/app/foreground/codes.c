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

static void array_destroy(gpointer array_ptr)
{
    g_array_unref(array_ptr);
}

static void codes_reset(Codes *codes);

//static guint key_from_index(GArray *keys, guint index);
//static guint key_to_index(GArray *keys, guint key);

Codes *codes_new(GArray *keys)
{
    Codes *codes = g_new(Codes, 1);

    // set up code generator
    codes->keys = g_array_copy(keys);
    codes->code_prefix = g_array_new(FALSE, FALSE, sizeof(guint));
    codes->key_index = 0;

    // init codes
    codes->codes = NULL;
    codes->codes_unused = NULL;
    codes->codes_to_controls = g_hash_table_new(NULL, NULL);

    return codes;
}

void codes_destroy(Codes *codes)
{
    // free codes
    g_list_free_full(codes->codes, array_destroy);
    g_list_free(codes->codes_unused);
    g_hash_table_unref(codes->codes_to_controls);

    // free code generator
    g_array_unref(codes->keys);
    g_array_unref(codes->code_prefix);

    g_free(codes);
}

static void codes_reset(Codes *codes)
{
    // reset code generator
    codes->code_prefix = g_array_remove_range(codes->code_prefix, 0, codes->code_prefix->len);
    codes->key_index = 0;

    // clear codes
    g_list_free_full(codes->codes, array_destroy);
    codes->codes = NULL;
    g_list_free(codes->codes_unused);
    codes->codes_unused = NULL;
    g_hash_table_remove_all(codes->codes_to_controls);
}

void codes_add(Codes *codes, Control *control)
{
    // check for unused mapping
    if (codes->codes_unused)
    {
        // get the unused mapping
        GArray *code = codes->codes_unused->data;
        codes->codes_unused = g_list_delete_link(codes->codes_unused, codes->codes_unused);

        // set the code
        g_hash_table_insert(codes->codes_to_controls, code, control);
        control_label(control, code);
    }

    // update the prefix if out of keys
    if (codes->key_index == codes->keys->len)
    {
        // remove the first tag
        GArray *code = codes->codes->data;
        codes->codes = g_list_delete_link(codes->codes, codes->codes);

        // use the first code as the new prefix
        g_array_unref(codes->code_prefix);
        codes->code_prefix = g_array_copy(code);

        // reset the key index
        codes->key_index = 0;

        // append a new key to the code
        code = g_array_append_val(code, g_array_index(codes->keys, guint, codes->key_index++));

        // set the new code
        control_label(g_hash_table_lookup(codes->codes_to_controls, code), code);

        // add code to the back
        codes->codes = g_list_append(codes->codes, code);
    }

    // create the new code
    GArray *code = g_array_append_val(g_array_copy(codes->code_prefix),
                                      g_array_index(codes->keys, guint, codes->key_index++));

    // set the new code
    g_hash_table_insert(codes->codes_to_controls, code, control);
    control_label(control, code);

    // add code to the back
    codes->codes = g_list_append(codes->codes, code);
}

void codes_remove(Codes *codes, Control *control)
{
    gboolean reset = TRUE;

    // search for the control
    GHashTableIter iter;
    gpointer code_ptr, control_ptr;
    g_hash_table_iter_init(&iter, codes->codes_to_controls);
    while (g_hash_table_iter_next(&iter, &code_ptr, &control_ptr))
    {
        // check if control matches
        if (control_ptr == control)
        {
            // unset the control code
            // todo: is this necessary?
            //control_unlabel(control);

            // mark the code as unused
            codes->codes_unused = g_list_append(codes->codes_unused, code_ptr);

            // remove the code data link
            g_hash_table_iter_remove(&iter);
        }
        else
        {
            // do not reset if other controls exist
            reset = FALSE;
        }
    }

    // if no codes are used then reset
    if (reset)
        codes_reset(codes);
}

//static guint key_from_index(GArray *keys, guint index)
//{
//    if (index >= keys->len)
//        return -1;
//    return g_array_index(keys, guint, index);
//}
//
//static guint key_to_index(GArray *keys, guint key)
//{
//    for (gint index = 0; index < keys->len; index++)
//        if (key == g_array_index(keys, guint, index))
//            return index;
//    return -1;
//}
