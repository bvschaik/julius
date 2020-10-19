#include "building/type.h"
#include "core/lang.h"

#include "core/buffer.h"
#include "core/file.h"
#include "core/io.h"
#include "core/string.h"
#include "translation/translation.h"

#include <stdlib.h>
#include <string.h>

#define MAX_TEXT_ENTRIES 1000
#define MAX_TEXT_DATA 200000
#define MIN_TEXT_SIZE (28 + MAX_TEXT_ENTRIES * 8)
#define MAX_TEXT_SIZE (MIN_TEXT_SIZE + MAX_TEXT_DATA)

#define MAX_MESSAGE_ENTRIES 400
#define MAX_MESSAGE_DATA 460000
#define MIN_MESSAGE_SIZE 32024
#define MAX_MESSAGE_SIZE (MIN_MESSAGE_SIZE + MAX_MESSAGE_DATA)

#define BUFFER_SIZE 400000

#define FILE_TEXT_ENG "c3.eng"
#define FILE_MM_ENG "c3_mm.eng"
#define FILE_TEXT_RUS "c3.rus"
#define FILE_MM_RUS "c3_mm.rus"
#define FILE_EDITOR_TEXT_ENG "c3_map.eng"
#define FILE_EDITOR_MM_ENG "c3_map_mm.eng"

static struct {
    struct {
        int32_t offset;
        int32_t in_use;
    } text_entries[MAX_TEXT_ENTRIES];
    uint8_t text_data[MAX_TEXT_DATA];

    lang_message message_entries[MAX_MESSAGE_ENTRIES];
    uint8_t message_data[MAX_MESSAGE_DATA];
} data;

static int file_exists_in_dir(const char *dir, const char *file)
{
    char path[2 * FILE_NAME_MAX];
    path[2 * FILE_NAME_MAX - 1] = 0;
    strncpy(path, dir, 2 * FILE_NAME_MAX - 1);
    strncat(path, "/", 2 * FILE_NAME_MAX - 1);
    strncat(path, file, 2 * FILE_NAME_MAX - 1);
    return file_exists(path, NOT_LOCALIZED);
}

int lang_dir_is_valid(const char *dir)
{
    if (file_exists_in_dir(dir, FILE_TEXT_ENG) && file_exists_in_dir(dir, FILE_MM_ENG)) {
        return 1;
    }
    if (file_exists_in_dir(dir, FILE_TEXT_RUS) && file_exists_in_dir(dir, FILE_MM_RUS)) {
        return 1;
    }
    return 0;
}

static void parse_text(buffer *buf)
{
    buffer_skip(buf, 28); // header
    for (int i = 0; i < MAX_TEXT_ENTRIES; i++) {
        data.text_entries[i].offset = buffer_read_i32(buf);
        data.text_entries[i].in_use = buffer_read_i32(buf);
    }
    buffer_read_raw(buf, data.text_data, MAX_TEXT_DATA);
}

static int load_text(const char *filename, int localizable, uint8_t *buf_data)
{
    buffer buf;
    int filesize = io_read_file_into_buffer(filename, localizable, buf_data, BUFFER_SIZE);
    if (filesize < MIN_TEXT_SIZE || filesize > MAX_TEXT_SIZE) {
        return 0;
    }
    buffer_init(&buf, buf_data, filesize);
    parse_text(&buf);
    return 1;
}

static uint8_t *get_message_text(int32_t offset)
{
    if (!offset) {
        return 0;
    }
    return &data.message_data[offset];
}

static void parse_message(buffer *buf)
{
    buffer_skip(buf, 24); // header
    for (int i = 0; i < MAX_MESSAGE_ENTRIES; i++) {
        lang_message *m = &data.message_entries[i];
        m->type = buffer_read_i16(buf);
        m->message_type = buffer_read_i16(buf);
        buffer_skip(buf, 2);
        m->x = buffer_read_i16(buf);
        m->y = buffer_read_i16(buf);
        m->width_blocks = buffer_read_i16(buf);
        m->height_blocks = buffer_read_i16(buf);
        m->image.id = buffer_read_i16(buf);
        m->image.x = buffer_read_i16(buf);
        m->image.y = buffer_read_i16(buf);
        buffer_skip(buf, 6); // unused image2 id, x, y
        m->title.x = buffer_read_i16(buf);
        m->title.y = buffer_read_i16(buf);
        m->subtitle.x = buffer_read_i16(buf);
        m->subtitle.y = buffer_read_i16(buf);
        buffer_skip(buf, 4);
        m->video.x = buffer_read_i16(buf);
        m->video.y = buffer_read_i16(buf);
        buffer_skip(buf, 14);
        m->urgent = buffer_read_i32(buf);

        m->video.text = get_message_text(buffer_read_i32(buf));
        buffer_skip(buf, 4);
        m->title.text = get_message_text(buffer_read_i32(buf));
        m->subtitle.text = get_message_text(buffer_read_i32(buf));
        m->content.text = get_message_text(buffer_read_i32(buf));
    }
    buffer_read_raw(buf, &data.message_data, MAX_MESSAGE_DATA);
}

static void set_message_parameters(lang_message* m, int title, int text, int urgent) {
    m->type = TYPE_MESSAGE;
    m->message_type = MESSAGE_TYPE_GENERAL;
    m->x = 0;
    m->y = 0;
    m->width_blocks = 30;
    m->height_blocks = 20;
    m->title.x = 0;
    m->title.y = 0;
    m->urgent = urgent;

    m->title.text = translation_for(title);
    m->content.text = translation_for(text);
}


void load_custom_messages(void) {
    int i = 320;
    for (; i < MAX_MESSAGE_ENTRIES; i++) {
        if (strlen(&data.message_entries[i].content.text) == 0) {
            break;
        }
    }

    // soldiers starving
    lang_message* m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_MESS_HALL_NEEDS_FOOD, TR_CITY_MESSAGE_TEXT_MESS_HALL_NEEDS_FOOD, 1);
    m->video.text = "smk//god_mars.smk";

    i += 1;

    // soldiers starving, no mess hall
    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_MESS_HALL_NEEDS_FOOD, TR_CITY_MESSAGE_TEXT_MESS_HALL_MISSING, 1);

    i += 1;

    // monument completed
    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_GRAND_TEMPLE_COMPLETE, TR_CITY_MESSAGE_TEXT_GRAND_TEMPLE_COMPLETE, 0);

    i += 1;

    // replacement Mercury blessing
    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_MERCURY_BLESSING, TR_CITY_MESSAGE_TEXT_MERCURY_BLESSING, 0);

    i += 1;

    // auto festivals
    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_PANTHEON_FESTIVAL, TR_CITY_MESSAGE_TEXT_PANTHEON_FESTIVAL_CERES, 0);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_PANTHEON_FESTIVAL, TR_CITY_MESSAGE_TEXT_PANTHEON_FESTIVAL_NEPTUNE, 0);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_PANTHEON_FESTIVAL, TR_CITY_MESSAGE_TEXT_PANTHEON_FESTIVAL_MERCURY, 0);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_PANTHEON_FESTIVAL, TR_CITY_MESSAGE_TEXT_PANTHEON_FESTIVAL_MARS, 0);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_PANTHEON_FESTIVAL, TR_CITY_MESSAGE_TEXT_PANTHEON_FESTIVAL_VENUS, 0);

}


static int load_message(const char *filename, int localizable, uint8_t *data_buffer)
{
    buffer buf;
    int filesize = io_read_file_into_buffer(filename, localizable, data_buffer, BUFFER_SIZE);
    if (filesize < MIN_MESSAGE_SIZE || filesize > MAX_MESSAGE_SIZE) {
        return 0;
    }
    buffer_init(&buf, data_buffer, filesize);
    parse_message(&buf);
    return 1;
}

static int load_files(const char *text_filename, const char *message_filename, int localizable)
{
    uint8_t *buffer = (uint8_t *) malloc(BUFFER_SIZE);
    if (!buffer) {
        return 0;
    }
    int success = load_text(text_filename, localizable, buffer) && load_message(message_filename, localizable, buffer);
    free(buffer);
    return success;
}

int lang_load(int is_editor)
{
    if (is_editor) {
        return load_files(FILE_EDITOR_TEXT_ENG, FILE_EDITOR_MM_ENG, MAY_BE_LOCALIZED);
    }
    // Prefer language files from localized dir, fall back to main dir
    return
        load_files(FILE_TEXT_ENG, FILE_MM_ENG, MUST_BE_LOCALIZED) ||
        load_files(FILE_TEXT_RUS, FILE_MM_RUS, MUST_BE_LOCALIZED) ||
        load_files(FILE_TEXT_ENG, FILE_MM_ENG, NOT_LOCALIZED) ||
        load_files(FILE_TEXT_RUS, FILE_MM_RUS, NOT_LOCALIZED);
}

const uint8_t *lang_get_string(int group, int index)
{
    if (group == 92 && !index) {
        return translation_for(TR_BUILDING_SMALL_TEMPLE_CERES_NAME);
    }    
    if (group == 93 && !index) {
        return translation_for(TR_BUILDING_SMALL_TEMPLE_NEPTUNE_NAME);
    }    
    if (group == 94 && !index) {
        return translation_for(TR_BUILDING_SMALL_TEMPLE_MERCURY_NAME);
    }    
    if (group == 95 && !index) {
        return translation_for(TR_BUILDING_SMALL_TEMPLE_MARS_NAME);
    }
    if (group == 96 && !index) {
        return translation_for(TR_BUILDING_SMALL_TEMPLE_VENUS_NAME);
    }

    if (group == 130) {
        switch (index) {
        case 641:
            return translation_for(TR_PHRASE_FIGURE_MISSIONARY_EXACT_4);
        default:
            break;
        }
    }

    
    // Building strings
    if (group == 28 || group == 41) {
        switch (index) {
        case BUILDING_ROADBLOCK:
            return translation_for(TR_BUILDING_ROADBLOCK);
        case BUILDING_WORKCAMP:
            return translation_for(TR_BUILDING_WORK_CAMP);
        case BUILDING_GRAND_TEMPLE_CERES:
            return translation_for(TR_BUILDING_GRAND_TEMPLE_CERES);
        case BUILDING_GRAND_TEMPLE_NEPTUNE:
            return translation_for(TR_BUILDING_GRAND_TEMPLE_NEPTUNE);
        case BUILDING_GRAND_TEMPLE_MERCURY:
            return translation_for(TR_BUILDING_GRAND_TEMPLE_MERCURY);
        case BUILDING_GRAND_TEMPLE_MARS:
            return translation_for(TR_BUILDING_GRAND_TEMPLE_MARS);
        case BUILDING_GRAND_TEMPLE_VENUS:
            return translation_for(TR_BUILDING_GRAND_TEMPLE_VENUS);
        case BUILDING_PANTHEON:
            return translation_for(TR_BUILDING_PANTHEON);
        case BUILDING_MENU_GRAND_TEMPLES:
            return translation_for(TR_BUILDING_GRAND_TEMPLE_MENU);
        case BUILDING_ENGINEER_GUILD:
            return translation_for(TR_BUILDING_ENGINEER_GUILD);
        case BUILDING_MESS_HALL:
            return translation_for(TR_BUILDING_MESS_HALL);
        case BUILDING_MENU_TREES:
            return translation_for(TR_BUILDING_MENU_TREES);
        case BUILDING_MENU_PATHS:
            return translation_for(TR_BUILDING_MENU_PATHS);
        case BUILDING_MENU_PARKS:
            return translation_for(TR_BUILDING_MENU_PARKS);
        case BUILDING_SMALL_POND:
            return translation_for(TR_BUILDING_SMALL_POND);
        case BUILDING_LARGE_POND:
            return translation_for(TR_BUILDING_LARGE_POND);
        case BUILDING_PINE_TREE:
            return translation_for(TR_BUILDING_PINE_TREE);
        case BUILDING_FIR_TREE:
            return translation_for(TR_BUILDING_FIR_TREE);
        case BUILDING_OAK_TREE:
            return translation_for(TR_BUILDING_OAK_TREE);
        case BUILDING_ELM_TREE:
            return translation_for(TR_BUILDING_ELM_TREE);
        case BUILDING_FIG_TREE:
            return translation_for(TR_BUILDING_FIG_TREE);
        case BUILDING_PLUM_TREE:
            return translation_for(TR_BUILDING_PLUM_TREE);
        case BUILDING_PALM_TREE:
            return translation_for(TR_BUILDING_PALM_TREE);
        case BUILDING_DATE_TREE:
            return translation_for(TR_BUILDING_DATE_TREE);
        case BUILDING_PINE_PATH:
            return translation_for(TR_BUILDING_PINE_PATH);
        case BUILDING_FIR_PATH:
            return translation_for(TR_BUILDING_FIR_PATH);
        case BUILDING_OAK_PATH:
            return translation_for(TR_BUILDING_OAK_PATH);
        case BUILDING_ELM_PATH:
            return translation_for(TR_BUILDING_ELM_PATH);
        case BUILDING_FIG_PATH:
            return translation_for(TR_BUILDING_FIG_PATH);
        case BUILDING_PLUM_PATH:
            return translation_for(TR_BUILDING_PLUM_PATH);
        case BUILDING_PALM_PATH:
            return translation_for(TR_BUILDING_PALM_PATH);
        case BUILDING_DATE_PATH:
            return translation_for(TR_BUILDING_DATE_PATH);
        case BUILDING_PAVILION_BLUE:
            return translation_for(TR_BUILDING_BLUE_PAVILION);
        case BUILDING_PAVILION_RED:
            return translation_for(TR_BUILDING_RED_PAVILION);
        case BUILDING_PAVILION_ORANGE:
            return translation_for(TR_BUILDING_ORANGE_PAVILION);
        case BUILDING_PAVILION_YELLOW:
            return translation_for(TR_BUILDING_YELLOW_PAVILION);
        case BUILDING_PAVILION_GREEN:
            return translation_for(TR_BUILDING_GREEN_PAVILION);
        case BUILDING_SMALL_STATUE_ALT:
            return translation_for(TR_BUILDING_SMALL_STATUE_ALT);
        case BUILDING_SMALL_STATUE_ALT_B:
            return translation_for(TR_BUILDING_SMALL_STATUE_ALT_B);
        case BUILDING_OBELISK:
            return translation_for(TR_BUILDING_OBELISK);
        default:
            break;
        }
    }
    
    const uint8_t *str = &data.text_data[data.text_entries[group].offset];
    uint8_t prev = 0;
    while (index > 0) {
        if (!*str && (prev >= ' ' || prev == 0)) {
            --index;
        }
        prev = *str;
        ++str;
    }
    while (*str < ' ') { // skip non-printables
        ++str;
    }
    return str;
}

const lang_message *lang_get_message(int id) {
    return &data.message_entries[id];
}
