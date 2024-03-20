#include "xml.h"

#include "campaign/file.h"
#include "campaign/mission.h"
#include "core/encoding.h"
#include "core/file.h"
#include "core/log.h"
#include "core/xml_parser.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define XML_TOTAL_ELEMENTS 5
#define CAMPAIGN_XML_VERSION 1

static int xml_start_campaign(void);
static int xml_start_intro(void);
static void xml_intro_text(const char *text);
static int xml_start_missions(void);
static int xml_start_mission(void);
static int xml_start_option(void);

static void xml_end_mission(void);

static const xml_parser_element xml_elements[XML_TOTAL_ELEMENTS] = {
    { "campaign", xml_start_campaign },
    { "intro", xml_start_intro, 0, "campaign", xml_intro_text },
    { "missions", xml_start_missions, 0, "campaign" },
    { "mission", xml_start_mission, xml_end_mission, "missions" },
    { "option", xml_start_option, 0, "mission" }
};

static struct {
    campaign_info *info;
    campaign_mission *current_mission;
    int missions_list_parsed;
    int success;
} data;

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

static int xml_start_intro(void)
{
    data.info->name = copy_string_from_xml(xml_parser_get_attribute_string("title"));
    return 1;
}

static void xml_intro_text(const char *text)
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
    return 1;
}

static int check_file_from_xml(const char *path, const char *file)
{
    char full_path[FILE_NAME_MAX];
    snprintf(full_path, FILE_NAME_MAX, "%s/%s", path, file);
    return campaign_file_exists(full_path);
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
    data.current_mission->background_image = xml_parser_copy_attribute_string("background_image");
    if (xml_parser_has_attribute("file") && !xml_start_option()) {
        return 0;
    }
    return 1;
}

static int xml_start_option(void)
{
    if (!data.success) {
        return 0;
    }
    campaign_mission_option *option = campaign_mission_new_option();
    if (!option) {
        log_error("Problem parsing campaign file - memory full", 0, 0);
        data.success = 0;
        return 0;
    }
    option->x = xml_parser_get_attribute_int("x");
    option->y = xml_parser_get_attribute_int("y");
    option->name = xml_parser_copy_attribute_string("name");
    option->description = xml_parser_copy_attribute_string("description");
    option->path = xml_parser_copy_attribute_string("file");

    option->image.path = xml_parser_copy_attribute_string("image");
    option->image.x = xml_parser_get_attribute_int("image_x");
    option->image.y = xml_parser_get_attribute_int("image_y");

    if (!option->path) {
        log_error("Problem parsing campaign file - missing path for mission option", 0, 0);
        data.success = 0;
        return 0;
    }
    if (!check_file_from_xml("scenario", option->path)) {
        log_error("Problem parsing campaign file - scenario file does not exist", option->path, 0);
        // Files in directories are debug only - don't prevent opening them even if files are missing
        if (campaign_file_is_zip()) {
            data.success = 0;
            return 0;
        }
    }
    if (option->image.path && !check_file_from_xml("image", option->image.path)) {
        log_info("Problem parsing campaign file - image file does not exist", option->image.path, 0);
    }
    data.current_mission->last_option = option->id;
    return 1;
}

static void xml_end_mission(void)
{
    if (!data.success) {
        data.current_mission = 0;
        return;
    }
    if (data.current_mission->last_option < data.current_mission->first_option) {
        log_error("Problem parsing campaign file - mission with no options. Mission index:",
            0, data.current_mission->id);
        data.success = 0;
    }
    data.current_mission = 0;
}

int campaign_xml_get_info(const char *xml_text, size_t xml_size, campaign_info *info)
{
    if (!xml_parser_init(xml_elements, XML_TOTAL_ELEMENTS)) {
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
