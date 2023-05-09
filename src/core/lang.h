#ifndef CORE_LANG_H
#define CORE_LANG_H

#include <stdint.h>

/**
 * @file
 * Language functions for localizable strings and messages
 */

/**
 * Type
 */
typedef enum {
    TYPE_MANUAL = 0,
    TYPE_ABOUT = 1,
    TYPE_MESSAGE = 2,
    TYPE_MISSION = 3
} lang_type;

/**
 * Message type
 */
typedef enum {
    MESSAGE_TYPE_GENERAL = 0,
    MESSAGE_TYPE_DISASTER = 1,
    MESSAGE_TYPE_IMPERIAL = 2,
    MESSAGE_TYPE_EMIGRATION = 3,
    MESSAGE_TYPE_TUTORIAL = 4,
    MESSAGE_TYPE_TRADE_CHANGE = 5,
    MESSAGE_TYPE_PRICE_CHANGE = 6,
    MESSAGE_TYPE_INVASION = 7,
    MESSAGE_TYPE_BUILDING_COMPLETION = 8
} lang_message_type;

/**
 * Image in a message
 */
struct lang_message_image {
    int id; /**< ID of the image */
    int x; /**< X offset */
    int y; /**< Y offset */
};

/**
 * Message string
 */
struct lang_message_string {
    uint8_t *text; /**< Text */
    int x; /**< X offset */
    int y; /**< Y offset */
};

/**
 * Message
 */
typedef struct {
    lang_type type;
    lang_message_type message_type;
    int x;
    int y;
    int width_blocks;
    int height_blocks;
    int urgent;
    struct lang_message_image image;
    struct lang_message_string title;
    struct lang_message_string subtitle;
    struct lang_message_string video;
    struct lang_message_string content;
} lang_message;

/**
 * Checks whether the directory contains language files
 * @param dir Directory to check
 * @return boolean true if it contains language files, false if not
 */
int lang_dir_is_valid(const char *dir);

/**
 * Loads the language files
 * @param is_editor Whether to load the editor language files or the regular ones
 * @return boolean true on success, false on failure
 */
int lang_load(int is_editor);
void load_augustus_messages(void);

/**
 * Gets a localized string
 * @param group Text group
 * @param index Index within the group
 * @return String
 */
const uint8_t *lang_get_string(int group, int index);

/**
 * Gets the message for the specified ID
 * @param id ID of the message
 * @return Message
 */
const lang_message *lang_get_message(int id);

#endif // CORE_LANG_H
