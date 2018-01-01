#include "distant_battle.h"

#include "building/menu.h"
#include "city/message.h"
#include "city/ratings.h"
#include "core/calc.h"
#include "empire/city.h"
#include "empire/object.h"
#include "empire/type.h"
#include "figure/formation_legion.h"
#include "game/time.h"
#include "scenario/data.h"

#include "Data/CityInfo.h"

int scenario_distant_battle_roman_travel_months()
{
    return scenario.empire.distant_battle_roman_travel_months;
}

void scenario_distant_battle_set_roman_travel_months()
{
    scenario.empire.distant_battle_roman_travel_months =
        empire_object_init_distant_battle_travel_months(EMPIRE_OBJECT_ROMAN_ARMY);
}

void scenario_distant_battle_set_enemy_travel_months()
{
    scenario.empire.distant_battle_enemy_travel_months =
        empire_object_init_distant_battle_travel_months(EMPIRE_OBJECT_ENEMY_ARMY);
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
}

static void update_time_traveled()
{
    if (Data_CityInfo.distantBattleMonthsToBattle < scenario.empire.distant_battle_enemy_travel_months) {
        Data_CityInfo.distantBattleEnemyMonthsTraveled =
            scenario.empire.distant_battle_enemy_travel_months - Data_CityInfo.distantBattleMonthsToBattle + 1;
    } else {
        Data_CityInfo.distantBattleEnemyMonthsTraveled = 1;
    }
    if (Data_CityInfo.distantBattleRomanMonthsToTravel >= 1) {
        if (scenario.empire.distant_battle_roman_travel_months - Data_CityInfo.distantBattleRomanMonthsTraveled >
            scenario.empire.distant_battle_enemy_travel_months - Data_CityInfo.distantBattleEnemyMonthsTraveled) {
            Data_CityInfo.distantBattleRomanMonthsToTravel -= 2;
        } else {
            Data_CityInfo.distantBattleRomanMonthsToTravel--;
        }
        if (Data_CityInfo.distantBattleRomanMonthsToTravel <= 1) {
            Data_CityInfo.distantBattleRomanMonthsToTravel = 1;
        }
        Data_CityInfo.distantBattleRomanMonthsTraveled =
            scenario.empire.distant_battle_roman_travel_months - Data_CityInfo.distantBattleRomanMonthsToTravel + 1;
        if (Data_CityInfo.distantBattleRomanMonthsTraveled < 1) {
            Data_CityInfo.distantBattleRomanMonthsTraveled = 1;
        }
        if (Data_CityInfo.distantBattleRomanMonthsTraveled > scenario.empire.distant_battle_roman_travel_months) {
            Data_CityInfo.distantBattleRomanMonthsTraveled = scenario.empire.distant_battle_roman_travel_months;
        }
    }
}

static void update_aftermath()
{
    if (Data_CityInfo.distantBattleRomanMonthsToReturn > 0) {
        Data_CityInfo.distantBattleRomanMonthsToReturn--;
        Data_CityInfo.distantBattleRomanMonthsTraveled = Data_CityInfo.distantBattleRomanMonthsToReturn;
        if (Data_CityInfo.distantBattleRomanMonthsToReturn <= 0) {
            if (Data_CityInfo.distantBattleCityMonthsUntilRoman) {
                // soldiers return - not in time
                city_message_post(1, MESSAGE_TROOPS_RETURN_FAILED, 0, Data_CityInfo.exitPointGridOffset);
            } else {
                // victorious
                city_message_post(1, MESSAGE_TROOPS_RETURN_VICTORIOUS, 0, Data_CityInfo.exitPointGridOffset);
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

static int player_has_won()
{
    int won;
    int pct_loss;
    if (Data_CityInfo.distantBattleRomanStrength < Data_CityInfo.distantBattleEnemyStrength) {
        won = 0;
        pct_loss = 100;
    } else {
        won = 1;
        int pctAdvantage = calc_percentage(
            Data_CityInfo.distantBattleRomanStrength - Data_CityInfo.distantBattleEnemyStrength,
            Data_CityInfo.distantBattleRomanStrength);
        if (pctAdvantage < 10) {
            pct_loss = 70;
        } else if (pctAdvantage < 25) {
            pct_loss = 50;
        } else if (pctAdvantage < 50) {
            pct_loss = 25;
        } else if (pctAdvantage < 75) {
            pct_loss = 15;
        } else if (pctAdvantage < 100) {
            pct_loss = 10;
        } else if (pctAdvantage < 150) {
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
        Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
    } else if (Data_CityInfo.distantBattleRomanMonthsToTravel > 2) {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_TOO_LATE, 0, 0);
        city_ratings_change_favor(-25);
        set_city_foreign();
        Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
        Data_CityInfo.distantBattleRomanMonthsToReturn = Data_CityInfo.distantBattleRomanMonthsTraveled;
    } else if (!player_has_won()) {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_TOO_WEAK, 0, 0);
        city_ratings_change_favor(-10);
        set_city_foreign();
        Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
        Data_CityInfo.distantBattleRomanMonthsTraveled = 0;
        // no return: all soldiers killed
    } else {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_WON, 0, 0);
        city_ratings_change_favor(25);
        Data_CityInfo.triumphalArchesAvailable++;
        building_menu_update();
        Data_CityInfo.distantBattleWonCount++;
        Data_CityInfo.distantBattleCityMonthsUntilRoman = 0;
        Data_CityInfo.distantBattleRomanMonthsToReturn = Data_CityInfo.distantBattleRomanMonthsTraveled;
    }
    Data_CityInfo.distantBattleMonthsToBattle = 0;
    Data_CityInfo.distantBattleEnemyMonthsTraveled = 0;
    Data_CityInfo.distantBattleRomanMonthsToTravel = 0;
}

void scenario_distant_battle_process()
{
    for (int i = 0; i < MAX_INVASIONS; i++) {
        if (scenario.invasions[i].type == INVASION_TYPE_DISTANT_BATTLE &&
            game_time_year() == scenario.invasions[i].year + scenario.start_year &&
            game_time_month() == scenario.invasions[i].month &&
            scenario.empire.distant_battle_enemy_travel_months > 4 &&
            scenario.empire.distant_battle_roman_travel_months > 4 &&
            Data_CityInfo.distantBattleMonthsToBattle <= 0 &&
            Data_CityInfo.distantBattleRomanMonthsToReturn <= 0 &&
            Data_CityInfo.distantBattleRomanMonthsToTravel <= 0 &&
            Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0) {
            
            city_message_post(1, MESSAGE_CAESAR_REQUESTS_ARMY, 0, 0);
            Data_CityInfo.distantBattleEnemyMonthsTraveled = 1;
            Data_CityInfo.distantBattleRomanMonthsTraveled = 1;
            Data_CityInfo.distantBattleMonthsToBattle = 24;
            Data_CityInfo.distantBattleEnemyStrength = scenario.invasions[i].amount;
            Data_CityInfo.distantBattleTotalCount++;
            Data_CityInfo.distantBattleRomanMonthsToReturn = 0;
            Data_CityInfo.distantBattleRomanMonthsToTravel = 0;
            return;
        }
    }

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
