#ifndef BUILDING_DESTRUCTION_H
#define BUILDING_DESTRUCTION_H

#include "building/building.h"

void building_destroy_collapse(building *b);

int building_destroy_first_of_type(building_type type);

#endif // BUILDING_DESTRUCTION_H
