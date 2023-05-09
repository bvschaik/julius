#ifndef CUSTOM_MESSAGES_H
#define CUSTOM_MESSAGES_H

#include "core/buffer.h"
#include "scenario/custom_media.h"
#include "scenario/message_media_text_blob.h"

typedef enum {
    CUSTOM_MESSAGES_CURRENT_VERSION = 1,

    CUSTOM_MESSAGES_VERSION_NONE = 0,
    CUSTOM_MESSAGES_VERSION_INITIAL = 1,
} custom_messages_version;

typedef struct {
    int id;
    int in_use;
    text_blob_string_t *linked_uid;
    text_blob_string_t *title;
    text_blob_string_t *subtitle;
    text_blob_string_t *display_text;
    custom_media_t *linked_media;
    custom_media_t *linked_background_music;
} custom_message_t;

void custom_messages_clear(void);
void custom_messages_clear_all(void);
int custom_messages_get_id_by_uid(const uint8_t *message_uid);
custom_message_t *custom_messages_get(int message_id);
custom_message_t *custom_messages_create_blank(void);
custom_message_t *custom_messages_create(const uint8_t *uid_text);
int custom_messages_count(void);

void custom_messages_save_state(buffer *buffer);
void custom_messages_load_state(buffer *messages_buffer, buffer *media_buffer);

uint8_t *custom_messages_get_title(custom_message_t *message);
uint8_t *custom_messages_get_subtitle(custom_message_t *message);
uint8_t *custom_messages_get_text(custom_message_t *message);

const char *custom_messages_get_video(custom_message_t *message);
const char *custom_messages_get_audio(custom_message_t *message);
const char *custom_messages_get_background_music(custom_message_t *message);

#endif // CUSTOM_MESSAGES_H
