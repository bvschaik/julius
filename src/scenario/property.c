#include "property.h"

#include "core/calc.h"
#include "core/string.h"
#include "game/campaign.h"
#include "graphics/image.h"
#include "scenario/data.h"

int scenario_is_custom(void)
{
    return scenario.settings.is_custom;
}

void scenario_set_custom(int custom)
{
    scenario.settings.is_custom = custom;
}

int scenario_campaign_rank(void)
{
    return scenario.campaign.rank;
}

void scenario_set_campaign_rank(int rank)
{
    scenario.campaign.rank = rank;
}

int scenario_campaign_mission(void)
{
    return scenario.campaign.mission;
}

void scenario_set_campaign_mission(int mission)
{
    scenario.campaign.mission = mission;
}

int scenario_is_tutorial_1(void)
{
    return game_campaign_is_original() && scenario.campaign.rank == 0;
}

int scenario_is_tutorial_2(void)
{
    return game_campaign_is_original() && scenario.campaign.rank == 1;
}

int scenario_is_tutorial_3(void)
{
    return game_campaign_is_original() && scenario.campaign.rank == 2;
}

int scenario_starting_favor(void)
{
    return scenario.settings.starting_favor;
}

int scenario_starting_personal_savings(void)
{
    return scenario.settings.starting_personal_savings;
}

void scenario_set_starting_personal_savings(int personal_savings)
{
    scenario.settings.starting_personal_savings = personal_savings;
}

const uint8_t *scenario_name(void)
{
    return scenario.scenario_name;
}

void scenario_set_name(const uint8_t *name)
{
    string_copy(name, scenario.scenario_name, MAX_SCENARIO_NAME);
}

const uint8_t *scenario_player_name(void)
{
    return scenario.settings.player_name;
}

void scenario_set_player_name(const uint8_t *name)
{
    string_copy(name, scenario.settings.player_name, MAX_PLAYER_NAME);
}

void scenario_save_campaign_player_name(void)
{
    string_copy(scenario.settings.player_name, scenario.campaign.player_name, MAX_PLAYER_NAME);
}

void scenario_restore_campaign_player_name(void)
{
    string_copy(scenario.campaign.player_name, scenario.settings.player_name, MAX_PLAYER_NAME);
}

int scenario_is_open_play(void)
{
    return scenario.is_open_play;
}

int scenario_open_play_id(void)
{
    return scenario.open_play_scenario_id;
}

scenario_climate scenario_property_climate(void)
{
    return scenario.climate;
}

int scenario_property_start_year(void)
{
    return scenario.start_year;
}

int scenario_property_rome_supplies_wheat(void)
{
    return scenario.rome_supplies_wheat;
}

int scenario_property_enemy(void)
{
    return scenario.enemy_id;
}

int scenario_property_player_rank(void)
{
    return scenario.player_rank;
}

int scenario_property_caesar_salary(void)
{
    return scenario.caesar_salary;
}

int scenario_image_id(void)
{
    return scenario.image_id;
}

const uint8_t *scenario_brief_description(void)
{
    return scenario.brief_description;
}

int scenario_initial_funds(void)
{
    return scenario.initial_funds;
}

int scenario_rescue_loan(void)
{
    return scenario.rescue_loan;
}

int scenario_intro_message(void)
{
    return scenario.intro_custom_message_id;
}

int scenario_victory_message(void)
{
    return scenario.victory_custom_message_id;
}

void scenario_change_climate(scenario_climate climate)
{
    climate = calc_bound(climate, CLIMATE_CENTRAL, CLIMATE_DESERT);
    scenario.climate = climate;
    image_load_climate(scenario_property_climate(), 0, 0, 0);
}
