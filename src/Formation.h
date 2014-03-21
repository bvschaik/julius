#ifndef FORMATION_H
#define FORMATION_H

void Formation_clearList();

void Formation_deleteFortAndBanner(int formationId);

void Formation_setMaxSoldierPerLegion();

int Formation_getNumLegions();

int Formation_getLegionFormationId(int legionId);

void Formation_legionReturnHome(int formationId);

#endif
