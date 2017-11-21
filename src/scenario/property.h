#ifndef SCENARIO_PROPERTY_H
#define SCENARIO_PROPERTY_H

#include <stdint.h>

typedef enum
{
    CLIMATE_CENTRAL = 0,
    CLIMATE_NORTHERN = 1,
    CLIMATE_DESERT = 2
} scenario_climate;

int scenario_is_open_play();

int scenario_open_play_id();

scenario_climate scenario_property_climate();

int scenario_property_start_year();

int scenario_property_rome_supplies_wheat();

int scenario_property_enemy();

int scenario_property_player_rank();

int scenario_image_id();

const uint8_t *scenario_brief_description();

int scenario_initial_funds();

int scenario_rescue_loan();

#endif // SCENARIO_PROPERTY_H
