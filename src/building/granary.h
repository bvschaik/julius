#ifndef BUILDING_GRANARY_H
#define BUILDING_GRANARY_H

#include "building/building.h"

enum {
    GRANARY_TASK_NONE = -1,
    GRANARY_TASK_GETTING = 0
};

int building_granary_get_amount(building *granary, int resource);

int building_granary_add_resource(building *granary, int resource, int is_produced);

int building_granary_remove_resource(building *granary, int resource, int amount);

int building_granary_remove_for_getting_deliveryman(building *src, building *dst, int *resource);

int building_granary_determine_worker_task(building *granary);

void building_granaries_calculate_stocks();

int building_granary_for_storing(int x, int y, int resource, int distance_from_entry, int road_network_id,
                                 int force_on_stockpile, int *understaffed, int *x_dst, int *y_dst);

int building_getting_granary_for_storing(int x, int y, int resource, int distance_from_entry, int road_network_id,
                                         int *x_dst, int *y_dst);

int building_granary_for_getting(building *src, int *x_dst, int *y_dst);

void building_granary_bless();

void building_granary_warehouse_curse(int big);

#endif // BUILDING_GRANARY_H
