#ifndef FORMATION_H
#define FORMATION_H

void Formation_clearList();
void Formation_clearInvasionInfo();

void Formation_deleteFortAndBanner(int formationId);

void Formation_setMaxSoldierPerLegion();

int Formation_getNumLegions();

int Formation_getLegionFormationId(int legionId);

void Formation_legionReturnHome(int formationId);

void Formation_calculateLegionTotals();
int Formation_anyLegionNeedsSoldiers();
int Formation_getClosestMilitaryAcademy(int formationId);

void Formation_updateAfterDeath(int formationId);

int Formation_getInvasionGridOffset(int invasionSeq);

void Formation_calculateWalkers();

void Formation_dispatchLegionsToDistantBattle();
void Formation_legionsReturnFromDistantBattle();
void Formation_legionKillSoldiersInDistantBattle(int killPercentage);

#endif
