#ifndef BUILDING_DISTRIBUTION_H
#define BUILDING_DISTRIBUTION_H

#include "building/building.h"
#include "game/resource.h"

typedef struct {
    int building_id;
    int min_distance;
} inventory_data;

int building_distribution_pick_inventory_to_fetch(
    const building *b, inventory_data *data, int min_stock, int pick_first_found, int inventory_flags);

inventory_data *building_distribution_get_inventory_data(building *b, int needed_inventory, int max_distance);

#endif // BUILDING_DISTRIBUTION_H
