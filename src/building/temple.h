#ifndef BUILDING_TEMPLE_H
#define BUILDING_TEMPLE_H

#include "building/building.h"

int building_temple_get_storage_destination(building *temple);
int building_temple_mars_food_to_deliver(building *temple, int mess_hall_id);

#endif // BUILDING_TEMPLE_H
