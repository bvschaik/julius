#include "Trader.h"

#include "Empire.h"
#include "Resource.h"
#include "Terrain.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Empire.h"
#include "Data/Trade.h"
#include "Data/Walker.h"

#include <string.h>

void Trader_clearList()
{
	memset(Data_Walker_Traders, 0, MAX_TRADERS * sizeof(struct Data_Walker_Trader));
	Data_Walker_Trader_Extra.nextTraderId = 0;
}

void Trader_create(int walkerId)
{
	Data_Walkers[walkerId].traderId = Data_Walker_Trader_Extra.nextTraderId;
	memset(&Data_Walker_Traders[Data_Walker_Trader_Extra.nextTraderId], 0, sizeof(struct Data_Walker_Trader));
	if (++Data_Walker_Trader_Extra.nextTraderId >= 100) {
		Data_Walker_Trader_Extra.nextTraderId = 0;
	}
}

void Trader_sellResource(int walkerId, int resourceId)
{
	int traderId = Data_Walkers[walkerId].traderId;
	Data_Walker_Traders[traderId].totalSold++;
	Data_Walker_Traders[traderId].soldResources[resourceId]++;
	Data_Walker_Traders[traderId].moneySoldResources += Data_TradePrices[resourceId].sell;
}

void Trader_buyResource(int walkerId, int resourceId)
{
	int traderId = Data_Walkers[walkerId].traderId;
	Data_Walker_Traders[traderId].totalBought++;
	Data_Walker_Traders[traderId].boughtResources[resourceId]++;
	Data_Walker_Traders[traderId].moneyBoughtResources += Data_TradePrices[resourceId].buy;
}

int Trader_getClosestWarehouseForTradeCaravan(int walkerId, int x, int y, int cityId, int distanceFromEntry, int roadNetworkId, int *warehouseX, int *warehouseY)
{
	int exportable[16];
	int importable[16];
	for (int r = 1; r < 16; r++) {
		exportable[r] = Empire_canExportResourceToCity(cityId, r);
		if (Data_Walkers[walkerId].traderAmountBought >= 8) {
			exportable[r] = 0;
		}
		if (cityId) { // exclude own city, shouldn't happen, but still..
			importable[r] = Empire_canImportResourceFromCity(cityId, r);
		} else {
			importable[r] = 0;
		}
		if (Data_Walkers[walkerId].traderAmountSold >= 8) {
			exportable[r] = 0;
		}
	}
	int numImportable = 0;
	for (int r = 1; r < 16; r++) {
		if (importable[r]) {
			numImportable++;
		}
	}
	int minDistance = 10000;
	int minBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse != 1 || Data_Buildings[i].type != Building_Warehouse) {
			continue;
		}
		if (!Data_Buildings[i].hasRoadAccess || Data_Buildings[i].distanceFromEntry <= 0) {
			continue;
		}
		int storageId = Data_Buildings[i].storageId;
		int numImportsForWarehouse = 0;
		for (int r = 1; r < 16; r++) {
			if (Data_Building_Storages[storageId].resourceState[r] != BuildingStorageState_Emptying &&
				Empire_canImportResourceFromCity(cityId, r)) {
				numImportsForWarehouse++;
			}
		}
		int distancePenalty = 32;
		int spaceId = i;
		for (int s = 0; s < 8; s++) {
			spaceId = Data_Buildings[spaceId].nextPartBuildingId;
			if (spaceId && exportable[Data_Buildings[spaceId].subtype.warehouseResourceId]) {
				distancePenalty -= 4;
			}
			if (numImportable && numImportsForWarehouse && !Data_Building_Storages[storageId].emptyAll) {
				for (int r = 1; r < 16; r++) {
					if (++Data_CityInfo.tradeNextImportResourceCaravan > 15) {
						Data_CityInfo.tradeNextImportResourceCaravan = 1;
					}
					if (Data_Building_Storages[storageId].resourceState[Data_CityInfo.tradeNextImportResourceCaravan] != BuildingStorageState_Emptying) {
						break;
					}
				}
				if (Data_Building_Storages[storageId].resourceState[Data_CityInfo.tradeNextImportResourceCaravan] != BuildingStorageState_Emptying) {
					if (Data_Buildings[i].subtype.warehouseResourceId == Resource_None) {
						distancePenalty -= 16;
					}
					if (spaceId && importable[Data_Buildings[i].subtype.warehouseResourceId] &&
						Data_Buildings[i].loadsStored < 4 && Data_Buildings[i].subtype.warehouseResourceId == Data_CityInfo.tradeNextImportResourceCaravan) {
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
	for (int r = 1; r < 16; r++) {
		importable[r] = Empire_canImportResourceFromCity(cityId, r);
	}
	if (++Data_CityInfo.tradeNextImportResourceDocker > 15) {
		Data_CityInfo.tradeNextImportResourceDocker = 1;
	}
	for (int i = 1; i < 16 && !importable[Data_CityInfo.tradeNextImportResourceDocker]; i++) {
		if (++Data_CityInfo.tradeNextImportResourceDocker) {
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
		if (Data_Buildings[i].inUse != 1 || Data_Buildings[i].type != Building_Warehouse) {
			continue;
		}
		if (!Data_Buildings[i].hasRoadAccess || Data_Buildings[i].distanceFromEntry <= 0) {
			continue;
		}
		if (Data_Buildings[i].roadNetworkId != roadNetworkId) {
			continue;
		}
		int storageId = Data_Buildings[i].storageId;
		if (Data_Building_Storages[storageId].resourceState[resourceId] != BuildingStorageState_Emptying &&
			!Data_Building_Storages[storageId].emptyAll) {
			int distancePenalty = 32;
			int spaceId = i;
			for (int s = 0; s < 8; s++) {
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
	for (int r = 1; r < 16; r++) {
		exportable[r] = Empire_canExportResourceToCity(cityId, r);
	}
	if (++Data_CityInfo.tradeNextExportResourceDocker > 15) {
		Data_CityInfo.tradeNextExportResourceDocker = 1;
	}
	for (int i = 1; i < 16 && !exportable[Data_CityInfo.tradeNextExportResourceDocker]; i++) {
		if (++Data_CityInfo.tradeNextExportResourceDocker) {
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
		if (Data_Buildings[i].inUse != 1 || Data_Buildings[i].type != Building_Warehouse) {
			continue;
		}
		if (!Data_Buildings[i].hasRoadAccess || Data_Buildings[i].distanceFromEntry <= 0) {
			continue;
		}
		if (Data_Buildings[i].roadNetworkId != roadNetworkId) {
			continue;
		}
		int distancePenalty = 32;
		int storageId = i;
		for (int s = 0; s < 8; s++) {
			storageId = Data_Buildings[storageId].nextPartBuildingId;
			if (storageId &&
				Data_Buildings[storageId].subtype.warehouseResourceId == resourceId &&
				Data_Buildings[storageId].loadsStored > 0) {
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
	if (Data_Buildings[buildingId].type != Building_Warehouse) {
		return 0;
	}
	
	int routeId = Data_Empire_Cities[cityId].routeId;
	// try existing storage bay with the same resource
	int storageId = buildingId;
	for (int i = 0; i < 8; i++) {
		storageId = Data_Buildings[storageId].nextPartBuildingId;
		if (storageId > 0 &&
			Data_Buildings[storageId].loadsStored &&
			Data_Buildings[storageId].loadsStored < 4 &&
			Data_Buildings[storageId].subtype.warehouseResourceId == resourceId) {
			Data_Empire_Trade.tradedThisYear[routeId][resourceId]++;
			Resource_addImportedResourceToWarehouseSpace(storageId, resourceId);
			return 1;
		}
	}
	// try unused storage bay
	storageId = buildingId;
	for (int i = 0; i < 8; i++) {
		storageId = Data_Buildings[storageId].nextPartBuildingId;
		if (storageId > 0 && Data_Buildings[storageId].subtype.warehouseResourceId == Resource_None) {
			Data_Empire_Trade.tradedThisYear[routeId][resourceId]++;
			Resource_addImportedResourceToWarehouseSpace(storageId, resourceId);
			return 1;
		}
	}
	return 0;
}

int Trader_tryExportResource(int buildingId, int resourceId, int cityId)
{
	if (Data_Buildings[buildingId].type != Building_Warehouse) {
		return 0;
	}
	
	int storageId = buildingId;
	for (int i = 0; i < 8; i++) {
		storageId = Data_Buildings[storageId].nextPartBuildingId;
		if (storageId > 0) {
			if (Data_Buildings[storageId].loadsStored &&
				Data_Buildings[storageId].subtype.warehouseResourceId == resourceId) {
				Data_Empire_Trade.tradedThisYear[Data_Empire_Cities[cityId].routeId][resourceId]++;
				Resource_removeExportedResourceFromWarehouseSpace(storageId, resourceId);
				return 1;
			}
		}
	}
	return 0;
}
