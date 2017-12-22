#ifndef BUILDING_WAREHOUSE_H
#define BUILDING_WAREHOUSE_H

#include "building/building.h"

enum {
    WAREHOUSE_ROOM = 0,
    WAREHOUSE_FULL = 1,
    WAREHOUSE_SOME_ROOM = 2
};

enum {
    WAREHOUSE_TASK_NONE = -1,
    WAREHOUSE_TASK_GETTING = 0,
    WAREHOUSE_TASK_DELIVERING = 1
};

int building_warehouse_get_space_info(building *warehouse);

int building_warehouse_get_amount(building *warehouse, int resource);

int building_warehouse_add_resource(building *b, int resource);

int building_warehouse_remove_resource(building *warehouse, int resource, int amount);

void building_warehouse_remove_resource_curse(building *warehouse, int amount);

void building_warehouse_space_set_image(building *space, int resource);

void building_warehouse_space_add_import(building *space, int resource);

void building_warehouse_space_remove_export(building *space, int resource);

void building_warehouses_add_resource(int resource, int amount);

int building_warehouses_remove_resource(int resource, int amount);

void building_warehouses_calculate_stocks();

int building_warehouse_for_storing(int src_building_id, int x, int y, int resource,
                                   int distance_from_entry, int road_network_id, int *understaffed,
                                   int *x_dst, int *y_dst);

int building_warehouse_for_getting(building *src, int resource, int *x_dst, int *y_dst);

int building_warehouse_determine_warehouseman_task(building *warehouse, int *resource);

#endif // BUILDING_WAREHOUSE_H
