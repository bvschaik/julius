#ifndef BUILDING_DISTRIBUTION_H
#define BUILDING_DISTRIBUTION_H

#include "building/building.h"
#include "figure/figure.h"
#include "game/resource.h"

typedef struct {
    int needed;
    int building_id;
    int min_distance;
} resource_storage_info;

#define BASELINE_STOCK 50

int building_distribution_is_good_accepted(resource_type resource, const building *b);
void building_distribution_toggle_good_accepted(resource_type resource, building *b);
void building_distribution_unaccept_all_goods(building *b);

void building_distribution_update_demands(building *b);

int building_distribution_resource_is_handled(resource_type resource, building_type type);
int building_distribution_get_handled_resources_for_building(const building *b,
    resource_storage_info info[RESOURCE_MAX]);

resource_type building_distribution_fetch(const building *b, const resource_storage_info info[RESOURCE_MAX],
    int min_stock, int pick_first);
int building_distribution_get_resource_storages_for_building(resource_storage_info info[RESOURCE_MAX],
    building *start, int max_distance);
int building_distribution_get_resource_storages_for_figure(resource_storage_info info[RESOURCE_MAX],
    building_type type, int road_network, figure *start, int max_distance);

#endif // BUILDING_DISTRIBUTION_H
