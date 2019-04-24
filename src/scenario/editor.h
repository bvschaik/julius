#ifndef SCENARIO_EDITOR_H
#define SCENARIO_EDITOR_H

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

void scenario_editor_request_get(int index, editor_request *request);
void scenario_editor_request_delete(int index);
void scenario_editor_request_save(int index, editor_request *request);

void scenario_editor_invasion_get(int index, editor_invasion *invasion);
void scenario_editor_invasion_delete(int index);
void scenario_editor_invasion_save(int index, editor_invasion *invasion);

void scenario_editor_price_change_get(int index, editor_price_change *price_change);
void scenario_editor_price_change_delete(int index);
void scenario_editor_price_change_save(int index, editor_price_change *price_change);

void scenario_editor_cycle_image(int forward);

void scenario_editor_cycle_climate(void);

void scenario_editor_set_enemy(int enemy_id);

int scenario_editor_is_building_allowed(int id);

void scenario_editor_toggle_building_allowed(int id);

void scenario_editor_set_player_rank(int rank);

void scenario_editor_set_initial_funds(int amount);
void scenario_editor_set_rescue_loan(int amount);

void scenario_editor_toggle_rome_supplies_wheat(void);

void scenario_editor_toggle_flotsam(void);

void scenario_editor_set_milestone_year(int milestone_percentage, int year);

#endif // SCENARIO_EDITOR_H
