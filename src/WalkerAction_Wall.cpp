#include "FigureAction_private.h"

#include "Figure.h"
#include "Routing.h"
#include "Sound.h"
#include "Terrain.h"
#include "Util.h"

#include "Data/Formation.h"
#include "Data/Grid.h"

static int ballistaFiringOffsets[] = {
	0, 1, 2, 3, 4, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static int towerSentryFiringOffsets[] = {
	0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void FigureAction_ballista(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[f->buildingId];
	f->terrainUsage = FigureTerrainUsage_Walls;
	f->useCrossCountry = 0;
	f->isGhost = 1;
	f->heightAdjustedTicks = 10;
	f->currentHeight = 45;
	
	if (!BuildingIsInUse(f->buildingId) || b->walkerId4 != walkerId) {
		f->state = FigureState_Dead;
	}
	if (b->numWorkers <= 0 || b->walkerId <= 0) {
		f->state = FigureState_Dead;
	}
	Figure_removeFromTileList(walkerId);
	switch (Data_Settings_Map.orientation) {
		case Dir_0_Top: f->x = b->x; f->y = b->y; break;
		case Dir_2_Right: f->x = b->x + 1; f->y = b->y; break;
		case Dir_4_Bottom: f->x = b->x + 1; f->y = b->y + 1; break;
		case Dir_6_Left: f->x = b->x; f->y = b->y + 1; break;
	}
	f->gridOffset = GridOffset(f->x, f->y);
	Figure_addToTileList(walkerId);

	switch (f->actionState) {
		case FigureActionState_149_Corpse:
			f->state = FigureState_Dead;
			break;
		case FigureActionState_180_BallistaCreated:
			f->waitTicks++;
			if (f->waitTicks > 20) {
				f->waitTicks = 0;
				int xTile, yTile;
				if (FigureAction_CombatSoldier_getMissileTarget(walkerId, 15, &xTile, &yTile)) {
					f->actionState = FigureActionState_181_BallistaFiring;
					f->waitTicksMissile = Constant_FigureProperties[f->type].missileFrequency;
				}
			}
			break;
		case FigureActionState_181_BallistaFiring:
			f->waitTicksMissile++;
			if (f->waitTicksMissile > Constant_FigureProperties[f->type].missileFrequency) {
				int xTile, yTile;
				if (FigureAction_CombatSoldier_getMissileTarget(walkerId, 15, &xTile, &yTile)) {
					f->direction = Routing_getDirectionForMissileShooter(f->x, f->y, xTile, yTile);
					f->waitTicksMissile = 0;
					Figure_createMissile(walkerId, f->x, f->y, xTile, yTile, Figure_Bolt);
					Sound_Effects_playChannel(SoundChannel_BallistaShoot);
				} else {
					f->actionState = FigureActionState_180_BallistaCreated;
				}
			}
			break;
	}
	int dir = WalkerActionDirection(f);
	if (f->actionState == FigureActionState_181_BallistaFiring) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Ballista) + dir +
			8 * ballistaFiringOffsets[f->waitTicksMissile / 4];
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_Ballista) + dir;
	}
}

static void towerSentryPickTarget(int walkerId, struct Data_Walker *f)
{
	if (Data_Formation_Extra.numEnemyFormations <= 0) {
		return;
	}
	if (f->actionState == FigureActionState_150_Attack ||
		f->actionState == FigureActionState_149_Corpse) {
		return;
	}
	if (f->inBuildingWaitTicks) {
		return;
	}
	f->waitTicksNextTarget++;
	if (f->waitTicksNextTarget >= 40) {
		f->waitTicksNextTarget = 0;
		int xTile, yTile;
		if (FigureAction_CombatSoldier_getMissileTarget(walkerId, 10, &xTile, &yTile)) {
			f->actionState = FigureActionState_172_TowerSentryFiring;
			f->destinationX = f->x;
			f->destinationY = f->y;
		}
	}
}

static int towerSentryInitPatrol(struct Data_Building *b, int *xTile, int *yTile)
{
	int dir = b->walkerRoamDirection;
	int x = b->x;
	int y = b->y;
	switch (dir) {
		case Dir_0_Top: y -= 8; break;
		case Dir_2_Right: x += 8; break;
		case Dir_4_Bottom: y += 8; break;
		case Dir_6_Left: x -= 8; break;
	}
	BoundToMap(x, y);

	if (Terrain_getWallTileWithinRadius(x, y, 6, xTile, yTile)) {
		b->walkerRoamDirection += 2;
		if (b->walkerRoamDirection > 6) b->walkerRoamDirection = 0;
		return 1;
	}
	for (int i = 0; i < 4; i++) {
		dir = b->walkerRoamDirection;
		b->walkerRoamDirection += 2;
		if (b->walkerRoamDirection > 6) b->walkerRoamDirection = 0;
		x = b->x;
		y = b->y;
		switch (dir) {
			case Dir_0_Top: y -= 3; break;
			case Dir_2_Right: x += 3; break;
			case Dir_4_Bottom: y += 3; break;
			case Dir_6_Left: x -= 3; break;
		}
		BoundToMap(x, y);
		if (Terrain_getWallTileWithinRadius(x, y, 6, xTile, yTile)) {
			return 1;
		}
	}
	return 0;
}

void FigureAction_towerSentry(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[f->buildingId];
	f->terrainUsage = FigureTerrainUsage_Walls;
	f->useCrossCountry = 0;
	f->isGhost = 1;
	f->heightAdjustedTicks = 10;
	f->maxRoamLength = 800;
	if (!BuildingIsInUse(f->buildingId) || b->walkerId != walkerId) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	
	towerSentryPickTarget(walkerId, f);
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_170_TowerSentryAtRest:
			f->graphicOffset = 0;
			f->waitTicks++;
			if (f->waitTicks > 40) {
				f->waitTicks = 0;
				int xTile, yTile;
				if (towerSentryInitPatrol(b, &xTile, &yTile)) {
					f->actionState = FigureActionState_171_TowerSentryPatrolling;
					f->destinationX = xTile;
					f->destinationY = yTile;
					FigureRoute_remove(walkerId);
				}
			}
			break;
		case FigureActionState_171_TowerSentryPatrolling:
			FigureMovement_walkTicks(walkerId, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_173_TowerSentryReturning;
				f->destinationX = f->sourceX;
				f->destinationY = f->sourceY;
				FigureRoute_remove(walkerId);
			} else if (f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost) {
				f->actionState = FigureActionState_170_TowerSentryAtRest;
			}
			break;
		case FigureActionState_172_TowerSentryFiring:
			FigureMovement_walkTicksTowerSentry(walkerId, 1);
			f->waitTicksMissile++;
			if (f->waitTicksMissile > Constant_FigureProperties[f->type].missileFrequency) {
				int xTile, yTile;
				if (FigureAction_CombatSoldier_getMissileTarget(walkerId, 10, &xTile, &yTile)) {
					f->direction = Routing_getDirectionForMissileShooter(f->x, f->y, xTile, yTile);
					f->waitTicksMissile = 0;
					Figure_createMissile(walkerId, f->x, f->y, xTile, yTile, Figure_Javelin);
				} else {
					f->actionState = FigureActionState_173_TowerSentryReturning;
					f->destinationX = f->sourceX;
					f->destinationY = f->sourceY;
					FigureRoute_remove(walkerId);
				}
			}
			break;
		case FigureActionState_173_TowerSentryReturning:
			FigureMovement_walkTicks(walkerId, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_170_TowerSentryAtRest;
			} else if (f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_174_TowerSentryGoingToTower:
			f->terrainUsage = FigureTerrainUsage_Roads;
			f->isGhost = 0;
			f->heightAdjustedTicks = 0;
			FigureMovement_walkTicks(walkerId, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				Figure_removeFromTileList(walkerId);
				f->sourceX = f->x = b->x;
				f->sourceY = f->y = b->y;
				f->gridOffset = GridOffset(f->x, f->y);
				Figure_addToTileList(walkerId);
				f->actionState = FigureActionState_170_TowerSentryAtRest;
				FigureRoute_remove(walkerId);
			} else if (f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			}
			break;
	}
	if (Data_Grid_terrain[f->gridOffset] & Terrain_Wall) {
		f->currentHeight = 18;
	} else if (Data_Grid_terrain[f->gridOffset] & Terrain_Gatehouse) {
		f->inBuildingWaitTicks = 24;
	} else if (f->actionState != FigureActionState_174_TowerSentryGoingToTower) {
		f->state = FigureState_Dead;
	}
	if (f->inBuildingWaitTicks) {
		f->inBuildingWaitTicks--;
		f->heightAdjustedTicks = 0;
	}
	int dir = WalkerActionDirection(f);
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = GraphicId(ID_Graphic_Figure_TowerSentry) +
			136 + WalkerActionCorpseGraphicOffset(f);
	} else if (f->actionState == FigureActionState_172_TowerSentryFiring) {
		f->graphicId = GraphicId(ID_Graphic_Figure_TowerSentry) +
			dir + 96 + 8 * towerSentryFiringOffsets[f->waitTicksMissile / 2];
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_TowerSentry) +
			dir + 8 * f->graphicOffset;
	}
}

void FigureAction_TowerSentry_reroute()
{
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Walker *f = &Data_Walkers[i];
		if (f->type != Figure_TowerSentry || Data_Grid_routingWalls[f->gridOffset] == 0) {
			continue;
		}
		// tower sentry got off wall due to rotation
		int xTile, yTile;
		if (Terrain_getWallTileWithinRadius(f->x, f->y, 2, &xTile, &yTile)) {
			FigureRoute_remove(i);
			f->progressOnTile = 0;
			Figure_removeFromTileList(i);
			f->previousTileX = f->x = xTile;
			f->previousTileY = f->y = yTile;
			f->crossCountryX = 15 * xTile;
			f->crossCountryY = 15 * yTile;
			f->gridOffset = GridOffset(xTile, yTile);
			Figure_addToTileList(i);
			f->actionState = FigureActionState_173_TowerSentryReturning;
			f->destinationX = f->sourceX;
			f->destinationY = f->sourceY;
		} else {
			// Teleport back to tower
			Figure_removeFromTileList(i);
			struct Data_Building *b = &Data_Buildings[f->buildingId];
			f->sourceX = f->x = b->x;
			f->sourceY = f->y = b->y;
			f->gridOffset = GridOffset(f->x, f->y);
			Figure_addToTileList(i);
			f->actionState = FigureActionState_170_TowerSentryAtRest;
			FigureRoute_remove(i);
		}
	}
}
