#include "xml.h"

#include "core/encoding.h"
#include "core/file.h"
#include "core/log.h"
#include "core/xml_parser.h"
#include "game/campaign/file.h"
#include "game/campaign/mission.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define XML_TOTAL_ELEMENTS 5
#define CAMPAIGN_XML_VERSION 1

static int xml_start_campaign(void);
static int xml_start_description(void);
static void xml_description_text(const char *text);
static int xml_start_missions(void);
static int xml_start_mission(void);
static int xml_start_scenario(void);

static void xml_end_mission(void);

static const char *RANKS[] = {
    "citizen", "clerk", "engineer", "architect", "quaestor",
    "procurator", "aedile", "praetor", "consul", "proconsul",
    "caesar"
};

static const xml_parser_element xml_elements[XML_TOTAL_ELEMENTS] = {
    { "campaign", xml_start_campaign },
    { "description", xml_start_description, 0, "campaign", xml_description_text },
    { "missions", xml_start_missions, 0, "campaign" },
    { "mission", xml_start_mission, xml_end_mission, "missions" },
    { "scenario", xml_start_scenario, 0, "mission" }
};

static struct {
    campaign_info *info;
    campaign_mission *current_mission;
    int missions_list_parsed;
    int success;
} data;

static const char *FANFARE_TYPES[3] = { "none", "peaceful", "military" };
static const char *FANFARE_FILES[3] = { 0, "fanfare_nu1.wav", "fanfare_nu5.wav" };

static int xml_start_campaign(void)
{
    if (xml_parser_get_attribute_int("version") > CAMPAIGN_XML_VERSION) {
        data.success = 0;
        return 0;
    }
    return 1;
}

static const uint8_t *copy_string_from_xml(const char *text)
{
    if (!text || !*text) {
        return 0;
    }
    size_t length = strlen(text) + 1;
    uint8_t *result = malloc(sizeof(uint8_t) * length);
    if (!result) {
        return 0;
    }
    encoding_from_utf8(text, result, (int) length);
    return result;
}

static int xml_start_description(void)
{
    data.info->name = copy_string_from_xml(xml_parser_get_attribute_string("title"));
    data.info->author = copy_string_from_xml(xml_parser_get_attribute_string("author"));
    if (!data.info->name) {
        log_error("Problem parsing campaign file - missing name", 0, 0);
        data.success = 0;
        return 0;
    }
    return 1;
}

static void xml_description_text(const char *text)
{
    data.info->description = copy_string_from_xml(text);
}

static int xml_start_missions(void)
{
    if (!data.success) {
        return 0;
    }
    if (data.missions_list_parsed) {
        log_error("Problem parsing campaign file - duplicate mission list", 0, 0);
        data.success = 0;
        return 0;
    }
    data.missions_list_parsed = 1;
    if (!campaign_mission_init()) {
        log_error("Problem parsing campaign file - unable to allocate memory for campaigns", 0, 0);
        data.success = 0;
        return 0;
    }
    data.info->starting_rank = xml_parser_get_attribute_enum("starting_rank", &RANKS[1], 10, 1);
    return 1;
}

static const char *create_full_campaign_path(const char *path, const char *file)
{
    size_t full_path_length = sizeof(CAMPAIGNS_DIRECTORY "/") + strlen(path) + strlen(file) + 1;
    char *full_path = malloc(full_path_length);
    if (!full_path) {
        return 0;
    }
    snprintf(full_path, full_path_length, "%s/%s/%s", CAMPAIGNS_DIRECTORY, path, file);
    const char *filename = campaign_file_remove_prefix(full_path);
    if (!filename || !campaign_file_exists(filename)) {
        free(full_path);
        return 0;
    }
    return full_path;
}

static const char *create_full_regular_path(const char *path, const char *file)
{
    size_t full_path_length = strlen(path) + strlen(file) + 2;
    char *full_path = malloc(full_path_length);
    if (!full_path) {
        return 0;
    }
    snprintf(full_path, full_path_length, "%s/%s", path, file);
    if (!file_exists(full_path, MAY_BE_LOCALIZED)) {
        free(full_path);
        return 0;
    }
    return full_path; 
}

static int xml_start_mission(void)
{
    if (!data.success) {
        return 0;
    }
    data.current_mission = campaign_mission_new();
    if (!data.current_mission) {
        log_error("Problem parsing campaign file - memory full", 0, 0);
        data.success = 0;
        return 0;
    }
    data.info->number_of_missions++;
    data.current_mission->title = copy_string_from_xml(xml_parser_get_attribute_string("title"));
    const char *background_image = xml_parser_get_attribute_string("background_image");
    if (background_image) {        
        data.current_mission->background_image.path = create_full_campaign_path("image", background_image);
    }
    const char *intro_video = xml_parser_get_attribute_string("video");
    if (intro_video) {
        data.current_mission->intro_video = create_full_campaign_path("video", intro_video);
        if (!data.current_mission->intro_video) {
            if (file_has_extension(intro_video, "smk")) {
                data.current_mission->intro_video = create_full_regular_path("smk", intro_video);
            } else if (file_has_extension(intro_video, "mpg")) {
                data.current_mission->intro_video = create_full_regular_path("mpg", intro_video);
            }
        }
    }
    data.current_mission->next_rank = xml_parser_get_attribute_enum("next_rank", RANKS, 11, 0);

    if (xml_parser_has_attribute("max_personal_savings")) {
        data.current_mission->max_personal_savings = xml_parser_get_attribute_int("max_personal_savings");
    } else {
        data.current_mission->max_personal_savings = INT_MAX;
    }
    if (xml_parser_has_attribute("file") && !xml_start_scenario()) {
        return 0;
    }
    return 1;
}

static int xml_start_scenario(void)
{
    if (!data.success) {
        return 0;
    }
    campaign_scenario *scenario = campaign_mission_new_scenario();
    if (!scenario) {
        log_error("Problem parsing campaign file - memory full", 0, 0);
        data.success = 0;
        return 0;
    }
    scenario->x = xml_parser_get_attribute_int("x");
    scenario->y = xml_parser_get_attribute_int("y");
    scenario->name = copy_string_from_xml(xml_parser_get_attribute_string("name"));
    scenario->description = copy_string_from_xml(xml_parser_get_attribute_string("description"));
    if (xml_parser_has_attribute("fanfare")) {
        const char *fanfare = 0;
        int default_fanfare = xml_parser_get_attribute_enum("fanfare", FANFARE_TYPES, 3, 0);
        if (default_fanfare > 0) {
            fanfare = FANFARE_FILES[default_fanfare];
        } else if (default_fanfare == -1) {
            fanfare = xml_parser_get_attribute_string("fanfare");
        }
        if (fanfare) {
            scenario->fanfare = create_full_campaign_path("audio", fanfare);
            if (!scenario->fanfare) {
                if (file_has_extension(fanfare, "wav")) {
                    scenario->fanfare = create_full_regular_path("wavs", fanfare);
                } else if (file_has_extension(fanfare, "mp3")) {
                    scenario->fanfare = create_full_regular_path("mp3", fanfare);
                }
            }
        }
    }


    const char *scenario_path = xml_parser_get_attribute_string("file");
    if (!scenario_path) {
        log_error("Problem parsing campaign file - missing path for mission scenario", 0, 0);
        data.success = 0;
        return 0;
    }
    scenario->path = create_full_campaign_path("scenario", scenario_path);
    if (!scenario->path) {
        log_error("Problem parsing campaign file - scenario file does not exist", scenario_path, 0);
        // Files in directories are debug only - don't prevent opening them even if files are missing
        if (campaign_file_is_zip()) {
            data.success = 0;
            return 0;
        }
    }

    if (!scenario->name) {
        char name[FILE_NAME_MAX];
        snprintf(name, FILE_NAME_MAX, "Scenario %d", scenario->id);
        scenario->name = copy_string_from_xml(name);
        if (!scenario->name) {
            log_error("Problem parsing campaign file - memory full", 0, 0);
            data.success = 0;
            return 0;
        }
    }
    data.current_mission->last_scenario = scenario->id;
    return 1;
}

static void xml_end_mission(void)
{
    if (!data.success) {
        data.current_mission = 0;
        return;
    }
    if (data.current_mission->last_scenario < data.current_mission->first_scenario) {
        log_error("Problem parsing campaign file - mission with no scenarios. Mission index:",
            0, data.current_mission->id);
        data.success = 0;
    }
    data.current_mission = 0;
}

int campaign_xml_get_info(const char *xml_text, size_t xml_size, campaign_info *info)
{
    if (!xml_parser_init(xml_elements, XML_TOTAL_ELEMENTS, 0)) {
        return 0;
    }
    data.info = info;
    data.success = 1;
    data.missions_list_parsed = 0;

    if (!xml_parser_parse(xml_text, (unsigned int) xml_size, 1)) {
        data.success = 0;
    }
    xml_parser_free();

    return data.success;
}
