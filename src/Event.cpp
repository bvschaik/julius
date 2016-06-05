#include "Event.h"

#include "Building.h"
#include "Calc.h"
#include "CityInfo.h"
#include "Empire.h"
#include "Formation.h"
#include "PlayerMessage.h"
#include "Random.h"
#include "Resource.h"
#include "Routing.h"
#include "SidebarMenu.h"
#include "Sound.h"
#include "TerrainGraphics.h"
#include "Util.h"
#include "Walker.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Empire.h"
#include "Data/Event.h"
#include "Data/Grid.h"
#include "Data/Random.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/Trade.h"
#include "Data/Tutorial.h"

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
static void advanceEarthquakeToTile(int x, int y);
static int canAdvanceEarthquakeToTile(int x, int y);

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
			Data_CityInfo_Extra.gameTimeYear == Data_Scenario.invasions.year[i] + Data_Scenario.startYear &&
			Data_CityInfo_Extra.gameTimeMonth == Data_Scenario.invasions_month[i] &&
			Data_Scenario.distantBattleTravelMonthsEnemy > 4 &&
			Data_Scenario.distantBattleTravelMonthsRoman > 4 &&
			Data_CityInfo.distantBattleMonthsToBattle <= 0 &&
			Data_CityInfo.distantBattleRomanMonthsToReturn <= 0 &&
			Data_CityInfo.distantBattleRomanMonthsToTravel <= 0 &&
			Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0) {
			
			PlayerMessage_post(1, Message_30_CaesarRequestsArmy, 0, 0);
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
		PlayerMessage_post(1, Message_84_DistantBattleLostNoTroops, 0, 0);
		CityInfo_Ratings_changeFavor(-50);
		setDistantBattleCityForeign();
		Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
	} else if (Data_CityInfo.distantBattleRomanMonthsToTravel > 2) {
		PlayerMessage_post(1, Message_85_DistantBattleLostTooLate, 0, 0);
		CityInfo_Ratings_changeFavor(-25);
		setDistantBattleCityForeign();
		Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
		Data_CityInfo.distantBattleRomanMonthsToReturn = Data_CityInfo.distantBattleRomanMonthsTraveled;
	} else if (!playerWonDistantBattle()) {
		PlayerMessage_post(1, Message_86_DistantBattleLostTooWeak, 0, 0);
		CityInfo_Ratings_changeFavor(-10);
		setDistantBattleCityForeign();
		Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
		Data_CityInfo.distantBattleRomanMonthsTraveled = 0;
		// no return: all soldiers killed
	} else {
		PlayerMessage_post(1, Message_87_DistantBattleWon, 0, 0);
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
				PlayerMessage_post(1, Message_88_TroopsReturnFailed, 0, Data_CityInfo.exitPointGridOffset);
			} else {
				// victorious
				PlayerMessage_post(1, Message_89_TroopsReturnVictorious, 0, Data_CityInfo.exitPointGridOffset);
			}
			Data_CityInfo.distantBattleRomanMonthsTraveled = 0;
			Formation_legionsReturnFromDistantBattle();
		}
	} else if (Data_CityInfo.distantBattleCityMonthsUntilRoman > 0) {
		Data_CityInfo.distantBattleCityMonthsUntilRoman--;
		if (Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0) {
			PlayerMessage_post(1, Message_90_DistantBattleCityRetaken, 0, 0);
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
		int pctAdvantage = Calc_getPercentage(
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
	Data_Scenario.distantBattleTravelMonthsRoman = 0;
	for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
		if (Data_Empire_Objects[i].inUse && Data_Empire_Objects[i].type == EmpireObject_RomanArmy) {
			Data_Scenario.distantBattleTravelMonthsRoman++;
			Data_Empire_Objects[i].distantBattleTravelMonths = Data_Scenario.distantBattleTravelMonthsRoman;
		}
	}
}

void Event_calculateDistantBattleEnemyTravelTime()
{
	Data_Scenario.distantBattleTravelMonthsEnemy = 0;
	for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
		if (Data_Empire_Objects[i].inUse && Data_Empire_Objects[i].type == EmpireObject_EnemyArmy) {
			Data_Scenario.distantBattleTravelMonthsEnemy++;
			Data_Empire_Objects[i].distantBattleTravelMonths = Data_Scenario.distantBattleTravelMonthsEnemy;
		}
	}
}

static void setDistantBattleCityVulnerable()
{
	if (Data_CityInfo.distantBattleCityId) {
		Data_Empire_Cities[Data_CityInfo.distantBattleCityId].cityType = EmpireCity_VulnerableRoman;
	}
}

static void setDistantBattleCityForeign()
{
	if (Data_CityInfo.distantBattleCityId) {
		Data_Empire_Cities[Data_CityInfo.distantBattleCityId].cityType = EmpireCity_DistantForeign;
	}
}

void Event_initDistantBattleCity()
{
	Data_CityInfo.distantBattleCityId = 0;
	for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
		if (Data_Empire_Objects[i].inUse &&
			Data_Empire_Objects[i].type == EmpireObject_City &&
			Data_Empire_Objects[i].cityType == EmpireCity_VulnerableRoman) {
			Data_CityInfo.distantBattleCityId = i;
			break;
		}
	}
}

void Event_initRequests()
{
	for (int i = 0; i < MAX_EVENTS; i++) {
		Random_generateNext();
		if (Data_Scenario.requests.resourceId[i]) {
			Data_Scenario.requests_month[i] = (Data_Random.random1_7bit & 7) + 2;
			Data_Scenario.requests_monthsToComply[i] = 12 * Data_Scenario.requests.deadlineYears[i];
		}
	}
}

void Event_handleRequests()
{
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (!Data_Scenario.requests.resourceId[i] || Data_Scenario.requests_state[i] > RequestState_DispatchedLate) {
			continue;
		}
		int state = Data_Scenario.requests_state[i];
		if (state == RequestState_Dispatched || state == RequestState_DispatchedLate) {
			--Data_Scenario.requests_monthsToComply[i];
			if (Data_Scenario.requests_monthsToComply[i] <= 0) {
				if (state == RequestState_Dispatched) {
					PlayerMessage_post(1, Message_32_RequestReceived, i, 0);
					CityInfo_Ratings_changeFavor(Data_Scenario.requests_favor[i]);
				} else {
					PlayerMessage_post(1, Message_35_RequestReceivedLate, i, 0);
					CityInfo_Ratings_changeFavor(Data_Scenario.requests_favor[i] / 2);
				}
				Data_Scenario.requests_state[i] = RequestState_Received;
				Data_Scenario.requests_isVisible[i] = 0;
			}
		} else {
			// normal or overdue
			if (Data_Scenario.requests_isVisible[i]) {
				--Data_Scenario.requests_monthsToComply[i];
				if (state == RequestState_Normal) {
					if (Data_Scenario.requests_monthsToComply[i] == 12) {
						// reminder
						PlayerMessage_post(1, Message_31_RequestReminder, i, 0);
					}
					if (Data_Scenario.requests_monthsToComply[i] <= 0) {
						PlayerMessage_post(1, Message_33_RequestRefused, i, 0);
						Data_Scenario.requests_state[i] = RequestState_Overdue;
						Data_Scenario.requests_monthsToComply[i] = 24;
						CityInfo_Ratings_changeFavor(-3);
						Data_CityInfo.ratingFavorIgnoredRequestPenalty = 3;
					}
				} else if (state == RequestState_Overdue) {
					if (Data_Scenario.requests_monthsToComply[i] <= 0) {
						PlayerMessage_post(1, Message_34_RequestRefusedOverdue, i, 0);
						Data_Scenario.requests_state[i] = RequestState_Ignored;
						Data_Scenario.requests_isVisible[i] = 0;
						CityInfo_Ratings_changeFavor(-5);
						Data_CityInfo.ratingFavorIgnoredRequestPenalty = 5;
					}
				}
				if (!Data_Scenario.requests_canComplyDialogShown[i] &&
					Data_CityInfo.resourceStored[Data_Scenario.requests.resourceId[i]] >= Data_Scenario.requests.amount[i]) {
					Data_Scenario.requests_canComplyDialogShown[i] = 1;
					PlayerMessage_post(1, Message_115_RequestCanComply, i, 0);
				}
			} else {
				// request is not visible
				int year = Data_Scenario.startYear;
				if (IsTutorial2()) {
					if (!Data_Tutorial.tutorial2.potteryMade) {
						return;
					}
					year = Data_Tutorial.tutorial2.potteryMadeYear;
				}
				if (Data_CityInfo_Extra.gameTimeYear == year + Data_Scenario.requests.year[i] &&
					Data_CityInfo_Extra.gameTimeMonth == Data_Scenario.requests_month[i]) {
					Data_Scenario.requests_isVisible[i] = 1;
					if (Data_CityInfo.resourceStored[Data_Scenario.requests.resourceId[i]] >= Data_Scenario.requests.amount[i]) {
						Data_Scenario.requests_canComplyDialogShown[i] = 1;
					}
					if (Data_Scenario.requests.resourceId[i] == Resource_Denarii) {
						PlayerMessage_post(1, Message_29_CaesarRequestsMoney, i, 0);
					} else if (Data_Scenario.requests.resourceId[i] == Resource_Troops) {
						PlayerMessage_post(1, Message_30_CaesarRequestsArmy, i, 0);
					} else {
						PlayerMessage_post(1, Message_28_CaesarRequestsGoods, i, 0);
					}
				}
			}
		}
	}
}

void Event_dispatchRequest(int id)
{
	if (Data_Scenario.requests_state[id] == RequestState_Normal) {
		Data_Scenario.requests_state[id] = RequestState_Dispatched;
	} else {
		Data_Scenario.requests_state[id] = RequestState_DispatchedLate;
	}
	Data_Scenario.requests_monthsToComply[id] = (Data_Random.random1_7bit & 3) + 1;
	Data_Scenario.requests_isVisible[id] = 0;
	int amount = Data_Scenario.requests.amount[id];
	if (Data_Scenario.requests.resourceId[id] == Resource_Denarii) {
		Data_CityInfo.treasury -= amount;
		Data_CityInfo.financeSundriesThisYear += amount;
	} else if (Data_Scenario.requests.resourceId[id] == Resource_Troops) {
		CityInfo_Population_removePeopleForTroopRequest(amount);
		Resource_removeFromCityWarehouses(Resource_Weapons, amount);
	} else {
		Resource_removeFromCityWarehouses(Data_Scenario.requests.resourceId[id], amount);
	}
}

void Event_initDemandChanges()
{
	for (int i = 0; i < MAX_EVENTS; i++) {
		Random_generateNext();
		if (Data_Scenario.demandChanges.year[i]) {
			Data_Scenario.demandChanges.month[i] = (Data_Random.random1_7bit & 7) + 2;
		}
	}
}

void Event_handleDemandChanges()
{
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (!Data_Scenario.demandChanges.year[i]) {
			continue;
		}
		if (Data_CityInfo_Extra.gameTimeYear != Data_Scenario.demandChanges.year[i] + Data_Scenario.startYear ||
			Data_CityInfo_Extra.gameTimeMonth != Data_Scenario.demandChanges.month[i]) {
			continue;
		}
		int route = Data_Scenario.demandChanges.routeId[i];
		int resource = Data_Scenario.demandChanges.resourceId[i];
		int cityId = Empire_getCityForTradeRoute(route);
		if (Data_Scenario.demandChanges.isRise[i]) {
			int max = 0;
			switch (Data_Empire_Trade.maxPerYear[route][resource]) {
				case 0: max = 15; break;
				case 15: max = 25; break;
				case 25: max = 40; break;
				default: continue;
			}
			Data_Empire_Trade.maxPerYear[route][resource] = max;
			if (Empire_isTradeRouteOpen(route)) {
				PlayerMessage_post(1, Message_74_IncreasedTrading, cityId, resource);
			}
		} else {
			int max;
			switch (Data_Empire_Trade.maxPerYear[route][resource]) {
				case 40: max = 25; break;
				case 25: max = 15; break;
				case 15: max = 0; break;
				default: continue;
			}
			Data_Empire_Trade.maxPerYear[route][resource] = max;
			if (Empire_isTradeRouteOpen(route)) {
				if (max > 0) {
					PlayerMessage_post(1, Message_75_DecreasedTrading, cityId, resource);
				} else {
					PlayerMessage_post(1, Message_76_TradeStopped, cityId, resource);
				}
			}
		}
	}
}

void Event_initPriceChanges()
{
	for (int i = 0; i < MAX_EVENTS; i++) {
		Random_generateNext();
		if (Data_Scenario.priceChanges.year[i]) {
			Data_Scenario.priceChanges.month[i] = (Data_Random.random1_7bit & 7) + 2;
		}
	}
}

void Event_handlePricesChanges()
{
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (!Data_Scenario.priceChanges.year[i]) {
			continue;
		}
		if (Data_CityInfo_Extra.gameTimeYear != Data_Scenario.priceChanges.year[i] + Data_Scenario.startYear ||
			Data_CityInfo_Extra.gameTimeMonth != Data_Scenario.priceChanges.month[i]) {
			continue;
		}
		int amount = Data_Scenario.priceChanges.amount[i];
		int resource = Data_Scenario.priceChanges.resourceId[i];
		if (Data_Scenario.priceChanges.isRise[i]) {
			Data_TradePrices[resource].buy += amount;
			Data_TradePrices[resource].sell += amount;
			PlayerMessage_post(1, Message_78_PriceIncreased, amount, resource);
		} else if (Data_TradePrices[resource].sell > 0) {
			if (Data_TradePrices[resource].sell <= amount) {
				Data_TradePrices[resource].buy = 2;
				Data_TradePrices[resource].sell = 0;
			} else {
				Data_TradePrices[resource].buy -= amount;
				Data_TradePrices[resource].sell -= amount;
			}
			PlayerMessage_post(1, Message_79_PriceDecreased, amount, resource);
		}
	}
}

void Event_handleEarthquake()
{
	if (Data_Scenario.earthquakeSeverity == Earthquake_None ||
		Data_Scenario.earthquakePoint.x == -1 || Data_Scenario.earthquakePoint.y == -1) {
		return;
	}
	if (Data_Event.earthquake.state == SpecialEvent_NotStarted) {
		if (Data_CityInfo_Extra.gameTimeYear == Data_Event.earthquake.gameYear &&
			Data_CityInfo_Extra.gameTimeMonth == Data_Event.earthquake.month) {
			Data_Event.earthquake.state = SpecialEvent_InProgress;
			Data_Event.earthquake.duration = 0;
			Data_Event.earthquake.delay = 0;
			advanceEarthquakeToTile(Data_Event.earthquake.expand[0].x, Data_Event.earthquake.expand[0].y);
			PlayerMessage_post(1, Message_62_Earthquake, 0,
				GridOffset(Data_Event.earthquake.expand[0].x, Data_Event.earthquake.expand[0].y));
		}
	} else if (Data_Event.earthquake.state == SpecialEvent_InProgress) {
		Data_Event.earthquake.delay++;
		if (Data_Event.earthquake.delay >= Data_Event.earthquake.maxDelay) {
			Data_Event.earthquake.delay = 0;
			Data_Event.earthquake.duration++;
			if (Data_Event.earthquake.duration >= Data_Event.earthquake.maxDuration) {
				Data_Event.earthquake.state = SpecialEvent_Finished;
			}
			int dx, dy, index;
			switch (Data_Random.random1_7bit & 0xf) {
				case 0: index = 0; dx = 0; dy = -1; break;
				case 1: index = 1; dx = 1; dy = 0; break;
				case 2: index = 2; dx = 0; dy = 1; break;
				case 3: index = 3; dx = -1; dy = 0; break;
				case 4: index = 0; dx = 0; dy = -1; break;
				case 5: index = 0; dx = -1; dy = 0; break;
				case 6: index = 0; dx = 1; dy = 0; break;
				case 7: index = 1; dx = 1; dy = 0; break;
				case 8: index = 1; dx = 0; dy = -1; break;
				case 9: index = 1; dx = 0; dy = 1; break;
				case 10: index = 2; dx = 0; dy = 1; break;
				case 11: index = 2; dx = -1; dy = 0; break;
				case 12: index = 2; dx = 1; dy = 0; break;
				case 13: index = 3; dx = -1; dy = 0; break;
				case 14: index = 3; dx = 0; dy = -1; break;
				case 15: index = 3; dx = 0; dy = 1; break;
				default: return;
			}
			int x = Data_Event.earthquake.expand[index].x + dx;
			int y = Data_Event.earthquake.expand[index].y + dy;
			BOUND(x, 0, Data_Settings_Map.width - 1);
			BOUND(y, 0, Data_Settings_Map.height - 1);
			if (canAdvanceEarthquakeToTile(x, y)) {
				Data_Event.earthquake.expand[index].x = x;
				Data_Event.earthquake.expand[index].y = y;
				advanceEarthquakeToTile(x, y);
			}
		}
	}
}

static void advanceEarthquakeToTile(int x, int y)
{
	int gridOffset = GridOffset(x, y);
	int buildingId = Data_Grid_buildingIds[gridOffset];
	if (buildingId) {
		Building_collapseOnFire(buildingId, 0);
		Building_collapseLinked(buildingId, 1);
		Sound_Effects_playChannel(SoundChannel_Explosion);
		Data_Buildings[buildingId].state = BuildingState_DeletedByGame;
	}
	Data_Grid_terrain[gridOffset] = 0;
	TerrainGraphics_setTileEarthquake(x, y);
	TerrainGraphics_updateAllGardens();
	TerrainGraphics_updateAllRoads();
	TerrainGraphics_updateRegionPlazas(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
	
	Routing_determineLandCitizen();
	Routing_determineLandNonCitizen();
	Routing_determineWalls();
	
	Figure_createDustCloud(x, y, 1);
}

static int canAdvanceEarthquakeToTile(int x, int y)
{
	int terrain = Data_Grid_terrain[GridOffset(x, y)];
	if (terrain & (Terrain_Elevation | Terrain_Rock | Terrain_Water)) {
		return 0;
	} else {
		return 1;
	}
}

void Event_handleGladiatorRevolt()
{
	if (!Data_Scenario.gladiatorRevolt.enabled) {
		return;
	}
	if (Data_Event.gladiatorRevolt.state == SpecialEvent_NotStarted) {
		if (Data_CityInfo_Extra.gameTimeYear == Data_Event.gladiatorRevolt.gameYear &&
			Data_CityInfo_Extra.gameTimeMonth == Data_Event.gladiatorRevolt.month) {
			if (Data_CityInfo_Buildings.gladiatorSchool.working > 0) {
				Data_Event.gladiatorRevolt.state = SpecialEvent_InProgress;
				PlayerMessage_post(1, Message_63_GladiatorRevolt, 0, 0);
			} else {
				Data_Event.gladiatorRevolt.state = SpecialEvent_Finished;
			}
		}
	} else if (Data_Event.gladiatorRevolt.state == SpecialEvent_InProgress) {
		if (Data_Event.gladiatorRevolt.endMonth == Data_CityInfo_Extra.gameTimeMonth) {
			Data_Event.gladiatorRevolt.state = SpecialEvent_Finished;
			PlayerMessage_post(1, Message_73_GladiatorRevoltFinished, 0, 0);
		}
	}
}

void Event_handleEmperorChange()
{
	if (!Data_Scenario.emperorChange.enabled) {
		return;
	}
	if (Data_Event.emperorChange.state == 0) {
		if (Data_CityInfo_Extra.gameTimeYear == Data_Event.emperorChange.gameYear &&
			Data_CityInfo_Extra.gameTimeMonth == Data_Event.emperorChange.month) {
			Data_Event.emperorChange.state = 1; // done
			PlayerMessage_post(1, Message_64_EmperorChange, 0, 0);
		}
	}
}

void Event_handleRandomEvents()
{
	int event = randomEventProbability[Data_Random.random1_7bit];
	if (event <= 0) {
		return;
	}
	switch (event) {
		case RandomEvent_RomeRaisesWages:
			if (Data_Scenario.raiseWagesEnabled) {
				if (Data_CityInfo.wagesRome < 45) {
					Data_CityInfo.wagesRome += 1 + (Data_Random.random2_7bit & 3);
					if (Data_CityInfo.wagesRome > 45) {
						Data_CityInfo.wagesRome = 45;
					}
					PlayerMessage_post(1, Message_68_RomeRaisesWages, 0, 0);
				}
			}
			break;
		case RandomEvent_RomeLowersWages:
			if (Data_Scenario.lowerWagesEnabled) {
				if (Data_CityInfo.wagesRome > 5) {
					Data_CityInfo.wagesRome -= 1 + (Data_Random.random2_7bit & 3);
					PlayerMessage_post(1, Message_69_RomeLowersWages, 0, 0);
				}
			}
			break;
		case RandomEvent_LandTradeDisrupted:
			if (Data_Scenario.landTradeProblemEnabled) {
				if (Data_CityInfo.tradeNumOpenLandRoutes > 0) {
					Data_CityInfo.tradeLandProblemDuration = 48;
					if (Data_Scenario.climate == Climate_Desert) {
						PlayerMessage_post(1, Message_65_LandTradeDisruptedSandstorms, 0, 0);
					} else {
						PlayerMessage_post(1, Message_67_LandTradeDisruptedLandslides, 0, 0);
					}
				}
			}
			break;
		case RandomEvent_SeaTradeDisrupted:
			if (Data_Scenario.seaTradeProblemEnabled) {
				if (Data_CityInfo.tradeNumOpenSeaRoutes > 0) {
					Data_CityInfo.tradeSeaProblemDuration = 48;
					PlayerMessage_post(1, Message_66_SeaTradeDisrupted, 0, 0);
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
					PlayerMessage_post(1, Message_70_ContaminatedWater, 0, 0);
				}
			}
			break;
		case RandomEvent_IronMineCollapsed:
			if (Data_Scenario.ironMineCollapseEnabled) {
				int gridOffset = Building_collapseFirstOfType(Building_IronMine);
				if (gridOffset) {
					PlayerMessage_post(1, Message_71_IronMineCollaped, 0, gridOffset);
				}
			}
			break;
		case RandomEvent_ClayPitFlooded:
			if (Data_Scenario.clayPitFloodEnabled) {
				int gridOffset = Building_collapseFirstOfType(Building_ClayPit);
				if (gridOffset) {
					PlayerMessage_post(1, Message_72_ClayPitFlooded, 0, gridOffset);
				}
			}
			break;
	}
}
