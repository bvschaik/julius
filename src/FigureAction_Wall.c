#include "FigureAction_private.h"

#include "Figure.h"
#include "Routing.h"
#include "Sound.h"
#include "Terrain.h"

#include "Data/Grid.h"

#include "core/calc.h"
#include "figure/enemy_army.h"
#include "figure/type.h"

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

void FigureAction_ballista(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	struct Data_Building *b = &Data_Buildings[f->buildingId];
	f->terrainUsage = FigureTerrainUsage_Walls;
	f->useCrossCountry = 0;
	f->isGhost = 1;
	f->heightAdjustedTicks = 10;
	f->currentHeight = 45;
	
	if (!BuildingIsInUse(f->buildingId) || b->figureId4 != figureId) {
		f->state = FigureState_Dead;
	}
	if (b->numWorkers <= 0 || b->figureId <= 0) {
		f->state = FigureState_Dead;
	}
	Figure_removeFromTileList(figureId);
	switch (Data_Settings_Map.orientation) {
		case Dir_0_Top: f->x = b->x; f->y = b->y; break;
		case Dir_2_Right: f->x = b->x + 1; f->y = b->y; break;
		case Dir_4_Bottom: f->x = b->x + 1; f->y = b->y + 1; break;
		case Dir_6_Left: f->x = b->x; f->y = b->y + 1; break;
	}
	f->gridOffset = GridOffset(f->x, f->y);
	Figure_addToTileList(figureId);

	switch (f->actionState) {
		case FigureActionState_149_Corpse:
			f->state = FigureState_Dead;
			break;
		case FigureActionState_180_BallistaCreated:
			f->waitTicks++;
			if (f->waitTicks > 20) {
				f->waitTicks = 0;
				int xTile, yTile;
				if (FigureAction_CombatSoldier_getMissileTarget(figureId, 15, &xTile, &yTile)) {
					f->actionState = FigureActionState_181_BallistaFiring;
					f->waitTicksMissile = Constant_FigureProperties[f->type].missileFrequency;
				}
			}
			break;
		case FigureActionState_181_BallistaFiring:
			f->waitTicksMissile++;
			if (f->waitTicksMissile > Constant_FigureProperties[f->type].missileFrequency) {
				int xTile, yTile;
				if (FigureAction_CombatSoldier_getMissileTarget(figureId, 15, &xTile, &yTile)) {
					f->direction = Routing_getDirectionForMissileShooter(f->x, f->y, xTile, yTile);
					f->waitTicksMissile = 0;
					Figure_createMissile(figureId, f->x, f->y, xTile, yTile, FIGURE_BOLT);
					Sound_Effects_playChannel(SoundChannel_BallistaShoot);
				} else {
					f->actionState = FigureActionState_180_BallistaCreated;
				}
			}
			break;
	}
	int dir = FigureActionDirection(f);
	if (f->actionState == FigureActionState_181_BallistaFiring) {
		f->graphicId = image_group(ID_Graphic_Figure_Ballista) + dir +
			8 * ballistaFiringOffsets[f->waitTicksMissile / 4];
	} else {
		f->graphicId = image_group(ID_Graphic_Figure_Ballista) + dir;
	}
}

static void towerSentryPickTarget(int figureId, struct Data_Figure *f)
{
	if (enemy_army_total_enemy_formations() <= 0) {
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
		if (FigureAction_CombatSoldier_getMissileTarget(figureId, 10, &xTile, &yTile)) {
			f->actionState = FigureActionState_172_TowerSentryFiring;
			f->destinationX = f->x;
			f->destinationY = f->y;
		}
	}
}

static int towerSentryInitPatrol(struct Data_Building *b, int *xTile, int *yTile)
{
	int dir = b->figureRoamDirection;
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
		b->figureRoamDirection += 2;
		if (b->figureRoamDirection > 6) b->figureRoamDirection = 0;
		return 1;
	}
	for (int i = 0; i < 4; i++) {
		dir = b->figureRoamDirection;
		b->figureRoamDirection += 2;
		if (b->figureRoamDirection > 6) b->figureRoamDirection = 0;
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

void FigureAction_towerSentry(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	struct Data_Building *b = &Data_Buildings[f->buildingId];
	f->terrainUsage = FigureTerrainUsage_Walls;
	f->useCrossCountry = 0;
	f->isGhost = 1;
	f->heightAdjustedTicks = 10;
	f->maxRoamLength = 800;
	if (!BuildingIsInUse(f->buildingId) || b->figureId != figureId) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	
	towerSentryPickTarget(figureId, f);
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(figureId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(figureId);
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
					FigureRoute_remove(figureId);
				}
			}
			break;
		case FigureActionState_171_TowerSentryPatrolling:
			FigureMovement_walkTicks(figureId, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_173_TowerSentryReturning;
				f->destinationX = f->sourceX;
				f->destinationY = f->sourceY;
				FigureRoute_remove(figureId);
			} else if (f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost) {
				f->actionState = FigureActionState_170_TowerSentryAtRest;
			}
			break;
		case FigureActionState_172_TowerSentryFiring:
			FigureMovement_walkTicksTowerSentry(figureId, 1);
			f->waitTicksMissile++;
			if (f->waitTicksMissile > Constant_FigureProperties[f->type].missileFrequency) {
				int xTile, yTile;
				if (FigureAction_CombatSoldier_getMissileTarget(figureId, 10, &xTile, &yTile)) {
					f->direction = Routing_getDirectionForMissileShooter(f->x, f->y, xTile, yTile);
					f->waitTicksMissile = 0;
					Figure_createMissile(figureId, f->x, f->y, xTile, yTile, FIGURE_JAVELIN);
				} else {
					f->actionState = FigureActionState_173_TowerSentryReturning;
					f->destinationX = f->sourceX;
					f->destinationY = f->sourceY;
					FigureRoute_remove(figureId);
				}
			}
			break;
		case FigureActionState_173_TowerSentryReturning:
			FigureMovement_walkTicks(figureId, 1);
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
			FigureMovement_walkTicks(figureId, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				Figure_removeFromTileList(figureId);
				f->sourceX = f->x = b->x;
				f->sourceY = f->y = b->y;
				f->gridOffset = GridOffset(f->x, f->y);
				Figure_addToTileList(figureId);
				f->actionState = FigureActionState_170_TowerSentryAtRest;
				FigureRoute_remove(figureId);
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
	int dir = FigureActionDirection(f);
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = image_group(ID_Graphic_Figure_TowerSentry) +
			136 + FigureActionCorpseGraphicOffset(f);
	} else if (f->actionState == FigureActionState_172_TowerSentryFiring) {
		f->graphicId = image_group(ID_Graphic_Figure_TowerSentry) +
			dir + 96 + 8 * towerSentryFiringOffsets[f->waitTicksMissile / 2];
	} else {
		f->graphicId = image_group(ID_Graphic_Figure_TowerSentry) +
			dir + 8 * f->graphicOffset;
	}
}

void FigureAction_TowerSentry_reroute()
{
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Figure *f = &Data_Figures[i];
		if (f->type != FIGURE_TOWER_SENTRY || Data_Grid_routingWalls[f->gridOffset] == 0) {
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
