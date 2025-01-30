#include "message_media_text_blob.h"

#include "core/log.h"
#include "core/encoding.h"
#include "core/string.h"
#include "scenario/custom_messages.h"
#include "scenario/custom_media.h"
#include "scenario/scenario.h"

#include <stdlib.h>
#include <string.h>

#define MESSAGE_MEDIA_TEXT_BLOB_INITIAL_SIZE (4 * 1024 * 1024)
#define MESSAGE_MEDIA_TEXT_BLOB_SIZE_INCREASE_OVERSHOOT (1 * 1024 * 1024)
#define MESSAGE_MEDIA_TEXT_BLOB_INITIAL_ENTRIES 1000
#define MESSAGE_MEDIA_TEXT_BLOB_ENTRIES_INCREASE_OVERSHOOT 100

static message_media_text_blob_t message_media_text_blob;

static void resize_text_blob(int needed_space)
{
    if (!message_media_text_blob.max_size_text_blob) {
        int size = MESSAGE_MEDIA_TEXT_BLOB_INITIAL_SIZE * sizeof(uint8_t);
        message_media_text_blob.text_blob = (uint8_t *) malloc(size);
        message_media_text_blob.max_size_text_blob = size;
    }
    int needed_max_size = message_media_text_blob.size + needed_space;
    if (message_media_text_blob.max_size_text_blob < needed_max_size) {
        int size = (needed_max_size + MESSAGE_MEDIA_TEXT_BLOB_SIZE_INCREASE_OVERSHOOT) * sizeof(uint8_t);
        uint8_t *new_blob = realloc(message_media_text_blob.text_blob, size);
        if (new_blob) {
            message_media_text_blob.text_blob = new_blob;
            message_media_text_blob.max_size_text_blob = size;
        } else {
            log_error("Failed to realloc the message_media_text_blob.text_blob, the game will now probably crash.", 0, 0);
        }
    }
}

static void resize_text_entries(int needed_entries)
{
    if (!message_media_text_blob.max_size_text_entries) {
        size_t size = MESSAGE_MEDIA_TEXT_BLOB_INITIAL_ENTRIES * sizeof(text_blob_string_t);
        message_media_text_blob.text_entries = (text_blob_string_t *) malloc(size);
        message_media_text_blob.max_size_text_entries = size;
    }
    int needed_max_count = message_media_text_blob.entry_count + needed_entries;
    if (message_media_text_blob.max_size_text_entries < needed_max_count * sizeof(text_blob_string_t)) {
        size_t size = (needed_max_count + MESSAGE_MEDIA_TEXT_BLOB_ENTRIES_INCREASE_OVERSHOOT) * sizeof(text_blob_string_t);
        text_blob_string_t *new_blob = realloc(message_media_text_blob.text_entries, size);
        if (new_blob) {
            message_media_text_blob.text_entries = new_blob;
            message_media_text_blob.max_size_text_entries = size;
        } else {
            log_error("Failed to realloc the message_media_text_blob.text_entries, the game will now probably crash.", 0, 0);
        }
    }
}

static int get_highest_entry_id(void)
{
    message_media_text_blob.highest_id = 0;
    for (int i = 0; i < message_media_text_blob.entry_count; i++) {
        if (message_media_text_blob.text_entries[i].id > message_media_text_blob.highest_id) {
            message_media_text_blob.highest_id = message_media_text_blob.text_entries[i].id;
        }
    }

    return message_media_text_blob.highest_id;
}

message_media_text_blob_t *message_media_text_get_data(void)
{
    return &message_media_text_blob;
}

void message_media_text_blob_clear(void)
{
    if (message_media_text_blob.size > 0) {
        memset(message_media_text_blob.text_blob, 0, sizeof(message_media_text_blob.size));
        message_media_text_blob.size = 0;
        free(message_media_text_blob.text_blob);
        message_media_text_blob.max_size_text_blob = 0;
        resize_text_entries(0);
    }

    if (message_media_text_blob.entry_count > 0) {
        memset(message_media_text_blob.text_entries, 0, message_media_text_blob.entry_count * sizeof(text_blob_string_t));
        message_media_text_blob.entry_count = 0;
        free(message_media_text_blob.text_entries);
        message_media_text_blob.max_size_text_entries = 0;
        resize_text_blob(0);
    }
    message_media_text_blob.highest_id = 0;
}

const text_blob_string_t *message_media_text_blob_get_entry(int id)
{
    for (int i = 0; i < message_media_text_blob.entry_count; i++) {
        if (message_media_text_blob.text_entries[i].id == id) {
            return &message_media_text_blob.text_entries[i];
        }
    }
    return 0;
}

const uint8_t *message_media_text_blob_get_text(int offset)
{
    if (!offset) {
        return 0;
    }
    return &message_media_text_blob.text_blob[offset];
}

static text_blob_string_t *create_text_blob(int length)
{
    resize_text_entries(1);
    resize_text_blob(length);
    int offset = message_media_text_blob.size;
    int index = message_media_text_blob.entry_count;

    if (offset + length >= message_media_text_blob.max_size_text_blob ||
        index * sizeof(text_blob_string_t) >= message_media_text_blob.max_size_text_entries) {
        log_error("This will overfill the message_media_text_blob. The game will now crash.", 0, 0);
    }

    message_media_text_blob.text_entries[index].id = ++message_media_text_blob.highest_id;
    message_media_text_blob.text_entries[index].index = index;
    message_media_text_blob.text_entries[index].in_use = 1;
    message_media_text_blob.text_entries[index].length = length;
    message_media_text_blob.text_entries[index].offset = offset;

    message_media_text_blob.text_entries[index].text = &message_media_text_blob.text_blob[offset];

    message_media_text_blob.entry_count++;
    message_media_text_blob.size += length;

    return &message_media_text_blob.text_entries[index];
}

const text_blob_string_t *message_media_text_blob_add(const uint8_t *text)
{
    int length = string_length(text) + 1; // +1 to allow for end of string.
    text_blob_string_t *entry = create_text_blob(length);

    string_copy(text, entry->text, length);

    return entry;
}

const text_blob_string_t *message_media_text_blob_add_encoded(const char *text)
{
    int length = (int) (sizeof(uint8_t) * (strlen(text) + 1));
    uint8_t *output = malloc(length);
    if (!output) {
        log_error("Unable to create temporary output - out of memory.", 0, 0);
        return 0;
    }
    encoding_from_utf8(text, output, length);
    length = string_length(output) + 1; // +1 to allow for end of string.
    text_blob_string_t *entry = create_text_blob(length);

    string_copy(output, entry->text, length);

    free(output);

    return entry;
}

void message_media_text_blob_save_state(buffer *blob_buffer, buffer *meta_buffer)
{
    message_media_text_blob_remove_unused();
    uint32_t array_size = message_media_text_blob.size;
    uint32_t struct_size = sizeof(uint8_t);
    buffer_init_dynamic_array(blob_buffer, array_size, struct_size);

    if (array_size) {
        buffer_write_raw(blob_buffer, message_media_text_blob.text_blob, array_size);
    }

    array_size = message_media_text_blob.entry_count;
    struct_size = (3 * sizeof(int32_t));
    buffer_init_dynamic_array(meta_buffer, array_size, struct_size);

    for (int i = 0; i < array_size; i++) {
        buffer_write_i32(meta_buffer, message_media_text_blob.text_entries[i].id);
        buffer_write_i32(meta_buffer, message_media_text_blob.text_entries[i].offset);
        buffer_write_i32(meta_buffer, message_media_text_blob.text_entries[i].length);
    }
}

void message_media_text_blob_load_state(buffer *blob_buffer, buffer *meta_buffer)
{
    unsigned int size = buffer_load_dynamic_array(blob_buffer);

    message_media_text_blob_clear();
    resize_text_blob(size);

    message_media_text_blob.size = size;
    buffer_read_raw(blob_buffer, message_media_text_blob.text_blob, message_media_text_blob.size);

    size = buffer_load_dynamic_array(meta_buffer);

    resize_text_entries(size);
    message_media_text_blob.entry_count = size;
    for (unsigned int i = 0; i < size; i++) {
        message_media_text_blob.text_entries[i].id = buffer_read_i32(meta_buffer);
        message_media_text_blob.text_entries[i].index = i;
        message_media_text_blob.text_entries[i].in_use = 1;
        int offset = buffer_read_i32(meta_buffer);
        message_media_text_blob.text_entries[i].offset = offset;
        message_media_text_blob.text_entries[i].length = buffer_read_i32(meta_buffer);
        message_media_text_blob.text_entries[i].text = &message_media_text_blob.text_blob[offset];
    }
    get_highest_entry_id();
}

static void update_text_blob_link(int text_id, text_blob_string_t *new_text_link)
{
    if (custom_messages_relink_text_blob(text_id, new_text_link)) {
        return;
    } else if (custom_media_relink_text_blob(text_id, new_text_link)) {
        return;
    } else {
        log_error("update_text_blob_link -> Failed to find old link to update, the game will now probably crash.", 0, 0);
    }
}

void message_media_text_blob_mark_entry_as_unused(const text_blob_string_t *entry)
{
    if (!entry || !entry->in_use) {
        return;
    }

    message_media_text_blob.text_entries[entry->index].in_use = 0;
}

void message_media_text_blob_remove_unused(void)
{
    int unused_count = 0;
    for (int i = 0; i < message_media_text_blob.entry_count; i++) {
        if (message_media_text_blob.text_entries[i].in_use == 0) {
            unused_count++;
        }
    }
    if (unused_count == 0) {
        return;
    }

    text_blob_string_t *new_text_entries = (text_blob_string_t *) malloc(message_media_text_blob.max_size_text_entries);
    uint8_t *new_text_blob = (uint8_t *) malloc(message_media_text_blob.max_size_text_blob);

    int index = 0;
    int new_size = 0;
    int new_offset = 0;
    for (int i = 0; i < message_media_text_blob.entry_count; i++) {
        if (message_media_text_blob.text_entries[i].in_use == 0) {
            continue;
        }
        int text_id = message_media_text_blob.text_entries[i].id;
        new_text_entries[index].id = text_id;
        new_text_entries[index].index = index;
        new_text_entries[index].in_use = 1;
        int length = message_media_text_blob.text_entries[i].length;
        int offset = message_media_text_blob.text_entries[i].offset;
        new_text_entries[index].length = length;
        new_text_entries[index].offset = new_offset;

        string_copy(&message_media_text_blob.text_blob[offset], &new_text_blob[new_offset], length);
        new_text_entries[index].text = &new_text_blob[new_offset];
        new_size += length;
        new_offset += length;

        update_text_blob_link(text_id, &new_text_entries[index]);

        index++;
    }
    
    free(message_media_text_blob.text_entries);
    free(message_media_text_blob.text_blob);

    message_media_text_blob.text_blob = new_text_blob;
    message_media_text_blob.text_entries = new_text_entries;
    message_media_text_blob.entry_count = index;
    message_media_text_blob.size = new_size;
}
