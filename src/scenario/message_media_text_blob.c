#include "message_media_text_blob.h"

#include "core/log.h"
#include "core/string.h"

#include <stdlib.h>
#include <string.h>

#define MESSAGE_MEDIA_TEXT_BLOB_INITIAL_SIZE (100 * 1024)
#define MESSAGE_MEDIA_TEXT_BLOB_SIZE_INCREASE_OVERSHOOT (10 * 1024)
#define MESSAGE_MEDIA_TEXT_BLOB_INITIAL_ENTRIES 100
#define MESSAGE_MEDIA_TEXT_BLOB_ENTRIES_INCREASE_OVERSHOOT 10

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
        int size = needed_max_size + MESSAGE_MEDIA_TEXT_BLOB_SIZE_INCREASE_OVERSHOOT;
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
        int size = MESSAGE_MEDIA_TEXT_BLOB_INITIAL_ENTRIES * sizeof(text_blob_string_t);
        message_media_text_blob.text_entries = (text_blob_string_t *) malloc(size);
        message_media_text_blob.max_size_text_entries = size;
    }
    int needed_max_count = message_media_text_blob.entry_count + needed_entries;
    if (message_media_text_blob.max_size_text_entries < needed_max_count * sizeof(text_blob_string_t)) {
        int size = (needed_max_count + MESSAGE_MEDIA_TEXT_BLOB_ENTRIES_INCREASE_OVERSHOOT) * sizeof(text_blob_string_t);
        text_blob_string_t *new_blob = realloc(message_media_text_blob.text_entries, size);
        if (new_blob) {
            message_media_text_blob.text_entries = new_blob;
            message_media_text_blob.max_size_text_entries = size;
        } else {
            log_error("Failed to realloc the message_media_text_blob.text_entries, the game will now probably crash.", 0, 0);
        }
    }
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
}

text_blob_string_t *message_media_text_blob_get_entry(int id)
{
    if (id < 0 || id >= message_media_text_blob.entry_count) {
        return 0;
    }
    return &message_media_text_blob.text_entries[id];
}

uint8_t *message_media_text_blob_get_text(int offset)
{
    if (!offset) {
        return 0;
    }
    return &message_media_text_blob.text_blob[offset];
}

text_blob_string_t *message_media_text_blob_add(const uint8_t *text)
{
    resize_text_entries(1);
    int length = string_length(text) + 1; // +1 to allow for end of string.
    resize_text_blob(length);
    int offset = message_media_text_blob.size;
    int index = message_media_text_blob.entry_count;

    if (offset + length >= message_media_text_blob.max_size_text_blob ||
        index * sizeof(text_blob_string_t) >= message_media_text_blob.max_size_text_entries) {
        log_error("This will overfill the message_media_text_blob. The game will now crash.", 0, 0);
    }

    message_media_text_blob.text_entries[index].id = index;
    message_media_text_blob.text_entries[index].in_use = 1;
    message_media_text_blob.text_entries[index].length = length;
    message_media_text_blob.text_entries[index].offset = offset;

    string_copy(text, &message_media_text_blob.text_blob[offset], length);
    message_media_text_blob.text_entries[index].text = &message_media_text_blob.text_blob[offset];

    message_media_text_blob.entry_count++;
    message_media_text_blob.size += length;

    return &message_media_text_blob.text_entries[index];
}

void message_media_text_blob_save_state(buffer *blob_buffer, buffer *meta_buffer)
{
    int32_t array_size = message_media_text_blob.size;
    int32_t struct_size = sizeof(uint8_t);
    buffer_init_dynamic_piece(blob_buffer,
        MESSAGE_MEDIA_TEXT_BLOB_VERSION,
        array_size,
        struct_size);

    buffer_write_raw(blob_buffer, message_media_text_blob.text_blob, array_size);

    array_size = message_media_text_blob.entry_count;
    struct_size = (3 * sizeof(int32_t));
    buffer_init_dynamic_piece(meta_buffer,
        MESSAGE_MEDIA_TEXT_BLOB_VERSION,
        array_size,
        struct_size);

    for (int i = 0; i < array_size; i++) {
        buffer_write_i32(meta_buffer, message_media_text_blob.text_entries[i].id);
        buffer_write_i32(meta_buffer, message_media_text_blob.text_entries[i].offset);
        buffer_write_i32(meta_buffer, message_media_text_blob.text_entries[i].length);
    }
}

void message_media_text_blob_load_state(buffer *blob_buffer, buffer *meta_buffer)
{
    message_media_text_blob_clear();
    resize_text_entries(0);
    resize_text_blob(0);

    int buffer_size, version, array_size, struct_size;
    buffer_load_dynamic_piece_header_data(blob_buffer,
        &buffer_size,
        &version,
        &array_size,
        &struct_size);

    message_media_text_blob.size = buffer_size;    
    buffer_read_raw(blob_buffer, message_media_text_blob.text_blob, message_media_text_blob.size);

    buffer_load_dynamic_piece_header_data(meta_buffer,
        &buffer_size,
        &version,
        &array_size,
        &struct_size);

    message_media_text_blob.entry_count = array_size;
    for (int i = 0; i < array_size; i++) {
        message_media_text_blob.text_entries[i].id = buffer_read_i32(meta_buffer);
        message_media_text_blob.text_entries[i].in_use = 1;
        int offset = buffer_read_i32(meta_buffer);
        message_media_text_blob.text_entries[i].offset = offset;
        message_media_text_blob.text_entries[i].length = buffer_read_i32(meta_buffer);
        message_media_text_blob.text_entries[i].text = &message_media_text_blob.text_blob[offset];
    }
}

