#include "editor.h"

#include "core/lang.h"
#include "core/string.h"
#include "map/grid.h"
#include "scenario/data.h"
#include "scenario/empire.h"
#include "scenario/property.h"

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

static void init_point(map_point *point)
{
    point->x = -1;
    point->y = -1;
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

    scenario.initial_funds = 1000;
    scenario.rescue_loan = 500;
    scenario.start_year = -500;

    scenario.win_criteria.milestone25_year = 10;
    scenario.win_criteria.milestone50_year = 20;
    scenario.win_criteria.milestone75_year = 30;

    for (int i = 0; i < MAX_ALLOWED_BUILDINGS; i++) {
        scenario.allowed_buildings[i] = 1;
    }
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

    for (int i = 0; i < MAX_REQUESTS; i++) {
        scenario.requests[i].deadline_years = 5;
        scenario.requests[i].favor = 8;
    }
    for (int i = 0; i < MAX_INVASIONS; i++) {
        scenario.invasions[i].from = 8;
    }

    scenario.random_events.max_wages = 45;
    scenario.random_events.min_wages = 5;

    scenario.is_saved = 1;
}

void scenario_editor_set_native_images(int image_hut, int image_meeting, int image_crops)
{
    scenario.native_images.hut = image_hut;
    scenario.native_images.meeting = image_meeting;
    scenario.native_images.crops = image_crops;
}

void scenario_editor_request_get(int index, editor_request *request)
{
    request->year = scenario.requests[index].year;
    request->amount = scenario.requests[index].amount;
    request->resource = scenario.requests[index].resource;
    request->deadline_years = scenario.requests[index].deadline_years;
    request->favor = scenario.requests[index].favor;
}

static void sort_requests(void)
{
    for (int i = 0; i < MAX_REQUESTS; i++) {
        for (int j = MAX_REQUESTS - 1; j > 0; j--) {
            request_t *current = &scenario.requests[j];
            request_t *prev = &scenario.requests[j-1];
            if (current->resource && (!prev->resource || prev->year > current->year)) {
                request_t tmp = *current;
                *current = *prev;
                *prev = tmp;
            }
        }
    }
}

void scenario_editor_request_delete(int index)
{
    scenario.requests[index].year = 0;
    scenario.requests[index].amount = 0;
    scenario.requests[index].resource = 0;
    scenario.requests[index].deadline_years = 5;
    scenario.requests[index].favor = 8;
    sort_requests();
    scenario.is_saved = 0;
}

void scenario_editor_request_save(int index, editor_request *request)
{
    scenario.requests[index].year = request->year;
    scenario.requests[index].amount = request->amount;
    scenario.requests[index].resource = request->resource;
    scenario.requests[index].deadline_years = request->deadline_years;
    scenario.requests[index].favor = request->favor;
    sort_requests();
    scenario.is_saved = 0;
}

void scenario_editor_invasion_get(int index, editor_invasion *invasion)
{
    invasion->year = scenario.invasions[index].year;
    invasion->type = scenario.invasions[index].type;
    invasion->amount = scenario.invasions[index].amount;
    invasion->from = scenario.invasions[index].from;
    invasion->attack_type = scenario.invasions[index].attack_type;
}

static void sort_invasions(void)
{
    for (int i = 0; i < MAX_INVASIONS; i++) {
        for (int j = MAX_INVASIONS - 1; j > 0; j--) {
            invasion_t *current = &scenario.invasions[j];
            invasion_t *prev = &scenario.invasions[j-1];
            if (current->type && (!prev->type || prev->year > current->year)) {
                invasion_t tmp = *current;
                *current = *prev;
                *prev = tmp;
            }
        }
    }
}

void scenario_editor_invasion_delete(int index)
{
    scenario.invasions[index].year = 0;
    scenario.invasions[index].amount = 0;
    scenario.invasions[index].type = 0;
    scenario.invasions[index].from = 8;
    scenario.invasions[index].attack_type = 0;
    sort_invasions();
    scenario.is_saved = 0;
}

void scenario_editor_invasion_save(int index, editor_invasion *invasion)
{
    scenario.invasions[index].year = invasion->type ? invasion->year : 0;
    scenario.invasions[index].amount = invasion->type ? invasion->amount : 0;
    scenario.invasions[index].type = invasion->type;
    scenario.invasions[index].from = invasion->from;
    scenario.invasions[index].attack_type = invasion->attack_type;
    sort_invasions();
    scenario.is_saved = 0;
}

void scenario_editor_price_change_get(int index, editor_price_change *price_change)
{
    price_change->year = scenario.price_changes[index].year;
    price_change->resource = scenario.price_changes[index].resource;
    price_change->amount = scenario.price_changes[index].amount;
    price_change->is_rise = scenario.price_changes[index].is_rise;
}

static void sort_price_changes(void)
{
    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        if (!scenario.price_changes[i].resource) {
            scenario.price_changes[i].year = 0;
        }
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        for (int j = MAX_PRICE_CHANGES - 1; j > 0; j--) {
            price_change_t *current = &scenario.price_changes[j];
            price_change_t *prev = &scenario.price_changes[j-1];
            if (current->year && (!prev->year || prev->year > current->year)) {
                price_change_t tmp = *current;
                *current = *prev;
                *prev = tmp;
            }
        }
    }
}

void scenario_editor_price_change_delete(int index)
{
    scenario.price_changes[index].year = 0;
    scenario.price_changes[index].resource = 0;
    scenario.price_changes[index].amount = 0;
    scenario.price_changes[index].is_rise = 0;
    sort_price_changes();
    scenario.is_saved = 0;
}

void scenario_editor_price_change_save(int index, editor_price_change *price_change)
{
    scenario.price_changes[index].year = price_change->year;
    scenario.price_changes[index].resource = price_change->resource;
    scenario.price_changes[index].amount = price_change->amount;
    scenario.price_changes[index].is_rise = price_change->is_rise;
    sort_price_changes();
    scenario.is_saved = 0;
}

void scenario_editor_demand_change_get(int index, editor_demand_change *demand_change)
{
    demand_change->year = scenario.demand_changes[index].year;
    demand_change->resource = scenario.demand_changes[index].resource;
    demand_change->route_id = scenario.demand_changes[index].route_id;
    demand_change->amount = scenario.demand_changes[index].amount;
}

static void sort_demand_changes(void)
{
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        if (!scenario.demand_changes[i].resource) {
            scenario.demand_changes[i].year = 0;
        }
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        for (int j = MAX_DEMAND_CHANGES - 1; j > 0; j--) {
            demand_change_t *current = &scenario.demand_changes[j];
            demand_change_t *prev = &scenario.demand_changes[j-1];
            if (current->year && (!prev->year || prev->year > current->year)) {
                demand_change_t tmp = *current;
                *current = *prev;
                *prev = tmp;
            }
        }
    }
}

void scenario_editor_demand_change_delete(int index)
{
    scenario.demand_changes[index].year = 0;
    scenario.demand_changes[index].resource = 0;
    scenario.demand_changes[index].route_id = 0;
    scenario.demand_changes[index].amount = 0;
    sort_demand_changes();
    scenario.is_saved = 0;
}

void scenario_editor_demand_change_save(int index, editor_demand_change *demand_change)
{
    scenario.demand_changes[index].year = demand_change->year;
    scenario.demand_changes[index].resource = demand_change->resource;
    scenario.demand_changes[index].route_id = demand_change->route_id;
    scenario.demand_changes[index].amount = demand_change->amount;
    sort_demand_changes();
    scenario.is_saved = 0;
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
    scenario.is_saved = 0;
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
    scenario.is_saved = 0;
}

void scenario_editor_update_brief_description(const uint8_t *new_description)
{
    if (!string_equals(scenario.brief_description, new_description)) {
        string_copy(new_description, scenario.brief_description, MAX_BRIEF_DESCRIPTION);
        scenario.is_saved = 0;
    }
}

void scenario_editor_set_enemy(int enemy_id)
{
    scenario.enemy_id = enemy_id;
    scenario.is_saved = 0;
}

void scenario_editor_change_empire(int change)
{
    scenario.empire.id += change;
    if (scenario.empire.id < 0) {
        scenario.empire.id = 39;
    } else if (scenario.empire.id >= 40) {
        scenario.empire.id = 0;
    }
    scenario.is_saved = 0;
}

void scenario_editor_set_custom_empire(const uint8_t *filename)
{
    scenario.empire.id = SCENARIO_CUSTOM_EMPIRE;
    string_copy(filename, scenario.empire.custom_name, sizeof(scenario.empire.custom_name));
}

void scenario_editor_unset_custom_empire(void)
{
    scenario.empire.id = 0;
    memset(scenario.empire.custom_name, 0, sizeof(scenario.empire.custom_name));
}

int scenario_editor_is_building_allowed(int id)
{
    return scenario.allowed_buildings[id];
}

void scenario_editor_toggle_building_allowed(int id)
{
    scenario.allowed_buildings[id] = scenario.allowed_buildings[id] ? 0 : 1;
    scenario.is_saved = 0;
}

void scenario_editor_set_player_rank(int rank)
{
    scenario.player_rank = rank;
    scenario.is_saved = 0;
}

void scenario_editor_set_initial_funds(int amount)
{
    scenario.initial_funds = amount;
    scenario.is_saved = 0;
}

void scenario_editor_set_rescue_loan(int amount)
{
    scenario.rescue_loan = amount;
    scenario.is_saved = 0;
}

void scenario_editor_toggle_rome_supplies_wheat(void)
{
    scenario.rome_supplies_wheat = !scenario.rome_supplies_wheat;
    scenario.is_saved = 0;
}

void scenario_editor_toggle_flotsam(void)
{
    scenario.flotsam_enabled = !scenario.flotsam_enabled;
    scenario.is_saved = 0;
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
    scenario.is_saved = 0;
}

void scenario_editor_set_start_year(int year)
{
    scenario.start_year = year;
    scenario.is_saved = 0;
}

void scenario_editor_toggle_open_play(void)
{
    scenario.is_open_play = !scenario.is_open_play;
    if (scenario.is_open_play) {
        scenario.open_play_scenario_id = 12; // fix it to 12: first unused entry
    }
    scenario.is_saved = 0;
}

void scenario_editor_toggle_culture(void)
{
    scenario.win_criteria.culture.enabled = !scenario.win_criteria.culture.enabled;
    scenario.is_saved = 0;
}

void scenario_editor_set_culture(int goal)
{
    scenario.win_criteria.culture.goal = goal;
    scenario.is_saved = 0;
}

void scenario_editor_toggle_prosperity(void)
{
    scenario.win_criteria.prosperity.enabled = !scenario.win_criteria.prosperity.enabled;
    scenario.is_saved = 0;
}

void scenario_editor_set_prosperity(int goal)
{
    scenario.win_criteria.prosperity.goal = goal;
    scenario.is_saved = 0;
}

void scenario_editor_toggle_peace(void)
{
    scenario.win_criteria.peace.enabled = !scenario.win_criteria.peace.enabled;
    scenario.is_saved = 0;
}

void scenario_editor_set_peace(int goal)
{
    scenario.win_criteria.peace.goal = goal;
    scenario.is_saved = 0;
}

void scenario_editor_toggle_favor(void)
{
    scenario.win_criteria.favor.enabled = !scenario.win_criteria.favor.enabled;
    scenario.is_saved = 0;
}

void scenario_editor_set_favor(int goal)
{
    scenario.win_criteria.favor.goal = goal;
    scenario.is_saved = 0;
}

void scenario_editor_toggle_population(void)
{
    scenario.win_criteria.population.enabled = !scenario.win_criteria.population.enabled;
    scenario.is_saved = 0;
}

void scenario_editor_set_population(int goal)
{
    scenario.win_criteria.population.goal = goal;
    scenario.is_saved = 0;
}

void scenario_editor_toggle_time_limit(void)
{
    scenario.win_criteria.time_limit.enabled = !scenario.win_criteria.time_limit.enabled;
    if (scenario.win_criteria.time_limit.enabled) {
        scenario.win_criteria.survival_time.enabled = 0;
    }
    scenario.is_saved = 0;
}

void scenario_editor_set_time_limit(int years)
{
    scenario.win_criteria.time_limit.years = years;
    scenario.is_saved = 0;
}

void scenario_editor_toggle_survival_time(void)
{
    scenario.win_criteria.survival_time.enabled = !scenario.win_criteria.survival_time.enabled;
    if (scenario.win_criteria.survival_time.enabled) {
        scenario.win_criteria.time_limit.enabled = 0;
    }
    scenario.is_saved = 0;
}

void scenario_editor_set_survival_time(int years)
{
    scenario.win_criteria.survival_time.years = years;
    scenario.is_saved = 0;
}
