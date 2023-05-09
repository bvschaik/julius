#ifndef CUSTOM_MEDIA_H
#define CUSTOM_MEDIA_H

#include "core/buffer.h"
#include "scenario/message_media_text_blob.h"

typedef enum {
    CUSTOM_MEDIA_CURRENT_VERSION = 1,

    CUSTOM_MEDIA_VERSION_NONE = 0,
    CUSTOM_MEDIA_VERSION_INITIAL = 1,
} custom_media_version;

typedef enum {
    CUSTOM_MEDIA_UNDEFINED = 0,
    CUSTOM_MEDIA_SOUND = 1,
    CUSTOM_MEDIA_VIDEO = 2,
} custom_media_type;

typedef enum {
    CUSTOM_MEDIA_LINK_TYPE_UNDEFINED = 0,
    CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_MAIN = 1,
    CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_BACKGROUND_MUSIC = 2,
} custom_media_link_type;

typedef struct {
    int id;
    custom_media_type type;
    text_blob_string_t *filename;
    custom_media_link_type link_type;
    int link_id;
} custom_media_t;

void custom_media_clear(void);
custom_media_t *custom_media_get(int media_id);
custom_media_t *custom_media_create_blank(void);
custom_media_t *custom_media_create(custom_media_type type, const uint8_t *filename, custom_media_link_type link_type, int link_id);

void custom_media_save_state(buffer *buffer);
void custom_media_load_state_entry(buffer *buffer, custom_media_t *entry, custom_media_link_type *link_type, int *link_id);

#endif // CUSTOM_MEDIA_H
