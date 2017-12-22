#ifndef BUILDING_WAREHOUSE_H
#define BUILDING_WAREHOUSE_H

#include "building/building.h"

void building_warehouse_space_set_image(building *space, int resource);

int building_warehouse_add_resource(building *b, int resource);

int building_warehouse_remove_resource(building *warehouse, int resource, int amount);

void building_warehouses_add_resource(int resource, int amount);

int building_warehouses_remove_resource(int resource, int amount);

void building_warehouses_calculate_stocks();


#endif // BUILDING_WAREHOUSE_H
