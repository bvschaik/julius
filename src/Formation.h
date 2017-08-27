#ifndef FORMATION_H
#define FORMATION_H

void Formation_clearInvasionInfo();

int Formation_createLegion(int buildingId);

void Formation_deleteFortAndBanner(int formationId);

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

void Formation_calculateFigures();

void Formation_dispatchLegionsToDistantBattle();
void Formation_legionsReturnFromDistantBattle();
void Formation_legionKillSoldiersInDistantBattle(int killPercentage);

int Formation_marsCurseFort();

int Formation_Rioter_getTargetBuilding(int *xTile, int *yTile);

void Formation_Tick_updateRestMorale();
void Formation_Tick_updateAll(int secondTime);

#endif
