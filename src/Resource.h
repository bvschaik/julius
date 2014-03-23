#ifndef RESOURCE_H
#define RESOURCE_H

void Resource_calculateWarehouseStocks();

void Resource_removeFromCityStorage(int resource, int amount);

int Resource_getGraphicIdOffset(int resource, int type);

#endif
