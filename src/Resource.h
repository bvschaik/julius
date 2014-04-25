#ifndef RESOURCE_H
#define RESOURCE_H

void Resource_calculateWarehouseStocks();
void Resource_calculateWorkshopStocks();

void Resource_addToCityWarehouses(int resource, int amount);
int Resource_removeFromCityWarehouses(int resource, int amount);

int Resource_addToWarehouse(int buildingId, int resource);
int Resource_removeFromWarehouse(int buildingId, int resource, int amount);

void Resource_addImportedResourceToWarehouseSpace(int spaceBuildingId, int resourceId);
void Resource_removeExportedResourceFromWarehouseSpace(int spaceBuildingId, int resourceId);

int Resource_getDistance(int x1, int y1, int x2, int y2, int distToEntry1, int distToEntry2);

int Resource_getAmountStoredInWarehouse(int buildingId, int resource);

int Resource_getWarehouseSpaceInfo(int buildingId);

int Resource_getGraphicIdOffset(int resource, int type);

#endif
