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

static GArray *codes_new_code(Codes *codes);
static void codes_wrap_code(Codes *codes);
static void codes_reset(Codes *codes);
static void codes_apply_code(Codes *codes);

Codes *codes_new(CodesConfig *config)
{
    Codes *codes = g_new(Codes, 1);

    // init current code
    codes->code = g_array_new(FALSE, FALSE, sizeof(guint));

    // save tag config
    codes->tag_config = config->tag;

    // set up code generator
    codes->keys = g_array_sized_new(FALSE, FALSE, sizeof(guint), config->keys->len);
    for (gint index = 0; index < config->keys->len; index++)
    {
        guint key = gdk_keyval_to_lower(g_array_index(config->keys, guint, index));
        g_array_append_val(codes->keys, key);
    }
    codes->code_prefix = g_array_new(FALSE, FALSE, sizeof(guint));
    codes->key_index = 0;
    codes->no_repeat = config->no_repeat;

    // init tags
    codes->tags = NULL;
    codes->tags_used = g_hash_table_new(NULL, NULL);
    codes->tags_unused = NULL;

    return codes;
}

void codes_destroy(Codes *codes)
{
    // free tags
    g_list_free_full(codes->tags, wrap_tag_destroy);
    g_hash_table_unref(codes->tags_used);
    g_list_free(codes->tags_unused);

    // free code generator
    g_array_unref(codes->keys);
    g_array_unref(codes->code_prefix);

    // free current code
    g_array_unref(codes->code);

    g_free(codes);
}

Tag *codes_allocate(Codes *codes)
{
    // check for unused mapping
    if (codes->tags_unused)
    {
        // get the unused mapping
        Tag *tag = codes->tags_unused->data;

        // mark tag as used
        codes->tags_unused = g_list_delete_link(codes->tags_unused, codes->tags_unused);
        g_hash_table_add(codes->tags_used, tag);

        // return reused tag
        return tag;
    }

    // create a new tag
    Tag *tag = tag_new(&codes->tag_config);

    // create and set the new code
    GArray *code = codes_new_code(codes);
    tag_set_code(tag, code);
    g_array_unref(code);

    // add tag to the list
    codes->tags = g_list_append(codes->tags, tag);
    g_hash_table_add(codes->tags_used, tag);

    // return new tag
    return tag;
}

static GArray *codes_new_code(Codes *codes)
{
    // wrap code if at end
    if (codes->key_index == codes->keys->len)
        codes_wrap_code(codes);

    // check for no repeat
    if (codes->no_repeat && codes->code_prefix->len > 0)
    {
        guint next_key = g_array_index(codes->keys, guint, codes->key_index);
        guint last_key = g_array_index(codes->code_prefix, guint, codes->code_prefix->len - 1);
        if (next_key == last_key)
        {
            codes->key_index++;

            // wrap code if at end
            if (codes->key_index == codes->keys->len)
                codes_wrap_code(codes);
        }
    }

    // append key to code prefix
    return g_array_append_val(g_array_copy(codes->code_prefix),
                              g_array_index(codes->keys, guint, codes->key_index++));
}

static void codes_wrap_code(Codes *codes)
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
    GArray *code = codes_new_code(codes);
    tag_set_code(tag, code);
    g_array_unref(code);

    // readd tag to the list
    codes->tags = g_list_append(codes->tags, tag);
}

void codes_deallocate(Codes *codes, Tag *tag)
{
    // remove tag from used
    gboolean found = g_hash_table_remove(codes->tags_used, tag);
    if (!found)
        return;

    // add tag to unused
    codes->tags_unused = g_list_append(codes->tags_unused, tag);

    // if no codes are used then reset
    if (g_hash_table_size(codes->tags_used) == 0)
        codes_reset(codes);
}

static void codes_reset(Codes *codes)
{
    // reset code generator
    codes->code_prefix = g_array_remove_range(codes->code_prefix, 0, codes->code_prefix->len);
    codes->key_index = 0;

    // clear tags
    g_list_free_full(codes->tags, wrap_tag_destroy);
    codes->tags = NULL;
    g_hash_table_remove_all(codes->tags_used);
    g_list_free(codes->tags_unused);
    codes->tags_unused = NULL;

    // reset current code
    codes->code = g_array_remove_range(codes->code, 0, codes->code->len);
}

void codes_add_key(Codes *codes, guint key)
{
    // convert to lower
    key = gdk_keyval_to_lower(key);

    // make sure key is valid
    gboolean key_exists = FALSE;
    for (gint index = 0; index < codes->keys->len; index++)
        if (key == g_array_index(codes->keys, guint, index))
            key_exists = TRUE;
    if (!key_exists)
        return;

    // add key
    g_array_append_val(codes->code, key);

    // apply code
    codes_apply_code(codes);
}

void codes_pop_key(Codes *codes)
{
    // make sure a key can be popped
    if (codes->code->len == 0)
        return;

    // remove last key
    codes->code = g_array_remove_index(codes->code, codes->code->len - 1);

    // apply code
    codes_apply_code(codes);
}

static void codes_apply_code(Codes *codes)
{
    // do nothing if no tags are used
    if (g_hash_table_size(codes->tags_used) == 0)
        return;

    // check each tag against the code
    gboolean valid = FALSE;
    GHashTableIter iter;
    gpointer tag_ptr, null_ptr;
    g_hash_table_iter_init(&iter, codes->tags_used);
    while (g_hash_table_iter_next(&iter, &tag_ptr, &null_ptr))
        if (tag_apply_code(tag_ptr, codes->code))
            valid = TRUE;

    // reset the code if no matches
    if (!valid)
    {
        codes->code = g_array_remove_range(codes->code, 0, codes->code->len);
        codes_apply_code(codes);
    }
}

Tag *codes_matched_tag(Codes *codes)
{
    // search the tags
    GHashTableIter iter;
    gpointer tag_ptr, null_ptr;
    g_hash_table_iter_init(&iter, codes->tags_used);
    while (g_hash_table_iter_next(&iter, &tag_ptr, &null_ptr))
        if (tag_matches_code(tag_ptr))
            return tag_ptr;

    // no tag found
    return NULL;
}