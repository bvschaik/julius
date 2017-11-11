#include "Trader.h"

#include "Empire.h"
#include "Figure.h"
#include "PlayerMessage.h"
#include "Resource.h"
#include "Terrain.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Message.h"
#include "Data/Scenario.h"

#include "Data/Figure.h"

#include "building/count.h"
#include "building/storage.h"
#include "empire/city.h"
#include "empire/trade_prices.h"
#include "empire/trade_route.h"
#include "figure/type.h"

#include <string.h>

static int generateTrader(int cityId, empire_city *city)
{
	int maxTradersOnMap = 0;
	int numResources = 0;
	for (int r = Resource_Min; r < Resource_Max; r++) {
		if (city->buys_resource[r] || city->sells_resource[r]) {
			++numResources;
			switch (trade_route_limit(city->route_id, r)) {
				case 15: maxTradersOnMap += 1; break;
				case 25: maxTradersOnMap += 2; break;
				case 40: maxTradersOnMap += 3; break;
			}
		}
	}
	if (numResources > 1) {
		if (maxTradersOnMap % numResources) {
			maxTradersOnMap = maxTradersOnMap / numResources + 1;
		} else {
			maxTradersOnMap = maxTradersOnMap / numResources;
		}
	}
	if (maxTradersOnMap <= 0) {
		return 0;
	}

	int index;
	if (maxTradersOnMap == 1) {
		if (!city->trader_figure_ids[0]) {
			index = 0;
		} else {
			return 0;
		}
	} else if (maxTradersOnMap == 2) {
		if (!city->trader_figure_ids[0]) {
			index = 0;
		} else if (!city->trader_figure_ids[1]) {
			index = 1;
		} else {
			return 0;
		}
	} else { // 3
		if (!city->trader_figure_ids[0]) {
			index = 0;
		} else if (!city->trader_figure_ids[1]) {
			index = 1;
		} else if (!city->trader_figure_ids[2]) {
			index = 2;
		} else {
			return 0;
		}
	}

	if (city->trader_entry_delay > 0) {
		city->trader_entry_delay--;
		return 0;
	}
	city->trader_entry_delay = city->is_sea_trade ? 30 : 4;

	if (city->is_sea_trade) {
		// generate ship
		if (Data_CityInfo.numWorkingDocks > 0 &&
			(Data_Scenario.riverEntryPoint.x != -1 || Data_Scenario.riverEntryPoint.y != -1) &&
			!Data_CityInfo.tradeSeaProblemDuration) {
			int shipId = Figure_create(FIGURE_TRADE_SHIP,
				Data_Scenario.riverEntryPoint.x, Data_Scenario.riverEntryPoint.y, 0);
			city->trader_figure_ids[index] = shipId;
			Data_Figures[shipId].empireCityId = cityId;
			Data_Figures[shipId].actionState = FigureActionState_110_TradeShipCreated;
			Data_Figures[shipId].waitTicks = 10;
			return 1;
		}
	} else {
		// generate caravan and donkeys
		if (!Data_CityInfo.tradeLandProblemDuration) {
			// caravan head
			int caravanId = Figure_create(FIGURE_TRADE_CARAVAN,
				Data_CityInfo.entryPointX, Data_CityInfo.entryPointY, 0);
			city->trader_figure_ids[index] = caravanId;
			Data_Figures[caravanId].empireCityId = cityId;
			Data_Figures[caravanId].actionState = FigureActionState_100_TradeCaravanCreated;
			Data_Figures[caravanId].waitTicks = 10;
			// donkey 1
			int donkey1 = Figure_create(FIGURE_TRADE_CARAVAN_DONKEY,
				Data_CityInfo.entryPointX, Data_CityInfo.entryPointY, 0);
			Data_Figures[donkey1].actionState = FigureActionState_100_TradeCaravanCreated;
			Data_Figures[donkey1].inFrontFigureId = caravanId;
			// donkey 2
			int donkey2 = Figure_create(FIGURE_TRADE_CARAVAN_DONKEY,
				Data_CityInfo.entryPointX, Data_CityInfo.entryPointY, 0);
			Data_Figures[donkey2].actionState = FigureActionState_100_TradeCaravanCreated;
			Data_Figures[donkey2].inFrontFigureId = donkey1;
			return 1;
		}
	}
	return 0;
}

int canGenerateTraderForCity(int city_id, empire_city *city)
{
    if (city->is_sea_trade) {
        if (Data_CityInfo.numWorkingDocks <= 0) {
            if (Data_Message.messageCategoryCount[MessageDelay_NoWorkingDock] > 0) {
                Data_Message.messageCategoryCount[MessageDelay_NoWorkingDock]--;
            } else {
                PlayerMessage_post(1, Message_117_NoWorkingDock, 0, 0);
                Data_Message.messageCategoryCount[MessageDelay_NoWorkingDock] = 384; // 1 year
            }
            return 0;
        }
        if (Data_Scenario.riverEntryPoint.x == -1 && Data_Scenario.riverEntryPoint.y == -1) {
            return 0;
        }
        Data_CityInfo.tradeNumOpenSeaRoutes++;
    } else {
        Data_CityInfo.tradeNumOpenLandRoutes++;
    }
    return generateTrader(city_id, city);
}

void Trader_tick()
{
	Data_CityInfo.tradeNumOpenSeaRoutes = 0;
	Data_CityInfo.tradeNumOpenLandRoutes = 0;
	// Wine types
	Data_CityInfo.resourceWineTypesAvailable = building_count_industry_total(RESOURCE_WINE) > 0 ? 1 : 0;
    if (Data_CityInfo.resourceTradeStatus[Resource_Wine] == TradeStatus_Import) {
        Data_CityInfo.resourceWineTypesAvailable += empire_city_count_wine_sources();
    }
	// Update trade problems
	if (Data_CityInfo.tradeLandProblemDuration > 0) {
		Data_CityInfo.tradeLandProblemDuration--;
	} else {
		Data_CityInfo.tradeLandProblemDuration = 0;
	}
	if (Data_CityInfo.tradeSeaProblemDuration > 0) {
		Data_CityInfo.tradeSeaProblemDuration--;
	} else {
		Data_CityInfo.tradeSeaProblemDuration = 0;
	}
	// Generate traders
	empire_city_foreach_open_until(canGenerateTraderForCity);
}

int Trader_getClosestWarehouseForTradeCaravan(int figureId, int x, int y, int cityId, int distanceFromEntry, int *warehouseX, int *warehouseY)
{
	int exportable[Resource_Max];
	int importable[Resource_Max];
	exportable[Resource_None] = 0;
	importable[Resource_None] = 0;
	for (int r = Resource_Min; r < Resource_Max; r++) {
		exportable[r] = Empire_canExportResourceToCity(cityId, r);
		if (Data_Figures[figureId].traderAmountBought >= 8) {
			exportable[r] = 0;
		}
		if (cityId) {
			importable[r] = Empire_canImportResourceFromCity(cityId, r);
		} else { // exclude own city (id=0), shouldn't happen, but still..
			importable[r] = 0;
		}
		if (Data_Figures[figureId].loadsSoldOrCarrying >= 8) {
			importable[r] = 0;
		}
	}
	int numImportable = 0;
	for (int r = Resource_Min; r < Resource_Max; r++) {
		if (importable[r]) {
			numImportable++;
		}
	}
	int minDistance = 10000;
	int minBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i) || Data_Buildings[i].type != BUILDING_WAREHOUSE) {
			continue;
		}
		if (!Data_Buildings[i].hasRoadAccess || Data_Buildings[i].distanceFromEntry <= 0) {
			continue;
		}
		const building_storage *s = building_storage_get(Data_Buildings[i].storage_id);
		int numImportsForWarehouse = 0;
		for (int r = Resource_Min; r < Resource_Max; r++) {
			if (s->resource_state[r] != BUILDING_STORAGE_STATE_NOT_ACCEPTING && Empire_canImportResourceFromCity(cityId, r)) {
				numImportsForWarehouse++;
			}
		}
		int distancePenalty = 32;
		int spaceId = i;
		for (int spaceCounter = 0; spaceCounter < 8; spaceCounter++) {
			spaceId = Data_Buildings[spaceId].nextPartBuildingId;
			if (spaceId && exportable[Data_Buildings[spaceId].subtype.warehouseResourceId]) {
				distancePenalty -= 4;
			}
			if (numImportable && numImportsForWarehouse && !s->empty_all) {
				for (int r = Resource_Min; r < Resource_Max; r++) {
					Data_CityInfo.tradeNextImportResourceCaravan++;
					if (Data_CityInfo.tradeNextImportResourceCaravan > 15) {
						Data_CityInfo.tradeNextImportResourceCaravan = 1;
					}
					if (s->resource_state[Data_CityInfo.tradeNextImportResourceCaravan] != BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
						break;
					}
				}
				if (s->resource_state[Data_CityInfo.tradeNextImportResourceCaravan] != BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
					if (Data_Buildings[spaceId].subtype.warehouseResourceId == RESOURCE_NONE) {
						distancePenalty -= 16;
					}
					if (spaceId && importable[Data_Buildings[spaceId].subtype.warehouseResourceId] &&
						Data_Buildings[spaceId].loadsStored < 4 &&
						Data_Buildings[spaceId].subtype.warehouseResourceId == Data_CityInfo.tradeNextImportResourceCaravan) {
						distancePenalty -= 8;
					}
				}
			}
		}
		if (distancePenalty < 32) {
			int distance = Resource_getDistance(
				Data_Buildings[i].x, Data_Buildings[i].y, x, y,
				distanceFromEntry, Data_Buildings[i].distanceFromEntry);
			distance += distancePenalty;
			if (distance < minDistance) {
				minDistance = distance;
				minBuildingId = i;
			}
		}
	}
	if (!minBuildingId) {
		return 0;
	}
	if (Data_Buildings[minBuildingId].hasRoadAccess == 1) {
		*warehouseX = Data_Buildings[minBuildingId].x;
		*warehouseY = Data_Buildings[minBuildingId].y;
	} else if (!Terrain_hasRoadAccess(
			Data_Buildings[minBuildingId].x, Data_Buildings[minBuildingId].y, 3,
			warehouseX, warehouseY)) {
		return 0;
	}
	return minBuildingId;
}

int Trader_getClosestWarehouseForImportDocker(int x, int y, int cityId, int distanceFromEntry, int roadNetworkId, int *warehouseX, int *warehouseY)
{
	int importable[16];
	importable[Resource_None] = 0;
	for (int r = Resource_Min; r < Resource_Max; r++) {
		importable[r] = Empire_canImportResourceFromCity(cityId, r);
	}
	Data_CityInfo.tradeNextImportResourceDocker++;
	if (Data_CityInfo.tradeNextImportResourceDocker > 15) {
		Data_CityInfo.tradeNextImportResourceDocker = 1;
	}
	for (int i = Resource_Min; i < Resource_Max && !importable[Data_CityInfo.tradeNextImportResourceDocker]; i++) {
		Data_CityInfo.tradeNextImportResourceDocker++;
		if (Data_CityInfo.tradeNextImportResourceDocker > 15) {
			Data_CityInfo.tradeNextImportResourceDocker = 1;
		}
	}
	if (!importable[Data_CityInfo.tradeNextImportResourceDocker]) {
		return 0;
	}
	int minDistance = 10000;
	int minBuildingId = 0;
	int resourceId = Data_CityInfo.tradeNextImportResourceDocker;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i) || Data_Buildings[i].type != BUILDING_WAREHOUSE) {
			continue;
		}
		if (!Data_Buildings[i].hasRoadAccess || Data_Buildings[i].distanceFromEntry <= 0) {
			continue;
		}
		if (Data_Buildings[i].roadNetworkId != roadNetworkId) {
			continue;
		}
		const building_storage *s = building_storage_get(Data_Buildings[i].storage_id);
		if (s->resource_state[resourceId] != BUILDING_STORAGE_STATE_NOT_ACCEPTING && !s->empty_all) {
			int distancePenalty = 32;
			int spaceId = i;
			for (int spaceCounter = 0; spaceCounter < 8; spaceCounter++) {
				spaceId = Data_Buildings[spaceId].nextPartBuildingId;
				if (spaceId && Data_Buildings[spaceId].subtype.warehouseResourceId == Resource_None) {
					distancePenalty -= 8;
				}
				if (spaceId &&
					Data_Buildings[spaceId].subtype.warehouseResourceId == resourceId &&
					Data_Buildings[spaceId].loadsStored < 4) {
					distancePenalty -= 4;
				}
			}
			if (distancePenalty < 32) {
				int distance = Resource_getDistance(
					Data_Buildings[i].x, Data_Buildings[i].y, x, y,
					distanceFromEntry, Data_Buildings[i].distanceFromEntry);
				// prefer emptier warehouse
				distance += distancePenalty;
				if (distance < minDistance) {
					minDistance = distance;
					minBuildingId = i;
				}
			}
		}
	}
	if (!minBuildingId) {
		return 0;
	}
	if (Data_Buildings[minBuildingId].hasRoadAccess == 1) {
		*warehouseX = Data_Buildings[minBuildingId].x;
		*warehouseY = Data_Buildings[minBuildingId].y;
	} else if (!Terrain_hasRoadAccess(
			Data_Buildings[minBuildingId].x, Data_Buildings[minBuildingId].y, 3,
			warehouseX, warehouseY)) {
		return 0;
	}
	return minBuildingId;
}

int Trader_getClosestWarehouseForExportDocker(int x, int y, int cityId, int distanceFromEntry, int roadNetworkId, int *warehouseX, int *warehouseY)
{
	int exportable[16];
	exportable[Resource_None] = 0;
	for (int r = Resource_Min; r < Resource_Max; r++) {
		exportable[r] = Empire_canExportResourceToCity(cityId, r);
	}
	Data_CityInfo.tradeNextExportResourceDocker++;
	if (Data_CityInfo.tradeNextExportResourceDocker > 15) {
		Data_CityInfo.tradeNextExportResourceDocker = 1;
	}
	for (int i = Resource_Min; i < Resource_Max && !exportable[Data_CityInfo.tradeNextExportResourceDocker]; i++) {
		Data_CityInfo.tradeNextExportResourceDocker++;
		if (Data_CityInfo.tradeNextExportResourceDocker > 15) {
			Data_CityInfo.tradeNextExportResourceDocker = 1;
		}
	}
	if (!exportable[Data_CityInfo.tradeNextExportResourceDocker]) {
		return 0;
	}
	int minDistance = 10000;
	int minBuildingId = 0;
	int resourceId = Data_CityInfo.tradeNextExportResourceDocker;
	
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i) || Data_Buildings[i].type != BUILDING_WAREHOUSE) {
			continue;
		}
		if (!Data_Buildings[i].hasRoadAccess || Data_Buildings[i].distanceFromEntry <= 0) {
			continue;
		}
		if (Data_Buildings[i].roadNetworkId != roadNetworkId) {
			continue;
		}
		int distancePenalty = 32;
		int spaceId = i;
		for (int s = 0; s < 8; s++) {
			spaceId = Data_Buildings[spaceId].nextPartBuildingId;
			if (spaceId &&
				Data_Buildings[spaceId].subtype.warehouseResourceId == resourceId &&
				Data_Buildings[spaceId].loadsStored > 0) {
				distancePenalty--;
			}
		}
		if (distancePenalty < 32) {
			int distance = Resource_getDistance(
				Data_Buildings[i].x, Data_Buildings[i].y, x, y,
				distanceFromEntry, Data_Buildings[i].distanceFromEntry);
			// prefer fuller warehouse
			distance += distancePenalty;
			if (distance < minDistance) {
				minDistance = distance;
				minBuildingId = i;
			}
		}
	}
	if (!minBuildingId) {
		return 0;
	}
	if (Data_Buildings[minBuildingId].hasRoadAccess == 1) {
		*warehouseX = Data_Buildings[minBuildingId].x;
		*warehouseY = Data_Buildings[minBuildingId].y;
	} else if (!Terrain_hasRoadAccess(
			Data_Buildings[minBuildingId].x, Data_Buildings[minBuildingId].y, 3,
			warehouseX, warehouseY)) {
		return 0;
	}
	return minBuildingId;
}

int Trader_tryImportResource(int buildingId, int resourceId, int cityId)
{
	if (Data_Buildings[buildingId].type != BUILDING_WAREHOUSE) {
		return 0;
	}
	
	int routeId = empire_city_get_route_id(cityId);
	// try existing storage bay with the same resource
	int spaceId = buildingId;
	for (int i = 0; i < 8; i++) {
		spaceId = Data_Buildings[spaceId].nextPartBuildingId;
		if (spaceId > 0 &&
			Data_Buildings[spaceId].loadsStored &&
			Data_Buildings[spaceId].loadsStored < 4 &&
			Data_Buildings[spaceId].subtype.warehouseResourceId == resourceId) {
			trade_route_increase_traded(routeId, resourceId);
			Resource_addImportedResourceToWarehouseSpace(spaceId, resourceId);
			return 1;
		}
	}
	// try unused storage bay
	spaceId = buildingId;
	for (int i = 0; i < 8; i++) {
		spaceId = Data_Buildings[spaceId].nextPartBuildingId;
		if (spaceId > 0 && Data_Buildings[spaceId].subtype.warehouseResourceId == Resource_None) {
			trade_route_increase_traded(routeId, resourceId);
			Resource_addImportedResourceToWarehouseSpace(spaceId, resourceId);
			return 1;
		}
	}
	return 0;
}

int Trader_tryExportResource(int buildingId, int resourceId, int cityId)
{
	if (Data_Buildings[buildingId].type != BUILDING_WAREHOUSE) {
		return 0;
	}
	
	int spaceId = buildingId;
	for (int i = 0; i < 8; i++) {
		spaceId = Data_Buildings[spaceId].nextPartBuildingId;
		if (spaceId > 0) {
			if (Data_Buildings[spaceId].loadsStored &&
				Data_Buildings[spaceId].subtype.warehouseResourceId == resourceId) {
				trade_route_increase_traded(empire_city_get_route_id(cityId), resourceId);
				Resource_removeExportedResourceFromWarehouseSpace(spaceId, resourceId);
				return 1;
			}
		}
	}
	return 0;
}
