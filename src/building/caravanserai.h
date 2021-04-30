#ifndef BUILDING_CARANVASERAI_H
#define BUILDING_CARANVASERAI_H

#include "building/building.h"

#define MAX_FOOD 1200

int building_caravanserai_enough_foods(building *caravanserai);
int building_caravanserai_get_storage_destination(building *caravanserai);
int building_caravanserai_is_fully_functional(void);

#endif // BUILDING_CARANVASERAI_H
