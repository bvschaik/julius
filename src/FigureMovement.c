#include "FigureMovement.h"

#include "Building.h"
#include "Figure.h"
#include "FigureAction.h"
#include "Terrain.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Grid.h"
#include "Data/State.h"
#include "Data/Figure.h"

#include "core/calc.h"
#include "figure/route.h"
#include "game/time.h"
#include "map/random.h"
#include "map/grid.h"
#include "map/routing_terrain.h"

static void FigureMovement_walkTicksInternal(int figureId, int numTicks, int roamingEnabled);

void FigureMovement_advanceTick(struct Data_Figure *f)
{
	switch (f->direction) {
		case Dir_0_Top:
			f->crossCountryY--;
			break;
		case Dir_1_TopRight:
			f->crossCountryX++;
			f->crossCountryY--;
			break;
		case Dir_2_Right:
			f->crossCountryX++;
			break;
		case Dir_3_BottomRight:
			f->crossCountryX++;
			f->crossCountryY++;
			break;
		case Dir_4_Bottom:
			f->crossCountryY++;
			break;
		case Dir_5_BottomLeft:
			f->crossCountryX--;
			f->crossCountryY++;
			break;
		case Dir_6_Left:
			f->crossCountryX--;
			break;
		case Dir_7_TopLeft:
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

static void setTargetHeightBridge(struct Data_Figure *f)
{
	f->heightAdjustedTicks = 18;
	if (Data_Grid_spriteOffsets[f->gridOffset] <= 6) {
		// low bridge
		switch (Data_Grid_spriteOffsets[f->gridOffset]) {
			case 1: case 4: f->targetHeight = 10; break;
			case 2: case 3: f->targetHeight = 16; break;
			default: f->targetHeight = 20; break;
		}
	} else {
		// ship bridge
		switch (Data_Grid_spriteOffsets[f->gridOffset]) {
			case 7: case 8: case 9: case 10: f->targetHeight = 14; break;
			case 13: f->targetHeight = 30; break;
			default: f->targetHeight = 36; break;
		}
	}
}

static void figureMoveToNextTile(int figureId, struct Data_Figure *f)
{
	int oldX = f->x;
	int oldY = f->y;
	Figure_removeFromTileList(figureId);
	switch (f->direction) {
		default:
			return;
		case Dir_0_Top:
			f->y--;
			f->gridOffset -= 162;
			break;
		case Dir_1_TopRight:
			f->x++; f->y--;
			f->gridOffset -= 161;
			break;
		case Dir_2_Right:
			f->x++;
			f->gridOffset += 1;
			break;
		case Dir_3_BottomRight:
			f->x++; f->y++;
			f->gridOffset += 163;
			break;
		case Dir_4_Bottom:
			f->y++;
			f->gridOffset += 162;
			break;
		case Dir_5_BottomLeft:
			f->x--; f->y++;
			f->gridOffset += 161;
			break;
		case Dir_6_Left:
			f->x--;
			f->gridOffset -= 1;
			break;
		case Dir_7_TopLeft:
			f->x--; f->y--;
			f->gridOffset -= 163;
			break;
	}
	Figure_addToTileList(figureId);
	if (Data_Grid_terrain[f->gridOffset] & Terrain_Road) {
		f->isOnRoad = 1;
		if (Data_Grid_terrain[f->gridOffset] & Terrain_Water) { // bridge
			setTargetHeightBridge(f);
		}
	} else {
		f->isOnRoad = 0;
	}
	FigureAction_Combat_attackFigure(figureId, Data_Grid_figureIds[f->gridOffset]);
	f->previousTileX = oldX;
	f->previousTileY = oldY;
}

void FigureMovement_initRoaming(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	struct Data_Building *b = &Data_Buildings[f->buildingId];
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
		case Dir_0_Top: y -= 8; break;
		case Dir_2_Right: x += 8; break;
		case Dir_4_Bottom: y += 8; break;
		case Dir_6_Left: x -= 8; break;
	}
	BoundToMap(x, y);
	int xRoad, yRoad;
	if (Terrain_getClosestRoadWithinRadius(x, y, 1, 6, &xRoad, &yRoad)) {
		f->destinationX = xRoad;
		f->destinationY = yRoad;
	} else {
		f->roamChooseDestination = 1;
	}
}

static void roamSetDirection(struct Data_Figure *f)
{
	int gridOffset = GridOffset(f->x, f->y);
	int direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
	if (direction >= 8) {
		direction = 0;
	}
	int roadOffsetDir1 = 0;
	int roadDir1 = 0;
	for (int i = 0, dir = direction; i < 8; i++) {
		if (dir % 2 == 0 && Data_Grid_terrain[gridOffset + map_grid_direction_delta(dir)] & Terrain_Road) {
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
		if (dir % 2 == 0 && Data_Grid_terrain[gridOffset + map_grid_direction_delta(dir)] & Terrain_Road) {
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

void FigureMovement_roamTicks(int figureId, int numTicks)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	if (f->roamChooseDestination == 0) {
		FigureMovement_walkTicksInternal(figureId, numTicks, 1);
		if (f->direction == DirFigure_8_AtDestination) {
			f->roamChooseDestination = 1;
			f->roamLength = 0;
		} else if (f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost) {
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
			if (Figure_provideServiceCoverage(figureId)) {
				return;
			}
			int roadTiles[8];
			int adjacentRoadTiles = Terrain_getAdjacentRoadTilesForRoaming(f->gridOffset, roadTiles);
			if (adjacentRoadTiles == 3 && Terrain_getSurroundingRoadTilesForRoaming(f->gridOffset, roadTiles) >= 5) {
				// go in the straight direction of a double-wide road
				adjacentRoadTiles = 2;
				if (cameFromDirection == Dir_0_Top || cameFromDirection == Dir_4_Bottom) {
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
				if (cameFromDirection == Dir_0_Top || cameFromDirection == Dir_4_Bottom) {
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
			figureMoveToNextTile(figureId, f);
			FigureMovement_advanceTick(f);
		}
	}
}

static void figureSetNextRouteTileDirection(int figureId, struct Data_Figure *f)
{
	if (f->routingPathId > 0) {
		if (f->routingPathCurrentTile < f->routingPathLength) {
			f->direction = figure_route_get_direction(f->routingPathId, f->routingPathCurrentTile);
		} else {
			figure_route_remove(figureId);
			f->direction = DirFigure_8_AtDestination;
		}
	} else { // should be at destination
		f->direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
		if (f->direction != DirFigure_8_AtDestination) {
			f->direction = DirFigure_10_Lost;
		}
	}
}

static void figureAdvanceRouteTile(struct Data_Figure *f, int roamingEnabled)
{
	if (f->direction >= 8) {
		return;
	}
	int targetGridOffset = f->gridOffset + map_grid_direction_delta(f->direction);
	int targetTerrain = Data_Grid_terrain[targetGridOffset] & Terrain_c75f;
	if (f->isBoat) {
		if (!(targetTerrain & Terrain_Water)) {
			f->direction = DirFigure_9_Reroute;
		}
	} else if (f->terrainUsage == FigureTerrainUsage_Enemy) {
		if (!map_routing_noncitizen_is_passable(targetGridOffset)) {
			f->direction = DirFigure_9_Reroute;
		} else if (map_routing_is_destroyable(targetGridOffset)) {
			int causeDamage = 1;
			int maxDamage = 0;
			switch (map_routing_get_destroyable(targetGridOffset)) {
				case DESTROYABLE_BUILDING:
					maxDamage = 10;
					break;
				case DESTROYABLE_AQUEDUCT_GARDEN:
					if (Data_Grid_terrain[targetGridOffset] & Terrain_GardenAccessRampRubble) {
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
				f->direction = DirFigure_11_Attack;
				if (!(game_time_tick() & 3)) {
					Building_increaseDamageByEnemy(targetGridOffset, maxDamage);
				}
			}
		}
	} else if (f->terrainUsage == FigureTerrainUsage_Walls) {
		if (!map_routing_is_wall_passable(targetGridOffset)) {
			f->direction = DirFigure_9_Reroute;
		}
	} else if (targetTerrain & (Terrain_Road | Terrain_AccessRamp)) {
		if (roamingEnabled && targetTerrain & Terrain_Building) {
			if (Data_Buildings[Data_Grid_buildingIds[targetGridOffset]].type == BUILDING_GATEHOUSE) {
				// do not allow roaming through gatehouse
				f->direction = DirFigure_9_Reroute;
			}
		}
	} else if (targetTerrain & Terrain_Building) {
		int type = Data_Buildings[Data_Grid_buildingIds[targetGridOffset]].type;
		switch (type) {
			case BUILDING_WAREHOUSE:
			case BUILDING_GRANARY:
			case BUILDING_TRIUMPHAL_ARCH:
			case BUILDING_FORT_GROUND:
				break; // OK to walk
			default:
				f->direction = DirFigure_9_Reroute;
		}
	} else if (targetTerrain) {
		f->direction = DirFigure_9_Reroute;
	}
}

static void FigureMovement_walkTicksInternal(int figureId, int numTicks, int roamingEnabled)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	while (numTicks > 0) {
		numTicks--;
		f->progressOnTile++;
		if (f->progressOnTile < 15) {
			FigureMovement_advanceTick(f);
		} else {
			Figure_provideServiceCoverage(figureId);
			f->progressOnTile = 15;
			if (f->routingPathId <= 0) {
				figure_route_add(figureId);
			}
			figureSetNextRouteTileDirection(figureId, f);
			figureAdvanceRouteTile(f, roamingEnabled);
			if (f->direction >= 8) {
				break;
			}
			f->routingPathCurrentTile++;
			f->previousTileDirection = f->direction;
			f->progressOnTile = 0;
			figureMoveToNextTile(figureId, f);
			FigureMovement_advanceTick(f);
		}
	}
}

void FigureMovement_walkTicks(int figureId, int numTicks)
{
	FigureMovement_walkTicksInternal(figureId, numTicks, 0);
}

void FigureMovement_followTicks(int figureId, int leaderFigureId, int numTicks)
{
	struct Data_Figure *f = &Data_Figures[figureId];
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
				Data_Figures[leaderFigureId].previousTileX,
				Data_Figures[leaderFigureId].previousTileY);
			if (f->direction >= 8) {
				break;
			}
			f->previousTileDirection = f->direction;
			f->progressOnTile = 0;
			figureMoveToNextTile(figureId, f);
			FigureMovement_advanceTick(f);
		}
	}
}

void FigureMovement_walkTicksTowerSentry(int figureId, int numTicks)
{
	struct Data_Figure *f = &Data_Figures[figureId];
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

void FigureMovement_crossCountrySetDirection(int figureId, int xSrc, int ySrc, int xDst, int yDst, int isMissile)
{
	// all x/y are in 1/15th of a tile
	struct Data_Figure *f = &Data_Figures[figureId];
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
				case Dir_1_TopRight: case Dir_7_TopLeft: f->direction = Dir_0_Top; break;
				case Dir_3_BottomRight: case Dir_5_BottomLeft: f->direction = Dir_4_Bottom; break;
			}
		}
		if (f->ccDeltaX > 2 * f->ccDeltaY) {
			switch (f->direction) {
				case Dir_1_TopRight: case Dir_3_BottomRight: f->direction = Dir_2_Right; break;
				case Dir_5_BottomLeft: case Dir_7_TopLeft: f->direction = Dir_6_Left; break;
			}
		}
	}
	if (f->ccDeltaX >= f->ccDeltaY) {
		f->ccDirection = 1;
	} else {
		f->ccDirection = 2;
	}
}

static void crossCountryUpdateDelta(struct Data_Figure *f)
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

static void crossCountryAdvanceX(struct Data_Figure *f)
{
	if (f->crossCountryX < f->ccDestinationX) {
		f->crossCountryX++;
	} else if (f->crossCountryX > f->ccDestinationX) {
		f->crossCountryX--;
	}
}

static void crossCountryAdvanceY(struct Data_Figure *f)
{
	if (f->crossCountryY < f->ccDestinationY) {
		f->crossCountryY++;
	} else if (f->crossCountryY > f->ccDestinationY) {
		f->crossCountryY--;
	}
}

static void crossCountryAdvance(struct Data_Figure *f)
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

int FigureMovement_crossCountryWalkTicks(int figureId, int numTicks)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	Figure_removeFromTileList(figureId);
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
	f->gridOffset = GridOffset(f->x, f->y);
	if (Data_Grid_terrain[f->gridOffset] & Terrain_Building) {
		f->inBuildingWaitTicks = 8;
	} else if (f->inBuildingWaitTicks) {
		f->inBuildingWaitTicks--;
	}
	Figure_addToTileList(figureId);
	return isAtDestination;
}

int FigureMovement_canLaunchCrossCountryMissile(int xSrc, int ySrc, int xDst, int yDst)
{
	int height = 0;
	struct Data_Figure *f = &Data_Figures[0];
	f->crossCountryX = 15 * xSrc;
	f->crossCountryY = 15 * ySrc;
	if (Data_Grid_terrain[GridOffset(xSrc, ySrc)] & Terrain_WallOrGatehouse) {
		height = 6;
	}
	FigureMovement_crossCountrySetDirection(0, 15 * xSrc, 15 * ySrc, 15 * xDst, 15 * yDst, 0);

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
			int gridOffset = GridOffset(f->x, f->y);
			if (Data_Grid_terrain[gridOffset] & (Terrain_Wall | Terrain_Gatehouse | Terrain_Tree)) {
				break;
			}
			if (Data_Grid_terrain[gridOffset] & Terrain_Building && Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
				break;
			}
		}
	}
	return 0;
}
