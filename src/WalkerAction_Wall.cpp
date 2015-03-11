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
	w->terrainUsage = WalkerTerrainUsage_Walls;
	w->useCrossCountry = 0;
	w->isGhost = 1;
	w->heightFromGround = 10;
	w->currentHeight = 45;
	
	if (!BuildingIsInUse(w->buildingId) || b->walkerId4 != walkerId) {
		w->state = WalkerState_Dead;
	}
	if (b->numWorkers <= 0 || b->walkerId <= 0) {
		w->state = WalkerState_Dead;
	}
	Walker_removeFromTileList(walkerId);
	switch (Data_Settings_Map.orientation) {
		case 0: w->x = b->x; w->y = b->y; break;
		case 2: w->x = b->x + 1; w->y = b->y; break;
		case 4: w->x = b->x + 1; w->y = b->y + 1; break;
		case 6: w->x = b->x; w->y = b->y + 1; break;
	}
	w->gridOffset = GridOffset(w->x, w->y);
	Walker_addToTileList(walkerId);

	switch (w->actionState) {
		case WalkerActionState_149_Corpse:
			w->state = WalkerState_Dead;
			break;
		case WalkerActionState_180_BallistaCreated:
			w->waitTicks++;
			if (w->waitTicks > 20) {
				w->waitTicks = 0;
				int xTile, yTile;
				if (WalkerAction_CombatSoldier_getMissileTarget(walkerId, 15, &xTile, &yTile)) {
					w->actionState = WalkerActionState_181_BallistaFiring;
					w->waitTicksMissile = Constant_WalkerProperties[w->type].missileFrequency;
				}
			}
			break;
		case WalkerActionState_181_BallistaFiring:
			w->waitTicksMissile++;
			if (w->waitTicksMissile > Constant_WalkerProperties[w->type].missileFrequency) {
				int xTile, yTile;
				if (WalkerAction_CombatSoldier_getMissileTarget(walkerId, 15, &xTile, &yTile)) {
					w->direction = Routing_getDirectionForMissileShooter(w->x, w->y, xTile, yTile);
					w->waitTicksMissile = 0;
					Walker_createMissile(walkerId, w->x, w->y, xTile, yTile, Walker_Bolt);
					Sound_Effects_playChannel(SoundChannel_BallistaShoot);
				} else {
					w->actionState = WalkerActionState_180_BallistaCreated;
				}
			}
			break;
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == WalkerActionState_181_BallistaFiring) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Ballista) + dir +
			8 * ballistaFiringOffsets[w->waitTicksMissile / 4];
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Ballista) + dir;
	}
}

static void towerSentryPickTarget(int walkerId, struct Data_Walker *w)
{
	if (Data_Formation_Extra.numEnemyFormations <= 0) {
		return;
	}
	if (w->actionState == WalkerActionState_150_Attack ||
		w->actionState == WalkerActionState_149_Corpse) {
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
			w->actionState = WalkerActionState_172_TowerSentryFiring;
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
		case 0: y -= 8; break;
		case 2: x += 8; break;
		case 4: y += 8; break;
		case 6: x -= 8; break;
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
			case 0: y -= 3; break;
			case 2: x += 3; break;
			case 4: y += 3; break;
			case 6: x -= 3; break;
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
	w->terrainUsage = WalkerTerrainUsage_Walls;
	w->useCrossCountry = 0;
	w->isGhost = 1;
	w->heightFromGround = 10;
	w->maxRoamLength = 800;
	if (!BuildingIsInUse(w->buildingId) || b->walkerId != walkerId) {
		w->state = WalkerState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	
	towerSentryPickTarget(walkerId, w);
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_170_TowerSentryAtRest:
			w->graphicOffset = 0;
			w->waitTicks++;
			if (w->waitTicks > 40) {
				w->waitTicks = 0;
				int xTile, yTile;
				if (towerSentryInitPatrol(w, b, &xTile, &yTile)) {
					w->actionState = WalkerActionState_171_TowerSentryPatrolling;
					w->destinationX = xTile;
					w->destinationY = yTile;
					WalkerRoute_remove(walkerId);
				}
			}
			break;
		case WalkerActionState_171_TowerSentryPatrolling:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_173_TowerSentryReturning;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 9 || w->direction == 10) {
				w->actionState = WalkerActionState_170_TowerSentryAtRest;
			}
			break;
		case WalkerActionState_172_TowerSentryFiring:
			WalkerMovement_walkTicksTowerSentry(walkerId, 1);
			w->waitTicksMissile++;
			if (w->waitTicksMissile > Constant_WalkerProperties[w->type].missileFrequency) {
				int xTile, yTile;
				if (WalkerAction_CombatSoldier_getMissileTarget(walkerId, 10, &xTile, &yTile)) {
					w->direction = Routing_getDirectionForMissileShooter(w->x, w->y, xTile, yTile);
					w->waitTicksMissile = 0;
					Walker_createMissile(walkerId, w->x, w->y, xTile, yTile, Walker_Javelin);
				} else {
					w->actionState = WalkerActionState_173_TowerSentryReturning;
					w->destinationX = w->sourceX;
					w->destinationY = w->sourceY;
					WalkerRoute_remove(walkerId);
				}
			}
			break;
		case WalkerActionState_173_TowerSentryReturning:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_170_TowerSentryAtRest;
			} else if (w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_174_TowerSentryGoingToTower:
			w->terrainUsage = WalkerTerrainUsage_Roads;
			w->isGhost = 0;
			w->heightFromGround = 0;
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8) {
				Walker_removeFromTileList(walkerId);
				w->sourceX = w->x = b->x;
				w->sourceY = w->y = b->y;
				w->gridOffset = GridOffset(w->x, w->y);
				Walker_addToTileList(walkerId);
				w->actionState = WalkerActionState_170_TowerSentryAtRest;
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
	}
	if (Data_Grid_terrain[w->gridOffset] & Terrain_Wall) {
		w->currentHeight = 18;
	} else if (Data_Grid_terrain[w->gridOffset] & Terrain_Gatehouse) {
		w->inBuildingWaitTicks = 24;
	} else if (w->actionState != WalkerActionState_174_TowerSentryGoingToTower) {
		w->state = WalkerState_Dead;
	}
	if (w->inBuildingWaitTicks) {
		w->inBuildingWaitTicks--;
		w->heightFromGround = 0;
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_TowerSentry) +
			136 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_172_TowerSentryFiring) {
		w->graphicId = GraphicId(ID_Graphic_Walker_TowerSentry) +
			dir + 96 + 8 * towerSentryFiringOffsets[w->waitTicksMissile / 2];
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_TowerSentry) +
			dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_TowerSentry_reroute()
{
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->type != Walker_TowerSentry || Data_Grid_routingWalls[w->gridOffset] == 0) {
			continue;
		}
		// tower sentry got off wall due to rotation
		int xTile, yTile;
		if (Terrain_getWallTileWithinRadius(w->x, w->y, 2, &xTile, &yTile)) {
			WalkerRoute_remove(i);
			w->progressOnTile = 0;
			Walker_removeFromTileList(i);
			w->previousTileX = w->x = xTile;
			w->previousTileY = w->y = yTile;
			w->crossCountryX = 15 * xTile;
			w->crossCountryY = 15 * yTile;
			w->gridOffset = GridOffset(xTile, yTile);
			Walker_addToTileList(i);
			w->actionState = WalkerActionState_173_TowerSentryReturning;
			w->destinationX = w->sourceX;
			w->destinationY = w->sourceY;
		} else {
			// Teleport back to tower
			Walker_removeFromTileList(i);
			struct Data_Building *b = &Data_Buildings[w->buildingId];
			w->sourceX = w->x = b->x;
			w->sourceY = w->y = b->y;
			w->gridOffset = GridOffset(w->x, w->y);
			Walker_addToTileList(i);
			w->actionState = WalkerActionState_170_TowerSentryAtRest;
			WalkerRoute_remove(i);
		}
	}
}
