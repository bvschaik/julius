#ifndef BUILDING_LIGHTHOUSE_H
#define BUILDING_LIGHTHOUSE_H

#include "building/building.h"

int building_lighthouse_enough_timber(building *lighthouse);
int building_lighthouse_get_storage_destination(building *lighthouse);
int building_lighthouse_is_fully_functional(void);
void building_lighthouse_consume_timber(void);

#endif //BUILDING_LIGHTHOUSE_H
