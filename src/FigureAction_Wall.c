#include "FigureAction_private.h"

#include "Figure.h"
#include "Terrain.h"

#include "Data/Grid.h"

#include "core/calc.h"
#include "figure/enemy_army.h"
#include "figure/properties.h"
#include "figure/route.h"
#include "figure/type.h"
#include "map/figure.h"
#include "map/routing_terrain.h"
#include "sound/effect.h"

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

void FigureAction_ballista(figure *f)
{
	struct Data_Building *b = &Data_Buildings[f->buildingId];
	f->terrainUsage = FigureTerrainUsage_Walls;
	f->useCrossCountry = 0;
	f->isGhost = 1;
	f->heightAdjustedTicks = 10;
	f->currentHeight = 45;
	
	if (!BuildingIsInUse(f->buildingId) || b->figureId4 != f->id) {
		f->state = FigureState_Dead;
	}
	if (b->numWorkers <= 0 || b->figureId <= 0) {
		f->state = FigureState_Dead;
	}
	map_figure_delete(f);
	switch (Data_State.map.orientation) {
		case Dir_0_Top: f->x = b->x; f->y = b->y; break;
		case Dir_2_Right: f->x = b->x + 1; f->y = b->y; break;
		case Dir_4_Bottom: f->x = b->x + 1; f->y = b->y + 1; break;
		case Dir_6_Left: f->x = b->x; f->y = b->y + 1; break;
	}
	f->gridOffset = GridOffset(f->x, f->y);
	map_figure_add(f);

	switch (f->actionState) {
		case FigureActionState_149_Corpse:
			f->state = FigureState_Dead;
			break;
		case FigureActionState_180_BallistaCreated:
			f->waitTicks++;
			if (f->waitTicks > 20) {
				f->waitTicks = 0;
				int xTile, yTile;
				if (FigureAction_CombatSoldier_getMissileTarget(f, 15, &xTile, &yTile)) {
					f->actionState = FigureActionState_181_BallistaFiring;
					f->waitTicksMissile = figure_properties_for_type(f->type)->missile_delay;
				}
			}
			break;
		case FigureActionState_181_BallistaFiring:
			f->waitTicksMissile++;
			if (f->waitTicksMissile > figure_properties_for_type(f->type)->missile_delay) {
				int xTile, yTile;
				if (FigureAction_CombatSoldier_getMissileTarget(f, 15, &xTile, &yTile)) {
					f->direction = calc_missile_shooter_direction(f->x, f->y, xTile, yTile);
					f->waitTicksMissile = 0;
					Figure_createMissile(f->id, f->x, f->y, xTile, yTile, FIGURE_BOLT);
					sound_effect_play(SOUND_EFFECT_BALLISTA_SHOOT);
				} else {
					f->actionState = FigureActionState_180_BallistaCreated;
				}
			}
			break;
	}
	int dir = FigureActionDirection(f);
	if (f->actionState == FigureActionState_181_BallistaFiring) {
		f->graphicId = image_group(GROUP_FIGURE_BALLISTA) + dir +
			8 * ballistaFiringOffsets[f->waitTicksMissile / 4];
	} else {
		f->graphicId = image_group(GROUP_FIGURE_BALLISTA) + dir;
	}
}

static void towerSentryPickTarget(figure *f)
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
		if (FigureAction_CombatSoldier_getMissileTarget(f, 10, &xTile, &yTile)) {
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

void FigureAction_towerSentry(figure *f)
{
	struct Data_Building *b = &Data_Buildings[f->buildingId];
	f->terrainUsage = FigureTerrainUsage_Walls;
	f->useCrossCountry = 0;
	f->isGhost = 1;
	f->heightAdjustedTicks = 10;
	f->maxRoamLength = 800;
	if (!BuildingIsInUse(f->buildingId) || b->figureId != f->id) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	
	towerSentryPickTarget(f);
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
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
					figure_route_remove(f);
				}
			}
			break;
		case FigureActionState_171_TowerSentryPatrolling:
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_173_TowerSentryReturning;
				f->destinationX = f->sourceX;
				f->destinationY = f->sourceY;
				figure_route_remove(f);
			} else if (f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost) {
				f->actionState = FigureActionState_170_TowerSentryAtRest;
			}
			break;
		case FigureActionState_172_TowerSentryFiring:
			FigureMovement_walkTicksTowerSentry(f, 1);
			f->waitTicksMissile++;
			if (f->waitTicksMissile > figure_properties_for_type(f->type)->missile_delay) {
				int xTile, yTile;
				if (FigureAction_CombatSoldier_getMissileTarget(f, 10, &xTile, &yTile)) {
					f->direction = calc_missile_shooter_direction(f->x, f->y, xTile, yTile);
					f->waitTicksMissile = 0;
					Figure_createMissile(f->id, f->x, f->y, xTile, yTile, FIGURE_JAVELIN);
				} else {
					f->actionState = FigureActionState_173_TowerSentryReturning;
					f->destinationX = f->sourceX;
					f->destinationY = f->sourceY;
					figure_route_remove(f);
				}
			}
			break;
		case FigureActionState_173_TowerSentryReturning:
			FigureMovement_walkTicks(f, 1);
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
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				map_figure_delete(f);
				f->sourceX = f->x = b->x;
				f->sourceY = f->y = b->y;
				f->gridOffset = GridOffset(f->x, f->y);
				map_figure_add(f);
				f->actionState = FigureActionState_170_TowerSentryAtRest;
				figure_route_remove(f);
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
		f->graphicId = image_group(GROUP_FIGURE_TOWER_SENTRY) +
			136 + FigureActionCorpseGraphicOffset(f);
	} else if (f->actionState == FigureActionState_172_TowerSentryFiring) {
		f->graphicId = image_group(GROUP_FIGURE_TOWER_SENTRY) +
			dir + 96 + 8 * towerSentryFiringOffsets[f->waitTicksMissile / 2];
	} else {
		f->graphicId = image_group(GROUP_FIGURE_TOWER_SENTRY) +
			dir + 8 * f->graphicOffset;
	}
}

void FigureAction_TowerSentry_reroute()
{
	for (int i = 1; i < MAX_FIGURES; i++) {
		figure *f = figure_get(i);
		if (f->type != FIGURE_TOWER_SENTRY || map_routing_is_wall_passable(f->gridOffset)) {
			continue;
		}
		// tower sentry got off wall due to rotation
		int xTile, yTile;
		if (Terrain_getWallTileWithinRadius(f->x, f->y, 2, &xTile, &yTile)) {
			figure_route_remove(f);
			f->progressOnTile = 0;
			map_figure_delete(f);
			f->previousTileX = f->x = xTile;
			f->previousTileY = f->y = yTile;
			f->crossCountryX = 15 * xTile;
			f->crossCountryY = 15 * yTile;
			f->gridOffset = GridOffset(xTile, yTile);
			map_figure_add(f);
			f->actionState = FigureActionState_173_TowerSentryReturning;
			f->destinationX = f->sourceX;
			f->destinationY = f->sourceY;
		} else {
			// Teleport back to tower
			map_figure_delete(f);
			struct Data_Building *b = &Data_Buildings[f->buildingId];
			f->sourceX = f->x = b->x;
			f->sourceY = f->y = b->y;
			f->gridOffset = GridOffset(f->x, f->y);
			map_figure_add(f);
			f->actionState = FigureActionState_170_TowerSentryAtRest;
			figure_route_remove(f);
		}
	}
}
