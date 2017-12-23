#ifndef BUILDING_H
#define BUILDING_H

#include "building/building.h"

void Building_updateHighestIds();

void Building_clearList();

void Building_GameTick_updateState();

int Building_collapseFirstOfType(int buildingType);
void Building_collapseLastPlaced();
void Building_collapseOnFire(int buildingId, int hasPlague);
void Building_collapseLinked(int buildingId, int onFire);

void Building_increaseDamageByEnemy(int gridOffset, int maxDamage);
void Building_destroyByEnemy(int x, int y, int gridOffset);

void Building_setDesirability();
void Building_decayHousesCovered();
void Building_decayTaxCollectorAccess();
void Building_determineGraphicIdsForOrientedBuildings();

void Building_GameTick_checkAccessToRome();

#endif
