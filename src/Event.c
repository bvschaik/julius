#include "Event.h"

#include "Building.h"
#include "CityInfo.h"
#include "Figure.h"
#include "Formation.h"
#include "Resource.h"
#include "Routing.h"
#include "SidebarMenu.h"
#include "TerrainGraphics.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Event.h"
#include "Data/Grid.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"

#include "building/count.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/random.h"
#include "empire/city.h"
#include "empire/object.h"
#include "empire/trade_prices.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "game/time.h"
#include "game/tutorial.h"
#include "scenario/property.h"
#include "sound/effect.h"

#include <string.h>

enum {
	RandomEvent_RomeRaisesWages = 1,
	RandomEvent_RomeLowersWages = 2,
	RandomEvent_LandTradeDisrupted = 3,
	RandomEvent_SeaTradeDisrupted = 4,
	RandomEvent_ContaminatedWater = 5,
	RandomEvent_IronMineCollapsed = 6,
	RandomEvent_ClayPitFlooded = 7
};

static void updateTimeTraveledDistantBattle();
static void fightDistantBattle();
static void updateDistantBattleAftermath();
static int playerWonDistantBattle();
static void setDistantBattleCityVulnerable();
static void setDistantBattleCityForeign();

static const int randomEventProbability[128] = {
	0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 0, 3, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 6,
	0, 0, 2, 0, 0, 0, 7, 0, 5, 0, 0, 7, 0, 0, 0, 0,
	0, 7, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
	6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 6, 0, 0,
	0, 7, 0, 1, 6, 0, 0, 0, 0, 0, 2, 0, 0, 4, 0, 0,
	0, 0, 3, 0, 7, 4, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0
};

void Event_handleDistantBattle()
{
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (Data_Scenario.invasions.type[i] == InvasionType_DistantBattle &&
			game_time_year() == Data_Scenario.invasions.year[i] + Data_Scenario.startYear &&
			game_time_month() == Data_Scenario.invasions_month[i] &&
			Data_Scenario.distantBattleTravelMonthsEnemy > 4 &&
			Data_Scenario.distantBattleTravelMonthsRoman > 4 &&
			Data_CityInfo.distantBattleMonthsToBattle <= 0 &&
			Data_CityInfo.distantBattleRomanMonthsToReturn <= 0 &&
			Data_CityInfo.distantBattleRomanMonthsToTravel <= 0 &&
			Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0) {
			
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
	
	if (Data_CityInfo.distantBattleMonthsToBattle > 0) {
		--Data_CityInfo.distantBattleMonthsToBattle;
		if (Data_CityInfo.distantBattleMonthsToBattle > 0) {
			updateTimeTraveledDistantBattle();
		} else {
			fightDistantBattle();
		}
	} else {
		updateDistantBattleAftermath();
	}
}

static void updateTimeTraveledDistantBattle()
{
	if (Data_CityInfo.distantBattleMonthsToBattle < Data_Scenario.distantBattleTravelMonthsEnemy) {
		Data_CityInfo.distantBattleEnemyMonthsTraveled =
			Data_Scenario.distantBattleTravelMonthsEnemy - Data_CityInfo.distantBattleMonthsToBattle + 1;
	} else {
		Data_CityInfo.distantBattleEnemyMonthsTraveled = 1;
	}
	if (Data_CityInfo.distantBattleRomanMonthsToTravel >= 1) {
		if (Data_Scenario.distantBattleTravelMonthsRoman - Data_CityInfo.distantBattleRomanMonthsTraveled >
			Data_Scenario.distantBattleTravelMonthsEnemy - Data_CityInfo.distantBattleEnemyMonthsTraveled) {
			Data_CityInfo.distantBattleRomanMonthsToTravel -= 2;
		} else {
			Data_CityInfo.distantBattleRomanMonthsToTravel--;
		}
		if (Data_CityInfo.distantBattleRomanMonthsToTravel <= 1) {
			Data_CityInfo.distantBattleRomanMonthsToTravel = 1;
		}
		Data_CityInfo.distantBattleRomanMonthsTraveled =
			Data_Scenario.distantBattleTravelMonthsRoman - Data_CityInfo.distantBattleRomanMonthsToTravel + 1;
		if (Data_CityInfo.distantBattleRomanMonthsTraveled < 1) {
			Data_CityInfo.distantBattleRomanMonthsTraveled = 1;
		}
		if (Data_CityInfo.distantBattleRomanMonthsTraveled > Data_Scenario.distantBattleTravelMonthsRoman) {
			Data_CityInfo.distantBattleRomanMonthsTraveled = Data_Scenario.distantBattleTravelMonthsRoman;
		}
	}
}

static void fightDistantBattle()
{
	if (Data_CityInfo.distantBattleRomanMonthsToTravel <= 0) {
		city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_NO_TROOPS, 0, 0);
		CityInfo_Ratings_changeFavor(-50);
		setDistantBattleCityForeign();
		Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
	} else if (Data_CityInfo.distantBattleRomanMonthsToTravel > 2) {
		city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_TOO_LATE, 0, 0);
		CityInfo_Ratings_changeFavor(-25);
		setDistantBattleCityForeign();
		Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
		Data_CityInfo.distantBattleRomanMonthsToReturn = Data_CityInfo.distantBattleRomanMonthsTraveled;
	} else if (!playerWonDistantBattle()) {
		city_message_post(1, MESSAGE_DISTANT_BATTLE_LOST_TOO_WEAK, 0, 0);
		CityInfo_Ratings_changeFavor(-10);
		setDistantBattleCityForeign();
		Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
		Data_CityInfo.distantBattleRomanMonthsTraveled = 0;
		// no return: all soldiers killed
	} else {
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

static void updateDistantBattleAftermath()
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
			Formation_legionsReturnFromDistantBattle();
		}
	} else if (Data_CityInfo.distantBattleCityMonthsUntilRoman > 0) {
		Data_CityInfo.distantBattleCityMonthsUntilRoman--;
		if (Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0) {
			city_message_post(1, MESSAGE_DISTANT_BATTLE_CITY_RETAKEN, 0, 0);
			setDistantBattleCityVulnerable();
		}
	}
}

static int playerWonDistantBattle()
{
	int won;
	int pctLoss;
	if (Data_CityInfo.distantBattleRomanStrength < Data_CityInfo.distantBattleEnemyStrength) {
		won = 0;
		pctLoss = 100;
	} else {
		won = 1;
		int pctAdvantage = calc_percentage(
			Data_CityInfo.distantBattleRomanStrength - Data_CityInfo.distantBattleEnemyStrength,
			Data_CityInfo.distantBattleRomanStrength);
		if (pctAdvantage < 10) {
			pctLoss = 70;
		} else if (pctAdvantage < 25) {
			pctLoss = 50;
		} else if (pctAdvantage < 50) {
			pctLoss = 25;
		} else if (pctAdvantage < 75) {
			pctLoss = 15;
		} else if (pctAdvantage < 100) {
			pctLoss = 10;
		} else if (pctAdvantage < 150) {
			pctLoss = 5;
		} else {
			pctLoss = 0;
		}
	}
	Formation_legionKillSoldiersInDistantBattle(pctLoss);
	return won;
}

void Event_calculateDistantBattleRomanTravelTime()
{
    Data_Scenario.distantBattleTravelMonthsRoman =
        empire_object_init_distant_battle_travel_months(EMPIRE_OBJECT_ROMAN_ARMY);
}

void Event_calculateDistantBattleEnemyTravelTime()
{
    Data_Scenario.distantBattleTravelMonthsEnemy =
        empire_object_init_distant_battle_travel_months(EMPIRE_OBJECT_ENEMY_ARMY);
}

static void setDistantBattleCityVulnerable()
{
	if (Data_CityInfo.distantBattleCityId) {
		empire_city_set_vulnerable(Data_CityInfo.distantBattleCityId);
	}
}

static void setDistantBattleCityForeign()
{
	if (Data_CityInfo.distantBattleCityId) {
		empire_city_set_foreign(Data_CityInfo.distantBattleCityId);
	}
}

void Event_handleRandomEvents()
{
	int event = randomEventProbability[random_byte()];
	if (event <= 0) {
		return;
	}
	switch (event) {
		case RandomEvent_RomeRaisesWages:
			if (Data_Scenario.raiseWagesEnabled) {
				if (Data_CityInfo.wagesRome < 45) {
					Data_CityInfo.wagesRome += 1 + (random_byte_alt() & 3);
					if (Data_CityInfo.wagesRome > 45) {
						Data_CityInfo.wagesRome = 45;
					}
					city_message_post(1, MESSAGE_ROME_RAISES_WAGES, 0, 0);
				}
			}
			break;
		case RandomEvent_RomeLowersWages:
			if (Data_Scenario.lowerWagesEnabled) {
				if (Data_CityInfo.wagesRome > 5) {
					Data_CityInfo.wagesRome -= 1 + (random_byte_alt() & 3);
					city_message_post(1, MESSAGE_ROME_LOWERS_WAGES, 0, 0);
				}
			}
			break;
		case RandomEvent_LandTradeDisrupted:
			if (Data_Scenario.landTradeProblemEnabled) {
				if (Data_CityInfo.tradeNumOpenLandRoutes > 0) {
					Data_CityInfo.tradeLandProblemDuration = 48;
					if (scenario_property_climate() == CLIMATE_DESERT) {
						city_message_post(1, MESSAGE_LAND_TRADE_DISRUPTED_SANDSTORMS, 0, 0);
					} else {
						city_message_post(1, MESSAGE_LAND_TRADE_DISRUPTED_LANDSLIDES, 0, 0);
					}
				}
			}
			break;
		case RandomEvent_SeaTradeDisrupted:
			if (Data_Scenario.seaTradeProblemEnabled) {
				if (Data_CityInfo.tradeNumOpenSeaRoutes > 0) {
					Data_CityInfo.tradeSeaProblemDuration = 48;
					city_message_post(1, MESSAGE_SEA_TRADE_DISRUPTED, 0, 0);
				}
			}
			break;
		case RandomEvent_ContaminatedWater:
			if (Data_Scenario.contaminatedWaterEnabled) {
				if (Data_CityInfo.population > 200) {
					int change;
					if (Data_CityInfo.healthRate > 80) {
						change = -50;
					} else if (Data_CityInfo.healthRate > 60) {
						change = -40;
					} else {
						change = -25;
					}
					CityInfo_Population_changeHealthRate(change);
					city_message_post(1, MESSAGE_CONTAMINATED_WATER, 0, 0);
				}
			}
			break;
		case RandomEvent_IronMineCollapsed:
			if (Data_Scenario.ironMineCollapseEnabled) {
				int gridOffset = Building_collapseFirstOfType(BUILDING_IRON_MINE);
				if (gridOffset) {
					city_message_post(1, MESSAGE_IRON_MINE_COLLAPED, 0, gridOffset);
				}
			}
			break;
		case RandomEvent_ClayPitFlooded:
			if (Data_Scenario.clayPitFloodEnabled) {
				int gridOffset = Building_collapseFirstOfType(BUILDING_CLAY_PIT);
				if (gridOffset) {
					city_message_post(1, MESSAGE_CLAY_PIT_FLOODED, 0, gridOffset);
				}
			}
			break;
	}
}
