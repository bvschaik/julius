#include "Event.h"
#include "PlayerMessage.h"
#include "Random.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Empire.h"
#include "Data/Invasion.h"
#include "Data/Random.h"
#include "Data/Scenario.h"
#include "Data/Trade.h"

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
