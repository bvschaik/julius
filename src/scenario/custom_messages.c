#include "custom_messages.h"

#include "core/array.h"
#include "core/encoding.h"
#include "core/file.h"
#include "core/log.h"
#include "core/string.h"

#define CUSTOM_MESSAGES_ARRAY_SIZE_STEP 100

#define AUDIO_FILE_PATHS_COUNT 4
static const char AUDIO_FILE_PATHS[][32] = {
    "community/audio/",
    "mp3/",
    "wavs/",
};

#define VIDEO_FILE_PATHS_COUNT 2
static const char VIDEO_FILE_PATHS[][32] = {
    "community/video/",
    "smk/",
};

static array(custom_message_t) custom_messages;

static struct {
    char filepath_media[FILE_NAME_MAX];
    char filepath_audio[FILE_NAME_MAX];
    char temp_filepath[FILE_NAME_MAX];
} data;

static int entry_in_use(const custom_message_t *entry)
{
    return entry->in_use != 0;
}

static void new_entry(custom_message_t *obj, int position)
{
    obj->id = position;
}

void custom_messages_clear(void)
{
    if (!array_init(custom_messages, CUSTOM_MESSAGES_ARRAY_SIZE_STEP, new_entry, entry_in_use) ||
        !array_next(custom_messages)) {
        log_error("Unable to allocate enough memory for the custom media array. The game will now crash.", 0, 0);
    }
}

void custom_messages_clear_all(void)
{
    custom_messages_clear();
    custom_media_clear();
    message_media_text_blob_clear();
}

custom_message_t *custom_messages_get(int message_id)
{
    return array_item(custom_messages, message_id);
}

int custom_messages_get_id_by_uid(const uint8_t *message_uid)
{
    custom_message_t *entry;
    array_foreach(custom_messages, entry) {
        if (entry && entry->linked_uid) {
            const uint8_t *current_uid = entry->linked_uid->text;
            if (string_equals(current_uid, message_uid)) {
                return i;
            }
        }
    }
    return 0;
}

custom_message_t *custom_messages_create_blank(void)
{
    custom_message_t *entry = 0;
    array_new_item(custom_messages, 1, entry);
    if (!entry) {
        return 0;
    }
    entry->in_use = 1;

    return entry;
}

custom_message_t *custom_messages_create(const uint8_t *uid_text)
{
    int duplicate_message_id = custom_messages_get_id_by_uid(uid_text);
    if (duplicate_message_id) {
        return 0;
    }

    custom_message_t *entry = custom_messages_create_blank();
    entry->linked_uid = message_media_text_blob_add(uid_text);

    return entry;
}

int custom_messages_count(void)
{
    return custom_messages.size - 1;
}

void custom_messages_save_state(buffer *buffer)
{
    int32_t array_size = custom_messages.size;
    int32_t struct_size = (4 * sizeof(int32_t));
    buffer_init_dynamic_piece(buffer,
        CUSTOM_MESSAGES_CURRENT_VERSION,
        array_size,
        struct_size);

    custom_message_t *entry;
    array_foreach(custom_messages, entry) {
        int linked_uid = entry && entry->linked_uid && entry->linked_uid->in_use ? entry->linked_uid->id : -1;
        buffer_write_i32(buffer, linked_uid);
        int title_id = entry && entry->title && entry->title->in_use ? entry->title->id : -1;
        buffer_write_i32(buffer, title_id);
        int subtitle_id = entry && entry->subtitle && entry->subtitle->in_use ? entry->subtitle->id : -1;
        buffer_write_i32(buffer, subtitle_id);
        int text_id = entry && entry->display_text && entry->display_text->in_use ? entry->display_text->id : -1;
        buffer_write_i32(buffer, text_id);
    }
}

static void link_media(custom_media_t *media, custom_media_link_type link_type, int link_id)
{
    if (link_id <= 0) {
        log_error("Unlinked custom media entry found. The game will probably crash.", 0, 0);
    }

    switch (link_type) {
        case CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_MAIN:
            {
                custom_message_t *message = custom_messages_get(link_id);
                message->linked_media = media;
            }
            break;
        case CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_BACKGROUND_MUSIC:
            {
                custom_message_t *message = custom_messages_get(link_id);
                message->linked_background_music = media;
            }
            break;
        default:
            log_error("Unhandled custom media link type. The game will probably crash.", 0, 0);
            break;
    }
}

void custom_messages_load_state(buffer *messages_buffer, buffer *media_buffer)
{
    int buffer_size, version, array_size, struct_size;
    buffer_load_dynamic_piece_header_data(messages_buffer,
        &buffer_size,
        &version,
        &array_size,
        &struct_size);
        
    // Entry 0 is kept empty.
    buffer_skip(messages_buffer, (4 * sizeof(int32_t)));
    // Expects the media text blob to be loaded already.
    for (int i = 1; i < array_size; i++) {
        custom_message_t *entry = custom_messages_create_blank();

        int linked_text_blob_id = buffer_read_i32(messages_buffer);
        entry->linked_uid = message_media_text_blob_get_entry(linked_text_blob_id);

        linked_text_blob_id = buffer_read_i32(messages_buffer);
        entry->title = message_media_text_blob_get_entry(linked_text_blob_id);

        linked_text_blob_id = buffer_read_i32(messages_buffer);
        entry->subtitle = message_media_text_blob_get_entry(linked_text_blob_id);

        linked_text_blob_id = buffer_read_i32(messages_buffer);
        entry->display_text = message_media_text_blob_get_entry(linked_text_blob_id);
    }

    buffer_load_dynamic_piece_header_data(media_buffer,
        &buffer_size,
        &version,
        &array_size,
        &struct_size);

    // Entry 0 is kept empty.
    buffer_skip(media_buffer, (4 * sizeof(int32_t)) + (1 * sizeof(int16_t)));
    custom_media_link_type link_type = 0;
    int link_id = 0;
    for (int i = 1; i < array_size; i++) {
        custom_media_t *media = custom_media_create_blank();
        custom_media_load_state_entry(media_buffer, media, &link_type, &link_id);
        link_media(media, link_type, link_id);
    }
}

uint8_t *custom_messages_get_title(custom_message_t *message)
{
    if (message->title && message->title->in_use) {
        return message->title->text;
    } else {
        return 0;
    }
}

uint8_t *custom_messages_get_subtitle(custom_message_t *message)
{
    if (message->subtitle && message->subtitle->in_use) {
        return message->subtitle->text;
    } else {
        return 0;
    }
}

uint8_t *custom_messages_get_text(custom_message_t *message)
{
    if (message->display_text && message->display_text->in_use) {
        return message->display_text->text;
    } else {
        return 0;
    }
}

static int check_for_file_in_dir(const uint8_t *filename, const char *directory, int *final_length)
{
    uint8_t filepath[FILE_NAME_MAX];

    const uint8_t *converted = string_from_ascii(directory);
    int converted_length = string_length(converted);
    int filename_length = string_length(filename);
    *final_length = converted_length + filename_length;

    string_copy(converted, &filepath[0], converted_length + 1);
    string_copy(filename, &filepath[converted_length], filename_length + 1);

    encoding_to_utf8(filepath, data.temp_filepath, FILE_NAME_MAX, 0);
    int found = file_exists(data.temp_filepath, MAY_BE_LOCALIZED);
    if (found) {
        return 1;
    }
    return 0;
}

static int search_for_sound_file(const uint8_t *filename)
{
    for (int i = 0; i < AUDIO_FILE_PATHS_COUNT; i++) {
        int final_length = 0;
        int found = check_for_file_in_dir(filename, AUDIO_FILE_PATHS[i], &final_length);
        if (found) {
            memset(data.filepath_media, 0, sizeof(data.filepath_media));

            strncpy(&data.filepath_media[0], data.temp_filepath, final_length);
            return 1;
        }
    }
    return 0;
}

static int search_for_video_file(const uint8_t *filename)
{
    for (int i = 0; i < VIDEO_FILE_PATHS_COUNT; i++) {
        int final_length = 0;
        int found = check_for_file_in_dir(filename, VIDEO_FILE_PATHS[i], &final_length);
        if (found) {
            memset(data.filepath_media, 0, sizeof(data.filepath_media));

            strncpy(&data.filepath_media[0], data.temp_filepath, final_length);
            return 1;
        }
    }
    return 0;
}

static int search_for_background_music_file(const uint8_t *filename)
{
    for (int i = 0; i < AUDIO_FILE_PATHS_COUNT; i++) {
        int final_length = 0;
        int found = check_for_file_in_dir(filename, AUDIO_FILE_PATHS[i], &final_length);
        if (found) {
            memset(data.filepath_audio, 0, sizeof(data.filepath_audio));

            strncpy(&data.filepath_audio[0], data.temp_filepath, final_length);
            return 1;
        }
    }
    return 0;
}

const char *custom_messages_get_video(custom_message_t *message)
{
    if (message->linked_media && message->linked_media->type == CUSTOM_MEDIA_VIDEO && message->linked_media->link_type == CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_MAIN) {
        if (search_for_video_file(message->linked_media->filename->text)) {
            return data.filepath_media;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

const char *custom_messages_get_audio(custom_message_t *message)
{
    if (message->linked_media && message->linked_media->type == CUSTOM_MEDIA_SOUND && message->linked_media->link_type == CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_MAIN) {
        if (search_for_sound_file(message->linked_media->filename->text)) {
            return data.filepath_media;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

const char *custom_messages_get_background_music(custom_message_t *message)
{
    if (message->linked_background_music && message->linked_background_music->type == CUSTOM_MEDIA_SOUND && message->linked_background_music->link_type == CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_BACKGROUND_MUSIC) {
        if (search_for_background_music_file(message->linked_background_music->filename->text)) {
            return data.filepath_audio;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}
