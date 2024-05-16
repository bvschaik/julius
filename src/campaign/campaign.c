#include "campaign.h"

#include "campaign/file.h"
#include "campaign/mission.h"
#include "campaign/xml.h"
#include "core/file.h"
#include "core/log.h"
#include "game/file.h"

#include <stdlib.h>
#include <string.h>

static struct {
    int active;
    campaign_info campaign;
    char file_name[FILE_NAME_MAX];
    char suspended_filename[FILE_NAME_MAX];
    campaign_mission_info mission_info;
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
        snprintf(data.file_name, FILE_NAME_MAX, "%s", filename);
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

static int fill_mission_info(const campaign_mission *mission)
{
    if (!mission) {
        data.mission_info.title = 0;
        data.mission_info.intro_video = 0;
        data.mission_info.background_image = 0;
        data.mission_info.first_scenario = 0;
        data.mission_info.total_scenarios = 0;
        data.mission_info.max_personal_savings = 0;
        data.mission_info.next_rank = CAMPAIGN_NO_RANK;
        return 0;
    } else {
        data.mission_info.title = mission->title;
        data.mission_info.intro_video = mission->intro_video;
        data.mission_info.background_image = mission->background_image;
        data.mission_info.max_personal_savings = mission->max_personal_savings;
        data.mission_info.next_rank = mission->next_rank;
        data.mission_info.first_scenario = mission->first_scenario;
        data.mission_info.total_scenarios = mission->last_scenario - mission->first_scenario + 1;
        return 1;
    }
}

const campaign_mission_info *campaign_get_current_mission(int scenario_id)
{
    if (!data.active || !fill_mission_info(campaign_mission_current(scenario_id))) {
        return 0;
    }
    return &data.mission_info;
}

const campaign_mission_info *campaign_get_next_mission(int last_scenario_id)
{
    if (!data.active || !fill_mission_info(campaign_mission_next(last_scenario_id))) {
        return 0;
    }
    return &data.mission_info;
}

int campaign_load_scenario(int scenario_id)
{
    if (!data.active) {
        return 0;
    }
    const campaign_scenario *scenario = campaign_mission_get_scenario(scenario_id);
    if (!scenario || !scenario->path) {
        return 0;
    }
    size_t length;
    uint8_t *scenario_data = campaign_load_file(scenario->path, &length);
    if (!scenario_data) {
        return 0;
    }
    log_info("Loading custom campaign scenario", file_remove_directory(scenario->path), scenario->id);
    int is_save_game = file_has_extension(scenario->path, "sav") || file_has_extension(scenario->path, "svx");
    int result = game_file_start_scenario_from_buffer(scenario_data, (int) length, is_save_game);
    free(scenario_data);
    return result;
}

const campaign_scenario *campaign_get_scenario(int scenario_id)
{
    if (!data.active) {
        return 0;
    }
    return campaign_mission_get_scenario(scenario_id);
}

void campaign_suspend(void)
{
    snprintf(data.suspended_filename, FILE_NAME_MAX, "%s", data.file_name);
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
    free((uint8_t *) data.campaign.author);
    free((uint8_t *) data.campaign.description);
    data.campaign.name = 0;
    data.campaign.author = 0;
    data.campaign.description = 0;
    data.campaign.number_of_missions = 0;
    data.mission_info.background_image = 0;
    data.mission_info.first_scenario = 0;
    data.mission_info.total_scenarios = 0;
    data.file_name[0] = 0;
    data.active = 0;
}
