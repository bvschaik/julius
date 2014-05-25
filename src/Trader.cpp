#include "Trader.h"

#include "Empire.h"
#include "PlayerMessage.h"
#include "Resource.h"
#include "Terrain.h"
#include "Walker.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Empire.h"
#include "Data/Message.h"
#include "Data/Scenario.h"
#include "Data/Trade.h"
#include "Data/Walker.h"

#include <string.h>

void Trader_clearList()
{
	memset(Data_Walker_Traders, 0, MAX_TRADERS * sizeof(struct Data_Walker_Trader));
	Data_Walker_Extra.nextTraderId = 0;
}

void Trader_create(int walkerId)
{
	Data_Walkers[walkerId].traderId = Data_Walker_Extra.nextTraderId;
	memset(&Data_Walker_Traders[Data_Walker_Extra.nextTraderId], 0, sizeof(struct Data_Walker_Trader));
	if (++Data_Walker_Extra.nextTraderId >= 100) {
		Data_Walker_Extra.nextTraderId = 0;
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

static int generateTrader(int cityId)
{
	struct Data_Empire_City *c = &Data_Empire_Cities[cityId];
	int maxTradersOnMap = 0;
	int numResources = 0;
	for (int r = 1; r < 16; r++) {
		if (c->buysResourceFlag[r] || c->sellsResourceFlag[r]) {
			++numResources;
			switch (Data_Empire_Trade.maxPerYear[c->routeId][r]) {
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
		if (!c->traderWalkerIds[0]) {
			index = 0;
		} else {
			return 0;
		}
	} else if (maxTradersOnMap == 2) {
		if (!c->traderWalkerIds[0]) {
			index = 0;
		} else if (!c->traderWalkerIds[1]) {
			index = 1;
		} else {
			return 0;
		}
	} else { // 3
		if (!c->traderWalkerIds[0]) {
			index = 0;
		} else if (!c->traderWalkerIds[1]) {
			index = 1;
		} else if (!c->traderWalkerIds[2]) {
			index = 2;
		} else {
			return 0;
		}
	}

	if (c->traderEntryDelay > 0) {
		c->traderEntryDelay--;
		return 0;
	}
	c->traderEntryDelay = c->isSeaTrade ? 30 : 4;

	if (c->isSeaTrade) {
		// generate ship
		if (!Data_CityInfo.tradeSeaProblemDuration) {
			int shipId = Walker_create(Walker_TradeShip,
				Data_Scenario.riverEntryPoint.x, Data_Scenario.riverEntryPoint.y, 0);
			c->traderWalkerIds[index] = shipId;
			Data_Walkers[shipId].empireCityId = cityId;
			Data_Walkers[shipId].actionState = WalkerActionState_110_TradeShipCreated;
			Data_Walkers[shipId].waitTicks = 10;
			return 1;
		}
	} else {
		// generate caravan and donkeys
		if (!Data_CityInfo.tradeLandProblemDuration) {
			// caravan head
			int caravanId = Walker_create(Walker_TradeCaravan,
				Data_CityInfo.entryPointX, Data_CityInfo.entryPointY, 0);
			c->traderWalkerIds[index] = caravanId;
			Data_Walkers[caravanId].empireCityId = cityId;
			Data_Walkers[caravanId].actionState = WalkerActionState_100_TradeCaravanCreated;
			Data_Walkers[caravanId].waitTicks = 10;
			// donkey 1
			int donkey1 = Walker_create(Walker_TradeCaravanDonkey,
				Data_CityInfo.entryPointX, Data_CityInfo.entryPointY, 0);
			Data_Walkers[donkey1].actionState = WalkerActionState_100_TradeCaravanCreated;
			Data_Walkers[donkey1].inFrontWalkerId = caravanId;
			// donkey 2
			int donkey2 = Walker_create(Walker_TradeCaravanDonkey,
				Data_CityInfo.entryPointX, Data_CityInfo.entryPointY, 0);
			Data_Walkers[donkey2].actionState = WalkerActionState_100_TradeCaravanCreated;
			Data_Walkers[donkey2].inFrontWalkerId = donkey1;
			return 1;
		}
	}
	return 0;
}

void Trader_tick()
{
	Data_CityInfo.tradeNumOpenSeaRoutes = 0;
	Data_CityInfo.tradeNumOpenLandRoutes = 0;
	// Wine types
	Data_CityInfo.resourceWineTypesAvailable = Data_CityInfo_Buildings.industry.total[Resource_Wine] > 0 ? 1 : 0;
	for (int i = 1; i < MAX_EMPIRE_CITIES; i++) {
		if (Data_Empire_Cities[i].inUse &&
			Data_Empire_Cities[i].isOpen &&
			Data_Empire_Cities[i].sellsResourceFlag[Resource_Wine] &&
			Data_CityInfo.resourceTradeStatus[Resource_Wine] == TradeStatus_Import) {
			++Data_CityInfo.resourceWineTypesAvailable;
		}
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
	for (int i = 1; i < MAX_EMPIRE_CITIES; i++) {
		if (!Data_Empire_Cities[i].inUse || !Data_Empire_Cities[i].isOpen) {
			continue;
		}
		if (Data_Empire_Cities[i].isSeaTrade) {
			if (Data_CityInfo.numWorkingDocks <= 0) {
				if (Data_Message.messageCategoryCount[MessageCount_NoWorkingDock] > 0) {
					Data_Message.messageCategoryCount[MessageCount_NoWorkingDock]--;
				} else {
					PlayerMessage_post(1, 117, 0, 0);
					Data_Message.messageCategoryCount[MessageCount_NoWorkingDock] = 384; // 1 year
				}
				continue;
			}
			if (Data_Scenario.riverEntryPoint.x == -1 || Data_Scenario.riverEntryPoint.y == -1) {
				continue;
			}
			Data_CityInfo.tradeNumOpenSeaRoutes++;
		} else {
			Data_CityInfo.tradeNumOpenLandRoutes++;
		}
		if (generateTrader(i)) {
			return;
		}
	}
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
			if (Data_Building_Storages[storageId].resourceState[r] != BuildingStorageState_NotAccepting &&
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
					if (Data_Building_Storages[storageId].resourceState[Data_CityInfo.tradeNextImportResourceCaravan] != BuildingStorageState_NotAccepting) {
						break;
					}
				}
				if (Data_Building_Storages[storageId].resourceState[Data_CityInfo.tradeNextImportResourceCaravan] != BuildingStorageState_NotAccepting) {
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
		if (Data_Building_Storages[storageId].resourceState[resourceId] != BuildingStorageState_NotAccepting &&
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
