#ifndef BUILDING_ANIMATION_H
#define BUILDING_ANIMATION_H

#include "building/building.h"

int building_animation_offset(building *b, int image_id, int grid_offset);

int building_animation_advance_warehouse_flag(building *b, int image_id);

int building_animation_advance_fumigation(building *b);

#endif // BUILDING_ANIMATION_H
