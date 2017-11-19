#include "property.h"

#include "scenario/data.h"

int scenario_is_custom()
{
    return scenario.settings.is_custom;
}

void scenario_set_custom(int custom)
{
    scenario.settings.is_custom = custom;
}

int scenario_campaign_rank()
{
    return scenario.settings.campaign_rank;
}

void scenario_set_campaign_rank(int rank)
{
    scenario.settings.campaign_rank = rank;
}

int scenario_campaign_mission()
{
    return scenario.settings.campaign_mission;
}

void scenario_set_campaign_mission(int mission)
{
    scenario.settings.campaign_mission = mission;
}

int scenario_is_tutorial_1()
{
    return !scenario.settings.is_custom && scenario.settings.campaign_rank == 0;
}

int scenario_is_tutorial_2()
{
    return !scenario.settings.is_custom && scenario.settings.campaign_rank == 1;
}

int scenario_is_tutorial_3()
{
    return !scenario.settings.is_custom && scenario.settings.campaign_rank == 2;
}

int scenario_starting_favor()
{
    return scenario.settings.starting_favor;
}

int scenario_starting_personal_savings()
{
    return scenario.settings.starting_personal_savings;
}

int scenario_is_open_play()
{
    return scenario.is_open_play;
}

int scenario_open_play_id()
{
    return scenario.open_play_scenario_id;
}

scenario_climate scenario_property_climate()
{
    return scenario.climate;
}

int scenario_property_start_year()
{
    return scenario.start_year;
}

int scenario_property_rome_supplies_wheat()
{
    return scenario.rome_supplies_wheat;
}

int scenario_property_enemy()
{
    return scenario.enemy_id;
}

int scenario_property_player_rank()
{
    return scenario.player_rank;
}

int scenario_image_id()
{
    return scenario.image_id;
}

const uint8_t *scenario_brief_description()
{
    return scenario.brief_description;
}

int scenario_initial_funds()
{
    return scenario.initial_funds;
}

int scenario_rescue_loan()
{
    return scenario.rescue_loan;
}
