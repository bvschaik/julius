#ifndef FORMATION_H
#define FORMATION_H

#include "building/building.h"

void Formation_calculateLegionTotals();
int Formation_getClosestMilitaryAcademy(int formationId);

void Formation_updateAfterDeath(int formationId);

int Formation_getInvasionGridOffset(int invasionSeq);
int Formation_getLegionFormationAtGridOffset(int gridOffset);
int Formation_getFormationForBuilding(int gridOffset);

void Formation_calculateFigures();

int Formation_marsCurseFort();

int Formation_Rioter_getTargetBuilding(int *xTile, int *yTile);

void Formation_Tick_updateAll(int secondTime);

#endif
