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

static GArray *codes_next_code(Codes *codes);
static void codes_reset(Codes *codes);
static void codes_apply_code(Codes *codes);

// create a new codes manager
Codes *codes_new(CodesConfig *config)
{
    Codes *codes = g_new(Codes, 1);

    // init current code
    codes->code = g_array_new(FALSE, FALSE, sizeof(guint));

    // save tag config
    codes->tag_config = config->tag;

    // set up code generator
    codes->keys = g_array_copy(config->keys);
    codes->code_prefix = g_array_new(FALSE, FALSE, sizeof(guint));
    codes->key_index = 0;
    codes->consecutive_keys = config->consecutive_keys;

    // init tags
    codes->tags = NULL;
    codes->tags_used = g_hash_table_new(NULL, NULL);
    codes->tags_unused = NULL;

    return codes;
}

// destroy a codes manager
void codes_destroy(Codes *codes)
{
    // free tags
    g_list_free_full(codes->tags, (GDestroyNotify)tag_destroy);
    g_hash_table_unref(codes->tags_used);
    g_list_free(codes->tags_unused);

    // free code generator
    g_array_unref(codes->keys);
    g_array_unref(codes->code_prefix);

    // free current code
    g_array_unref(codes->code);

    g_free(codes);
}

// create a new tag with a unique code
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
    Tag *tag = tag_new(codes->tag_config);

    // create and set the new code
    GArray *code = codes_next_code(codes);
    tag_set_code(tag, code);
    g_array_unref(code);

    // add tag to the list
    codes->tags = g_list_append(codes->tags, tag);
    g_hash_table_add(codes->tags_used, tag);

    // return new tag
    return tag;
}

// creates a new code using a the code generator
static GArray *codes_next_code(Codes *codes)
{
    // use the first code as the new prefix if out of keys
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
        GArray *code = codes_next_code(codes);
        tag_set_code(tag, code);
        g_array_unref(code);

        // readd tag to the back of the list
        codes->tags = g_list_append(codes->tags, tag);
    }

    // claim the next key
    guint next_key = g_array_index(codes->keys, guint, codes->key_index);
    codes->key_index++;

    // skip to the next code if key is will be repeated
    if (!codes->consecutive_keys && codes->code_prefix->len > 0 &&
        next_key == g_array_index(codes->code_prefix, guint, codes->code_prefix->len - 1))
        return codes_next_code(codes);

    // return the key appended to the code prefix
    return g_array_append_val(g_array_copy(codes->code_prefix), next_key);
}

// removes a tag from use which may be reused. if no tags are used
// the code generator will be reset
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

// resets a code generator, destroying all existing tags and codes
static void codes_reset(Codes *codes)
{
    // reset code generator
    codes->code_prefix = g_array_remove_range(codes->code_prefix, 0, codes->code_prefix->len);
    codes->key_index = 0;

    // clear tags
    g_list_free_full(codes->tags, (GDestroyNotify)tag_destroy);
    codes->tags = NULL;
    g_hash_table_remove_all(codes->tags_used);
    g_list_free(codes->tags_unused);
    codes->tags_unused = NULL;

    // reset current code
    codes->code = g_array_remove_range(codes->code, 0, codes->code->len);
}

// appends a key to the current code and applies it to the tags
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

// removes the last key from the current code and applies the new one to the tags
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

// applies the current code to all the tags. if no tags match the current code
// is reset
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

// returns the tag that perfectly matches the current code, otherwise NULL
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
