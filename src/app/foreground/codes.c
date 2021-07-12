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

static void wrap_tag_destroy(gpointer tag_ptr)
{
    tag_destroy(tag_ptr);
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

    // init tags
    codes->tags = NULL;
    codes->tags_unused = NULL;

    return codes;
}

void codes_destroy(Codes *codes)
{
    // free tags
    g_list_free_full(codes->tags, wrap_tag_destroy);
    g_list_free(codes->tags_unused);

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

    // clear tags
    g_list_free_full(codes->tags, wrap_tag_destroy);
    codes->tags = NULL;
    g_list_free(codes->tags_unused);
    codes->tags_unused = NULL;
}

Tag *codes_allocate(Codes *codes)
{
    // check for unused mapping
    if (codes->tags_unused)
    {
        // get the unused mapping
        Tag *tag = codes->tags_unused->data;
        codes->tags_unused = g_list_delete_link(codes->tags_unused, codes->tags_unused);
        return tag;
    }

    // update the prefix if out of keys
    if (codes->key_index == codes->keys->len)
    {
        // remove the first tag
        Tag *tag = codes->tags->data;
        codes->tags = g_list_delete_link(codes->tags, codes->tags);

        // use the first code as the new prefix
        g_array_unref(codes->code_prefix);
        codes->code_prefix = tag_get_code(tag);

        // reset the key index
        codes->key_index = 0;

        // create and set the new code
        GArray *code = g_array_append_val(g_array_copy(codes->code_prefix),
                                          g_array_index(codes->keys, guint, codes->key_index++));
        tag_set_code(tag, code);
        g_array_unref(code);

        // add tag to the back
        codes->tags = g_list_append(codes->tags, tag);
    }

    // create a new tag
    Tag *tag = tag_new();

    // create and set the new code
    GArray *code = g_array_append_val(g_array_copy(codes->code_prefix),
                                      g_array_index(codes->keys, guint, codes->key_index++));
    tag_set_code(tag, code);
    g_array_unref(code);

    // add tag to the back
    codes->tags = g_list_append(codes->tags, tag);

    return tag;
}

void codes_deallocate(Codes *codes, Tag *tag)
{
    // add tag to unused
    codes->tags = g_list_append(codes->tags, tag);

    // if no codes are used then reset
    if (g_list_length(codes->tags) == g_list_length(codes->tags_unused))
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
