#include "property.h"

#include "Data/Scenario.h"

int scenario_is_open_play()
{
    return Data_Scenario.isOpenPlay;
}

int scenario_open_play_id()
{
    return Data_Scenario.openPlayScenarioId;
}

scenario_climate scenario_property_climate()
{
    return Data_Scenario.climate;
}

int scenario_property_start_year()
{
    return Data_Scenario.startYear;
}

int scenario_property_rome_supplies_wheat()
{
    return Data_Scenario.romeSuppliesWheat;
}

int scenario_property_enemy()
{
    return Data_Scenario.enemyId;
}

int scenario_property_player_rank()
{
    return Data_Scenario.playerRank;
}

int scenario_image_id()
{
    return Data_Scenario.imageId;
}

const uint8_t *scenario_brief_description()
{
    return Data_Scenario.briefDescription;
}
