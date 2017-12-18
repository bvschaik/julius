#ifndef BUILDING_H
#define BUILDING_H

#include "building/building.h"

void Building_updateHighestIds();

void Building_clearList();
int Building_create(int type, int x, int y);
void Building_delete(int buildingId);
void Building_deleteData(int buildingId);

void Building_GameTick_updateState();

int Building_getMainBuildingId(int buildingId);

int Building_collapseFirstOfType(int buildingType);
void Building_collapseLastPlaced();
void Building_collapseOnFire(int buildingId, int hasPlague);
void Building_collapseLinked(int buildingId, int onFire);

void Building_increaseDamageByEnemy(int gridOffset, int maxDamage);
void Building_destroyByEnemy(int x, int y, int gridOffset);

void Building_setDesirability();
void Building_decayHousesCovered();
void Building_decayTaxCollectorAccess();
void Building_determineGraphicIdsForOrientedBuildings();

void Building_GameTick_checkAccessToRome();

int Building_Market_getMaxFoodStock(int buildingId);
int Building_Market_getMaxGoodsStock(int buildingId);
int Building_Market_getDestinationGranaryWarehouse(building *market);

int Building_Dock_getNumIdleDockers(building *dock);
void Building_Dock_updateOpenWaterAccess();
int Building_Dock_isConnectedToOpenWater(int x, int y);

void Building_Mercury_removeResources(int bigCurse);
void Building_Mercury_fillGranary();

#endif
