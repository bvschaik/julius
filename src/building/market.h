#ifndef BUILDING_MARKET_H
#define BUILDING_MARKET_H

#include "building/distribution.h"

#define MARKET_MAX_DISTANCE 40

int building_market_get_max_food_stock(building* market);
int building_market_get_max_goods_stock(building* market);

int building_market_get_needed_inventory(building* market);
int building_market_fetch_inventory(building* market, inventory_storage_info* data, int needed_inventory);
int building_market_get_storage_destination(building* market);


#endif // BUILDING_MARKET_H