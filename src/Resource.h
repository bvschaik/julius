#ifndef RESOURCE_H
#define RESOURCE_H

#include "building/building.h"

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

#endif
