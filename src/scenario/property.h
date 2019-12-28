#ifndef SCENARIO_PROPERTY_H
#define SCENARIO_PROPERTY_H

#include <stdint.h>

typedef enum {
    CLIMATE_CENTRAL = 0,
    CLIMATE_NORTHERN = 1,
    CLIMATE_DESERT = 2
} scenario_climate;

int scenario_is_custom(void);
void scenario_set_custom(int custom);

int scenario_campaign_rank(void);
void scenario_set_campaign_rank(int rank);
int scenario_campaign_mission(void);
void scenario_set_campaign_mission(int mission);
int scenario_is_tutorial_1(void);
int scenario_is_tutorial_2(void);
int scenario_is_tutorial_3(void);

int scenario_starting_favor(void);
int scenario_starting_personal_savings(void);

const uint8_t *scenario_name(void);
void scenario_set_name(const uint8_t *name);

const uint8_t *scenario_player_name(void);
void scenario_set_player_name(const uint8_t *name);

/**
 * Save the current player name for carrying over to the next mission
 */
void scenario_save_campaign_player_name(void);

/**
 * Restore saved player name for next mission
 */
void scenario_restore_campaign_player_name(void);

int scenario_is_open_play(void);

int scenario_open_play_id(void);

scenario_climate scenario_property_climate(void);

int scenario_property_start_year(void);

int scenario_property_rome_supplies_wheat(void);

int scenario_property_enemy(void);

int scenario_property_player_rank(void);

int scenario_image_id(void);

const uint8_t *scenario_brief_description(void);

int scenario_initial_funds(void);

int scenario_rescue_loan(void);

#endif // SCENARIO_PROPERTY_H
