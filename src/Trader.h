#ifndef TRADER_H
#define TRADER_H

void Trader_clearList();
void Trader_create(int walkerId);

void Trader_tick();

void Trader_sellResource(int walkerId, int resourceId);
void Trader_buyResource(int walkerId, int resourceId);

int Trader_getClosestWarehouseForTradeCaravan(int walkerId, int x, int y, int cityId, int building1a, int roadNetworkId, int *warehouseX, int *warehouseY);
int Trader_getClosestWarehouseForImportDocker(int x, int y, int cityId, int building1a, int roadNetworkId, int *warehouseX, int *warehouseY);
int Trader_getClosestWarehouseForExportDocker(int x, int y, int cityId, int building1a, int roadNetworkId, int *warehouseX, int *warehouseY);

int Trader_tryImportResource(int buildingId, int resourceId, int cityId);
int Trader_tryExportResource(int buildingId, int resourceId, int cityId);

#endif
