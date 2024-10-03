#include "game/campaign.h"

#include "core/file.h"
#include "core/lang.h"
#include "core/log.h"
#include "game/campaign/file.h"
#include "game/campaign/mission.h"
#include "game/campaign/original.h"
#include "game/campaign/player_data.h"
#include "game/campaign/xml.h"
#include "game/file.h"
#include "translation/translation.h"

#include <stdlib.h>
#include <string.h>

static struct {
    int active;
    int is_custom;
    campaign_info campaign;
    char file_name[FILE_NAME_MAX];
    char suspended_filename[FILE_NAME_MAX];
    campaign_mission_info mission_info;
} data;

static void get_original_campaign_data(void)
{
    data.campaign.starting_rank = 0;
    data.campaign.number_of_missions = 11;
    data.campaign.current_mission = 11;
    data.campaign.name = lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ORIGINAL_CAMPAIGN_NAME);
    data.campaign.author = 0;
    data.campaign.description = lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ORIGINAL_CAMPAIGN_DESC);

    if (!campaign_original_setup()) {
        log_error("Error setting up original campaign data", 0, 0);
        game_campaign_clear();
        return;
    }
    data.active = 1;
}

static void get_custom_campaign_data(const char *filename)
{
    if (!campaign_file_open_zip()) {
        log_error("Error opening campaign file", 0, 0);
        game_campaign_clear();
        return;
    }
    log_info("Opening campaign settings file", 0, 0);

    size_t xml_size;
    char *xml_text = campaign_file_load("settings.xml", &xml_size);
    if (!xml_text) {
        log_error("Error loading campaign settings file", 0, 0);
        game_campaign_clear();
        return;
    }
    int result = campaign_xml_get_info(xml_text, xml_size, &data.campaign);
    campaign_file_close_zip();

    free(xml_text);

    if (!result) {
        game_campaign_clear();
    }

    data.campaign.current_mission = campaign_player_data_get_current_mission(filename);

    data.active = result;
}

int game_campaign_load(const char *filename)
{
    if (!filename || !*filename) {
        game_campaign_clear();
        return 0;
    }

    if (strcmp(filename, data.file_name) == 0) {
        data.active = 1;
        return 1;
    }

    game_campaign_clear();
    if (strcmp(filename, CAMPAIGN_ORIGINAL_NAME) == 0) {
        data.is_custom = 0;
        get_original_campaign_data();
    } else {
        data.is_custom = 1;
        campaign_file_set_path(filename);
        get_custom_campaign_data(filename);
    }
    if (data.active) {
        snprintf(data.file_name, FILE_NAME_MAX, "%s", filename);
    }
    return data.active;
}

int game_campaign_is_active(void)
{
    return data.active;
}

int game_campaign_is_custom(void)
{
    return data.is_custom;
}

int game_campaign_is_original(void)
{
    return data.active && !data.is_custom;
}

const char *game_campaign_get_name(void)
{
    static char disabled_file_name;
    return data.active ? data.file_name : &disabled_file_name;
}

const campaign_info *game_campaign_get_info(void)
{
    return data.active ? &data.campaign : 0;
}

int game_campaign_has_file(const char *filename)
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

uint8_t *game_campaign_load_file(const char *filename, size_t *length)
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
        data.mission_info.background_image.id = 0;
        data.mission_info.background_image.path = 0;
        data.mission_info.first_scenario = 0;
        data.mission_info.total_scenarios = 0;
        data.mission_info.max_personal_savings = 0;
        data.mission_info.next_rank = CAMPAIGN_NO_RANK;
        return 0;
    } else {
        data.mission_info.title = mission->title;
        data.mission_info.intro_video = mission->intro_video;
        data.mission_info.background_image.id = mission->background_image.id;
        data.mission_info.background_image.path = mission->background_image.path;
        data.mission_info.max_personal_savings = mission->max_personal_savings;
        data.mission_info.next_rank = mission->next_rank;
        data.mission_info.first_scenario = mission->first_scenario;
        data.mission_info.total_scenarios = mission->last_scenario - mission->first_scenario + 1;
        return 1;
    }
}

const campaign_mission_info *game_campaign_get_current_mission(int scenario_id)
{
    if (!data.active || !fill_mission_info(campaign_mission_current(scenario_id))) {
        return 0;
    }
    return &data.mission_info;
}

const campaign_mission_info *game_campaign_advance_mission(int last_scenario_id)
{
    if (!data.active) {
        return 0;
    }
    campaign_mission *mission = campaign_mission_next(last_scenario_id);
    if (!fill_mission_info(mission)) {
        mission = campaign_mission_current(last_scenario_id);
        if (mission && game_campaign_is_custom()) {
            campaign_player_data_update_current_mission(data.file_name, mission->id + 1);
        }
        return 0;
    }
    if (game_campaign_is_custom()) {
        campaign_player_data_update_current_mission(data.file_name, mission->id);
    }
    return &data.mission_info;
}

int game_campaign_load_scenario(int scenario_id)
{
    if (!data.active) {
        return 0;
    }
    const campaign_scenario *scenario = campaign_mission_get_scenario(scenario_id);
    if (!scenario || !scenario->path) {
        return 0;
    }
    size_t length;
    uint8_t *scenario_data;
    if (game_campaign_is_original()) {
        scenario_data = campaign_original_load_scenario(scenario_id, &length);
        log_info("Loading original campaign scenario", 0, scenario->id);
    } else {
        scenario_data = game_campaign_load_file(scenario->path, &length);
        log_info("Loading custom campaign scenario", file_remove_path(scenario->path), scenario->id);
    }
    int is_save_game = game_campaign_is_original() ||
        file_has_extension(scenario->path, "sav") || file_has_extension(scenario->path, "svx");
    int result = game_file_start_scenario_from_buffer(scenario_data, (int) length, is_save_game);
    free(scenario_data);
    return result;
}

int game_campaign_load_scenario_info(int scenario_id, saved_game_info *info)
{
    if (!data.active) {
        return 0;
    }
    const campaign_scenario *scenario = campaign_mission_get_scenario(scenario_id);
    if (!scenario || !scenario->path) {
        return 0;
    }
    size_t length;
    uint8_t *scenario_data;
    if (game_campaign_is_original()) {
        scenario_data = campaign_original_load_scenario(scenario_id, &length);
    } else {
        scenario_data = game_campaign_load_file(scenario->path, &length);
    }
    if (!scenario_data) {
        return 0;
    }
    buffer buf;
    buffer_init(&buf, scenario_data, (int) length);
    int result;

    if (game_campaign_is_original() ||
        file_has_extension(scenario->path, "sav") || file_has_extension(scenario->path, "svx")) {
        result = game_file_io_read_saved_game_info_from_buffer(&buf, info);
    } else {
        result = game_file_io_read_scenario_info_from_buffer(&buf, info);
    }
    free(scenario_data);
    return result;
}

const campaign_scenario *game_campaign_get_scenario(int scenario_id)
{
    if (!data.active) {
        return 0;
    }
    return campaign_mission_get_scenario(scenario_id);
}

void game_campaign_suspend(void)
{
    snprintf(data.suspended_filename, FILE_NAME_MAX, "%s", data.file_name);
    data.active = 0;
}

void game_campaign_restore(void)
{
    game_campaign_load(data.suspended_filename);
}

void game_campaign_clear(void)
{
    campaign_file_set_path(0);
    campaign_mission_clear();
    if (data.is_custom) {
        free((uint8_t *) data.campaign.name);
        free((uint8_t *) data.campaign.author);
        free((uint8_t *) data.campaign.description);
    }
    data.campaign.name = 0;
    data.campaign.author = 0;
    data.campaign.description = 0;
    data.campaign.number_of_missions = 0;
    data.mission_info.background_image.id = 0;
    data.mission_info.background_image.path = 0;
    data.mission_info.first_scenario = 0;
    data.mission_info.total_scenarios = 0;
    data.file_name[0] = 0;
    data.is_custom = 0;
    data.active = 0;
}
