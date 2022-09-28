#ifndef BUILDING_GRANARY_H
#define BUILDING_GRANARY_H

#include "building/building.h"
#include "map/point.h"

// make sure to update src/window/building/distribution.c so the number renders correctly
#define FULL_GRANARY 3200
#define THREEQUARTERS_GRANARY 2400
#define HALF_GRANARY 1600
#define QUARTER_GRANARY 800

enum {
    GRANARY_TASK_NONE = -1,
    GRANARY_TASK_GETTING = 0
};

int building_granary_add_import(building *granary, int resource, int land_trader);

int building_granary_remove_export(building *granary, int resource, int land_trader);

int building_granary_add_resource(building *granary, int resource, int is_produced);

int building_granary_remove_resource(building *granary, int resource, int amount);

int building_granaries_remove_resource(int resource, int amount);

int building_granaries_send_resources_to_rome(int resource, int amount);

int building_granary_remove_for_getting_deliveryman(building *src, building *dst, int *resource);

int building_granary_is_not_accepting(int resource, building* b);

int building_granary_is_full(int resource, building *b);

int building_granary_resource_amount(int resource, building *b);

int building_granary_determine_worker_task(building *granary);

void building_granaries_calculate_stocks(void);

int building_granary_accepts_storage(building *b, int resource, int *understaffed);

int building_granary_for_storing(int x, int y, int resource, int road_network_id,
                                 int force_on_stockpile, int *understaffed, map_point *dst);

int building_getting_granary_for_storing(int x, int y, int resource, int road_network_id, map_point *dst);

int building_granary_amount_can_get_from(building *destination, building *origin);

int building_granary_for_getting(building *src, map_point *dst, int min_amount);

void building_granary_bless(void);

void building_granary_warehouse_curse(int big);

int building_granary_is_getting(int resource, building* b);

void building_granary_update_built_granaries_capacity(void);

#endif // BUILDING_GRANARY_H
