#include "Event.h"

#include "Calc.h"
#include "CityInfo.h"
#include "Empire.h"
#include "Formation.h"
#include "PlayerMessage.h"
#include "Random.h"
#include "Resource.h"
#include "SidebarMenu.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Empire.h"
#include "Data/Invasion.h"
#include "Data/Random.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/Trade.h"
#include "Data/Tutorial.h"

static void updateTimeTraveledDistantBattle();
static void fightDistantBattle();
static void updateDistantBattleAftermath();
static int playerWonDistantBattle();
static void setDistantBattleCityVulnerable();
static void setDistantBattleCityForeign();

void Event_handleDistantBattle()
{
	for (int i = 0; i < 20; i++) {
		if (Data_Scenario.invasions.type[i] == InvasionType_DistantBattle &&
			Data_CityInfo_Extra.gameTimeYear == Data_Scenario.invasions.year[i] + Data_Scenario.startYear &&
			Data_CityInfo_Extra.gameTimeMonth == Data_Scenario.invasions_month[i] &&
			Data_Scenario.distantBattleTravelMonthsEnemy > 4 &&
			Data_Scenario.distantBattleTravelMonthsRoman > 4 &&
			Data_CityInfo.distantBattleMonthsToBattle <= 0 &&
			Data_CityInfo.distantBattleRomanMonthsToReturn <= 0 &&
			Data_CityInfo.distantBattleRomanMonthsToTravel <= 0 &&
			Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0) {
			
			PlayerMessage_post(1, 30, 0, 0);
			Data_CityInfo.distantBattleEnemyMonthsTraveled = 1;
			Data_CityInfo.distantBattleRomanMonthsTraveled = 1;
			Data_CityInfo.distantBattleMonthsToBattle = 24;
			Data_CityInfo.distantBattleEnemyStrength = (unsigned char) Data_Scenario.invasions.amount[i];
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
		PlayerMessage_post(1, 84, 0, 0); // no troops sent
		CityInfo_Ratings_changeFavor(-50);
		setDistantBattleCityForeign();
		Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
	} else if (Data_CityInfo.distantBattleRomanMonthsToTravel > 2) {
		PlayerMessage_post(1, 85, 0, 0); // too late
		CityInfo_Ratings_changeFavor(-25);
		setDistantBattleCityForeign();
		Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
		Data_CityInfo.distantBattleRomanMonthsToReturn = Data_CityInfo.distantBattleRomanMonthsTraveled;
	} else if (!playerWonDistantBattle()) {
		PlayerMessage_post(1, 86, 0, 0);
		CityInfo_Ratings_changeFavor(-10);
		setDistantBattleCityForeign();
		Data_CityInfo.distantBattleCityMonthsUntilRoman = 24;
		Data_CityInfo.distantBattleRomanMonthsTraveled = 0;
		// no return: all soldiers killed
	} else {
		PlayerMessage_post(1, 87, 0, 0);
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
				PlayerMessage_post(1, 88, 0, Data_CityInfo.exitPointGridOffset);
			} else {
				// victorious
				PlayerMessage_post(1, 89, 0, Data_CityInfo.exitPointGridOffset);
			}
			Data_CityInfo.distantBattleRomanMonthsTraveled = 0;
			Formation_legionsReturnFromDistantBattle();
		}
	} else if (Data_CityInfo.distantBattleCityMonthsUntilRoman > 0) {
		Data_CityInfo.distantBattleCityMonthsUntilRoman--;
		if (Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0) {
			PlayerMessage_post(1, 90, 0, 0);
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
	for (int i = 0; i < 200; i++) {
		if (Data_Empire_Objects[i].inUse && Data_Empire_Objects[i].type == EmpireObject_RomanArmy) {
			Data_Scenario.distantBattleTravelMonthsRoman++;
			Data_Empire_Objects[i].distantBattleTravelMonths = Data_Scenario.distantBattleTravelMonthsRoman;
		}
	}
}

void Event_calculateDistantBattleEnemyTravelTime()
{
	Data_Scenario.distantBattleTravelMonthsEnemy = 0;
	for (int i = 0; i < 200; i++) {
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
	for (int i = 0; i < 200; i++) {
		if (Data_Empire_Objects[i].inUse &&
			Data_Empire_Objects[i].type == EmpireObject_City &&
			Data_Empire_Objects[i].cityType == EmpireCity_VulnerableRoman) {
			Data_CityInfo.distantBattleCityId = i;
			break;
		}
	}
}

int Event_existsUpcomingInvasion()
{
	for (int i = 0; i < 101; i++) {
		if (Data_InvasionWarnings[i].inUse && Data_InvasionWarnings[i].handled) {
			return 1;
		}
	}
	return 0;
}

void Event_initRequests()
{
	for (int i = 0; i < 20; i++) {
		Random_generateNext();
		if (Data_Scenario.requests.resourceId[i]) {
			Data_Scenario.requests_month[i] = (Data_Random.random1_7bit & 7) + 2;
			Data_Scenario.requests_monthsToComply[i] = 12 * Data_Scenario.requests.deadlineYears[i];
		}
	}
}

void Event_handleRequests()
{
	for (int i = 0; i < 20; i++) {
		if (!Data_Scenario.requests.resourceId[i] || Data_Scenario.requests_state[i] > RequestState_DispatchedLate) {
			continue;
		}
		int state = Data_Scenario.requests_state[i];
		if (state == RequestState_Dispatched || state == RequestState_DispatchedLate) {
			if (--Data_Scenario.requests_monthsToComply[i] <= 0) {
				if (state == RequestState_Dispatched) {
					PlayerMessage_post(1, 32, i, 0);
					CityInfo_Ratings_changeFavor(Data_Scenario.requests_favor[i]);
				} else {
					PlayerMessage_post(1, 35, i, 0);
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
						PlayerMessage_post(1, 31, i, 0);
					}
					if (Data_Scenario.requests_monthsToComply[i] <= 0) {
						PlayerMessage_post(1, 33, i, 0);
						Data_Scenario.requests_state[i] = RequestState_Overdue;
						Data_Scenario.requests_monthsToComply[i] = 24;
						CityInfo_Ratings_changeFavor(-3);
						Data_CityInfo.favorIgnoredRequestPenalty = 3;
					}
				} else if (state == RequestState_Overdue) {
					if (Data_Scenario.requests_monthsToComply[i] <= 0) {
						PlayerMessage_post(1, 34, i, 0);
						Data_Scenario.requests_state[i] = RequestState_Ignored;
						Data_Scenario.requests_isVisible[i] = 0;
						CityInfo_Ratings_changeFavor(-5);
						Data_CityInfo.favorIgnoredRequestPenalty = 5;
					}
				}
				if (!Data_Scenario.requests_canComplyDialogShown[i] &&
					Data_CityInfo.resourceStored[Data_Scenario.requests.resourceId[i]] >= Data_Scenario.requests.amount[i]) {
					Data_Scenario.requests_canComplyDialogShown[i] = 1;
					PlayerMessage_post(1, 115, i, 0);
				}
			} else {
				// request is not visible
				int year = Data_Scenario.startYear;
				if (IsTutorial2()) {
					if (!Data_Tutorial_tutorial2.potteryMade) {
						return;
					}
					year = Data_Tutorial_tutorial2.potteryMadeYear;
				}
				if (Data_CityInfo_Extra.gameTimeYear == year + Data_Scenario.requests.year[i] &&
					Data_CityInfo_Extra.gameTimeMonth == Data_Scenario.requests_month[i]) {
					Data_Scenario.requests_isVisible[i] = 1;
					if (Data_CityInfo.resourceStored[Data_Scenario.requests.resourceId[i]] >= Data_Scenario.requests.amount[i]) {
						Data_Scenario.requests_canComplyDialogShown[i] = 1;
					}
					if (Data_Scenario.requests.resourceId[i] == Resource_Denarii) {
						PlayerMessage_post(1, 29, i, 0);
					} else if (Data_Scenario.requests.resourceId[i] == Resource_Troops) {
						PlayerMessage_post(1, 30, i, 0);
					} else {
						PlayerMessage_post(1, 31, i, 0);
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
	for (int i = 0; i < 20; i++) {
		Random_generateNext();
		if (Data_Scenario.demandChanges.resourceId[i]) {
			Data_Scenario.demandChanges.month[i] = (Data_Random.random1_7bit & 7) + 2;
		}
	}
}

void Event_handleDemandChanges()
{
	for (int i = 0; i < 20; i++) {
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
				PlayerMessage_post(1, 74, cityId, resource);
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
					PlayerMessage_post(1, 75, cityId, resource);
				} else {
					PlayerMessage_post(1, 76, cityId, resource);
				}
			}
		}
	}
}

void Event_initPriceChanges()
{
	for (int i = 0; i < 20; i++) {
		Random_generateNext();
		if (Data_Scenario.priceChanges.year[i]) {
			Data_Scenario.priceChanges.month[i] = (Data_Random.random1_7bit & 7) + 2;
		}
	}
}

void Event_handlePricesChanges()
{
	for (int i = 0; i < 20; i++) {
		if (!Data_Scenario.priceChanges.year[i]) {
			continue;
		}
		if (Data_CityInfo_Extra.gameTimeYear != 
			Data_Scenario.priceChanges.year[i] + Data_Scenario.startYear ||
			Data_CityInfo_Extra.gameTimeMonth != Data_Scenario.priceChanges.month[i]) {
			continue;
		}
		int amount = Data_Scenario.priceChanges.amount[i];
		int resource = Data_Scenario.priceChanges.resourceId[i];
		if (Data_Scenario.priceChanges.isRise[i]) {
			Data_TradePrices[resource].buy += amount;
			Data_TradePrices[resource].sell += amount;
			PlayerMessage_post(1, 78, amount, resource);
		} else if (Data_TradePrices[resource].sell > 0) {
			if (Data_TradePrices[resource].buy <= amount) {
				Data_TradePrices[resource].buy = 2;
				Data_TradePrices[resource].sell = 0;
			} else {
				Data_TradePrices[resource].buy -= amount;
				Data_TradePrices[resource].sell -= amount;
			}
			PlayerMessage_post(1, 79, amount, resource);
		}
	}
}
