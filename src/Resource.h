#ifndef RESOURCE_H
#define RESOURCE_H

void Resource_calculateWarehouseStocks();
void Resource_calculateWorkshopStocks();

void Resource_addToCityWarehouses(int resource, int amount);
int Resource_removeFromCityWarehouses(int resource, int amount);

int Resource_addToWarehouse(int buildingId, int resource);
int Resource_removeFromWarehouse(int buildingId, int resource, int amount);

int Resource_getAmountStoredInWarehouse(int buildingId, int resource);

int Resource_getWarehouseSpaceInfo(int buildingId);

int Resource_getGraphicIdOffset(int resource, int type);

#endif
