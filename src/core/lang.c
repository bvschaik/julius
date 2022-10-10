#include "building/type.h"
#include "core/lang.h"

#include "core/buffer.h"
#include "core/file.h"
#include "core/io.h"
#include "core/log.h"
#include "core/string.h"
#include "scenario/building.h"
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


static void set_message_parameters(lang_message *m, int title, int text, int urgent, int message_type)
{
    m->type = TYPE_MESSAGE;
    m->message_type = message_type;
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


void load_custom_messages(void)
{
    int i = 321;
    while (i < MAX_MESSAGE_ENTRIES) {
        if (!data.message_entries[i].content.text) {
            break;
        }
        i++;
    }

    if (i >= MAX_MESSAGE_ENTRIES) {
        log_error("Message entry max exceeded", "", 0);
        return;
    }

    // soldiers starving
    lang_message *m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_MESS_HALL_NEEDS_FOOD, TR_CITY_MESSAGE_TEXT_MESS_HALL_NEEDS_FOOD, 1,
        MESSAGE_TYPE_GENERAL);
    m->video.text = (uint8_t *) "smk/god_mars.smk";
    i += 1;

    // soldiers starving, no mess hall
    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_MESS_HALL_NEEDS_FOOD, TR_CITY_MESSAGE_TEXT_MESS_HALL_MISSING, 1,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    // monument completed
    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_GRAND_TEMPLE_COMPLETE, TR_CITY_MESSAGE_TEXT_GRAND_TEMPLE_COMPLETE, 0,
        MESSAGE_TYPE_BUILDING_COMPLETION);
    i += 1;

    // replacement Mercury blessing
    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_MERCURY_BLESSING, TR_CITY_MESSAGE_TEXT_MERCURY_BLESSING, 0,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    // auto festivals
    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_PANTHEON_FESTIVAL, TR_CITY_MESSAGE_TEXT_PANTHEON_FESTIVAL_CERES, 0,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_PANTHEON_FESTIVAL, TR_CITY_MESSAGE_TEXT_PANTHEON_FESTIVAL_NEPTUNE, 0,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_PANTHEON_FESTIVAL, TR_CITY_MESSAGE_TEXT_PANTHEON_FESTIVAL_MERCURY, 0,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_PANTHEON_FESTIVAL, TR_CITY_MESSAGE_TEXT_PANTHEON_FESTIVAL_MARS, 0,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_PANTHEON_FESTIVAL, TR_CITY_MESSAGE_TEXT_PANTHEON_FESTIVAL_VENUS, 0,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_MONUMENT_COMPLETE, TR_CITY_MESSAGE_TEXT_PANTHEON_COMPLETE, 0,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_MONUMENT_COMPLETE, TR_CITY_MESSAGE_TEXT_LIGHTHOUSE_COMPLETE, 0,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_NEPTUNE_BLESSING, TR_CITY_MESSAGE_TEXT_NEPTUNE_BLESSING, 0,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_VENUS_BLESSING, TR_CITY_MESSAGE_TEXT_VENUS_BLESSING, 0,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_MONUMENT_COMPLETE, TR_CITY_MESSAGE_TEXT_COLOSSEUM_COMPLETE, 0,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_MONUMENT_COMPLETE, TR_CITY_MESSAGE_TEXT_HIPPODROME_COMPLETE, 0,
        MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_COLOSSEUM_WORKING, TR_CITY_MESSAGE_TEXT_COLOSSEUM_WORKING, 1,
        MESSAGE_TYPE_GENERAL);
    m->video.text = (uint8_t *) "smk/1ST_GLAD.smk";
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_HIPPODROME_WORKING, TR_CITY_MESSAGE_TEXT_HIPPODROME_WORKING, 1,
        MESSAGE_TYPE_GENERAL);
    m->video.text = (uint8_t *) "smk/1st_Chariot.smk";
    i += 1;

    for (int j = 0; j < 12; ++j) {
        m = &data.message_entries[i];
        set_message_parameters(m, TR_CITY_MESSAGE_TITLE_GREAT_GAMES, TR_CITY_MESSAGE_TEXT_NAVAL_GAMES_PLANNING + j, 1,
            MESSAGE_TYPE_GENERAL);
        i += 1;
    }

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_LOOTING, TR_CITY_MESSAGE_TEXT_LOOTING, 1, MESSAGE_TYPE_DISASTER);
    i += 1;

    for (int j = 0; j < 3; ++j) {
        m = &data.message_entries[i];
        set_message_parameters(m, TR_CITY_MESSAGE_TITLE_GREAT_GAMES, TR_CITY_MESSAGE_TEXT_IMPERIAL_GAMES_PLANNING + j, 1,
            MESSAGE_TYPE_GENERAL);
        i += 1;
    }

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_SICKNESS, TR_CITY_MESSAGE_TEXT_SICKNESS, 1, MESSAGE_TYPE_DISASTER);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_EMPERORS_WRATH, TR_CITY_MESSAGE_TEXT_EMPERORS_WRATH, 1, MESSAGE_TYPE_GENERAL);
    m->video.text = (uint8_t *) "smk/Emp_send_army.smk";
    m->urgent = 1;
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_MARS_MINOR_CURSE_PREVENTED, TR_CITY_MESSAGE_TEXT_MARS_MINOR_CURSE_PREVENTED, 1, MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_ENEMIES_LEAVING, TR_CITY_MESSAGE_TEXT_ENEMIES_LEAVING, 1, MESSAGE_TYPE_GENERAL);
    i += 1;

    m = &data.message_entries[i];
    m->urgent = 1;
    set_message_parameters(m, TR_CITY_MESSAGE_TITLE_ROAD_TO_ROME_WARNING, TR_CITY_MESSAGE_TEXT_ROAD_TO_ROME_WARNING, 1, MESSAGE_TYPE_GENERAL);
    i += 1;

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
    if (group == CUSTOM_TRANSLATION) {
        return translation_for(index);
    }
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
    if (((group == 23 && index == 6) || (group == 27 && index == 6) || (group == 68 && index == 137))
        && scenario_building_allowed(BUILDING_WHARF)) {
        return translation_for(TR_RESOURCE_FISH);
    }

    if (group == 130) {
        switch (index) {
            case 641:
                return translation_for(TR_PHRASE_FIGURE_MISSIONARY_EXACT_4);
            default:
                break;
        }
    }

    if (group == 67 && index == 48) {
        return translation_for(TR_EDITOR_ALLOWED_BUILDINGS_MONUMENTS);
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
            case BUILDING_ARCHITECT_GUILD:
                return translation_for(TR_BUILDING_ARCHITECT_GUILD);
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
            case BUILDING_LIGHTHOUSE:
                return translation_for(TR_BUILDING_LIGHTHOUSE);
            case BUILDING_MENU_GOV_RES:
                return translation_for(TR_BUILDING_MENU_GOV_RES);
            case BUILDING_MENU_STATUES:
                return translation_for(TR_BUILDING_MENU_STATUES);
            case BUILDING_TAVERN:
                return translation_for(TR_BUILDING_TAVERN);
            case BUILDING_GRAND_GARDEN:
                return translation_for(TR_BUILDING_GRAND_GARDEN);
            case BUILDING_ARENA:
                return translation_for(TR_BUILDING_ARENA);
            case BUILDING_HORSE_STATUE:
                return translation_for(TR_BUILDING_HORSE_STATUE);
            case BUILDING_DOLPHIN_FOUNTAIN:
                return translation_for(TR_BUILDING_DOLPHIN_FOUNTAIN);
            case BUILDING_HEDGE_DARK:
                return translation_for(TR_BUILDING_HEDGE_DARK);
            case BUILDING_HEDGE_LIGHT:
                return translation_for(TR_BUILDING_HEDGE_LIGHT);
            case BUILDING_GARDEN_WALL:
                return translation_for(TR_BUILDING_GARDEN_WALL);
            case BUILDING_LEGION_STATUE:
                return translation_for(TR_BUILDING_LEGION_STATUE);
            case BUILDING_DECORATIVE_COLUMN:
                return translation_for(TR_BUILDING_DECORATIVE_COLUMN);
            case BUILDING_COLONNADE:
                return translation_for(TR_BUILDING_COLONNADE);
            case BUILDING_GARDEN_PATH:
                return translation_for(TR_BUILDING_GARDEN_PATH);
            case BUILDING_LARARIUM:
                return translation_for(TR_BUILDING_LARARIUM);
            case BUILDING_NYMPHAEUM:
                return translation_for(TR_BUILDING_NYMPHAEUM);
            case BUILDING_WATCHTOWER:
                return translation_for(TR_BUILDING_WATCHTOWER);
            case BUILDING_SMALL_MAUSOLEUM:
                return translation_for(TR_BUILDING_SMALL_MAUSOLEUM);
            case BUILDING_LARGE_MAUSOLEUM:
                return translation_for(TR_BUILDING_LARGE_MAUSOLEUM);
            case BUILDING_CARAVANSERAI:
                return translation_for(TR_BUILDING_CARAVANSERAI);
            case BUILDING_ROOFED_GARDEN_WALL:
                return translation_for(TR_BUILDING_ROOFED_GARDEN_WALL);
            case BUILDING_GARDEN_WALL_GATE:
                return translation_for(TR_BUILDING_GARDEN_WALL_GATE);
            case BUILDING_PALISADE:
                return translation_for(TR_BUILDING_PALISADE);
            case BUILDING_GLADIATOR_STATUE:
                return translation_for(TR_BUILDING_GLADIATOR_STATUE);
            case BUILDING_HIGHWAY:
                return translation_for(TR_BUILDING_HIGHWAY);
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

const lang_message *lang_get_message(int id)
{
    return &data.message_entries[id];
}
