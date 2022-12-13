#ifndef BUILDING_DISTRIBUTION_H
#define BUILDING_DISTRIBUTION_H

#include "building/building.h"
#include "figure/figure.h"
#include "game/resource.h"

typedef struct {
    int building_id;
    int min_distance;
} inventory_storage_info;

#define BASELINE_STOCK 50

int building_distribution_is_good_accepted(inventory_type resource, building *b);
void building_distribution_toggle_good_accepted(inventory_type resource, building *b);
void building_distribution_unaccept_all_goods(building *b);

void building_distribution_update_demands(building *b);

int building_distribution_fetch(const building *b, inventory_storage_info *info,
    int min_stock, int pick_first, int allowed);
int building_distribution_get_inventory_storages_for_building(inventory_storage_info *info, building *start, int max_distance);
int building_distribution_get_raw_material_storages_for_building(inventory_storage_info *info, building *start, int max_distance);
int building_distribution_get_inventory_storages_for_figure(inventory_storage_info *info, building_type type, int road_network, figure *start, int max_distance);
int building_distribution_get_raw_material_storages_for_figure(inventory_storage_info *info, building_type type, int road_network, figure *start, int max_distance);
#endif // BUILDING_DISTRIBUTION_H
