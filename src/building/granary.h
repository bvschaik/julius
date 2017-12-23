#ifndef BUILDING_GRANARY_H
#define BUILDING_GRANARY_H

#include "building/building.h"

int building_granary_get_amount(building *granary, int resource);

int building_granary_add_resource(building *granary, int resource, int count_as_produced);

int building_granary_remove_resource(building *granary, int resource, int amount);

#endif // BUILDING_GRANARY_H
