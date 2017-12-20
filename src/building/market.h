#ifndef BUILDING_MARKET_H
#define BUILDING_MARKET_H

#include "building/building.h"

int building_market_get_max_food_stock(building *market);
int building_market_get_max_goods_stock(building *market);
int building_market_get_storage_destination(building *market);

#endif // BUILDING_MARKET_H
