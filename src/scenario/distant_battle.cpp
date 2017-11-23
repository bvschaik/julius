#include "distant_battle.h"

#include "city/message.h"
#include "core/calc.h"
#include "empire/city.h"
#include "empire/object.h"
#include "empire/type.h"
#include "game/time.h"
#include "scenario/data.h"

#include <data>

#include "cityinfo.h"
#include "formation.h"
#include "sidebarmenu.h"

int scenario_distant_battle_roman_travel_months()
{
    return Data_Scenario.distantBattleTravelMonthsRoman;
}

void scenario_distant_battle_set_roman_travel_months()
{
    Data_Scenario.distantBattleTravelMonthsRoman =
        empire_object_init_distant_battle_travel_months(EMPIRE_OBJECT_ROMAN_ARMY);
}

void scenario_distant_battle_set_enemy_travel_months()
{
    Data_Scenario.distantBattleTravelMonthsEnemy =
        empire_object_init_distant_battle_travel_months(EMPIRE_OBJECT_ENEMY_ARMY);
}

static void set_city_vulnerable()
{
    if (Data_CityInfo.distantBattleCityId)
    {
        empire_city_set_vulnerable(Data_CityInfo.distantBattleCityId);
    }
}

static void set_city_foreign()
{
    if (Data_CityInfo.distantBattleCityId)
    {
        empire_city_set_foreign(Data_CityInfo.distantBattleCityId);
    }
}

static void update_time_traveled()
{
    if (Data_CityInfo.distantBattleMonthsToBattle < Data_Scenario.distantBattleTravelMonthsEnemy)
    {
        Data_CityInfo.distantBattleEnemyMonthsTraveled =
            Data_Scenario.distantBattleTravelMonthsEnemy - Data_CityInfo.distantBattleMonthsToBattle + 1;
    }
    else
    {
        Data_CityInfo.distantBattleEnemyMonthsTraveled = 1;
    }
    if (Data_CityInfo.distantBattleRomanMonthsToTravel >= 1)
    {
        if (Data_Scenario.distantBattleTravelMonthsRoman - Data_CityInfo.distantBattleRomanMonthsTraveled >
                Data_Scenario.distantBattleTravelMonthsEnemy - Data_CityInfo.distantBattleEnemyMonthsTraveled)
        {
            Data_CityInfo.distantBattleRomanMonthsToTravel -= 2;
        }
        else
        {
            Data_CityInfo.distantBattleRomanMonthsToTravel--;
        }
        if (Data_CityInfo.distantBattleRomanMonthsToTravel <= 1)
        {
            Data_CityInfo.distantBattleRomanMonthsToTravel = 1;
        }
        Data_CityInfo.distantBattleRomanMonthsTraveled =
            Data_Scenario.distantBattleTravelMonthsRoman - Data_CityInfo.distantBattleRomanMonthsToTravel + 1;
        if (Data_CityInfo.distantBattleRomanMonthsTraveled < 1)
        {
            Data_CityInfo.distantBattleRomanMonthsTraveled = 1;
        }
        if (Data_CityInfo.distantBattleRomanMonthsTraveled > Data_Scenario.distantBattleTravelMonthsRoman)
        {
            Data_CityInfo.distantBattleRomanMonthsTraveled = Data_Scenario.distantBattleTravelMonthsRoman;
        }
    }
}

static void update_aftermath()
{
    if (Data_CityInfo.distantBattleRomanMonthsToReturn > 0)
    {
        Data_CityInfo.distantBattleRomanMonthsToReturn--;
        Data_CityInfo.distantBattleRomanMonthsTraveled = Data_CityInfo.distantBattleRomanMonthsToReturn;
        if (Data_CityInfo.distantBattleRomanMonthsToReturn <= 0)
        {
            if (Data_CityInfo.distantBattleCityMonthsUntilRoman)
            {
                // soldiers return - not in time
                city_message_post(1, MESSAGE_TROOPS_RETURN_FAILED, 0, Data_CityInfo.exitPointGridOffset);
            }
            else
            {
                // victorious
                city_message_post(1, MESSAGE_TROOPS_RETURN_VICTORIOUS, 0, Data_CityInfo.exitPointGridOffset);
            }
            Data_CityInfo.distantBattleRomanMonthsTraveled = 0;
            Formation_legionsReturnFromDistantBattle();
        }
    }
    else if (Data_CityInfo.distantBattleCityMonthsUntilRoman > 0)
    {
        Data_CityInfo.distantBattleCityMonthsUntilRoman--;
        if (Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0)
        {
            city_message_post(1, MESSAGE_DISTANT_BATTLE_CITY_RETAKEN, 0, 0);
            set_city_vulnerable();
        }
    }
}

static int player_has_won()
{
    int won;
    int pctLoss;
    if (Data_CityInfo.distantBattleRomanStrength < Data_CityInfo.distantBattleEnemyStrength)
    {
        won = 0;
        pctLoss = 100;
    }
    else
    {
        won = 1;
        int pctAdvantage = calc_percentage(
                               Data_CityInfo.distantBattleRomanStrength - Data_CityInfo.distantBattleEnemyStrength,
                               Data_CityInfo.distantBattleRomanStrength);
        if (pctAdvantage < 10)
        {
            pctLoss = 70;
        }
        else if (pctAdvantage < 25)
        {
            pctLoss = 50;
        }
        else if (pctAdvantage < 50)
        {
            pctLoss = 25;
        }
        else if (pctAdvantage < 75)
        {
            pctLoss = 15;
        }
        else if (pctAdvantage < 100)
        {
            pctLoss = 10;
        }
        else if (pctAdvantage < 150)
        {
            pctLoss = 5;
        }
        else
        {
            pctLoss = 0;
        }
    }
    Formation_legionKillSoldiersInDistantBattle(pctLoss);
    return won;
}

static void fight_distant_battle()
{
    if (Data_CityInfo.distantBattleRomanMonthsToTravel <= 0)
    {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_NO_TROOPS, 0, 0);
        CityInfo_Ratings_changeFavor(-50);
        set_city_foreign();
        Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
    }
    else if (Data_CityInfo.distantBattleRomanMonthsToTravel > 2)
    {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_TOO_LATE, 0, 0);
        CityInfo_Ratings_changeFavor(-25);
        set_city_foreign();
        Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
        Data_CityInfo.distantBattleRomanMonthsToReturn = Data_CityInfo.distantBattleRomanMonthsTraveled;
    }
    else if (!player_has_won())
    {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_TOO_WEAK, 0, 0);
        CityInfo_Ratings_changeFavor(-10);
        set_city_foreign();
        Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
        Data_CityInfo.distantBattleRomanMonthsTraveled = 0;
        // no return: all soldiers killed
    }
    else
    {
        city_message_post(1, MESSAGE_DISTANT_BATTLE_WON, 0, 0);
        CityInfo_Ratings_changeFavor(25);
        Data_CityInfo.triumphalArchesAvailable++;
        SidebarMenu_enableBuildingMenuItems();
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
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
        if (Data_Scenario.invasions.type[i] == INVASION_TYPE_DISTANT_BATTLE &&
                game_time_year() == Data_Scenario.invasions.year[i] + Data_Scenario.startYear &&
                game_time_month() == Data_Scenario.invasions_month[i] &&
                Data_Scenario.distantBattleTravelMonthsEnemy > 4 &&
                Data_Scenario.distantBattleTravelMonthsRoman > 4 &&
                Data_CityInfo.distantBattleMonthsToBattle <= 0 &&
                Data_CityInfo.distantBattleRomanMonthsToReturn <= 0 &&
                Data_CityInfo.distantBattleRomanMonthsToTravel <= 0 &&
                Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0)
        {

            city_message_post(1, MESSAGE_CAESAR_REQUESTS_ARMY, 0, 0);
            Data_CityInfo.distantBattleEnemyMonthsTraveled = 1;
            Data_CityInfo.distantBattleRomanMonthsTraveled = 1;
            Data_CityInfo.distantBattleMonthsToBattle = 24;
            Data_CityInfo.distantBattleEnemyStrength = Data_Scenario.invasions.amount[i];
            Data_CityInfo.distantBattleTotalCount++;
            Data_CityInfo.distantBattleRomanMonthsToReturn = 0;
            Data_CityInfo.distantBattleRomanMonthsToTravel = 0;
            return;
        }
    }

    if (Data_CityInfo.distantBattleMonthsToBattle > 0)
    {
        --Data_CityInfo.distantBattleMonthsToBattle;
        if (Data_CityInfo.distantBattleMonthsToBattle > 0)
        {
            update_time_traveled();
        }
        else
        {
            fight_distant_battle();
        }
    }
    else
    {
        update_aftermath();
    }
}
