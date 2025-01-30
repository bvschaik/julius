#include "editor.h"

#include "core/lang.h"
#include "core/string.h"
#include "map/grid.h"
#include "scenario/allowed_building.h"
#include "scenario/criteria.h"
#include "scenario/custom_variable.h"
#include "scenario/demand_change.h"
#include "scenario/empire.h"
#include "scenario/property.h"
#include "scenario/request.h"
#include "scenario/invasion.h"
#include "scenario/scenario.h"

#include <string.h>

static const struct {
    int width;
    int height;
} MAP_SIZES[] = {
    {40, 40},
    {60, 60},
    {80, 80},
    {100, 100},
    {120, 120},
    {160, 160}
};

static int is_saved;

static void init_point(map_point *point)
{
    point->x = -1;
    point->y = -1;
}

int scenario_editor_is_saved(void)
{
    return is_saved;
}

void scenario_editor_set_as_saved(void)
{
    is_saved = 1;
}

void scenario_editor_set_as_unsaved(void)
{
    is_saved = 0;
}

void scenario_editor_create(int map_size)
{
    memset(&scenario, 0, sizeof(scenario));

    scenario.map.width = MAP_SIZES[map_size].width;
    scenario.map.height = MAP_SIZES[map_size].height;
    scenario.map.grid_border_size = GRID_SIZE - scenario.map.width;
    scenario.map.grid_start = (GRID_SIZE - scenario.map.height) / 2 * GRID_SIZE + (GRID_SIZE - scenario.map.width) / 2;

    string_copy(lang_get_string(44, 37), scenario.brief_description, MAX_BRIEF_DESCRIPTION);
    string_copy(lang_get_string(44, 38), scenario.briefing, MAX_BRIEFING);

    scenario.caesar_salary = 100;
    scenario.initial_funds = 1000;
    scenario.rescue_loan = 500;
    scenario.start_year = -500;

    scenario.win_criteria.milestone25_year = 10;
    scenario.win_criteria.milestone50_year = 20;
    scenario.win_criteria.milestone75_year = 30;

    scenario.rome_supplies_wheat = 0;

    scenario.win_criteria.culture.goal = 10;
    scenario.win_criteria.culture.enabled = 1;
    scenario.win_criteria.prosperity.goal = 10;
    scenario.win_criteria.prosperity.enabled = 1;
    scenario.win_criteria.peace.goal = 10;
    scenario.win_criteria.peace.enabled = 1;
    scenario.win_criteria.favor.goal = 10;
    scenario.win_criteria.favor.enabled = 1;
    scenario.win_criteria.population.goal = 0;
    scenario.win_criteria.population.enabled = 0;

    scenario.win_criteria.time_limit.years = 0;
    scenario.win_criteria.time_limit.enabled = 0;
    scenario.win_criteria.survival_time.years = 0;
    scenario.win_criteria.survival_time.enabled = 0;

    scenario.earthquake.severity = 0;
    scenario.earthquake.year = 0;

    init_point(&scenario.earthquake_point);
    init_point(&scenario.entry_point);
    init_point(&scenario.exit_point);
    init_point(&scenario.river_entry_point);
    init_point(&scenario.river_exit_point);
    for (int i = 0; i < MAX_INVASION_POINTS; i++) {
        init_point(&scenario.invasion_points[i]);
    }
    for (int i = 0; i < MAX_FISH_POINTS; i++) {
        init_point(&scenario.fishing_points[i]);
    }
    for (int i = 0; i < MAX_HERD_POINTS; i++) {
        init_point(&scenario.herd_points[i]);
    }

    scenario_request_clear_all();
    scenario_invasion_clear();
    scenario_demand_change_clear_all();
    scenario_custom_variable_delete_all();
    scenario_allowed_building_enable_all();

    scenario.random_events.max_wages = 45;
    scenario.random_events.min_wages = 5;
}

void scenario_editor_set_native_images(int image_hut, int image_meeting, int image_crops)
{
    scenario.native_images.hut = image_hut;
    scenario.native_images.meeting = image_meeting;
    scenario.native_images.crops = image_crops;
}

void scenario_editor_cycle_image(int forward)
{
    if (forward) {
        scenario.image_id++;
    } else {
        scenario.image_id--;
    }
    if (scenario.image_id < 0) {
        scenario.image_id = 15;
    }
    if (scenario.image_id > 15) {
        scenario.image_id = 0;
    }
    scenario_editor_set_as_unsaved();
}

void scenario_editor_cycle_climate(void)
{
    switch (scenario.climate) {
        case CLIMATE_CENTRAL:
            scenario.climate = CLIMATE_NORTHERN;
            break;
        case CLIMATE_NORTHERN:
            scenario.climate = CLIMATE_DESERT;
            break;
        case CLIMATE_DESERT:
        default:
            scenario.climate = CLIMATE_CENTRAL;
            break;
    }
    scenario_editor_set_as_unsaved();
}

void scenario_editor_update_brief_description(const uint8_t *new_description)
{
    if (!string_equals(scenario.brief_description, new_description)) {
        string_copy(new_description, scenario.brief_description, MAX_BRIEF_DESCRIPTION);
        scenario_editor_set_as_unsaved();
    }
}

void scenario_editor_set_enemy(int enemy_id)
{
    scenario.enemy_id = enemy_id;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_change_empire(int change)
{
    scenario.empire.id += change;
    if (scenario.empire.id < 0) {
        scenario.empire.id = 39;
    } else if (scenario.empire.id >= 40) {
        scenario.empire.id = 0;
    }
    scenario_editor_set_as_unsaved();
}

void scenario_editor_set_custom_empire(const char *filename)
{
    scenario.empire.id = SCENARIO_CUSTOM_EMPIRE;
    snprintf(scenario.empire.custom_name, FILE_NAME_MAX, "%s", filename);
}

void scenario_editor_unset_custom_empire(void)
{
    if (scenario.empire.id != SCENARIO_CUSTOM_EMPIRE) {
        return;
    }
    scenario.empire.id = 0;
    scenario.empire.custom_name[0] = 0;
}

void scenario_editor_set_player_rank(int rank)
{
    scenario.player_rank = rank;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_set_caesar_salary(int salary)
{
    if (salary <= 0) {
        salary = 100;
    } else if (salary > 60000) {
        salary = 60000;
    }
    scenario.caesar_salary = salary;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_set_initial_funds(int amount)
{
    scenario.initial_funds = amount;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_set_rescue_loan(int amount)
{
    scenario.rescue_loan = amount;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_toggle_rome_supplies_wheat(void)
{
    scenario.rome_supplies_wheat = !scenario.rome_supplies_wheat;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_toggle_flotsam(void)
{
    scenario.flotsam_enabled = !scenario.flotsam_enabled;
    scenario_editor_set_as_unsaved();
}

int scenario_editor_milestone_year(int milestone_percentage)
{
    switch (milestone_percentage) {
        case 25:
            return scenario.win_criteria.milestone25_year;
        case 50:
            return scenario.win_criteria.milestone50_year;
        case 75:
            return scenario.win_criteria.milestone75_year;
        default:
            return 0;
    }
}

void scenario_editor_set_milestone_year(int milestone_percentage, int year)
{
    switch (milestone_percentage) {
        case 25:
            scenario.win_criteria.milestone25_year = year;
            break;
        case 50:
            scenario.win_criteria.milestone50_year = year;
            break;
        case 75:
            scenario.win_criteria.milestone75_year = year;
            break;
        default:
            return;
    }
    scenario_editor_set_as_unsaved();
}

void scenario_editor_set_start_year(int year)
{
    scenario.start_year = year;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_toggle_open_play(void)
{
    scenario.is_open_play = !scenario.is_open_play;
    if (scenario.is_open_play) {
        scenario.open_play_scenario_id = 12; // fix it to 12: first unused entry
    }
    scenario_editor_set_as_unsaved();
}

void scenario_editor_toggle_culture(void)
{
    scenario.win_criteria.culture.enabled = !scenario.win_criteria.culture.enabled;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_set_culture(int goal)
{
    scenario.win_criteria.culture.goal = goal;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_toggle_prosperity(void)
{
    scenario.win_criteria.prosperity.enabled = !scenario.win_criteria.prosperity.enabled;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_set_prosperity(int goal)
{
    scenario.win_criteria.prosperity.goal = goal;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_toggle_peace(void)
{
    scenario.win_criteria.peace.enabled = !scenario.win_criteria.peace.enabled;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_set_peace(int goal)
{
    scenario.win_criteria.peace.goal = goal;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_toggle_favor(void)
{
    scenario.win_criteria.favor.enabled = !scenario.win_criteria.favor.enabled;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_set_favor(int goal)
{
    scenario.win_criteria.favor.goal = goal;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_toggle_population(void)
{
    scenario.win_criteria.population.enabled = !scenario.win_criteria.population.enabled;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_set_population(int goal)
{
    scenario.win_criteria.population.goal = goal;
    scenario_editor_set_as_unsaved();
}

void scenario_editor_toggle_time_limit(void)
{
    scenario.win_criteria.time_limit.enabled = !scenario.win_criteria.time_limit.enabled;
    if (scenario.win_criteria.time_limit.enabled) {
        scenario.win_criteria.survival_time.enabled = 0;
    }
    scenario_criteria_init_max_year();
    scenario_editor_set_as_unsaved();
}

void scenario_editor_set_time_limit(int years)
{
    scenario.win_criteria.time_limit.years = years;
    scenario_criteria_init_max_year();
    scenario_editor_set_as_unsaved();
}

void scenario_editor_toggle_survival_time(void)
{
    scenario.win_criteria.survival_time.enabled = !scenario.win_criteria.survival_time.enabled;
    if (scenario.win_criteria.survival_time.enabled) {
        scenario.win_criteria.time_limit.enabled = 0;
    }
    scenario_criteria_init_max_year();
    scenario_editor_set_as_unsaved();
}

void scenario_editor_set_survival_time(int years)
{
    scenario.win_criteria.survival_time.years = years;
    scenario_criteria_init_max_year();
    scenario_editor_set_as_unsaved();
}

int scenario_editor_get_custom_message_introduction(void)
{
    return scenario.intro_custom_message_id;
}

void scenario_editor_set_custom_message_introduction(int id)
{
    scenario.intro_custom_message_id = id;
}

int scenario_editor_get_custom_victory_message(void)
{
    return scenario.victory_custom_message_id;
}

void scenario_editor_set_custom_victory_message(int id)
{
    scenario.victory_custom_message_id = id;
}
