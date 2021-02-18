#ifndef BUILDING_DISTRIBUTION_H
#define BUILDING_DISTRIBUTION_H

#include "building/building.h"
#include "game/resource.h"

typedef struct {
    int building_id;
    int min_distance;
} inventory_storage_info;

#define BASELINE_STOCK 50

int building_distribution_fetch(const building *b, inventory_storage_info *info,
    int min_stock, int pick_first, int allowed);
int building_distribution_get_inventory_storages(inventory_storage_info *info, building_type type,
    int road_network, int x, int y, int max_distance);

#endif // BUILDING_DISTRIBUTION_H
