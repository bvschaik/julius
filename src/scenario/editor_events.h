#ifndef SCENARIO_EDITOR_EVENTS_H
#define SCENARIO_EDITOR_EVENTS_H

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
void scenario_editor_set_max_wages(int amount);
int scenario_editor_get_max_wages(void);

int scenario_editor_lower_wages_enabled(void);
void scenario_editor_lower_wages_toggle_enabled(void);
void scenario_editor_set_min_wages(int amount);
int scenario_editor_get_min_wages(void);

int scenario_editor_contaminated_water_enabled(void);
void scenario_editor_contaminated_water_toggle_enabled(void);

int scenario_editor_iron_mine_collapse_enabled(void);
void scenario_editor_iron_mine_collapse_toggle_enabled(void);

int scenario_editor_clay_pit_flooded_enabled(void);
void scenario_editor_clay_pit_flooded_toggle_enabled(void);

#endif // SCENARIO_EDITOR_EVENTS_H
