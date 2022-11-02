#ifndef SCENARIO_EDITOR_H
#define SCENARIO_EDITOR_H

#include <stdint.h>

typedef struct {
    int year;
    int resource;
    int amount;
    int deadline_years;
    int favor;
} editor_request;

typedef struct {
    int year;
    int type;
    int amount;
    int from;
    int attack_type;
} editor_invasion;

typedef struct {
    int year;
    int resource;
    int amount;
    int is_rise;
} editor_price_change;

typedef struct {
    int year;
    int resource;
    int route_id;
    int amount;
} editor_demand_change;

void scenario_editor_create(int map_size);

void scenario_editor_set_native_images(int image_hut, int image_meeting, int image_crops);

void scenario_editor_request_get(int index, editor_request *request);
void scenario_editor_request_delete(int index);
void scenario_editor_request_save(int index, editor_request *request);

void scenario_editor_invasion_get(int index, editor_invasion *invasion);
void scenario_editor_invasion_delete(int index);
void scenario_editor_invasion_save(int index, editor_invasion *invasion);

void scenario_editor_price_change_get(int index, editor_price_change *price_change);
void scenario_editor_price_change_delete(int index);
void scenario_editor_price_change_save(int index, editor_price_change *price_change);

void scenario_editor_demand_change_get(int index, editor_demand_change *demand_change);
void scenario_editor_demand_change_delete(int index);
void scenario_editor_demand_change_save(int index, editor_demand_change *demand_change);

void scenario_editor_cycle_image(int forward);

void scenario_editor_cycle_climate(void);

void scenario_editor_update_brief_description(const uint8_t *new_description);

void scenario_editor_set_enemy(int enemy_id);

void scenario_editor_change_empire(int change);
void scenario_editor_set_custom_empire(const uint8_t *file_name);
void scenario_editor_unset_custom_empire(void);

int scenario_editor_is_building_allowed(int id);

void scenario_editor_toggle_building_allowed(int id);

void scenario_editor_set_player_rank(int rank);

void scenario_editor_set_initial_funds(int amount);
void scenario_editor_set_rescue_loan(int amount);

void scenario_editor_toggle_rome_supplies_wheat(void);

void scenario_editor_toggle_flotsam(void);

int scenario_editor_milestone_year(int milestone_percentage);
void scenario_editor_set_milestone_year(int milestone_percentage, int year);

void scenario_editor_set_start_year(int year);

void scenario_editor_toggle_open_play(void);

void scenario_editor_toggle_culture(void);
void scenario_editor_set_culture(int goal);
void scenario_editor_toggle_prosperity(void);
void scenario_editor_set_prosperity(int goal);
void scenario_editor_toggle_peace(void);
void scenario_editor_set_peace(int goal);
void scenario_editor_toggle_favor(void);
void scenario_editor_set_favor(int goal);

void scenario_editor_toggle_population(void);
void scenario_editor_set_population(int goal);

void scenario_editor_toggle_time_limit(void);
void scenario_editor_set_time_limit(int years);
void scenario_editor_toggle_survival_time(void);
void scenario_editor_set_survival_time(int years);

#endif // SCENARIO_EDITOR_H
