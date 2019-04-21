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

void scenario_editor_request_get(int index, editor_request *request);

void scenario_editor_invasion_get(int index, editor_invasion *invasion);

void scenario_editor_cycle_image(int forward);

void scenario_editor_cycle_climate(void);

void scenario_editor_set_enemy(int enemy_id);

int scenario_editor_is_building_allowed(int id);

void scenario_editor_toggle_building_allowed(int id);

int scenario_editor_earthquake_severity(void);
void scenario_editor_earthquake_cycle_severity(void);
int scenario_editor_earthquake_year(void);
void scenario_editor_earthquake_set_year(int year);

int scenario_editor_gladiator_revolt_enabled(void);
int scenario_editor_gladiator_revolt_year(void);
void scenario_editor_gladiator_revolt_toggle_enabled(void);
void scenario_editor_gladiator_revolt_set_year(int year);

int scenario_editor_emperor_change_enabled(void);
int scenario_editor_emperor_change_year(void);
void scenario_editor_emperor_change_toggle_enabled(void);
void scenario_editor_emperor_change_set_year(int year);

int scenario_editor_sea_trade_problem_enabled(void);
void scenario_editor_sea_trade_problem_toggle_enabled(void);

int scenario_editor_land_trade_problem_enabled(void);
void scenario_editor_land_trade_problem_toggle_enabled(void);

int scenario_editor_raise_wages_enabled(void);
void scenario_editor_raise_wages_toggle_enabled(void);

int scenario_editor_lower_wages_enabled(void);
void scenario_editor_lower_wages_toggle_enabled(void);

int scenario_editor_contaminated_water_enabled(void);
void scenario_editor_contaminated_water_toggle_enabled(void);

int scenario_editor_iron_mine_collapse_enabled(void);
void scenario_editor_iron_mine_collapse_toggle_enabled(void);

int scenario_editor_clay_pit_flooded_enabled(void);
void scenario_editor_clay_pit_flooded_toggle_enabled(void);

#endif // SCENARIO_EDITOR_H
