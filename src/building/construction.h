#ifndef BUILDING_CONSTRUCTION_H
#define BUILDING_CONSTRUCTION_H

#include "building/type.h"

void building_construction_clear();

void building_construction_start(int x, int y);

void building_construction_update(int x, int y, building_type type);

void building_construction_place(int orientation, building_type type);

#endif // BUILDING_CONSTRUCTION_H
