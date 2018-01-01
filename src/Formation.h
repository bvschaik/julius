#ifndef FORMATION_H
#define FORMATION_H

#include "building/building.h"

void Formation_updateAfterDeath(int formationId);

void Formation_calculateFigures();

void Formation_Tick_updateAll(int secondTime);

#endif
