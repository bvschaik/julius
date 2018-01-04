#ifndef BUILDING_H
#define BUILDING_H

#include "building/building.h"

void Building_GameTick_updateState();

void Building_setDesirability();
void Building_decayHousesCovered();

void Building_GameTick_checkAccessToRome();

#endif
