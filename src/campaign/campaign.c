#include "campaign.h"

#include "campaign/file.h"
#include "campaign/mission.h"
#include "campaign/xml.h"
#include "core/file.h"
#include "core/log.h"

#include <stdlib.h>
#include <string.h>

static struct {
    int active;
    campaign_info campaign;
    char file_name[FILE_NAME_MAX];
    char suspended_filename[FILE_NAME_MAX];
    struct {
        campaign_mission_info info;
        int current_option_id;
        const campaign_mission *current;
    } mission;
} data;

static void get_campaign_data(void)
{
    if (!campaign_file_open_zip()) {
        log_error("Error opening campaign file", 0, 0);
        campaign_clear();
        return;
    }
    log_info("Opening campaign settings file", 0, 0);

    size_t xml_size;
    char *xml_text = campaign_file_load("settings.xml", &xml_size);
    if (!xml_text) {
        log_error("Error loading campaign settings file", 0, 0);
        campaign_clear();
        return;
    }
    int result = campaign_xml_get_info(xml_text, xml_size, &data.campaign);
    campaign_file_close_zip();

    free(xml_text);

    if (!result) {
        campaign_clear();
    }

    data.active = result;
}

int campaign_load(const char *filename)
{
    if (!filename || !*filename) {
        campaign_clear();
        return 0;
    }

    if (strcmp(filename, data.file_name) == 0) {
        data.active = 1;
        return 1;
    }

    campaign_clear();
    campaign_file_set_path(filename);
    get_campaign_data();
    if (data.active) {
        strncpy(data.file_name, filename, FILE_NAME_MAX);
    }
    return data.active;
}

int campaign_is_active(void)
{
    return data.active;
}

const char *campaign_get_name(void)
{
    static char disabled_file_name;
    return data.active ? data.file_name : &disabled_file_name;
}

const campaign_info *campaign_get_info(void)
{
    return data.active ? &data.campaign : 0;
}

int campaign_has_file(const char *filename)
{
    if (!data.active) {
        return 0;
    }
    filename = campaign_file_remove_prefix(filename);
    if (!filename) {
        return 0;
    }
    return campaign_file_exists(filename);
}

uint8_t *campaign_load_file(const char *filename, size_t *length)
{
    *length = 0;
    if (!data.active) {
        return 0;
    }
    filename = campaign_file_remove_prefix(filename);
    if (!filename) {
        return 0;
    }
    return campaign_file_load(filename, length);
}

static const campaign_mission_option *get_next_mission_option(void)
{
    if (!data.active || !data.mission.current || data.mission.current_option_id > data.mission.current->last_option) {
        return 0;
    }
    const campaign_mission_option *option = campaign_mission_get_option(data.mission.current_option_id);
    data.mission.current_option_id++;
    return option;
}

const campaign_mission_info *campaign_get_next_mission(int last_scenario_id)
{
    if (!data.active) {
        return 0;
    }
    const campaign_mission *current = campaign_mission_next(last_scenario_id);
    if (!current) {
        data.mission.current = 0;
        data.mission.info.background_image = 0;
        data.mission.current_option_id = 0;
        return 0;
    }
    data.mission.current = current;
    data.mission.info.background_image = current->background_image;
    data.mission.current_option_id = current->first_option;
    return &data.mission.info;
}

void campaign_suspend(void)
{
    strncpy(data.suspended_filename, data.file_name, FILE_NAME_MAX);
    data.active = 0;
}

void campaign_restore(void)
{
    campaign_load(data.suspended_filename);
}

void campaign_clear(void)
{
    campaign_file_set_path(0);
    campaign_mission_clear();
    free((uint8_t *) data.campaign.name);
    free((uint8_t *) data.campaign.description);
    data.campaign.name = 0;
    data.campaign.description = 0;
    data.campaign.number_of_missions = 0;
    data.mission.info.background_image = 0;
    data.mission.info.get_next_option = get_next_mission_option;
    data.mission.current_option_id = 0;
    data.mission.current = 0;
    data.file_name[0] = 0;
    data.active = 0;
}
