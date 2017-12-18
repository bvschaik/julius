#include "FigureMovement.h"

#include "Building.h"
#include "Figure.h"
#include "FigureAction.h"
#include "Terrain.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "core/calc.h"
#include "figure/route.h"
#include "game/time.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/random.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"

static void FigureMovement_walkTicksInternal(figure *f, int numTicks, int roamingEnabled);

void FigureMovement_advanceTick(figure *f)
{
	switch (f->direction) {
		case DIR_0_TOP:
			f->crossCountryY--;
			break;
		case DIR_1_TOP_RIGHT:
			f->crossCountryX++;
			f->crossCountryY--;
			break;
		case DIR_2_RIGHT:
			f->crossCountryX++;
			break;
		case DIR_3_BOTTOM_RIGHT:
			f->crossCountryX++;
			f->crossCountryY++;
			break;
		case DIR_4_BOTTOM:
			f->crossCountryY++;
			break;
		case DIR_5_BOTTOM_LEFT:
			f->crossCountryX--;
			f->crossCountryY++;
			break;
		case DIR_6_LEFT:
			f->crossCountryX--;
			break;
		case DIR_7_TOP_LEFT:
			f->crossCountryX--;
			f->crossCountryY--;
			break;
		default:
			break;
	}
	if (f->heightAdjustedTicks) {
		f->heightAdjustedTicks--;
		if (f->heightAdjustedTicks > 0) {
			f->isGhost = 1;
			if (f->currentHeight < f->targetHeight) {
				f->currentHeight++;
			}
			if (f->currentHeight > f->targetHeight) {
				f->currentHeight--;
			}
		} else {
			f->isGhost = 0;
		}
	} else {
		if (f->currentHeight) {
			f->currentHeight--;
		}
	}
}

static void setTargetHeightBridge(figure *f)
{
	f->heightAdjustedTicks = 18;
	f->targetHeight = map_bridge_height(f->gridOffset);
}

static void figureMoveToNextTile(figure *f)
{
	int oldX = f->x;
	int oldY = f->y;
	map_figure_delete(f);
	switch (f->direction) {
		default:
			return;
		case DIR_0_TOP:
			f->y--;
			f->gridOffset -= 162;
			break;
		case DIR_1_TOP_RIGHT:
			f->x++; f->y--;
			f->gridOffset -= 161;
			break;
		case DIR_2_RIGHT:
			f->x++;
			f->gridOffset += 1;
			break;
		case DIR_3_BOTTOM_RIGHT:
			f->x++; f->y++;
			f->gridOffset += 163;
			break;
		case DIR_4_BOTTOM:
			f->y++;
			f->gridOffset += 162;
			break;
		case DIR_5_BOTTOM_LEFT:
			f->x--; f->y++;
			f->gridOffset += 161;
			break;
		case DIR_6_LEFT:
			f->x--;
			f->gridOffset -= 1;
			break;
		case DIR_7_TOP_LEFT:
			f->x--; f->y--;
			f->gridOffset -= 163;
			break;
	}
	map_figure_add(f);
	if (map_terrain_is(f->gridOffset, TERRAIN_ROAD)) {
		f->isOnRoad = 1;
		if (map_terrain_is(f->gridOffset, TERRAIN_WATER)) { // bridge
			setTargetHeightBridge(f);
		}
	} else {
		f->isOnRoad = 0;
	}
	FigureAction_Combat_attackFigureAt(f, f->gridOffset);
	f->previousTileX = oldX;
	f->previousTileY = oldY;
}

void FigureMovement_initRoaming(figure *f)
{
	building *b = building_get(f->buildingId);
	f->progressOnTile = 15;
	f->roamChooseDestination = 0;
	f->roamTicksUntilNextTurn = -1;
	f->roamTurnDirection = 2;
	int roamDir = b->figureRoamDirection;
	b->figureRoamDirection += 2;
	if (b->figureRoamDirection > 6) {
		b->figureRoamDirection = 0;
	}
	int x = b->x;
	int y = b->y;
	switch (roamDir) {
		case DIR_0_TOP: y -= 8; break;
		case DIR_2_RIGHT: x += 8; break;
		case DIR_4_BOTTOM: y += 8; break;
		case DIR_6_LEFT: x -= 8; break;
	}
	map_grid_bound(&x, &y);
	int xRoad, yRoad;
	if (Terrain_getClosestRoadWithinRadius(x, y, 1, 6, &xRoad, &yRoad)) {
		f->destinationX = xRoad;
		f->destinationY = yRoad;
	} else {
		f->roamChooseDestination = 1;
	}
}

static void roamSetDirection(figure *f)
{
	int gridOffset = map_grid_offset(f->x, f->y);
	int direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
	if (direction >= 8) {
		direction = 0;
	}
	int roadOffsetDir1 = 0;
	int roadDir1 = 0;
	for (int i = 0, dir = direction; i < 8; i++) {
		if (dir % 2 == 0 && map_terrain_is(gridOffset + map_grid_direction_delta(dir), TERRAIN_ROAD)) {
			roadDir1 = dir;
			break;
		}
		dir++;
		if (dir > 7) dir = 0;
		roadOffsetDir1++;
	}
	int roadOffsetDir2 = 0;
	int roadDir2 = 0;
	for (int i = 0, dir = direction; i < 8; i++) {
		if (dir % 2 == 0 && map_terrain_is(gridOffset + map_grid_direction_delta(dir), TERRAIN_ROAD)) {
			roadDir2 = dir;
			break;
		}
		dir--;
		if (dir < 0) dir = 7;
		roadOffsetDir2++;
	}
	if (roadOffsetDir1 <= roadOffsetDir2) {
		f->direction = roadDir1;
		f->roamTurnDirection = 2;
	} else {
		f->direction = roadDir2;
		f->roamTurnDirection = -2;
	}
	f->roamTicksUntilNextTurn = 5;
}

void FigureMovement_roamTicks(figure *f, int numTicks)
{
	if (f->roamChooseDestination == 0) {
		FigureMovement_walkTicksInternal(f, numTicks, 1);
		if (f->direction == DIR_FIGURE_AT_DESTINATION) {
			f->roamChooseDestination = 1;
			f->roamLength = 0;
		} else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
			f->roamChooseDestination = 1;
		}
		if (f->roamChooseDestination) {
			f->roamTicksUntilNextTurn = 100;
			f->direction = f->previousTileDirection;
		} else {
			return;
		}
	}
	// no destination: walk to end of tile and pick a direction
	while (numTicks > 0) {
		numTicks--;
		f->progressOnTile++;
		if (f->progressOnTile < 15) {
			FigureMovement_advanceTick(f);
		} else {
			f->progressOnTile = 15;
			f->roamRandomCounter++;
			int cameFromDirection = (f->previousTileDirection + 4) % 8;
			if (Figure_provideServiceCoverage(f)) {
				return;
			}
			int roadTiles[8];
			int adjacentRoadTiles = Terrain_getAdjacentRoadTilesForRoaming(f->gridOffset, roadTiles);
			if (adjacentRoadTiles == 3 && Terrain_getSurroundingRoadTilesForRoaming(f->gridOffset, roadTiles) >= 5) {
				// go in the straight direction of a double-wide road
				adjacentRoadTiles = 2;
				if (cameFromDirection == DIR_0_TOP || cameFromDirection == DIR_4_BOTTOM) {
					if (roadTiles[0] && roadTiles[4]) {
						roadTiles[2] = roadTiles[6] = 0;
					} else {
						roadTiles[0] = roadTiles[4] = 0;
					}
				} else {
					if (roadTiles[2] && roadTiles[6]) {
						roadTiles[0] = roadTiles[4] = 0;
					} else {
						roadTiles[2] = roadTiles[6] = 0;
					}
				}
			}
			if (adjacentRoadTiles == 4 && Terrain_getSurroundingRoadTilesForRoaming(f->gridOffset, roadTiles) >= 8) {
				// go straight on when all surrounding tiles are road
				adjacentRoadTiles = 2;
				if (cameFromDirection == DIR_0_TOP || cameFromDirection == DIR_4_BOTTOM) {
					roadTiles[2] = roadTiles[6] = 0;
				} else {
					roadTiles[0] = roadTiles[4] = 0;
				}
			}
			if (adjacentRoadTiles <= 0) {
				f->roamLength = f->maxRoamLength; // end roaming walk
				return;
			}
			if (adjacentRoadTiles == 1) {
				int dir = 0;
				do {
					f->direction = 2 * dir;
				} while (!roadTiles[f->direction] && dir++ < 4);
			} else if (adjacentRoadTiles == 2) {
				if (f->roamTicksUntilNextTurn == -1) {
					roamSetDirection(f);
					cameFromDirection = -1;
				}
				// 1. continue in the same direction
				// 2. turn in the direction given by roamTurnDirection
				int dir = 0;
				do {
					if (roadTiles[f->direction] && f->direction != cameFromDirection) {
						break;
					}
					f->direction += f->roamTurnDirection;
					if (f->direction > 6) f->direction = 0;
					if (f->direction < 0) f->direction = 6;
				} while (dir++ < 4);
			} else { // > 2 road tiles
				f->direction = (f->roamRandomCounter + map_random_get(f->gridOffset)) & 6;
				if (!roadTiles[f->direction] || f->direction == cameFromDirection) {
					f->roamTicksUntilNextTurn--;
					if (f->roamTicksUntilNextTurn <= 0) {
						roamSetDirection(f);
						cameFromDirection = -1;
					}
					int dir = 0;
					do {
						if (roadTiles[f->direction] && f->direction != cameFromDirection) {
							break;
						}
						f->direction += f->roamTurnDirection;
						if (f->direction > 6) f->direction = 0;
						if (f->direction < 0) f->direction = 6;
					} while (dir++ < 4);
				}
			}
			f->routingPathCurrentTile++;
			f->previousTileDirection = f->direction;
			f->progressOnTile = 0;
			figureMoveToNextTile(f);
			FigureMovement_advanceTick(f);
		}
	}
}

static void figureSetNextRouteTileDirection(figure *f)
{
	if (f->routingPathId > 0) {
		if (f->routingPathCurrentTile < f->routingPathLength) {
			f->direction = figure_route_get_direction(f->routingPathId, f->routingPathCurrentTile);
		} else {
			figure_route_remove(f);
			f->direction = DIR_FIGURE_AT_DESTINATION;
		}
	} else { // should be at destination
		f->direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
		if (f->direction != DIR_FIGURE_AT_DESTINATION) {
			f->direction = DIR_FIGURE_LOST;
		}
	}
}

static void figureAdvanceRouteTile(figure *f, int roamingEnabled)
{
	if (f->direction >= 8) {
		return;
	}
	int targetGridOffset = f->gridOffset + map_grid_direction_delta(f->direction);
	if (f->isBoat) {
		if (!map_terrain_is(targetGridOffset, TERRAIN_WATER)) {
			f->direction = DIR_FIGURE_REROUTE;
		}
	} else if (f->terrainUsage == FigureTerrainUsage_Enemy) {
		if (!map_routing_noncitizen_is_passable(targetGridOffset)) {
			f->direction = DIR_FIGURE_REROUTE;
		} else if (map_routing_is_destroyable(targetGridOffset)) {
			int causeDamage = 1;
			int maxDamage = 0;
			switch (map_routing_get_destroyable(targetGridOffset)) {
				case DESTROYABLE_BUILDING:
					maxDamage = 10;
					break;
				case DESTROYABLE_AQUEDUCT_GARDEN:
					if (map_terrain_is(targetGridOffset, TERRAIN_GARDEN | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE)) {
						causeDamage = 0;
					} else {
						maxDamage = 10;
					}
					break;
				case DESTROYABLE_WALL:
					maxDamage = 200;
					break;
				case DESTROYABLE_GATEHOUSE:
                    maxDamage = 150;
                    break;
			}
			if (causeDamage) {
				f->attackDirection = f->direction;
				f->direction = DIR_FIGURE_ATTACK;
				if (!(game_time_tick() & 3)) {
					Building_increaseDamageByEnemy(targetGridOffset, maxDamage);
				}
			}
		}
	} else if (f->terrainUsage == FigureTerrainUsage_Walls) {
		if (!map_routing_is_wall_passable(targetGridOffset)) {
			f->direction = DIR_FIGURE_REROUTE;
		}
	} else if (map_terrain_is(targetGridOffset, TERRAIN_ROAD | TERRAIN_ACCESS_RAMP)) {
		if (roamingEnabled && map_terrain_is(targetGridOffset, TERRAIN_BUILDING)) {
			if (building_get(map_building_at(targetGridOffset))->type == BUILDING_GATEHOUSE) {
				// do not allow roaming through gatehouse
				f->direction = DIR_FIGURE_REROUTE;
			}
		}
	} else if (map_terrain_is(targetGridOffset, TERRAIN_BUILDING)) {
		int type = building_get(map_building_at(targetGridOffset))->type;
		switch (type) {
			case BUILDING_WAREHOUSE:
			case BUILDING_GRANARY:
			case BUILDING_TRIUMPHAL_ARCH:
			case BUILDING_FORT_GROUND:
				break; // OK to walk
			default:
				f->direction = DIR_FIGURE_REROUTE;
		}
	} else if (map_terrain_is(targetGridOffset, TERRAIN_IMPASSABLE)) {
		f->direction = DIR_FIGURE_REROUTE;
	}
}

static void FigureMovement_walkTicksInternal(figure *f, int numTicks, int roamingEnabled)
{
	while (numTicks > 0) {
		numTicks--;
		f->progressOnTile++;
		if (f->progressOnTile < 15) {
			FigureMovement_advanceTick(f);
		} else {
			Figure_provideServiceCoverage(f);
			f->progressOnTile = 15;
			if (f->routingPathId <= 0) {
				figure_route_add(f);
			}
			figureSetNextRouteTileDirection(f);
			figureAdvanceRouteTile(f, roamingEnabled);
			if (f->direction >= 8) {
				break;
			}
			f->routingPathCurrentTile++;
			f->previousTileDirection = f->direction;
			f->progressOnTile = 0;
			figureMoveToNextTile(f);
			FigureMovement_advanceTick(f);
		}
	}
}

void FigureMovement_walkTicks(figure *f, int numTicks)
{
	FigureMovement_walkTicksInternal(f, numTicks, 0);
}

void FigureMovement_followTicks(figure *f, int numTicks)
{
    const figure *leader = figure_get(f->inFrontFigureId);
	if (f->x == f->sourceX && f->y == f->sourceY) {
		f->isGhost = 1;
	}
	while (numTicks > 0) {
		numTicks--;
		f->progressOnTile++;
		if (f->progressOnTile < 15) {
			FigureMovement_advanceTick(f);
		} else {
			f->progressOnTile = 15;
			f->direction = calc_general_direction(f->x, f->y,
				leader->previousTileX, leader->previousTileY);
			if (f->direction >= 8) {
				break;
			}
			f->previousTileDirection = f->direction;
			f->progressOnTile = 0;
			figureMoveToNextTile(f);
			FigureMovement_advanceTick(f);
		}
	}
}

void FigureMovement_walkTicksTowerSentry(figure *f, int numTicks)
{
	while (numTicks > 0) {
		numTicks--;
		f->progressOnTile++;
		if (f->progressOnTile < 15) {
			FigureMovement_advanceTick(f);
		} else {
			f->progressOnTile = 15;
		}
	}
}

void FigureMovement_crossCountrySetDirection(figure *f, int xSrc, int ySrc, int xDst, int yDst, int isMissile)
{
	// all x/y are in 1/15th of a tile
	f->ccDestinationX = xDst;
	f->ccDestinationY = yDst;
	f->ccDeltaX = (xSrc > xDst) ? (xSrc - xDst) : (xDst - xSrc);
	f->ccDeltaY = (ySrc > yDst) ? (ySrc - yDst) : (yDst - ySrc);
	if (f->ccDeltaX < f->ccDeltaY) {
		f->ccDeltaXY = 2 * f->ccDeltaX - f->ccDeltaY;
	} else if (f->ccDeltaY < f->ccDeltaX) {
		f->ccDeltaXY = 2 * f->ccDeltaY - f->ccDeltaX;
	} else { // equal
		f->ccDeltaXY = 0;
	}
	if (isMissile) {
		f->direction = calc_missile_direction(xSrc, ySrc, xDst, yDst);
	} else {
		f->direction = calc_general_direction(xSrc, ySrc, xDst, yDst);
		if (f->ccDeltaY > 2 * f->ccDeltaX) {
			switch (f->direction) {
				case DIR_1_TOP_RIGHT: case DIR_7_TOP_LEFT: f->direction = DIR_0_TOP; break;
				case DIR_3_BOTTOM_RIGHT: case DIR_5_BOTTOM_LEFT: f->direction = DIR_4_BOTTOM; break;
			}
		}
		if (f->ccDeltaX > 2 * f->ccDeltaY) {
			switch (f->direction) {
				case DIR_1_TOP_RIGHT: case DIR_3_BOTTOM_RIGHT: f->direction = DIR_2_RIGHT; break;
				case DIR_5_BOTTOM_LEFT: case DIR_7_TOP_LEFT: f->direction = DIR_6_LEFT; break;
			}
		}
	}
	if (f->ccDeltaX >= f->ccDeltaY) {
		f->ccDirection = 1;
	} else {
		f->ccDirection = 2;
	}
}

static void crossCountryUpdateDelta(figure *f)
{
	if (f->ccDirection == 1) { // x
		if (f->ccDeltaXY >= 0) {
			f->ccDeltaXY += 2 * (f->ccDeltaY - f->ccDeltaX);
		} else {
			f->ccDeltaXY += 2 * f->ccDeltaY;
		}
		f->ccDeltaX--;
	} else { // y
		if (f->ccDeltaXY >= 0) {
			f->ccDeltaXY += 2 * (f->ccDeltaX - f->ccDeltaY);
		} else {
			f->ccDeltaXY += 2 * f->ccDeltaX;
		}
		f->ccDeltaY--;
	}
}

static void crossCountryAdvanceX(figure *f)
{
	if (f->crossCountryX < f->ccDestinationX) {
		f->crossCountryX++;
	} else if (f->crossCountryX > f->ccDestinationX) {
		f->crossCountryX--;
	}
}

static void crossCountryAdvanceY(figure *f)
{
	if (f->crossCountryY < f->ccDestinationY) {
		f->crossCountryY++;
	} else if (f->crossCountryY > f->ccDestinationY) {
		f->crossCountryY--;
	}
}

static void crossCountryAdvance(figure *f)
{
	crossCountryUpdateDelta(f);
	if (f->ccDirection == 2) { // y
		crossCountryAdvanceY(f);
		if (f->ccDeltaXY >= 0) {
			f->ccDeltaX--;
			crossCountryAdvanceX(f);
		}
	} else {
		crossCountryAdvanceX(f);
		if (f->ccDeltaXY >= 0) {
			f->ccDeltaY--;
			crossCountryAdvanceY(f);
		}
	}
}

int FigureMovement_crossCountryWalkTicks(figure *f, int numTicks)
{
	map_figure_delete(f);
	int isAtDestination = 0;
	while (numTicks > 0) {
		numTicks--;
		if (f->missileDamage > 0) {
			f->missileDamage--;
		} else {
			f->missileDamage = 0;
		}
		if (f->ccDeltaX + f->ccDeltaY <= 0) {
			isAtDestination = 1;
			break;
		}
		crossCountryAdvance(f);
	}
	f->x = f->crossCountryX / 15;
	f->y = f->crossCountryY / 15;
	f->gridOffset = map_grid_offset(f->x, f->y);
	if (map_terrain_is(f->gridOffset, TERRAIN_BUILDING)) {
		f->inBuildingWaitTicks = 8;
	} else if (f->inBuildingWaitTicks) {
		f->inBuildingWaitTicks--;
	}
	map_figure_add(f);
	return isAtDestination;
}

int FigureMovement_canLaunchCrossCountryMissile(int xSrc, int ySrc, int xDst, int yDst)
{
	int height = 0;
	figure *f = figure_get(0); // abuse unused figure 0 as scratch
	f->crossCountryX = 15 * xSrc;
	f->crossCountryY = 15 * ySrc;
	if (map_terrain_is(map_grid_offset(xSrc, ySrc), TERRAIN_WALL_OR_GATEHOUSE)) {
		height = 6;
	}
	FigureMovement_crossCountrySetDirection(f, 15 * xSrc, 15 * ySrc, 15 * xDst, 15 * yDst, 0);

	for (int guard = 0; guard < 1000; guard++) {
		for (int i = 0; i < 8; i++) {
			if (f->ccDeltaX + f->ccDeltaY <= 0) {
				return 1;
			}
			crossCountryAdvance(f);
		}
		f->x = f->crossCountryX / 15;
		f->y = f->crossCountryY / 15;
		if (height) {
			height--;
		} else {
			int gridOffset = map_grid_offset(f->x, f->y);
			if (map_terrain_is(gridOffset, TERRAIN_WALL | TERRAIN_GATEHOUSE | TERRAIN_TREE)) {
				break;
			}
			if (map_terrain_is(gridOffset, TERRAIN_BUILDING) && map_property_multi_tile_size(gridOffset) > 1) {
				break;
			}
		}
	}
	return 0;
}
