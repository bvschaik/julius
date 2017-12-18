#ifndef RESOURCE_H
#define RESOURCE_H

#include "building/building.h"

// general functions
void Resource_calculateWarehouseStocks();

int Resource_getGraphicIdOffset(int resource, int type);

// warehouse functions
void Resource_setWarehouseSpaceGraphic(building *space, int resource);

void Resource_addToCityWarehouses(int resource, int amount);
int Resource_removeFromCityWarehouses(int resource, int amount);

int Resource_getWarehouseForStoringResource(
	int srcBuildingId, int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *understaffed, int *xDst, int *yDst);
int Resource_getWarehouseForGettingResource(int srcBuildingId, int resource, int *xDst, int *yDst);

int Resource_addToWarehouse(int buildingId, int resource);
int Resource_removeFromWarehouse(int buildingId, int resource, int amountInLoads);
void Resource_removeFromWarehouseForMercury(int buildingId, int amountInLoads);

void Resource_addImportedResourceToWarehouseSpace(building *space, int resourceId);
void Resource_removeExportedResourceFromWarehouseSpace(building *space, int resourceId);

int Resource_getAmountStoredInWarehouse(int buildingId, int resource);

int Resource_getWarehouseSpaceInfo(int buildingId);

int Resource_determineWarehouseWorkerTask(int buildingId, int *resource);

// granary functions
void Resource_gatherGranaryGettingInfo();

int Resource_getGranaryForStoringFood(
	int forceOnStockpile, int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *understaffed, int *xDst, int *yDst);
int Resource_getGettingGranaryForStoringFood(
	int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *xDst, int *yDst);
int Resource_getGranaryForGettingFood(int srcBuildingId, int *xDst, int *yDst);
int Resource_getAmountStoredInGranary(building *granary, int resource);
int Resource_addToGranary(int buildingId, int resource, int countAsProduced);
int Resource_removeFromGranary(int buildingId, int resource, int amountInUnits);
int Resource_determineGranaryWorkerTask(building *granary);
int Resource_takeFoodFromGranaryForGettingDeliveryman(int dstBuildingId, int srcBuildingId, int *resource);

// other functions
int Resource_getWorkshopForRawMaterial(int x, int y, int resource, int distanceFromEntry, int roadNetworkId, int *xDst, int *yDst);
int Resource_getBarracksForWeapon(int xUnused, int yUnused, int resource, int roadNetworkId, int *xDst, int *yDst);

void Resource_addWeaponToBarracks(int buildingId);

enum {
	StorageFigureTask_Getting = 0,
	StorageFigureTask_Delivering = 1,
};

#endif
