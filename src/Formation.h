#ifndef FORMATION_H
#define FORMATION_H

#include "building/building.h"

void Formation_calculateLegionTotals();
int Formation_getClosestMilitaryAcademy(int formationId);

void Formation_updateAfterDeath(int formationId);

int Formation_getLegionFormationAtGridOffset(int gridOffset);
int Formation_getFormationForBuilding(int gridOffset);

void Formation_calculateFigures();

int Formation_marsCurseFort();

void Formation_Tick_updateAll(int secondTime);

#endif
