#include "Event.h"

#include "CityInfo.h"
#include "PlayerMessage.h"
#include "Random.h"
#include "Resource.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Empire.h"
#include "Data/Invasion.h"
#include "Data/Random.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/Trade.h"
#include "Data/Tutorial.h"

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
					CityInfo_Ratings_increaseFavor(Data_Scenario.requests_favor[i]);
				} else {
					PlayerMessage_post(1, 35, i, 0);
					CityInfo_Ratings_increaseFavor(Data_Scenario.requests_favor[i] / 2);
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
						CityInfo_Ratings_increaseFavor(-3);
						Data_CityInfo.favorIgnoredRequestPenalty = 3;
					}
				} else if (state == RequestState_Overdue) {
					if (Data_Scenario.requests_monthsToComply[i] <= 0) {
						PlayerMessage_post(1, 34, i, 0);
						Data_Scenario.requests_state[i] = RequestState_Ignored;
						Data_Scenario.requests_isVisible[i] = 0;
						CityInfo_Ratings_increaseFavor(-5);
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
