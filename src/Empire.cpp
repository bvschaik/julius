#include "Empire.h"
#include "FileSystem.h"

#include "Data/Empire.h"
#include "Data/Scenario.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/CityInfo.h"

#include <string.h>

static void fixGraphicIds();
static int isSeaTradeRoute(int routeId);
static int getTradeAmountCode(int index, int resource);
static void setTradeAmountCode(int index, int resource, int amountCode);

void Empire_load(int isCustomScenario, int empireId)
{
	if (isCustomScenario) {
		FileSystem_readFilePartIntoBuffer("c32.emp",
			Data_Empire_Index, 1280, 0);
	} else {
		FileSystem_readFilePartIntoBuffer("c3.emp",
			Data_Empire_Index, 1280, 0);
	}
	memset(Data_Empire_Objects, 0, 12800);
	int offset = 1280 + 12800 * /*Data_Scenario.*/empireId;
	if (isCustomScenario) {
		FileSystem_readFilePartIntoBuffer("c32.emp",
			Data_Empire_Objects, 12800, offset);
	} else {
		FileSystem_readFilePartIntoBuffer("c3.emp",
			Data_Empire_Objects, 12800, offset);
	}
	fixGraphicIds();
}

void Empire_initTradeRoutes()
{
	memset(Data_Empire_TradeCities, 0, 2706);
	int routeIndex = 0;
	for (int i = 0; i < 200; i++) {
		if (!Data_Empire_Objects[i].inUse
			|| Data_Empire_Objects[i].type != EmpireObject_City) {
			continue;
		}
		struct Data_Empire_Object *city = &Data_Empire_Objects[i];
		struct Data_Empire_TradeCity *route = &Data_Empire_TradeCities[routeIndex++];
		route->inUse = 1;
		route->cityType = city->cityType;
		route->cityNameId = city->cityNameId;
		if (city->tradeRouteId < 0) {
			city->tradeRouteId = 0;
		}
		if (city->tradeRouteId >= 20) {
			city->tradeRouteId = 19;
		}
		route->routeId = city->tradeRouteId;
		route->isOpen = city->tradeRouteOpen;
		route->costToOpen = city->tradeRouteCostToOpen;
		route->isSeaTrade = isSeaTradeRoute(city->tradeRouteId);
		
		for (int resource = 1; resource <= 15; resource++) {
			route->sellsResourceFlag[resource] = 0;
			route->buysResourceFlag[resource] = 0;
			if (route->cityType == EmpireCity_DistantRoman
				|| route->cityType == EmpireCity_DistantForeign
				|| route->cityType == EmpireCity_VulnerableRoman
				|| route->cityType == EmpireCity_FutureRoman) {
				continue;
			}
			if (Empire_citySellsResource(i, resource)) {
				route->sellsResourceFlag[resource] = 1;
			}
			if (Empire_cityBuysResource(i, resource)) {
				route->buysResourceFlag[resource] = 1;
			}
			int amountCode = getTradeAmountCode(i, resource);
			switch (amountCode) {
				case 1:
					Data_Empire_Trade.maxPerYear[route->routeId][resource] = 15;
					break;
				case 2:
					Data_Empire_Trade.maxPerYear[route->routeId][resource] = 25;
					break;
				case 3:
					Data_Empire_Trade.maxPerYear[route->routeId][resource] = 40;
					break;
				default:
					Data_Empire_Trade.maxPerYear[route->routeId][resource] = 0;
					break;
			}
			Data_Empire_Trade.tradedThisYear[route->routeId][resource] = 0;
		}
		route->__unused2 = 10;
		route->traderEntryDelay = 4;
		route->empireObjectId = i;
	}
}

int Empire_cityBuysResource(int index, int resource)
{
	for (int i = 0; i < 10; i++) {
		if (Data_Empire_Objects[index].citySells[i] == resource) {
			return 1;
		}
	}
	return 0;
}

int Empire_citySellsResource(int index, int resource)
{
	for (int i = 0; i < 10; i++) {
		if (Data_Empire_Objects[index].cityBuys[i] == resource) {
			return 1;
		}
	}
	return 0;
}

void Empire_determineDistantBattleCity()
{
	Data_CityInfo.distantBattleTradeCity = 0;
	for (int i = 0; i < 41; i++) {
		if (Data_Empire_TradeCities[i].inUse) {
			if (Data_Empire_TradeCities[i].cityType == EmpireCity_VulnerableRoman) {
				Data_CityInfo.distantBattleTradeCity = i;
			}
		}
	}
}

void Empire_resetYearlyTradeAmounts()
{
	for (int i = 0; i < 41; i++) {
		if (Data_Empire_TradeCities[i].inUse && Data_Empire_TradeCities[i].isOpen) {
			for (int resource = 1; resource <= 15; resource++) {
				Data_Empire_Trade.tradedThisYear[Data_Empire_TradeCities[i].routeId][resource] = 0;
			}
		}
	}
}

static void fixGraphicIds()
{
	int graphicId = 0;
	for (int i = 0; i < 200; i++) {
		if (Data_Empire_Objects[i].inUse
			&& Data_Empire_Objects[i].type == EmpireObject_City
			&& Data_Empire_Objects[i].cityType == EmpireCity_Ours) {
			graphicId = Data_Empire_Objects[i].graphicId;
			break;
		}
	}
	if (graphicId > 0 && graphicId != GraphicId(ID_Graphic_EmpireCity)) {
		// empire map uses old version of graphics: increase every graphic id
		int offset = GraphicId(ID_Graphic_EmpireCity) - graphicId;
		for (int i = 0; i < 200; i++) {
			if (!Data_Empire_Objects[i].inUse) {
				continue;
			}
			if (Data_Empire_Objects[i].graphicId) {
				Data_Empire_Objects[i].graphicId += offset;
				if (Data_Empire_Objects[i].graphicIdExpanded) {
					Data_Empire_Objects[i].graphicIdExpanded += offset;
				}
			}
		}
	}
}

static int isSeaTradeRoute(int routeId)
{
	for (int i = 0; i < 200; i++) {
		if (Data_Empire_Objects[i].inUse
			&& Data_Empire_Objects[i].tradeRouteId == routeId) {
			if (Data_Empire_Objects[i].type == EmpireObject_SeaTradeRoute) {
				return 1;
			}
			if (Data_Empire_Objects[i].type == EmpireObject_LandTradeRoute) {
				return 0;
			}
		}
	}
	return 0;
}

static int getTradeAmountCode(int index, int resource)
{
	if (Data_Empire_Objects[index].type != EmpireObject_City) {
		return 0;
	}
	if (Data_Empire_Objects[index].cityType <= 1 || Data_Empire_Objects[index].cityType >= 6) {
		return 0;
	}
	int resourceFlag = 1 << resource;
	if (Data_Empire_Objects[index].trade40 & resourceFlag) {
		return 3;
	}
	if (Data_Empire_Objects[index].trade25 & resourceFlag) {
		return 2;
	}
	if (Data_Empire_Objects[index].trade15 & resourceFlag) {
		return 1;
	}
	return 0;
}

static void setTradeAmountCode(int index, int resource, int amountCode)
{
	if (Data_Empire_Objects[index].type != EmpireObject_City) {
		return;
	}
	if (Data_Empire_Objects[index].cityType <= 1 || Data_Empire_Objects[index].cityType >= 6) {
		return;
	}
	int resourceFlag = 1 << resource;
	// clear flags
	Data_Empire_Objects[index].trade40 &= ~resourceFlag;
	Data_Empire_Objects[index].trade25 &= ~resourceFlag;
	Data_Empire_Objects[index].trade15 &= ~resourceFlag;
	// set correct flag
	if (amountCode == 1) {
		Data_Empire_Objects[index].trade15 |= resourceFlag;
	} else if (amountCode == 2) {
		Data_Empire_Objects[index].trade25 |= resourceFlag;
	} else if (amountCode == 3) {
		Data_Empire_Objects[index].trade40 |= resourceFlag;
	}
}

