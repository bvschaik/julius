#ifndef BUILDING_H
#define BUILDING_H

void Building_clearList();
int Building_create(int type, int x, int y);
void Building_delete(int buildingId);
void Building_deleteData(int buildingId);

void BuildingStorage_clearList();
int BuildingStorage_create();
void BuildingStorage_resetBuildingIds();


int Building_Market_getMaxFoodStock(int buildingId);
int Building_Market_getMaxGoodsStock(int buildingId);

#endif
