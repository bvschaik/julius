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

#include "Data/CityInfo.h"

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
    Data_CityInfo.distantBattleCityId = empire_city_get_vulnerable_roman();
}

int city_military_distant_battle_city()
{
    return Data_CityInfo.distantBattleCityId;
}

int city_military_distant_battle_city_is_roman()
{
    return Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0;
}

void city_military_dispatch_to_distant_battle(int roman_strength)
{
    Data_CityInfo.distantBattleRomanMonthsToTravel = scenario_distant_battle_roman_travel_months();
    Data_CityInfo.distantBattleRomanStrength = roman_strength;
}

int city_military_has_distant_battle()
{
    return Data_CityInfo.distantBattleMonthsToBattle > 0 ||
           Data_CityInfo.distantBattleRomanMonthsToReturn > 0 ||
           Data_CityInfo.distantBattleRomanMonthsToTravel > 0 ||
           Data_CityInfo.distantBattleCityMonthsUntilRoman > 0;
}

void city_military_init_distant_battle(int enemy_strength)
{
    Data_CityInfo.distantBattleEnemyMonthsTraveled = 1;
    Data_CityInfo.distantBattleRomanMonthsTraveled = 1;
    Data_CityInfo.distantBattleMonthsToBattle = 24;
    Data_CityInfo.distantBattleEnemyStrength = enemy_strength;
    Data_CityInfo.distantBattleTotalCount++;
    Data_CityInfo.distantBattleRomanMonthsToReturn = 0;
    Data_CityInfo.distantBattleRomanMonthsToTravel = 0;
}

static void update_time_traveled()
{
    int roman_travel_months = scenario_distant_battle_roman_travel_months();
    int enemy_travel_months = scenario_distant_battle_enemy_travel_months();
    if (Data_CityInfo.distantBattleMonthsToBattle < enemy_travel_months) {
        Data_CityInfo.distantBattleEnemyMonthsTraveled = enemy_travel_months - Data_CityInfo.distantBattleMonthsToBattle + 1;
    } else {
        Data_CityInfo.distantBattleEnemyMonthsTraveled = 1;
    }
    if (Data_CityInfo.distantBattleRomanMonthsToTravel >= 1) {
        if (roman_travel_months - Data_CityInfo.distantBattleRomanMonthsTraveled >
            enemy_travel_months - Data_CityInfo.distantBattleEnemyMonthsTraveled) {
            Data_CityInfo.distantBattleRomanMonthsToTravel -= 2;
        } else {
            Data_CityInfo.distantBattleRomanMonthsToTravel--;
        }
        if (Data_CityInfo.distantBattleRomanMonthsToTravel <= 1) {
            Data_CityInfo.distantBattleRomanMonthsToTravel = 1;
        }
        Data_CityInfo.distantBattleRomanMonthsTraveled = roman_travel_months - Data_CityInfo.distantBattleRomanMonthsToTravel + 1;
        if (Data_CityInfo.distantBattleRomanMonthsTraveled < 1) {
            Data_CityInfo.distantBattleRomanMonthsTraveled = 1;
        }
        if (Data_CityInfo.distantBattleRomanMonthsTraveled > roman_travel_months) {
            Data_CityInfo.distantBattleRomanMonthsTraveled = roman_travel_months;
        }
    }
}

static void set_city_vulnerable()
{
    if (Data_CityInfo.distantBattleCityId) {
        empire_city_set_vulnerable(Data_CityInfo.distantBattleCityId);
    }
}

static void set_city_foreign()
{
    if (Data_CityInfo.distantBattleCityId) {
        empire_city_set_foreign(Data_CityInfo.distantBattleCityId);
    }
    Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
}

static int player_has_won()
{
    int won;
    int pct_loss;
    if (Data_CityInfo.distantBattleRomanStrength < Data_CityInfo.distantBattleEnemyStrength) {
        won = 0;
        pct_loss = 100;
    } else {
        won = 1;
        int pct_advantage = calc_percentage(
            Data_CityInfo.distantBattleRomanStrength - Data_CityInfo.distantBattleEnemyStrength,
            Data_CityInfo.distantBattleRomanStrength);
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
    if (Data_CityInfo.distantBattleRomanMonthsToTravel <= 0) {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_NO_TROOPS, 0, 0);
        city_ratings_change_favor(-50);
        set_city_foreign();
    } else if (Data_CityInfo.distantBattleRomanMonthsToTravel > 2) {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_TOO_LATE, 0, 0);
        city_ratings_change_favor(-25);
        set_city_foreign();
        Data_CityInfo.distantBattleRomanMonthsToReturn = Data_CityInfo.distantBattleRomanMonthsTraveled;
    } else if (!player_has_won()) {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_TOO_WEAK, 0, 0);
        city_ratings_change_favor(-10);
        set_city_foreign();
        Data_CityInfo.distantBattleRomanMonthsTraveled = 0;
        // no return: all soldiers killed
    } else {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_WON, 0, 0);
        city_ratings_change_favor(25);
        city_buildings_earn_triumphal_arch();
        building_menu_update();
        Data_CityInfo.distantBattleWonCount++;
        Data_CityInfo.distantBattleCityMonthsUntilRoman = 0;
        Data_CityInfo.distantBattleRomanMonthsToReturn = Data_CityInfo.distantBattleRomanMonthsTraveled;
    }
    Data_CityInfo.distantBattleMonthsToBattle = 0;
    Data_CityInfo.distantBattleEnemyMonthsTraveled = 0;
    Data_CityInfo.distantBattleRomanMonthsToTravel = 0;
}

static void update_aftermath()
{
    if (Data_CityInfo.distantBattleRomanMonthsToReturn > 0) {
        Data_CityInfo.distantBattleRomanMonthsToReturn--;
        Data_CityInfo.distantBattleRomanMonthsTraveled = Data_CityInfo.distantBattleRomanMonthsToReturn;
        if (Data_CityInfo.distantBattleRomanMonthsToReturn <= 0) {
            if (Data_CityInfo.distantBattleCityMonthsUntilRoman) {
                // soldiers return - not in time
                city_message_post(1, MESSAGE_TROOPS_RETURN_FAILED, 0, city_data.map.exit_point.grid_offset);
            } else {
                // victorious
                city_message_post(1, MESSAGE_TROOPS_RETURN_VICTORIOUS, 0, city_data.map.exit_point.grid_offset);
            }
            Data_CityInfo.distantBattleRomanMonthsTraveled = 0;
            formation_legions_return_from_distant_battle();
        }
    } else if (Data_CityInfo.distantBattleCityMonthsUntilRoman > 0) {
        Data_CityInfo.distantBattleCityMonthsUntilRoman--;
        if (Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0) {
            city_message_post(1, MESSAGE_DISTANT_BATTLE_CITY_RETAKEN, 0, 0);
            set_city_vulnerable();
        }
    }
}

void city_military_process_distant_battle()
{
    if (Data_CityInfo.distantBattleMonthsToBattle > 0) {
        --Data_CityInfo.distantBattleMonthsToBattle;
        if (Data_CityInfo.distantBattleMonthsToBattle > 0) {
            update_time_traveled();
        } else {
            fight_distant_battle();
        }
    } else {
        update_aftermath();
    }
}
