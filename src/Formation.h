#ifndef FORMATION_H
#define FORMATION_H

void Formation_clearList();
void Formation_clearInvasionInfo();

int Formation_create(int walkerType, int formationLayout, int orientation, int x, int y);
int Formation_createLegion(int buildingId);

void Formation_deleteFortAndBanner(int formationId);

void Formation_changeMorale(int formationId, int amount);

void Formation_setMaxSoldierPerLegion();

int Formation_getNumLegions();

int Formation_getLegionFormationId(int legionId);

void Formation_legionReturnHome(int formationId);
void Formation_legionMoveTo(int formationId, int x, int y);

void Formation_calculateLegionTotals();
int Formation_anyLegionNeedsSoldiers();
int Formation_getClosestMilitaryAcademy(int formationId);
void Formation_setNewSoldierRequest(int buildingId);

void Formation_updateAfterDeath(int formationId);

int Formation_getInvasionGridOffset(int invasionSeq);
int Formation_getLegionFormationAtGridOffset(int gridOffset);
int Formation_getFormationForBuilding(int gridOffset);

void Formation_calculateWalkers();

void Formation_dispatchLegionsToDistantBattle();
void Formation_legionsReturnFromDistantBattle();
void Formation_legionKillSoldiersInDistantBattle(int killPercentage);

void Formation_moveHerdsAwayFrom(int x, int y);

int Formation_marsCurseFort();

int Formation_Rioter_getTargetBuilding(int *xTile, int *yTile);

void Formation_Tick_updateRestMorale();
void Formation_Tick_updateAll(int secondTime);

#endif
