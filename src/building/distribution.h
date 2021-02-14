#ifndef BUILDING_DISTRIBUTION_H
#define BUILDING_DISTRIBUTION_H

#include "building/building.h"
#include "game/resource.h"

typedef struct {
    int building_id;
    int min_distance;
} inventory_data;

#define BASELINE_STOCK 50

int building_distribution_fetch(const building *b, inventory_data *data, int min_stock, int pick_first, int allowed);
int building_distribution_get_inventory_data(inventory_data *data, building *b, int max_distance);

#endif // BUILDING_DISTRIBUTION_H
