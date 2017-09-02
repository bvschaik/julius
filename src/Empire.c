#include "Empire.h"
#include "PlayerMessage.h"

#include "Data/Empire.h"
#include "Data/Scenario.h"
#include "Data/Constants.h"
#include "Data/CityInfo.h"
#include "Data/Screen.h"

#include "building/count.h"
#include "core/io.h"
#include "empire/trade_route.h"
#include "game/time.h"
#include "graphics/image.h"

#include <string.h>

static void fixGraphicIds();
static int isSeaTradeRoute(int routeId);
static int getTradeAmountCode(int index, int resource);
static void setTradeAmountCode(int index, int resource, int amountCode);

void Empire_load(int isCustomScenario, int empireId)
{
	const char *filename = isCustomScenario ? "c32.emp" : "c3.emp";
	io_read_file_part_into_buffer(filename, Data_Empire_Index, 1280, 0);
	memset(Data_Empire_Objects, 0, 12800);
	int offset = 1280 + 12800 * Data_Scenario.empireId;
	io_read_file_part_into_buffer(filename, Data_Empire_Objects, 12800, offset);
	fixGraphicIds();
}

void Empire_initCities()
{
	memset(Data_Empire_Cities, 0, 2706);
	int routeIndex = 1;
	for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
		if (!Data_Empire_Objects[i].inUse
			|| Data_Empire_Objects[i].type != EmpireObject_City) {
			continue;
		}
		struct Data_Empire_Object *obj = &Data_Empire_Objects[i];
		struct Data_Empire_City *city = &Data_Empire_Cities[routeIndex++];
		city->inUse = 1;
		city->cityType = obj->cityType;
		city->cityNameId = obj->cityNameId;
		if (obj->tradeRouteId < 0) {
			obj->tradeRouteId = 0;
		}
		if (obj->tradeRouteId >= 20) {
			obj->tradeRouteId = 19;
		}
		city->routeId = obj->tradeRouteId;
		city->isOpen = obj->tradeRouteOpen;
		city->costToOpen = obj->tradeRouteCostToOpen;
		city->isSeaTrade = isSeaTradeRoute(obj->tradeRouteId);
		
		for (int resource = Resource_Min; resource < Resource_Max; resource++) {
			city->sellsResourceFlag[resource] = 0;
			city->buysResourceFlag[resource] = 0;
			if (city->cityType == EmpireCity_DistantRoman
				|| city->cityType == EmpireCity_DistantForeign
				|| city->cityType == EmpireCity_VulnerableRoman
				|| city->cityType == EmpireCity_FutureRoman) {
				continue;
			}
			if (Empire_citySellsResource(i, resource)) {
				city->sellsResourceFlag[resource] = 1;
			}
			if (Empire_cityBuysResource(i, resource)) {
				city->buysResourceFlag[resource] = 1;
			}
			int amountCode = getTradeAmountCode(i, resource);
			int routeId = city->routeId;{}
			int amount;
			switch (amountCode) {
				case 1: amount = 15; break;
				case 2: amount = 25; break;
				case 3: amount = 40; break;
				default: amount = 0; break;
			}
			trade_route_init(routeId, resource, amount);
		}
		city->__unused2 = 10;
		city->traderEntryDelay = 4;
		city->__unused3 = 0;
		city->traderFigureIds[0] = 0;
		city->traderFigureIds[1] = 0;
		city->traderFigureIds[2] = 0;
		city->empireObjectId = i;
	}
}

void Empire_initTradeAmountCodes()
{
	for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
		if (!Data_Empire_Objects[i].inUse
			|| Data_Empire_Objects[i].type != EmpireObject_City) {
			continue;
		}
		int totalAmount = 0;
		for (int res = Resource_Min; res < Resource_Max; res++) {
			totalAmount += getTradeAmountCode(i, res);
		}
		if (totalAmount) {
			for (int res = Resource_Min; res < Resource_Max; res++) {
				if (!Empire_citySellsResource(i, res) && !Empire_cityBuysResource(i, res)) {
					setTradeAmountCode(i, res, 0);
				}
			}
		} else {
			// reset everything to 25
			for (int res = Resource_Min; res < Resource_Max; res++) {
				setTradeAmountCode(i, res, 2);
			}
		}
	}
}

void Empire_scrollMap(int direction)
{
	if (direction == Dir_8_None) {
		return;
	}
	switch (direction) {
		case Dir_0_Top:
			Data_Empire.scrollY -= 20;
			break;
		case Dir_1_TopRight:
			Data_Empire.scrollX += 20;
			Data_Empire.scrollY -= 20;
			break;
		case Dir_2_Right:
			Data_Empire.scrollX += 20;
			break;
		case Dir_3_BottomRight:
			Data_Empire.scrollX += 20;
			Data_Empire.scrollY += 20;
			break;
		case Dir_4_Bottom:
			Data_Empire.scrollY += 20;
			break;
		case Dir_5_BottomLeft:
			Data_Empire.scrollX -= 20;
			Data_Empire.scrollY += 20;
			break;
		case Dir_6_Left:
			Data_Empire.scrollX -= 20;
			break;
		case Dir_7_TopLeft:
			Data_Empire.scrollX -= 20;
			Data_Empire.scrollY -= 20;
			break;
	};
	Empire_checkScrollBoundaries();
}

void Empire_checkScrollBoundaries()
{
	int maxX = Data_Empire_Sizes.width - (Data_Screen.width - 2 * Data_Empire_Sizes.borderSides);
	if (Data_Empire.scrollX >= maxX) {
		Data_Empire.scrollX = maxX - 1;
	}
	if (Data_Empire.scrollX < 0) {
		Data_Empire.scrollX = 0;
	}
	int maxY = Data_Empire_Sizes.height - (Data_Screen.height - Data_Empire_Sizes.borderTop - Data_Empire_Sizes.borderBottom);
	if (Data_Empire.scrollY >= maxY) {
		Data_Empire.scrollY = maxY - 1;
	}
	if (Data_Empire.scrollY < 0) {
		Data_Empire.scrollY = 0;
	}
}

int Empire_cityBuysResource(int objectId, int resource)
{
	for (int i = 0; i < 8; i++) {
		if (Data_Empire_Objects[objectId].cityBuys[i] == resource) {
			return 1;
		}
	}
	return 0;
}

int Empire_citySellsResource(int objectId, int resource)
{
	for (int i = 0; i < 10; i++) {
		if (Data_Empire_Objects[objectId].citySells[i] == resource) {
			return 1;
		}
	}
	return 0;
}

int Empire_canExportResourceToCity(int cityId, int resource)
{
	int routeId = Data_Empire_Cities[cityId].routeId;
	if (cityId && trade_route_limit_reached(routeId, resource)) {
		// quota reached
		return 0;
	}
	if (Data_CityInfo.resourceStored[resource] <= Data_CityInfo.resourceTradeExportOver[resource]) {
		// stocks too low
		return 0;
	}
	if (cityId == 0 || Data_Empire_Cities[cityId].buysResourceFlag[resource]) {
		return Data_CityInfo.resourceTradeStatus[resource] == TradeStatus_Export;
	} else {
		return 0;
	}
}

int Empire_canImportResourceFromCity(int cityId, int resource)
{
	if (!Data_Empire_Cities[cityId].sellsResourceFlag[resource]) {
		return 0;
	}
	if (Data_CityInfo.resourceTradeStatus[resource] != TradeStatus_Import) {
		return 0;
	}
	int routeId = Data_Empire_Cities[cityId].routeId;
	if (trade_route_limit_reached(routeId, resource)) {
		return 0;
	}

	int inStock = Data_CityInfo.resourceStored[resource];
	int maxInStock = 0;
	int finishedGood = Resource_None;
	switch (resource) {
		// food and finished materials
		case Resource_Wheat:
		case Resource_Vegetables:
		case Resource_Fruit:
		case Resource_Meat:
		case Resource_Pottery:
		case Resource_Furniture:
		case Resource_Oil:
		case Resource_Wine:
			if (Data_CityInfo.population < 2000) {
				maxInStock = 10;
			} else if (Data_CityInfo.population < 4000) {
				maxInStock = 20;
			} else if (Data_CityInfo.population < 6000) {
				maxInStock = 30;
			} else {
				maxInStock = 40;
			}
			break;

		case Resource_Marble:
		case Resource_Weapons:
			maxInStock = 10;
			break;

		case Resource_Clay:
			finishedGood = Resource_Pottery;
			break;
		case Resource_Timber:
			finishedGood = Resource_Furniture;
			break;
		case Resource_Olives:
			finishedGood = Resource_Oil;
			break;
		case Resource_Vines:
			finishedGood = Resource_Wine;
			break;
		case Resource_Iron:
			finishedGood = Resource_Weapons;
			break;
	}
	if (finishedGood) {
		maxInStock = 2 + 2 * building_count_industry_active(finishedGood);
	}
	return inStock < maxInStock ? 1 : 0;
}

int Empire_canImportResource(int resource)
{
	for (int i = 0; i < MAX_EMPIRE_CITIES; i++) {
		if (Data_Empire_Cities[i].inUse &&
			Data_Empire_Cities[i].cityType == EmpireCity_Trade &&
			Data_Empire_Cities[i].isOpen &&
			Data_Empire_Cities[i].sellsResourceFlag[resource] == 1) {
			return 1;
		}
	}
	return 0;
}

int Empire_canImportResourcePotentially(int resource)
{
	for (int i = 0; i < MAX_EMPIRE_CITIES; i++) {
		if (Data_Empire_Cities[i].inUse &&
			Data_Empire_Cities[i].cityType == EmpireCity_Trade &&
			Data_Empire_Cities[i].sellsResourceFlag[resource] == 1) {
			return 1;
		}
	}
	return 0;
}

int Empire_canExportResource(int resource)
{
	for (int i = 0; i < MAX_EMPIRE_CITIES; i++) {
		if (Data_Empire_Cities[i].inUse &&
			Data_Empire_Cities[i].cityType == EmpireCity_Trade &&
			Data_Empire_Cities[i].isOpen &&
			Data_Empire_Cities[i].buysResourceFlag[resource] == 1) {
			return 1;
		}
	}
	return 0;
}

int Empire_ourCityCanProduceResource(int resource)
{
	// finished goods: check imports of raw materials
	switch (resource) {
		case Resource_Pottery:
			resource = Resource_Clay;
			if (Empire_canImportResource(resource)) {
				return 1;
			}
			break;
		case Resource_Furniture:
			resource = Resource_Timber;
			if (Empire_canImportResource(resource)) {
				return 1;
			}
			break;
		case Resource_Oil:
			resource = Resource_Olives;
			if (Empire_canImportResource(resource)) {
				return 1;
			}
			break;
		case Resource_Wine:
			resource = Resource_Vines;
			if (Empire_canImportResource(resource)) {
				return 1;
			}
			break;
		case Resource_Weapons:
			resource = Resource_Iron;
			if (Empire_canImportResource(resource)) {
				return 1;
			}
			break;
	}
	// check if we can produce the raw materials
	for (int i = 0; i < MAX_EMPIRE_CITIES; i++) {
		if (Data_Empire_Cities[i].inUse &&
			Data_Empire_Cities[i].cityType == EmpireCity_Ours &&
			Data_Empire_Cities[i].sellsResourceFlag[resource] == 1) {
			return 1;
		}
	}
	return 0;
}

int Empire_ourCityCanProduceResourcePotentially(int resource)
{
	// finished goods: check imports of raw materials
	switch (resource) {
		case Resource_Pottery:
			resource = Resource_Clay;
			if (Empire_canImportResourcePotentially(resource)) {
				return 1;
			}
			break;
		case Resource_Furniture:
			resource = Resource_Timber;
			if (Empire_canImportResourcePotentially(resource)) {
				return 1;
			}
			break;
		case Resource_Oil:
			resource = Resource_Olives;
			if (Empire_canImportResourcePotentially(resource)) {
				return 1;
			}
			break;
		case Resource_Wine:
			resource = Resource_Vines;
			if (Empire_canImportResourcePotentially(resource)) {
				return 1;
			}
			break;
		case Resource_Weapons:
			resource = Resource_Iron;
			if (Empire_canImportResourcePotentially(resource)) {
				return 1;
			}
			break;
	}
	// check if we can produce the raw materials
	for (int i = 0; i < MAX_EMPIRE_CITIES; i++) {
		if (Data_Empire_Cities[i].inUse &&
			Data_Empire_Cities[i].cityType == EmpireCity_Ours &&
			Data_Empire_Cities[i].sellsResourceFlag[resource] == 1) {
			return 1;
		}
	}
	return 0;
}

void Empire_determineDistantBattleCity()
{
	Data_CityInfo.distantBattleCityId = 0;
	for (int i = 0; i < MAX_EMPIRE_CITIES; i++) {
		if (Data_Empire_Cities[i].inUse) {
			if (Data_Empire_Cities[i].cityType == EmpireCity_VulnerableRoman) {
				Data_CityInfo.distantBattleCityId = i;
			}
		}
	}
}

void Empire_resetYearlyTradeAmounts()
{
	for (int i = 0; i < MAX_EMPIRE_CITIES; i++) {
		if (Data_Empire_Cities[i].inUse && Data_Empire_Cities[i].isOpen) {
			trade_route_reset_traded(Data_Empire_Cities[i].routeId);
		}
	}
}

static void fixGraphicIds()
{
	int graphicId = 0;
	for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
		if (Data_Empire_Objects[i].inUse
			&& Data_Empire_Objects[i].type == EmpireObject_City
			&& Data_Empire_Objects[i].cityType == EmpireCity_Ours) {
			graphicId = Data_Empire_Objects[i].graphicId;
			break;
		}
	}
	if (graphicId > 0 && graphicId != image_group(ID_Graphic_EmpireCity)) {
		// empire map uses old version of graphics: increase every graphic id
		int offset = image_group(ID_Graphic_EmpireCity) - graphicId;
		for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
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
	for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
		if (Data_Empire_Objects[i].inUse && Data_Empire_Objects[i].tradeRouteId == routeId) {
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
	for (int i = 0; i < MAX_EMPIRE_CITIES; i++) {
		if (Data_Empire_Cities[i].inUse && Data_Empire_Cities[i].empireObjectId == empireObjectId) {
			return i;
		}
	}
	return 0;
}

int Empire_getCityForTradeRoute(int routeId)
{
	for (int i = 0; i < MAX_EMPIRE_CITIES; i++) {
		if (Data_Empire_Cities[i].inUse && Data_Empire_Cities[i].routeId == routeId) {
			return i;
		}
	}
	return 0;
}

int Empire_isTradeRouteOpen(int routeId)
{
	for (int i = 0; i < MAX_EMPIRE_CITIES; i++) {
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
	if (game_time_year() < Data_Scenario.empireExpansionYear + Data_Scenario.startYear) {
		return;
	}

	for (int i = 0; i < MAX_EMPIRE_CITIES; i++) {
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
			Data_Empire_Objects[i].graphicIdExpanded = image_group(ID_Graphic_EmpireCityTrade);
		} else if (Data_Empire_Cities[i].cityType == EmpireCity_DistantRoman) {
			Data_Empire_Objects[i].graphicIdExpanded = image_group(ID_Graphic_EmpireCityDistantRoman);
		}
	}

	Data_Scenario.empireHasExpanded = 1;
	PlayerMessage_post(1, Message_77_EmpireHasExpanded, 0, 0);
}
