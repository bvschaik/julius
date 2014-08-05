#ifndef BUILDING_H
#define BUILDING_H

void Building_updateHighestIds();

void Building_clearList();
int Building_create(int type, int x, int y);
void Building_delete(int buildingId);
void Building_deleteData(int buildingId);

void Building_clearDeleted();

int Building_getMainBuildingId(int buildingId);

int Building_collapseFirstOfType(int buildingType);
void Building_collapseLastPlaced();
void Building_collapseOnFire(int buildingId, int hasPlague);
void Building_collapseLinked(int buildingId, int onFire);

void Building_increaseDamageByEnemy(int gridOffset, int maxDamage);
void Building_destroyByEnemy(int x, int y, int gridOffset);

void Building_setDesirability();
void Building_decayHousesCovered();
void Building_determineGraphicIdsForOrientedBuildings();

void BuildingStorage_clearList();
int BuildingStorage_create();
void BuildingStorage_resetBuildingIds();

void Building_GameTick_checkAccessToRome();

void Building_Industry_updateProduction();
void Building_Industry_updateDoubleWheatProduction();
void Building_Industry_blessFarmsFromCeres();
void Building_Industry_witherFarmCropsFromCeres(int bigCurse);
int Building_Industry_hasProducedResource(int buildingId);
void Building_Industry_startNewProduction(int buildingId);

int Building_Market_getMaxFoodStock(int buildingId);
int Building_Market_getMaxGoodsStock(int buildingId);
int Building_Market_getDestinationGranaryWarehouse(int marketId);

int Building_Dock_getNumIdleDockers(int buildingId);
void Building_Dock_updateOpenWaterAccess();

void Building_Mercury_removeResources(int bigCurse);
void Building_Mercury_fillGranary();

#endif
