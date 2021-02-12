#ifndef BUILDING_MARKET_H
#define BUILDING_MARKET_H

#include "building/building.h"
#include "game/resource.h"

int building_market_get_max_food_stock(building *market);
int building_market_get_max_goods_stock(building *market);

int building_market_is_good_accepted(inventory_type resource, building *market);
void building_market_toggle_good_accepted(inventory_type resource, building *market);
void building_market_unaccept_all_goods(building *market);

void building_market_update_demands(building *market);
int building_market_get_needed_inventory(building *market);
int building_market_get_storage_destination(building *market, int needed_inventory);

// TODO put these where they belong, which is not here
int building_venus_temple_get_wine_destination(building* temple, building* grand_temple);
int building_mars_temple_food_to_deliver(building* b, int mess_hall_id);

#endif // BUILDING_MARKET_H
