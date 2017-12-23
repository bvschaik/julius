#ifndef BUILDING_GRANARY_H
#define BUILDING_GRANARY_H

#include "building/building.h"

enum {
    GRANARY_TASK_NONE = -1,
    GRANARY_TASK_GETTING = 0
};

int building_granary_get_amount(building *granary, int resource);

int building_granary_add_resource(building *granary, int resource, int count_as_produced);

int building_granary_remove_resource(building *granary, int resource, int amount);

int building_granary_determine_worker_task(building *granary);

void building_granaries_calculate_stocks();

int building_granary_for_storing(int src_building_id, int x, int y, int resource,
                                   int distance_from_entry, int road_network_id, int *understaffed,
                                   int *x_dst, int *y_dst);

int building_granary_for_getting(building *src, int *x_dst, int *y_dst);


#endif // BUILDING_GRANARY_H
