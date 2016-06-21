#ifndef TRADER_H
#define TRADER_H

void Trader_clearList();
void Trader_create(int figureId);

void Trader_tick();

void Trader_sellResource(int figureId, int resourceId);
void Trader_buyResource(int figureId, int resourceId);

int Trader_getClosestWarehouseForTradeCaravan(int figureId, int x, int y, int cityId, int distanceFromEntry, int *warehouseX, int *warehouseY);
int Trader_getClosestWarehouseForImportDocker(int x, int y, int cityId, int distanceFromEntry, int roadNetworkId, int *warehouseX, int *warehouseY);
int Trader_getClosestWarehouseForExportDocker(int x, int y, int cityId, int distanceFromEntry, int roadNetworkId, int *warehouseX, int *warehouseY);

int Trader_tryImportResource(int buildingId, int resourceId, int cityId);
int Trader_tryExportResource(int buildingId, int resourceId, int cityId);

#endif
