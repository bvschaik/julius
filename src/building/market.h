#ifndef BUILDING_MARKET_H
#define BUILDING_MARKET_H

#include "building/distribution.h"

int building_market_get_max_food_stock(building *market);
int building_market_get_max_goods_stock(building *market);
int building_venus_temple_get_wine_destination(building* temple, building* grand_temple);
int building_market_remove_resource(int building_id, int resource, int amount);
int building_market_get_destination_for_looting(int x, int y, int* x_tile, int* y_tile, int* resource);
int building_market_get_storage_destination(building *market);
int building_mars_temple_food_to_deliver(building* b, int mess_hall_id);
int is_good_accepted(inventory_type resource, building *market);
void toggle_good_accepted(inventory_type resource, building *market);
void unaccept_all_goods(building *market);
int building_market_get_needed_inventory(building *market);
int building_market_fetch_inventory(building *market, inventory_storage_info *data, int needed_inventory);
int building_market_get_storage_destination(building *market);

#endif // BUILDING_MARKET_H
