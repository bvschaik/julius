#include "emperor.h"

#include "city/finance.h"
#include "city/message.h"
#include "figure/formation.h"
#include "game/difficulty.h"
#include "game/time.h"
#include "scenario/property.h"
#include "scenario/invasion.h"

#include "Data/CityInfo.h"
#include "CityInfo.h"

static void update_debt_state()
{
    if (Data_CityInfo.treasury >= 0) {
        Data_CityInfo.monthsInDebt = -1;
        return;
    }
    if (Data_CityInfo.debtState == 0) {
        // provide bailout
        int rescue_loan = difficulty_adjust_money(scenario_rescue_loan());
        city_finance_process_donation(rescue_loan);
        
        CityInfo_Finance_calculateTotals();
        
        Data_CityInfo.debtState = 1;
        Data_CityInfo.monthsInDebt = 0;
        city_message_post(1, MESSAGE_CITY_IN_DEBT, 0, 0);
        CityInfo_Ratings_reduceProsperityAfterBailout();
    } else if (Data_CityInfo.debtState == 1) {
        Data_CityInfo.debtState = 2;
        Data_CityInfo.monthsInDebt = 0;
        city_message_post(1, MESSAGE_CITY_IN_DEBT_AGAIN, 0, 0);
        CityInfo_Ratings_changeFavor(-5);
    } else if (Data_CityInfo.debtState == 2) {
        if (Data_CityInfo.monthsInDebt == -1) {
            city_message_post(1, MESSAGE_CITY_IN_DEBT_AGAIN, 0, 0);
            Data_CityInfo.monthsInDebt = 0;
        }
        if (game_time_day() == 0) {
            Data_CityInfo.monthsInDebt++;
        }
        if (Data_CityInfo.monthsInDebt >= 12) {
            Data_CityInfo.debtState = 3;
            Data_CityInfo.monthsInDebt = 0;
            if (!Data_CityInfo.numImperialSoldiersInCity) {
                city_message_post(1, MESSAGE_CITY_STILL_IN_DEBT, 0, 0);
                CityInfo_Ratings_changeFavor(-10);
            }
        }
    } else if (Data_CityInfo.debtState == 3) {
        if (Data_CityInfo.monthsInDebt == -1) {
            city_message_post(1, MESSAGE_CITY_STILL_IN_DEBT, 0, 0);
            Data_CityInfo.monthsInDebt = 0;
        }
        if (game_time_day() == 0) {
            Data_CityInfo.monthsInDebt++;
        }
        if (Data_CityInfo.monthsInDebt >= 12) {
            Data_CityInfo.debtState = 4;
            Data_CityInfo.monthsInDebt = 0;
            if (!Data_CityInfo.numImperialSoldiersInCity) {
                CityInfo_Ratings_setMaxFavor(10);
            }
        }
    }
}

static void process_caesar_invasion()
{
    if (Data_CityInfo.numImperialSoldiersInCity) {
        // caesar invasion in progress
        Data_CityInfo.caesarInvasionDurationDayCountdown--;
        if (Data_CityInfo.ratingFavor >= 35 && Data_CityInfo.caesarInvasionDurationDayCountdown < 176) {
            formation_caesar_pause();
        } else if (Data_CityInfo.ratingFavor >= 22) {
            if (Data_CityInfo.caesarInvasionDurationDayCountdown > 0) {
                formation_caesar_retreat();
                if (!Data_CityInfo.caesarInvasionRetreatMessageShown) {
                    Data_CityInfo.caesarInvasionRetreatMessageShown = 1;
                    city_message_post(1, MESSAGE_CAESAR_ARMY_RETREAT, 0, 0);
                }
            } else if (Data_CityInfo.caesarInvasionDurationDayCountdown == 0) {
                city_message_post(1, MESSAGE_CAESAR_ARMY_CONTINUE, 0, 0); // a year has passed (11 months), siege goes on
            }
        }
    } else if (Data_CityInfo.caesarInvasionSoldiersDied && Data_CityInfo.caesarInvasionSoldiersDied >= Data_CityInfo.caesarInvasionSize) {
        // player defeated caesar army
        Data_CityInfo.caesarInvasionSize = 0;
        Data_CityInfo.caesarInvasionSoldiersDied = 0;
        if (Data_CityInfo.ratingFavor < 35) {
            CityInfo_Ratings_changeFavor(10);
            if (Data_CityInfo.caesarInvasionCount < 2) {
                city_message_post(1, MESSAGE_CAESAR_RESPECT_1, 0, 0);
            } else if (Data_CityInfo.caesarInvasionCount < 3) {
                city_message_post(1, MESSAGE_CAESAR_RESPECT_2, 0, 0);
            } else {
                city_message_post(1, MESSAGE_CAESAR_RESPECT_3, 0, 0);
            }
        }
    } else if (Data_CityInfo.caesarInvasionDaysUntilInvasion <= 0) {
        if (Data_CityInfo.ratingFavor <= 10) {
            // warn player that caesar is angry and will invade in a year
            Data_CityInfo.caesarInvasionWarningsGiven++;
            Data_CityInfo.caesarInvasionDaysUntilInvasion = 192;
            if (Data_CityInfo.caesarInvasionWarningsGiven <= 1) {
                city_message_post(1, MESSAGE_CAESAR_WRATH, 0, 0);
            }
        }
    } else {
        Data_CityInfo.caesarInvasionDaysUntilInvasion--;
        if (Data_CityInfo.caesarInvasionDaysUntilInvasion == 0) {
            // invade!
            int size;
            if (Data_CityInfo.caesarInvasionCount == 0) {
                size = 32;
            } else if (Data_CityInfo.caesarInvasionCount == 1) {
                size = 64;
            } else if (Data_CityInfo.caesarInvasionCount == 2) {
                size = 96;
            } else {
                size = 144;
            }
            if (scenario_invasion_start_from_caesar(size)) {
                Data_CityInfo.caesarInvasionCount++;
                Data_CityInfo.caesarInvasionDurationDayCountdown = 192;
                Data_CityInfo.caesarInvasionRetreatMessageShown = 0;
                Data_CityInfo.caesarInvasionSize = size;
                Data_CityInfo.caesarInvasionSoldiersDied = 0;
            }
        }
    }
}

void city_emperor_update()
{
    update_debt_state();
    process_caesar_invasion();
}
