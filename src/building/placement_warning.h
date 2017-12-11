#ifndef BUILDING_PLACEMENT_WARNING_H
#define BUILDING_PLACEMENT_WARNING_H

#include "building/type.h"

void building_placement_warning_reset();
void building_placement_warning_check_food_stocks(building_type type);
void building_placement_warning_check_reservoir(building_type type);
void building_placement_warning_check_all(building_type type, int x, int y, int size);

#endif // BUILDING_PLACEMENT_WARNING_H
