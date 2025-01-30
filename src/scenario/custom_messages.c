#include "custom_messages.h"

#include "core/array.h"
#include "core/encoding.h"
#include "core/file.h"
#include "core/log.h"
#include "core/string.h"
#include "game/campaign.h"

#define CUSTOM_MESSAGES_ARRAY_SIZE_STEP 100

static const char *AUDIO_FILE_PATHS[] = {
    CAMPAIGNS_DIRECTORY "/audio",
    "community/audio",
    "mp3",
    "wavs",
    0
};

static const char *VIDEO_FILE_PATHS[] = {
    CAMPAIGNS_DIRECTORY "/video",
    "community/video",
    "smk",
    "mpg",
    0
};

static array(custom_message_t) custom_messages;

static int entry_in_use(const custom_message_t *entry)
{
    return entry->in_use != 0;
}

static void new_entry(custom_message_t *obj, unsigned int position)
{
    obj->id = position;
}

void custom_messages_clear(void)
{
    if (custom_messages.size) {
        custom_message_t *entry;
        array_foreach(custom_messages, entry) {
            message_media_text_blob_mark_entry_as_unused(entry->linked_uid);
            message_media_text_blob_mark_entry_as_unused(entry->title);
            message_media_text_blob_mark_entry_as_unused(entry->subtitle);
            message_media_text_blob_mark_entry_as_unused(entry->display_text);
        }
    }

    if (!array_init(custom_messages, CUSTOM_MESSAGES_ARRAY_SIZE_STEP, new_entry, entry_in_use) ||
        !array_next(custom_messages)) {
        log_error("Unable to allocate enough memory for the custom media array. The game will now crash.", 0, 0);
    }
}

void custom_messages_clear_all(void)
{
    custom_messages_clear();
    custom_media_clear();
    message_media_text_blob_remove_unused();
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
                return array_index;
            }
        }
    }
    return 0;
}

custom_message_t *custom_messages_create_blank(void)
{
    custom_message_t *entry = 0;
    array_new_item_after_index(custom_messages, 1, entry);
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

void custom_messages_save_state(buffer *buf)
{
    uint32_t array_size = custom_messages.size;
    uint32_t struct_size = (4 * sizeof(int32_t));
    buffer_init_dynamic_array(buf, array_size, struct_size);

    custom_message_t *entry;
    array_foreach(custom_messages, entry) {
        int linked_uid = entry && entry->linked_uid && entry->linked_uid->in_use ? entry->linked_uid->id : -1;
        buffer_write_i32(buf, linked_uid);
        int title_id = entry && entry->title && entry->title->in_use ? entry->title->id : -1;
        buffer_write_i32(buf, title_id);
        int subtitle_id = entry && entry->subtitle && entry->subtitle->in_use ? entry->subtitle->id : -1;
        buffer_write_i32(buf, subtitle_id);
        int text_id = entry && entry->display_text && entry->display_text->in_use ? entry->display_text->id : -1;
        buffer_write_i32(buf, text_id);
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
                message->linked_media[media->type] = media;
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
    unsigned int array_size = buffer_load_dynamic_array(messages_buffer);

    // Entry 0 is kept empty.
    buffer_skip(messages_buffer, (4 * sizeof(int32_t)));
    // Expects the media text blob to be loaded already.
    for (unsigned int i = 1; i < array_size; i++) {
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

    array_size = buffer_load_dynamic_array(media_buffer);

    // Entry 0 is kept empty.
    buffer_skip(media_buffer, (4 * sizeof(int32_t)) + (1 * sizeof(int16_t)));
    custom_media_link_type link_type = 0;
    int link_id = 0;
    for (unsigned int i = 1; i < array_size; i++) {
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

static const char *check_for_file_in_dir(const char *filename, const char *directory)
{
    static char filepath[FILE_NAME_MAX];
    int location = PATH_LOCATION_ROOT;
    if (strncmp(directory, "community/", 10) == 0) {
        directory += 10;
        location = PATH_LOCATION_COMMUNITY;
    }
    if (snprintf(filepath, FILE_NAME_MAX, "%s/%s", directory, filename) > FILE_NAME_MAX) {
        log_error("Filename too long. The file will not be loaded.", filename, 0);
    }
    if (game_campaign_has_file(filepath)) {
        return filepath;
    }
    return dir_get_file_at_location(filepath, location);
}

static const char *search_for_file(const uint8_t *filename, const char *paths[])
{
    char filename_utf8[FILE_NAME_MAX];
    for (int i = 0; i <= encoding_system_uses_decomposed(); i++) {
        encoding_to_utf8(filename, filename_utf8, FILE_NAME_MAX, i);
        for (int j = 0; paths[j]; j++) {
            const char *path = check_for_file_in_dir(filename_utf8, paths[j]);
            if (path) {
                return path;
            }
        }
    }
    return 0;
}

uint8_t *custom_messages_get_video(custom_message_t *message)
{
    custom_media_t *media = message->linked_media[CUSTOM_MEDIA_VIDEO];
    if (media && media->type == CUSTOM_MEDIA_VIDEO &&
            media->link_type == CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_MAIN) {
        const char *path_utf8 = search_for_file(media->filename->text, VIDEO_FILE_PATHS);
        if (!path_utf8) {
            return 0;
        }
        static uint8_t path[FILE_NAME_MAX];
        encoding_from_utf8(path_utf8, path, FILE_NAME_MAX);
        return path;
    } else {
        return 0;
    }
}

const char *custom_messages_get_audio(custom_message_t *message)
{
    custom_media_t *media = message->linked_media[CUSTOM_MEDIA_SOUND];
    if (media && media->type == CUSTOM_MEDIA_SOUND &&
        media->link_type == CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_MAIN) {
        return search_for_file(media->filename->text, AUDIO_FILE_PATHS);
    } else {
        return 0;
    }
}

const char *custom_messages_get_speech(custom_message_t *message)
{
    custom_media_t *media = message->linked_media[CUSTOM_MEDIA_SPEECH];
    if (media && media->type == CUSTOM_MEDIA_SPEECH &&
        media->link_type == CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_MAIN) {
        return search_for_file(media->filename->text, AUDIO_FILE_PATHS);
    } else {
        return 0;
    }
}

const char *custom_messages_get_background_music(custom_message_t *message)
{
    if (message->linked_background_music &&
        message->linked_background_music->type == CUSTOM_MEDIA_SOUND &&
        message->linked_background_music->link_type == CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_BACKGROUND_MUSIC) {
        return search_for_file(message->linked_background_music->filename->text, AUDIO_FILE_PATHS);
    } else {
        return 0;
    }
}

const uint8_t *custom_messages_get_background_image(custom_message_t *message)
{
    custom_media_t *media = message->linked_media[CUSTOM_MEDIA_BACKGROUND_IMAGE];
    if (media && media->type == CUSTOM_MEDIA_BACKGROUND_IMAGE &&
        media->link_type == CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_MAIN) {
        return media->filename->text;
    } else {
        return 0;
    }
}

int custom_messages_relink_text_blob(int text_id, text_blob_string_t *new_text_link)
{
    custom_message_t *entry;
    array_foreach(custom_messages, entry) {
        if (entry && entry->linked_uid && entry->linked_uid->id == text_id) {
            entry->linked_uid = new_text_link;
            return 1;
        } else if (entry && entry->title && entry->title->id == text_id) {
            entry->title = new_text_link;
            return 1;
        } else if (entry && entry->subtitle && entry->subtitle->id == text_id) {
            entry->subtitle = new_text_link;
            return 1;
        } else if (entry && entry->display_text && entry->display_text->id == text_id) {
            entry->display_text = new_text_link;
            return 1;
        }
    }
    return 0;
}
