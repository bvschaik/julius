#include "WalkerAction_private.h"

#include "Routing.h"
#include "Sound.h"
#include "Terrain.h"
#include "Util.h"
#include "Walker.h"

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

void WalkerAction_ballista(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	w->terrainUsage = FigureTerrainUsage_Walls;
	w->useCrossCountry = 0;
	w->isGhost = 1;
	w->heightAdjustedTicks = 10;
	w->currentHeight = 45;
	
	if (!BuildingIsInUse(w->buildingId) || b->walkerId4 != walkerId) {
		w->state = FigureState_Dead;
	}
	if (b->numWorkers <= 0 || b->walkerId <= 0) {
		w->state = FigureState_Dead;
	}
	Figure_removeFromTileList(walkerId);
	switch (Data_Settings_Map.orientation) {
		case Dir_0_Top: w->x = b->x; w->y = b->y; break;
		case Dir_2_Right: w->x = b->x + 1; w->y = b->y; break;
		case Dir_4_Bottom: w->x = b->x + 1; w->y = b->y + 1; break;
		case Dir_6_Left: w->x = b->x; w->y = b->y + 1; break;
	}
	w->gridOffset = GridOffset(w->x, w->y);
	Figure_addToTileList(walkerId);

	switch (w->actionState) {
		case FigureActionState_149_Corpse:
			w->state = FigureState_Dead;
			break;
		case FigureActionState_180_BallistaCreated:
			w->waitTicks++;
			if (w->waitTicks > 20) {
				w->waitTicks = 0;
				int xTile, yTile;
				if (WalkerAction_CombatSoldier_getMissileTarget(walkerId, 15, &xTile, &yTile)) {
					w->actionState = FigureActionState_181_BallistaFiring;
					w->waitTicksMissile = Constant_WalkerProperties[w->type].missileFrequency;
				}
			}
			break;
		case FigureActionState_181_BallistaFiring:
			w->waitTicksMissile++;
			if (w->waitTicksMissile > Constant_WalkerProperties[w->type].missileFrequency) {
				int xTile, yTile;
				if (WalkerAction_CombatSoldier_getMissileTarget(walkerId, 15, &xTile, &yTile)) {
					w->direction = Routing_getDirectionForMissileShooter(w->x, w->y, xTile, yTile);
					w->waitTicksMissile = 0;
					Figure_createMissile(walkerId, w->x, w->y, xTile, yTile, Figure_Bolt);
					Sound_Effects_playChannel(SoundChannel_BallistaShoot);
				} else {
					w->actionState = FigureActionState_180_BallistaCreated;
				}
			}
			break;
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == FigureActionState_181_BallistaFiring) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Ballista) + dir +
			8 * ballistaFiringOffsets[w->waitTicksMissile / 4];
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_Ballista) + dir;
	}
}

static void towerSentryPickTarget(int walkerId, struct Data_Walker *w)
{
	if (Data_Formation_Extra.numEnemyFormations <= 0) {
		return;
	}
	if (w->actionState == FigureActionState_150_Attack ||
		w->actionState == FigureActionState_149_Corpse) {
		return;
	}
	if (w->inBuildingWaitTicks) {
		return;
	}
	w->waitTicksNextTarget++;
	if (w->waitTicksNextTarget >= 40) {
		w->waitTicksNextTarget = 0;
		int xTile, yTile;
		if (WalkerAction_CombatSoldier_getMissileTarget(walkerId, 10, &xTile, &yTile)) {
			w->actionState = FigureActionState_172_TowerSentryFiring;
			w->destinationX = w->x;
			w->destinationY = w->y;
		}
	}
}

static int towerSentryInitPatrol(struct Data_Walker *w, struct Data_Building *b, int *xTile, int *yTile)
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

void WalkerAction_towerSentry(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	w->terrainUsage = FigureTerrainUsage_Walls;
	w->useCrossCountry = 0;
	w->isGhost = 1;
	w->heightAdjustedTicks = 10;
	w->maxRoamLength = 800;
	if (!BuildingIsInUse(w->buildingId) || b->walkerId != walkerId) {
		w->state = FigureState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	
	towerSentryPickTarget(walkerId, w);
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_170_TowerSentryAtRest:
			w->graphicOffset = 0;
			w->waitTicks++;
			if (w->waitTicks > 40) {
				w->waitTicks = 0;
				int xTile, yTile;
				if (towerSentryInitPatrol(w, b, &xTile, &yTile)) {
					w->actionState = FigureActionState_171_TowerSentryPatrolling;
					w->destinationX = xTile;
					w->destinationY = yTile;
					FigureRoute_remove(walkerId);
				}
			}
			break;
		case FigureActionState_171_TowerSentryPatrolling:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_173_TowerSentryReturning;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->actionState = FigureActionState_170_TowerSentryAtRest;
			}
			break;
		case FigureActionState_172_TowerSentryFiring:
			WalkerMovement_walkTicksTowerSentry(walkerId, 1);
			w->waitTicksMissile++;
			if (w->waitTicksMissile > Constant_WalkerProperties[w->type].missileFrequency) {
				int xTile, yTile;
				if (WalkerAction_CombatSoldier_getMissileTarget(walkerId, 10, &xTile, &yTile)) {
					w->direction = Routing_getDirectionForMissileShooter(w->x, w->y, xTile, yTile);
					w->waitTicksMissile = 0;
					Figure_createMissile(walkerId, w->x, w->y, xTile, yTile, Figure_Javelin);
				} else {
					w->actionState = FigureActionState_173_TowerSentryReturning;
					w->destinationX = w->sourceX;
					w->destinationY = w->sourceY;
					FigureRoute_remove(walkerId);
				}
			}
			break;
		case FigureActionState_173_TowerSentryReturning:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_170_TowerSentryAtRest;
			} else if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
		case FigureActionState_174_TowerSentryGoingToTower:
			w->terrainUsage = FigureTerrainUsage_Roads;
			w->isGhost = 0;
			w->heightAdjustedTicks = 0;
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination) {
				Figure_removeFromTileList(walkerId);
				w->sourceX = w->x = b->x;
				w->sourceY = w->y = b->y;
				w->gridOffset = GridOffset(w->x, w->y);
				Figure_addToTileList(walkerId);
				w->actionState = FigureActionState_170_TowerSentryAtRest;
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
	}
	if (Data_Grid_terrain[w->gridOffset] & Terrain_Wall) {
		w->currentHeight = 18;
	} else if (Data_Grid_terrain[w->gridOffset] & Terrain_Gatehouse) {
		w->inBuildingWaitTicks = 24;
	} else if (w->actionState != FigureActionState_174_TowerSentryGoingToTower) {
		w->state = FigureState_Dead;
	}
	if (w->inBuildingWaitTicks) {
		w->inBuildingWaitTicks--;
		w->heightAdjustedTicks = 0;
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Figure_TowerSentry) +
			136 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == FigureActionState_172_TowerSentryFiring) {
		w->graphicId = GraphicId(ID_Graphic_Figure_TowerSentry) +
			dir + 96 + 8 * towerSentryFiringOffsets[w->waitTicksMissile / 2];
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_TowerSentry) +
			dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_TowerSentry_reroute()
{
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->type != Figure_TowerSentry || Data_Grid_routingWalls[w->gridOffset] == 0) {
			continue;
		}
		// tower sentry got off wall due to rotation
		int xTile, yTile;
		if (Terrain_getWallTileWithinRadius(w->x, w->y, 2, &xTile, &yTile)) {
			FigureRoute_remove(i);
			w->progressOnTile = 0;
			Figure_removeFromTileList(i);
			w->previousTileX = w->x = xTile;
			w->previousTileY = w->y = yTile;
			w->crossCountryX = 15 * xTile;
			w->crossCountryY = 15 * yTile;
			w->gridOffset = GridOffset(xTile, yTile);
			Figure_addToTileList(i);
			w->actionState = FigureActionState_173_TowerSentryReturning;
			w->destinationX = w->sourceX;
			w->destinationY = w->sourceY;
		} else {
			// Teleport back to tower
			Figure_removeFromTileList(i);
			struct Data_Building *b = &Data_Buildings[w->buildingId];
			w->sourceX = w->x = b->x;
			w->sourceY = w->y = b->y;
			w->gridOffset = GridOffset(w->x, w->y);
			Figure_addToTileList(i);
			w->actionState = FigureActionState_170_TowerSentryAtRest;
			FigureRoute_remove(i);
		}
	}
}
