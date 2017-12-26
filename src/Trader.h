#ifndef TRADER_H
#define TRADER_H

#include "figure/figure.h"

void Trader_tick();

int Trader_getClosestWarehouseForTradeCaravan(const figure *f, int x, int y, int cityId, int distanceFromEntry, int *warehouseX, int *warehouseY);

#endif
