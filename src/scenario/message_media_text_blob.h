#ifndef MESSAGE_MEDIA_TEXT_BLOB_H
#define MESSAGE_MEDIA_TEXT_BLOB_H

#include "core/buffer.h"

typedef enum {
    MESSAGE_MEDIA_TEXT_BLOB_VERSION = 1,

    MESSAGE_MEDIA_TEXT_BLOB_VERSION_NONE = 0,
    MESSAGE_MEDIA_TEXT_BLOB_VERSION_INITIAL = 1,
} message_media_text_blob_version;

typedef struct {
    int id;
    int in_use;
    int index;
    int offset;
    int length;
    uint8_t *text;
} text_blob_string_t;

typedef struct {
    int size;
    int entry_count;
    text_blob_string_t *text_entries;
    uint8_t *text_blob;

    size_t max_size_text_entries;
    int max_size_text_blob;

    int highest_id;
} message_media_text_blob_t;

message_media_text_blob_t *message_media_text_get_data(void);

void message_media_text_blob_clear(void);
const text_blob_string_t *message_media_text_blob_get_entry(int id);
const uint8_t *message_media_text_blob_get_text(int offset);
const text_blob_string_t *message_media_text_blob_add(const uint8_t *text);
const text_blob_string_t *message_media_text_blob_add_encoded(const char *text);

void message_media_text_blob_save_state(buffer *blob_buffer, buffer *meta_buffer);
void message_media_text_blob_load_state(buffer *blob_buffer, buffer *meta_buffer);

void message_media_text_blob_mark_entry_as_unused(const text_blob_string_t *entry);
void message_media_text_blob_remove_unused(void);

#endif // MESSAGE_MEDIA_TEXT_BLOB_H
