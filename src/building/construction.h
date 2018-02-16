#ifndef BUILDING_CONSTRUCTION_H
#define BUILDING_CONSTRUCTION_H

#include "building/type.h"

void building_construction_reset(building_type type);

void building_construction_clear_type();

building_type building_construction_type();

int building_construction_cost();

int building_construction_in_progress();

void building_construction_start(int x, int y);

void building_construction_update(int x, int y);

void building_construction_place();

#endif // BUILDING_CONSTRUCTION_H
