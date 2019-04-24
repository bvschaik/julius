#include "editor.h"

#include "scenario/data.h"
#include "scenario/property.h"

void scenario_editor_request_get(int index, editor_request *request)
{
    request->year = scenario.requests[index].year;
    request->amount = scenario.requests[index].amount;
    request->resource = scenario.requests[index].resource;
    request->deadline_years = scenario.requests[index].deadline_years;
    request->favor= scenario.requests[index].favor;
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
    scenario.requests[index].favor = request->deadline_years;
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

void scenario_editor_set_enemy(int enemy_id)
{
    scenario.enemy_id = enemy_id;
    scenario.is_saved = 0;
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
}

void scenario_editor_toggle_flotsam(void)
{
    scenario.flotsam_enabled = !scenario.flotsam_enabled;
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
