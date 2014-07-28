#include "Resource.h"

#include "Building.h"
#include "Calc.h"
#include "Terrain.h"
#include "Tutorial.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Model.h"
#include "Data/Trade.h"

void Resource_setWarehouseSpaceGraphic(int spaceId, int resource)
{
	int graphicId;
	if (Data_Buildings[spaceId].loadsStored <= 0) {
		graphicId = GraphicId(ID_Graphic_WarehouseStorageEmpty);
	} else {
		graphicId = GraphicId(ID_Graphic_WarehouseStorageFilled) +
			4 * (resource - 1) + Resource_getGraphicIdOffset(resource, 0) +
			Data_Buildings[spaceId].loadsStored - 1;
	}
	Data_Grid_graphicIds[Data_Buildings[spaceId].gridOffset] = graphicId;
}

void Resource_addToCityWarehouses(int resource, int amount)
{
	int buildingId = Data_CityInfo.resourceLastTargetWarehouse;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (++buildingId >= MAX_BUILDINGS) {
			buildingId = 1;
		}
		if (Data_Buildings[buildingId].inUse == 1 &&
			Data_Buildings[buildingId].type == Building_Warehouse) {
			Data_CityInfo.resourceLastTargetWarehouse = buildingId;
			while (amount && Resource_addToWarehouse(buildingId, resource)) {
				amount--;
			}
		}
	}
}

int Resource_removeFromCityWarehouses(int resource, int amount)
{
	int amountLeft = amount;
	int buildingId = Data_CityInfo.resourceLastTargetWarehouse;
	// first go for non-getting warehouses
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (++buildingId >= MAX_BUILDINGS) {
			buildingId = 1;
		}
		if (Data_Buildings[buildingId].inUse == 1 &&
			Data_Buildings[buildingId].type == Building_Warehouse) {
			int storageId = Data_Buildings[buildingId].storageId;
			if (Data_Building_Storages[storageId].resourceState[resource] != BuildingStorageState_Getting) {
				Data_CityInfo.resourceLastTargetWarehouse = buildingId;
				amountLeft = Resource_removeFromWarehouse(buildingId, resource, amountLeft);
			}
		}
	}
	// if that doesn't work, take it anyway
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (++buildingId >= MAX_BUILDINGS) {
			buildingId = 1;
		}
		if (Data_Buildings[buildingId].inUse == 1 &&
			Data_Buildings[buildingId].type == Building_Warehouse) {
			Data_CityInfo.resourceLastTargetWarehouse = buildingId;
			amountLeft = Resource_removeFromWarehouse(buildingId, resource, amountLeft);
		}
	}
	return amount - amountLeft;
}

int Resource_getWarehouseForStoringResource(
	int srcBuildingId, int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *understaffed, int *xDst, int *yDst)
{
	int minDist = 10000;
	int minBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (b->inUse != 1 || b->type != Building_WarehouseSpace) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0 || b->roadNetworkId != roadNetworkId) {
			continue;
		}
		int dstBuildingId = Building_getMainBuildingId(i);
		if (srcBuildingId == dstBuildingId) {
			continue;
		}
		int storageId = Data_Buildings[dstBuildingId].storageId;
		struct Data_Building_Storage *s = &Data_Building_Storages[storageId];
		if (s->resourceState[resource] == BuildingStorageState_NotAccepting || s->emptyAll) {
			continue;
		}
		int pctWorkers = Calc_getPercentage(
			Data_Buildings[dstBuildingId].numWorkers,
			Data_Model_Buildings[Data_Buildings[dstBuildingId].type].laborers);
		if (pctWorkers < 100) {
			if (understaffed) {
				*understaffed += 1;
			}
			continue;
		}
		int dist;
		if (b->subtype.warehouseResourceId) {
			if (b->subtype.warehouseResourceId == resource && b->loadsStored < 4) {
				dist = Resource_getDistance(b->x, b->y, x, y, distanceFromEntry, b->distanceFromEntry);
			} else {
				dist = 0;
			}
		} else { // empty warehouse space
			dist = Resource_getDistance(b->x, b->y, x, y, distanceFromEntry, b->distanceFromEntry);
		}
		if (dist > 0 && dist < minDist) {
			minDist = dist;
			minBuildingId = i;
		}
	}
	int resultBuildingId = Building_getMainBuildingId(minBuildingId);
	struct Data_Building *b = &Data_Buildings[resultBuildingId];
	if (b->hasRoadAccess == 1) {
		*xDst = b->x;
		*yDst = b->y;
	} else if (!Terrain_hasRoadAccess(b->x, b->y, 3, xDst, yDst)) {
		return 0;
	}
	return resultBuildingId;
}

int Resource_getWarehouseForGettingResource(int srcBuildingId, int resource, int *xDst, int *yDst)
{
	struct Data_Building *src = &Data_Buildings[srcBuildingId];
	int minDist = 0;
	int minBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (b->inUse != 1 || b->type != Building_Warehouse) {
			continue;
		}
		if (i == srcBuildingId) {
			continue;
		}
		int loadsStored = 0;
		int spaceId = i;
		struct Data_Building_Storage *s = &Data_Building_Storages[b->storageId];
		for (int t = 0; t < 8; t++) {
			spaceId = Data_Buildings[spaceId].nextPartBuildingId;
			if (spaceId > 0 && Data_Buildings[spaceId].loadsStored > 0) {
				if (Data_Buildings[spaceId].subtype.warehouseResourceId == resource) {
					loadsStored += Data_Buildings[spaceId].loadsStored;
				}
			}
		}
		if (loadsStored > 0 && s->resourceState[resource] != BuildingStorageState_Getting) {
			int dist = Resource_getDistance(b->x, b->y, src->x, src->y,
				src->distanceFromEntry, b->distanceFromEntry);
			dist -= 4 * loadsStored;
			if (dist < minDist) {
				minDist = dist;
				minBuildingId = i;
			}
		}
	}
	if (minBuildingId > 0) {
		*xDst = Data_Buildings[minBuildingId].x;
		*yDst = Data_Buildings[minBuildingId].y;
		return minBuildingId;
	} else {
		return 0;
	}
}

int Resource_addToWarehouse(int buildingId, int resource)
{
	if (buildingId <= 0) {
		return 0;
	}
	// check building itself
	int findSpace = 0;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->subtype.warehouseResourceId && b->subtype.warehouseResourceId != resource) {
		findSpace = 1;
	} else if (b->loadsStored >= 4) {
		findSpace = 1;
	} else if (b->type == Building_Warehouse) {
		findSpace = 1;
	}
	if (findSpace) {
		int spaceFound = 0;
		buildingId = Building_getMainBuildingId(buildingId);
		b = &Data_Buildings[buildingId];
		for (int i = 0; i < 8; i++) {
			buildingId = b->nextPartBuildingId;
			if (!buildingId) {
				return 0;
			}
			b = &Data_Buildings[buildingId];
			if (!b->subtype.warehouseResourceId || b->subtype.warehouseResourceId == resource) {
				if (b->loadsStored < 4) {
					spaceFound = 1;
					break;
				}
			}
		}
		if (!spaceFound) {
			return 0;
		}
	}
	Data_CityInfo.resourceSpaceInWarehouses[resource]--;
	Data_CityInfo.resourceStored[resource]++;
	b->subtype.warehouseResourceId = resource;
	b->loadsStored++;
	Tutorial_onAddToWarehouse();
	Resource_setWarehouseSpaceGraphic(buildingId, resource);
	return 1;
}

int Resource_removeFromWarehouse(int buildingId, int resource, int amount)
{
	// returns amount still needing removal
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->type != Building_Warehouse) {
		return amount;
	}
	for (int i = 0; i < 8; i++) {
		if (amount <= 0) {
			return 0;
		}
		buildingId = b->nextPartBuildingId;
		if (buildingId <= 0) {
			return amount;
		}
		b = &Data_Buildings[buildingId];
		if (b->subtype.warehouseResourceId != resource || b->loadsStored <= 0) {
			continue;
		}
		if (b->loadsStored > amount) {
			Data_CityInfo.resourceSpaceInWarehouses[resource] += amount;
			Data_CityInfo.resourceStored[resource] -= amount;
			b->loadsStored -= amount;
			amount = 0;
		} else {
			Data_CityInfo.resourceSpaceInWarehouses[resource] += b->loadsStored;
			Data_CityInfo.resourceStored[resource] -= b->loadsStored;
			amount -= b->loadsStored;
			b->loadsStored = 0;
			b->subtype.warehouseResourceId = Resource_None;
		}
		Resource_setWarehouseSpaceGraphic(buildingId, resource);
	}
	return amount;
}

void Resource_removeFromWarehouseForMercury(int buildingId, int amount)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->type != Building_Warehouse) {
		return;
	}
	for (int i = 0; i < 8; i++) {
		buildingId = b->nextPartBuildingId;
		b = &Data_Buildings[buildingId];
		if (buildingId <= 0) {
			continue;
		}
		int resource = b->subtype.warehouseResourceId;
		if (b->loadsStored > amount) {
			Data_CityInfo.resourceSpaceInWarehouses[resource] += amount;
			Data_CityInfo.resourceStored[resource] -= amount;
			b->loadsStored -= amount;
			amount = 0;
		} else {
			Data_CityInfo.resourceSpaceInWarehouses[resource] += b->loadsStored;
			Data_CityInfo.resourceStored[resource] -= b->loadsStored;
			amount -= b->loadsStored;
			b->loadsStored = 0;
			b->subtype.warehouseResourceId = Resource_None;
		}
		Resource_setWarehouseSpaceGraphic(buildingId, resource);
	}
}

int Resource_getAmountStoredInWarehouse(int buildingId, int resource)
{
	int loads = 0;
	for (int i = 0; i < 8; i++) {
		buildingId = Data_Buildings[buildingId].nextPartBuildingId;
		if (buildingId <= 0) {
			return 0;
		}
		if (resource && Data_Buildings[buildingId].subtype.warehouseResourceId == resource) {
			loads += Data_Buildings[buildingId].loadsStored;
		}
	}
	return loads;
}

int Resource_getWarehouseSpaceInfo(int buildingId)
{
	int totalLoads = 0;
	int emptySpaces = 0;
	for (int i = 0; i < 8; i++) {
		buildingId = Data_Buildings[buildingId].nextPartBuildingId;
		if (buildingId <= 0) {
			return 0;
		}
		if (Data_Buildings[buildingId].subtype.warehouseResourceId) {
			totalLoads += Data_Buildings[buildingId].loadsStored;
		} else {
			emptySpaces++;
		}
	}
	if (emptySpaces > 0) {
		return 0; // room available
	} else if (totalLoads < 32) {
		return 2; // some room available for existing goods
	} else {
		return 1; // full
	}
}

void Resource_addImportedResourceToWarehouseSpace(int spaceId, int resourceId)
{
	Data_CityInfo.resourceSpaceInWarehouses[resourceId]--;
	Data_CityInfo.resourceStored[resourceId]++;
	Data_Buildings[spaceId].loadsStored++;
	Data_Buildings[spaceId].subtype.warehouseResourceId = resourceId;
	
	Data_CityInfo.treasury -= Data_TradePrices[resourceId].buy;
	Data_CityInfo.financeImportsThisYear += Data_TradePrices[resourceId].buy;
	
	Resource_setWarehouseSpaceGraphic(spaceId, resourceId);
}

void Resource_removeExportedResourceFromWarehouseSpace(int spaceId, int resourceId)
{
	Data_CityInfo.resourceSpaceInWarehouses[resourceId]++;
	Data_CityInfo.resourceStored[resourceId]--;
	if (--Data_Buildings[spaceId].loadsStored <= 0) {
		Data_Buildings[spaceId].subtype.warehouseResourceId = Resource_None;
	}
	
	Data_CityInfo.treasury += Data_TradePrices[resourceId].sell;
	Data_CityInfo.financeExportsThisYear += Data_TradePrices[resourceId].sell;
	if (Data_CityInfo.godBlessingNeptuneDoubleTrade) {
		Data_CityInfo.treasury += Data_TradePrices[resourceId].sell;
		Data_CityInfo.financeExportsThisYear += Data_TradePrices[resourceId].sell;
	}
	
	Resource_setWarehouseSpaceGraphic(spaceId, resourceId);
}

