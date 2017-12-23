#ifndef BUILDING_CONSTRUCTION_H
#define BUILDING_CONSTRUCTION_H

#include "building/type.h"

void building_construction_update(int x_start, int y_start, int x_end, int y_end, building_type type);

void building_construction_place(int orientation, int x_start, int y_start, int x_end, int y_end, building_type type);

#endif // BUILDING_CONSTRUCTION_H
