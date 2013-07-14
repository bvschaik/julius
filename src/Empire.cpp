#include "Empire.h"
#include "FileSystem.h"
#include "PlayerMessage.h"

#include "Data/Empire.h"
#include "Data/Scenario.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/CityInfo.h"
#include "Data/Screen.h"

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
	int offset = 1280 + 12800 * Data_Scenario.empireId;
	if (isCustomScenario) {
		FileSystem_readFilePartIntoBuffer("c32.emp",
			Data_Empire_Objects, 12800, offset);
	} else {
		FileSystem_readFilePartIntoBuffer("c3.emp",
			Data_Empire_Objects, 12800, offset);
	}
	fixGraphicIds();
}

void Empire_initCities()
{
	memset(Data_Empire_Cities, 0, 2706);
	int routeIndex = 0;
	for (int i = 0; i < 200; i++) {
		if (!Data_Empire_Objects[i].inUse
			|| Data_Empire_Objects[i].type != EmpireObject_City) {
			continue;
		}
		struct Data_Empire_Object *city = &Data_Empire_Objects[i];
		struct Data_Empire_City *route = &Data_Empire_Cities[routeIndex++];
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
			int routeId = route->routeId;
			switch (amountCode) {
				case 1:
					Data_Empire_Trade.maxPerYear[routeId][resource] = 15;
					break;
				case 2:
					Data_Empire_Trade.maxPerYear[routeId][resource] = 25;
					break;
				case 3:
					Data_Empire_Trade.maxPerYear[routeId][resource] = 40;
					break;
				default:
					Data_Empire_Trade.maxPerYear[routeId][resource] = 0;
					break;
			}
			Data_Empire_Trade.tradedThisYear[routeId][resource] = 0;
		}
		route->__unused2 = 10;
		route->traderEntryDelay = 4;
		route->empireObjectId = i;
	}
}

void Empire_scrollMap(int direction)
{
	if (direction == Direction_None) {
		return;
	}
	switch (direction) {
		case Direction_Top:
			Data_Empire.scrollY -= 20;
			break;
		case Direction_TopRight:
			Data_Empire.scrollX += 20;
			Data_Empire.scrollY -= 20;
			break;
		case Direction_Right:
			Data_Empire.scrollX += 20;
			break;
		case Direction_BottomRight:
			Data_Empire.scrollX += 20;
			Data_Empire.scrollY += 20;
			break;
		case Direction_Bottom:
			Data_Empire.scrollY += 20;
			break;
		case Direction_BottomLeft:
			Data_Empire.scrollX -= 20;
			Data_Empire.scrollY += 20;
			break;
		case Direction_Left:
			Data_Empire.scrollX -= 20;
			break;
		case Direction_TopLeft:
			Data_Empire.scrollX -= 20;
			Data_Empire.scrollY -= 20;
			break;
	};
	Empire_checkScrollBoundaries();
}

void Empire_checkScrollBoundaries()
{
	if (Data_Empire.scrollX < 0) {
		Data_Empire.scrollX = 0;
	}
	int maxX = Data_Empire_Sizes.width - (Data_Screen.width - 2 * Data_Empire_Sizes.borderSides);
	if (Data_Empire.scrollX >= maxX) {
		Data_Empire.scrollX = maxX;
	}
	if (Data_Empire.scrollY < 0) {
		Data_Empire.scrollY = 0;
	}
	int maxY = Data_Empire_Sizes.height - (Data_Screen.height - Data_Empire_Sizes.borderTop - Data_Empire_Sizes.borderBottom);
	if (Data_Empire.scrollY >= maxY) {
		Data_Empire.scrollY = maxY - 1;
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
		if (Data_Empire_Cities[i].inUse) {
			if (Data_Empire_Cities[i].cityType == EmpireCity_VulnerableRoman) {
				Data_CityInfo.distantBattleTradeCity = i;
			}
		}
	}
}

void Empire_resetYearlyTradeAmounts()
{
	for (int i = 0; i < 41; i++) {
		if (Data_Empire_Cities[i].inUse && Data_Empire_Cities[i].isOpen) {
			int routeId = Data_Empire_Cities[i].routeId;
			for (int resource = 1; resource <= 15; resource++) {
				Data_Empire_Trade.tradedThisYear[routeId][resource] = 0;
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

int Empire_getCityForObject(int empireObjectId)
{
	for (int i = 0; i < 41; i++) {
		if (Data_Empire_Cities[i].inUse && Data_Empire_Cities[i].empireObjectId == empireObjectId) {
			return i;
		}
	}
	return 0;
}

int Empire_getCityForTradeRoute(int routeId)
{
	for (int i = 0; i < 41; i++) {
		if (Data_Empire_Cities[i].inUse && Data_Empire_Cities[i].routeId == routeId) {
			return i;
		}
	}
	return 0;
}

int Empire_isTradeWithCityOpen(int routeId)
{
	for (int i = 0; i < 41; i++) {
		if (Data_Empire_Cities[i].inUse && Data_Empire_Cities[i].routeId == routeId) {
			return Data_Empire_Cities[i].isOpen ? 1 : 0;
		}
	}
	return 0;
}

void Empire_handleExpandEvent()
{
	if (Data_Scenario.empireHasExpanded || Data_Scenario.empireExpansionYear <= 0) {
		return;
	}
	if (Data_CityInfo_Extra.gameTimeYear < Data_Scenario.empireExpansionYear + Data_Scenario.startYear) {
		return;
	}

	for (int i = 0; i < 41; i++) {
		if (!Data_Empire_Cities[i].inUse) {
			continue;
		}
		if (Data_Empire_Cities[i].cityType == EmpireCity_FutureTrade) {
			Data_Empire_Cities[i].cityType = EmpireCity_Trade;
		} else if (Data_Empire_Cities[i].cityType == EmpireCity_FutureRoman) {
			Data_Empire_Cities[i].cityType = EmpireCity_DistantRoman;
		} else {
			continue;
		}
		int objectId = Data_Empire_Cities[i].empireObjectId;
		Data_Empire_Objects[objectId].cityType = Data_Empire_Cities[i].cityType;
		if (Data_Empire_Cities[i].cityType == EmpireCity_Trade) {
			Data_Empire_Objects[i].graphicIdExpanded = GraphicId(ID_Graphic_EmpireCityTrade);
		} else if (Data_Empire_Cities[i].cityType == EmpireCity_DistantRoman) {
			Data_Empire_Objects[i].graphicIdExpanded = GraphicId(ID_Graphic_EmpireCityDistantRoman);
		}
	}

	Data_Scenario.empireHasExpanded = 1;
	PlayerMessage_post(1, 77, 0, 0);
}
