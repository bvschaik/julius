#include "editor_events.h"

#include "scenario/data.h"

int scenario_editor_earthquake_severity(void)
{
    return scenario.earthquake.severity;
}

int scenario_editor_earthquake_year(void)
{
    return scenario.earthquake.year;
}

void scenario_editor_earthquake_cycle_severity(void)
{
    scenario.earthquake.severity++;
    if (scenario.earthquake.severity > EARTHQUAKE_LARGE) {
        scenario.earthquake.severity = EARTHQUAKE_NONE;
    }
    scenario.is_saved = 0;
}

void scenario_editor_earthquake_set_year(int year)
{
    scenario.earthquake.year = year;
    scenario.is_saved = 0;
}

int scenario_editor_gladiator_revolt_enabled(void)
{
    return scenario.gladiator_revolt.enabled;
}

int scenario_editor_gladiator_revolt_year(void)
{
    return scenario.gladiator_revolt.year;
}

void scenario_editor_gladiator_revolt_toggle_enabled(void)
{
    scenario.gladiator_revolt.enabled = !scenario.gladiator_revolt.enabled;
    scenario.is_saved = 0;
}

void scenario_editor_gladiator_revolt_set_year(int year)
{
    scenario.gladiator_revolt.year = year;
    scenario.is_saved = 0;
}

int scenario_editor_emperor_change_enabled(void)
{
    return scenario.emperor_change.enabled;
}

int scenario_editor_emperor_change_year(void)
{
    return scenario.emperor_change.year;
}

void scenario_editor_emperor_change_toggle_enabled(void)
{
    scenario.emperor_change.enabled = !scenario.emperor_change.enabled;
    scenario.is_saved = 0;
}

void scenario_editor_emperor_change_set_year(int year)
{
    scenario.emperor_change.year = year;
    scenario.is_saved = 0;
}

int scenario_editor_sea_trade_problem_enabled(void)
{
    return scenario.random_events.sea_trade_problem;
}

void scenario_editor_sea_trade_problem_toggle_enabled(void)
{
    scenario.random_events.sea_trade_problem = !scenario.random_events.sea_trade_problem;
    scenario.is_saved = 0;
}

int scenario_editor_land_trade_problem_enabled(void)
{
    return scenario.random_events.land_trade_problem;
}

void scenario_editor_land_trade_problem_toggle_enabled(void)
{
    scenario.random_events.land_trade_problem = !scenario.random_events.land_trade_problem;
    scenario.is_saved = 0;
}

int scenario_editor_raise_wages_enabled(void)
{
    return scenario.random_events.raise_wages;
}

void scenario_editor_raise_wages_toggle_enabled(void)
{
    scenario.random_events.raise_wages = !scenario.random_events.raise_wages;
    scenario.is_saved = 0;
}

void scenario_editor_set_max_wages(int amount)
{
    scenario.random_events.max_wages = amount;
    scenario.is_saved = 0;
}

int scenario_editor_get_max_wages(void)
{
    return scenario.random_events.max_wages;
}

int scenario_editor_lower_wages_enabled(void)
{
    return scenario.random_events.lower_wages;
}

void scenario_editor_lower_wages_toggle_enabled(void)
{
    scenario.random_events.lower_wages = !scenario.random_events.lower_wages;
    scenario.is_saved = 0;
}

void scenario_editor_set_min_wages(int amount)
{
    scenario.random_events.min_wages = amount;
    scenario.is_saved = 0;
}

int scenario_editor_get_min_wages(void)
{
    return scenario.random_events.min_wages;
}

int scenario_editor_contaminated_water_enabled(void)
{
    return scenario.random_events.contaminated_water;
}

void scenario_editor_contaminated_water_toggle_enabled(void)
{
    scenario.random_events.contaminated_water = !scenario.random_events.contaminated_water;
    scenario.is_saved = 0;
}

int scenario_editor_iron_mine_collapse_enabled(void)
{
    return scenario.random_events.iron_mine_collapse;
}

void scenario_editor_iron_mine_collapse_toggle_enabled(void)
{
    scenario.random_events.iron_mine_collapse = !scenario.random_events.iron_mine_collapse;
    scenario.is_saved = 0;
}

int scenario_editor_clay_pit_flooded_enabled(void)
{
    return scenario.random_events.clay_pit_flooded;
}

void scenario_editor_clay_pit_flooded_toggle_enabled(void)
{
    scenario.random_events.clay_pit_flooded = !scenario.random_events.clay_pit_flooded;
    scenario.is_saved = 0;
}
