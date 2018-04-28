#include "military.h"

#include "building/menu.h"
#include "city/buildings.h"
#include "city/data_private.h"
#include "city/message.h"
#include "city/ratings.h"
#include "core/calc.h"
#include "empire/city.h"
#include "figure/formation.h"
#include "figure/formation_legion.h"
#include "scenario/distant_battle.h"

void city_military_clear_legionary_legions()
{
    city_data.military.legionary_legions = 0;
}

void city_military_add_legionary_legion()
{
    city_data.military.legionary_legions++;
}

int city_military_has_legionary_legions()
{
    return city_data.military.legionary_legions > 0;
}

int city_military_total_legions()
{
    return city_data.military.total_legions;
}

int city_military_total_soldiers()
{
    return city_data.military.total_soldiers;
}

int city_military_empire_service_legions()
{
    return city_data.military.empire_service_legions;
}

void city_military_clear_empire_service_legions()
{
    city_data.military.empire_service_legions = 0;
}

void city_military_update_totals()
{
    city_data.military.empire_service_legions = 0;
    city_data.military.total_soldiers = 0;
    city_data.military.total_legions = 0;
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        const formation *m = formation_get(i);
        if (m->in_use && m->is_legion) {
            city_data.military.total_legions++;
            city_data.military.total_soldiers += m->num_figures;
            if (m->empire_service && m->num_figures > 0) {
                city_data.military.empire_service_legions++;
            }
        }
    }
}

void city_military_determine_distant_battle_city()
{
    city_data.distant_battle.city = empire_city_get_vulnerable_roman();
}

int city_military_distant_battle_city()
{
    return city_data.distant_battle.city;
}

int city_military_distant_battle_city_is_roman()
{
    return city_data.distant_battle.city_foreign_months_left <= 0;
}

int city_military_distant_battle_enemy_strength()
{
    return city_data.distant_battle.enemy_strength;
}

void city_military_dispatch_to_distant_battle(int roman_strength)
{
    city_data.distant_battle.roman_months_to_travel_forth = scenario_distant_battle_roman_travel_months();
    city_data.distant_battle.roman_strength = roman_strength;
}

int city_military_distant_battle_roman_army_is_traveling()
{
    return city_data.distant_battle.roman_months_to_travel_forth > 0 ||
           city_data.distant_battle.roman_months_to_travel_back > 0;
}

int city_military_distant_battle_roman_army_is_traveling_forth()
{
    return city_data.distant_battle.roman_months_to_travel_forth > 0;
}

int city_military_distant_battle_roman_army_is_traveling_back()
{
    return city_data.distant_battle.roman_months_to_travel_back > 0;
}

int city_military_distant_battle_enemy_months_traveled()
{
    return city_data.distant_battle.enemy_months_traveled;
}

int city_military_distant_battle_roman_months_traveled()
{
    return city_data.distant_battle.roman_months_traveled;
}

int city_military_has_distant_battle()
{
    return city_data.distant_battle.months_until_battle > 0 ||
           city_data.distant_battle.roman_months_to_travel_back > 0 ||
           city_data.distant_battle.roman_months_to_travel_forth > 0 ||
           city_data.distant_battle.city_foreign_months_left > 0;
}

int city_military_months_until_distant_battle()
{
    return city_data.distant_battle.months_until_battle;
}

void city_military_init_distant_battle(int enemy_strength)
{
    city_data.distant_battle.enemy_months_traveled = 1;
    city_data.distant_battle.roman_months_traveled = 1;
    city_data.distant_battle.months_until_battle = 24;
    city_data.distant_battle.enemy_strength = enemy_strength;
    city_data.distant_battle.total_count++;
    city_data.distant_battle.roman_months_to_travel_back = 0;
    city_data.distant_battle.roman_months_to_travel_forth = 0;
}

static void update_time_traveled()
{
    int roman_travel_months = scenario_distant_battle_roman_travel_months();
    int enemy_travel_months = scenario_distant_battle_enemy_travel_months();
    if (city_data.distant_battle.months_until_battle < enemy_travel_months) {
        city_data.distant_battle.enemy_months_traveled = enemy_travel_months - city_data.distant_battle.months_until_battle + 1;
    } else {
        city_data.distant_battle.enemy_months_traveled = 1;
    }
    if (city_data.distant_battle.roman_months_to_travel_forth >= 1) {
        if (roman_travel_months - city_data.distant_battle.roman_months_traveled >
            enemy_travel_months - city_data.distant_battle.enemy_months_traveled) {
            city_data.distant_battle.roman_months_to_travel_forth -= 2;
        } else {
            city_data.distant_battle.roman_months_to_travel_forth--;
        }
        if (city_data.distant_battle.roman_months_to_travel_forth <= 1) {
            city_data.distant_battle.roman_months_to_travel_forth = 1;
        }
        city_data.distant_battle.roman_months_traveled = roman_travel_months - city_data.distant_battle.roman_months_to_travel_forth + 1;
        if (city_data.distant_battle.roman_months_traveled < 1) {
            city_data.distant_battle.roman_months_traveled = 1;
        }
        if (city_data.distant_battle.roman_months_traveled > roman_travel_months) {
            city_data.distant_battle.roman_months_traveled = roman_travel_months;
        }
    }
}

static void set_city_vulnerable()
{
    if (city_data.distant_battle.city) {
        empire_city_set_vulnerable(city_data.distant_battle.city);
    }
}

static void set_city_foreign()
{
    if (city_data.distant_battle.city) {
        empire_city_set_foreign(city_data.distant_battle.city);
    }
    city_data.distant_battle.city_foreign_months_left = 24;
}

static int player_has_won()
{
    int won;
    int pct_loss;
    if (city_data.distant_battle.roman_strength < city_data.distant_battle.enemy_strength) {
        won = 0;
        pct_loss = 100;
    } else {
        won = 1;
        int pct_advantage = calc_percentage(
            city_data.distant_battle.roman_strength - city_data.distant_battle.enemy_strength,
            city_data.distant_battle.roman_strength);
        if (pct_advantage < 10) {
            pct_loss = 70;
        } else if (pct_advantage < 25) {
            pct_loss = 50;
        } else if (pct_advantage < 50) {
            pct_loss = 25;
        } else if (pct_advantage < 75) {
            pct_loss = 15;
        } else if (pct_advantage < 100) {
            pct_loss = 10;
        } else if (pct_advantage < 150) {
            pct_loss = 5;
        } else {
            pct_loss = 0;
        }
    }
    formation_legions_kill_in_distant_battle(pct_loss);
    return won;
}

static void fight_distant_battle()
{
    if (city_data.distant_battle.roman_months_to_travel_forth <= 0) {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_NO_TROOPS, 0, 0);
        city_ratings_change_favor(-50);
        set_city_foreign();
    } else if (city_data.distant_battle.roman_months_to_travel_forth > 2) {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_TOO_LATE, 0, 0);
        city_ratings_change_favor(-25);
        set_city_foreign();
        city_data.distant_battle.roman_months_to_travel_back = city_data.distant_battle.roman_months_traveled;
    } else if (!player_has_won()) {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_TOO_WEAK, 0, 0);
        city_ratings_change_favor(-10);
        set_city_foreign();
        city_data.distant_battle.roman_months_traveled = 0;
        // no return: all soldiers killed
    } else {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_WON, 0, 0);
        city_ratings_change_favor(25);
        city_buildings_earn_triumphal_arch();
        building_menu_update();
        city_data.distant_battle.won_count++;
        city_data.distant_battle.city_foreign_months_left = 0;
        city_data.distant_battle.roman_months_to_travel_back = city_data.distant_battle.roman_months_traveled;
    }
    city_data.distant_battle.months_until_battle = 0;
    city_data.distant_battle.roman_months_traveled = 0;
    city_data.distant_battle.roman_months_to_travel_forth = 0;
}

static void update_aftermath()
{
    if (city_data.distant_battle.roman_months_to_travel_back > 0) {
        city_data.distant_battle.roman_months_to_travel_back--;
        city_data.distant_battle.roman_months_traveled = city_data.distant_battle.roman_months_to_travel_back;
        if (city_data.distant_battle.roman_months_to_travel_back <= 0) {
            if (city_data.distant_battle.city_foreign_months_left) {
                // soldiers return - not in time
                city_message_post(1, MESSAGE_TROOPS_RETURN_FAILED, 0, city_data.map.exit_point.grid_offset);
            } else {
                // victorious
                city_message_post(1, MESSAGE_TROOPS_RETURN_VICTORIOUS, 0, city_data.map.exit_point.grid_offset);
            }
            city_data.distant_battle.roman_months_traveled = 0;
            formation_legions_return_from_distant_battle();
        }
    } else if (city_data.distant_battle.city_foreign_months_left > 0) {
        city_data.distant_battle.city_foreign_months_left--;
        if (city_data.distant_battle.city_foreign_months_left <= 0) {
            city_message_post(1, MESSAGE_DISTANT_BATTLE_CITY_RETAKEN, 0, 0);
            set_city_vulnerable();
        }
    }
}

void city_military_process_distant_battle()
{
    if (city_data.distant_battle.months_until_battle > 0) {
        --city_data.distant_battle.months_until_battle;
        if (city_data.distant_battle.months_until_battle > 0) {
            update_time_traveled();
        } else {
            fight_distant_battle();
        }
    } else {
        update_aftermath();
    }
}
