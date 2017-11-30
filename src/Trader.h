#ifndef TRADER_H
#define TRADER_H

#include "figure/figure.h"

void Trader_tick();

int Trader_getClosestWarehouseForTradeCaravan(const figure *f, int x, int y, int cityId, int distanceFromEntry, int *warehouseX, int *warehouseY);
int Trader_getClosestWarehouseForImportDocker(int x, int y, int cityId, int distanceFromEntry, int roadNetworkId, int *warehouseX, int *warehouseY);
int Trader_getClosestWarehouseForExportDocker(int x, int y, int cityId, int distanceFromEntry, int roadNetworkId, int *warehouseX, int *warehouseY);

int Trader_tryImportResource(int buildingId, int resourceId, int cityId);
int Trader_tryExportResource(int buildingId, int resourceId, int cityId);

#endif
