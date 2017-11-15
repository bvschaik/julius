#include "Event.h"

#include "Building.h"
#include "core/calc.h"
#include "CityInfo.h"
#include "Figure.h"
#include "Formation.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Grid.h"
#include "Data/Invasion.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/Figure.h"

#include "city/message.h"
#include "core/random.h"
#include "empire/object.h"
#include "empire/type.h"
#include "figure/formation.h"
#include "figure/name.h"
#include "game/difficulty.h"
#include "game/time.h"
#include "scenario/invasion.h"
#include "scenario/map.h"
#include "scenario/property.h"


int Event_existsUpcomingInvasion()
{
	for (int i = 0; i < MAX_INVASION_WARNINGS; i++) {
		if (Data_InvasionWarnings[i].inUse && Data_InvasionWarnings[i].handled) {
			return 1;
		}
	}
	return 0;
}

static void updateDebtState()
{
	if (Data_CityInfo.treasury >= 0) {
		Data_CityInfo.monthsInDebt = -1;
		return;
	}
	if (Data_CityInfo.debtState == 0) {
		// provide bailout
		int rescueLoan = difficulty_adjust_money(Data_Scenario.rescueLoan);
		Data_CityInfo.treasury += rescueLoan;
		Data_CityInfo.financeDonatedThisYear += rescueLoan;
		
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


void Event_Caesar_update()
{
	updateDebtState();
	scenario_invasion_process_caesar();
}
