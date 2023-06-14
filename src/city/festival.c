#include "festival.h"

#include "building/warehouse.h"
#include "building/monument.h"
#include "core/calc.h"
#include "city/constants.h"
#include "city/data_private.h"
#include "city/emperor.h"
#include "city/finance.h"
#include "city/message.h"
#include "city/sentiment.h"
#include "core/config.h"
#include "game/time.h"

auto_festival autofestivals[5] = {
    {0, 3}, // ceres, april
    {1, 6}, // neptune, july
    {2, 4}, // mercury, may
    {3, 2}, // mars, march
    {4, 3}, // venus, april
};

int city_festival_is_planned(void)
{
    return city_data.festival.planned.size != FESTIVAL_NONE;
}

int city_festival_months_since_last(void)
{
    return city_data.festival.months_since_festival;
}

int city_festival_small_cost(void)
{
    return city_data.festival.small_cost;
}

int city_festival_large_cost(void)
{
    return city_data.festival.large_cost;
}

int city_festival_grand_cost(void)
{
    return city_data.festival.grand_cost;
}

int city_festival_grand_wine(void)
{
    return city_data.festival.grand_wine;
}

int city_festival_out_of_wine(void)
{
    return city_data.festival.not_enough_wine;
}

int city_festival_selected_god(void)
{
    return city_data.festival.selected.god;
}

void city_festival_select_god(int god_id)
{
    city_data.festival.selected.god = god_id;
}

int city_festival_selected_size(void)
{
    return city_data.festival.selected.size;
}

int city_festival_selected_game_id(void)
{
    return city_data.games.selected_games_id;
}

int city_festival_games_active(void)
{
    if (city_data.games.games_is_active) {
        return city_data.games.selected_games_id;
    }
    return 0;
}

int city_festival_games_active_venue_type(void)
{
    if (city_data.games.games_is_active) {
        return city_data.games.selected_games_id <= 3 ? BUILDING_COLOSSEUM : BUILDING_HIPPODROME;
    }
    return 0;
}

int city_festival_games_bonus_active(int game_id)
{
    switch (game_id) {
        case 1:
            return city_data.games.naval_battle_bonus_months;
        case 2:
            return city_data.games.executions_bonus_months;
        case 3:
            return city_data.games.imperial_games_bonus_months;
        case 4:
            return city_data.games.games_4_bonus_months;
        default:
            return 0;
    }
}

int city_festival_games_remaining_duration(void)
{
    return city_data.games.remaining_duration;
}

int city_festival_games_planning_time(void)
{
    return city_data.games.months_to_go;
}

int city_festival_games_cooldown(void)
{
    return 0; //calc_bound(36 - city_data.games.months_since_last, 0, 36);
}

int city_festival_select_size(int size)
{
    if (size == FESTIVAL_GRAND && city_data.festival.not_enough_wine) {
        return 0;
    }
    city_data.festival.selected.size = size;
    return 1;
}

void city_festival_schedule(void)
{
    city_data.festival.planned.god = city_data.festival.selected.god;
    city_data.festival.planned.size = city_data.festival.selected.size;
    int cost;
    if (city_data.festival.selected.size == FESTIVAL_SMALL) {
        city_data.festival.planned.months_to_go = 2;
        cost = city_data.festival.small_cost;
    } else if (city_data.festival.selected.size == FESTIVAL_LARGE) {
        city_data.festival.planned.months_to_go = 3;
        cost = city_data.festival.large_cost;
    } else {
        city_data.festival.planned.months_to_go = 4;
        cost = city_data.festival.grand_cost;
    }

    city_finance_process_sundry(cost);

    if (city_data.festival.selected.size == FESTIVAL_GRAND) {
        int wine_needed = city_data.festival.grand_wine;
        if (building_monument_gt_module_is_active(VENUS_MODULE_1_DISTRIBUTE_WINE)) {
            building *venus_gt = building_get(building_monument_get_venus_gt());
            if (wine_needed <= venus_gt->resources[RESOURCE_WINE]) {
                venus_gt->resources[RESOURCE_WINE] -= wine_needed;
                wine_needed = 0;
            } else {
                wine_needed -= venus_gt->resources[RESOURCE_WINE];
                venus_gt->resources[RESOURCE_WINE] = 0;
            }
        }
        building_warehouses_remove_resource(RESOURCE_WINE, wine_needed);
    }
}

static void festival_sentiment_and_deity(int size, int god_id)
{
    if (city_data.festival.first_festival_effect_months <= 0) {
        city_data.festival.first_festival_effect_months = 12;
        switch (size) {
            case FESTIVAL_SMALL: city_data.sentiment.blessing_festival_boost += 6; break;
            case FESTIVAL_LARGE: city_data.sentiment.blessing_festival_boost += 9; break;
            case FESTIVAL_GRAND: city_data.sentiment.blessing_festival_boost += 18; break;
        }
    } else if (city_data.festival.second_festival_effect_months <= 0) {
        city_data.festival.second_festival_effect_months = 12;
        switch (size) {
            case FESTIVAL_SMALL: city_data.sentiment.blessing_festival_boost += 2; break;
            case FESTIVAL_LARGE: city_data.sentiment.blessing_festival_boost += 3; break;
            case FESTIVAL_GRAND: city_data.sentiment.blessing_festival_boost += 9; break;
        }
    }
    city_data.festival.months_since_festival = 1;
    city_data.religion.gods[god_id].months_since_festival = 0;
}

static void throw_auto_festival(int god_id)
{
    festival_sentiment_and_deity(1, god_id);
    city_message_post(0, MESSAGE_AUTO_FESTIVAL_CERES + god_id, 0, 0);
}

static void throw_party(void)
{
    festival_sentiment_and_deity(city_data.festival.planned.size, city_data.festival.planned.god);
    switch (city_data.festival.planned.size) {
        case FESTIVAL_SMALL: city_message_post(1, MESSAGE_SMALL_FESTIVAL, 0, 0); break;
        case FESTIVAL_LARGE: city_message_post(1, MESSAGE_LARGE_FESTIVAL, 0, 0); break;
        case FESTIVAL_GRAND: city_message_post(1, MESSAGE_GRAND_FESTIVAL, 0, 0); break;
    }
    city_data.festival.planned.size = FESTIVAL_NONE;
    city_data.festival.planned.months_to_go = 0;
}

void city_festival_update(void)
{
    city_data.festival.months_since_festival++;
    if (city_data.festival.first_festival_effect_months) {
        --city_data.festival.first_festival_effect_months;
    }
    if (city_data.festival.second_festival_effect_months) {
        --city_data.festival.second_festival_effect_months;
    }
    if (city_festival_is_planned()) {
        city_data.festival.planned.months_to_go--;
        if (city_data.festival.planned.months_to_go <= 0) {
            throw_party();
        }
    }

    if (building_monument_working(BUILDING_PANTHEON)) {
        for (int god = 0; god <= 4; ++god) {
            if (game_time_total_years() % 5 == god && game_time_month() == autofestivals[god].month) {
                throw_auto_festival(god);
            }
        }
    }
}

void city_festival_calculate_costs(void)
{
    int wine_available = city_data.resource.stored_in_warehouses[RESOURCE_WINE];
    if (building_monument_gt_module_is_active(VENUS_MODULE_1_DISTRIBUTE_WINE)) {
        building *venus_gt = building_get(building_monument_get_venus_gt());
        wine_available += venus_gt->resources[RESOURCE_WINE];
    }

    city_data.festival.small_cost = city_data.population.population / 20 + 10;
    city_data.festival.large_cost = city_data.population.population / 10 + 20;
    city_data.festival.grand_cost = city_data.population.population / 5 + 40;
    city_data.festival.grand_wine = city_data.population.population / 500 + 1;
    city_data.festival.not_enough_wine = 0;
    if (wine_available < city_data.festival.grand_wine) {
        city_data.festival.not_enough_wine = 1;
        if (city_data.festival.selected.size == FESTIVAL_GRAND) {
            city_data.festival.selected.size = FESTIVAL_LARGE;
        }
    }
}