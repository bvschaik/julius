#include "WalkerMovement.h"

#include "Building.h"
#include "Routing.h"
#include "Terrain.h"
#include "Walker.h"
#include "WalkerAction.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Grid.h"
#include "Data/Routes.h"
#include "Data/Settings.h"
#include "Data/Walker.h"

static int roamingEnabled = 0;

void WalkerMovement_advanceTick(struct Data_Walker *w)
{
	switch (w->direction) {
		case 0:
			w->crossCountryY--;
			break;
		case 1:
			w->crossCountryX++;
			w->crossCountryY--;
			break;
		case 2:
			w->crossCountryX++;
			break;
		case 3:
			w->crossCountryX++;
			w->crossCountryY++;
			break;
		case 4:
			w->crossCountryY++;
			break;
		case 5:
			w->crossCountryX--;
			w->crossCountryY++;
			break;
		case 6:
			w->crossCountryX--;
			break;
		case 7:
			w->crossCountryX--;
			w->crossCountryY--;
			break;
		default:
			break;
	}
	if (w->heightFromGround) {
		w->heightFromGround--;
		if (w->heightFromGround > 0) {
			w->isGhost = 1;
			if (w->currentHeight < w->targetHeight) {
				w->currentHeight++;
			}
			if (w->currentHeight > w->targetHeight) {
				w->currentHeight--;
			}
		} else {
			w->isGhost = 0;
		}
	} else {
		if (w->currentHeight) {
			w->currentHeight--;
		}
	}
}

static void setTargetHeightBridge(struct Data_Walker *w)
{
	w->heightFromGround = 18;
	if (Data_Grid_spriteOffsets[w->gridOffset] <= 6) {
		switch (Data_Grid_spriteOffsets[w->gridOffset]) {
			case 1: case 4: w->targetHeight = 10; break;
			case 2: case 3: w->targetHeight = 16; break;
			default: w->targetHeight = 20; break;
		}
	} else {
		switch (Data_Grid_spriteOffsets[w->gridOffset]) {
			case 7: case 8: case 9: case 10: w->targetHeight = 14; break;
			case 13: w->targetHeight = 30;
			default: w->targetHeight = 36;
		}
	}
}

static void walkerMoveToNextTile(int walkerId, struct Data_Walker *w)
{
	int oldX = w->x;
	int oldY = w->y;
	Walker_removeFromTileList(walkerId);
	switch (w->direction) {
		default:
			return;
		case 0:
			w->y--;
			w->gridOffset -= 162;
			break;
		case 1:
			w->x++; w->y--;
			w->gridOffset -= 161;
			break;
		case 2:
			w->x++;
			w->gridOffset += 1;
			break;
		case 3:
			w->x++; w->y++;
			w->gridOffset += 163;
			break;
		case 4:
			w->y++;
			w->gridOffset += 162;
			break;
		case 5:
			w->x--; w->y++;
			w->gridOffset += 161;
			break;
		case 6:
			w->x--;
			w->gridOffset -= 1;
			break;
		case 7:
			w->x--; w->y--;
			w->gridOffset -= 163;
			break;
	}
	Walker_addToTileList(walkerId);
	if (Data_Grid_terrain[w->gridOffset] & Terrain_Road) {
		w->isOnRoad = 1;
		if (Data_Grid_terrain[w->gridOffset] & Terrain_Water) { // bridge
			setTargetHeightBridge(w);
		}
	} else {
		w->isOnRoad = 0;
	}
	WalkerAction_Combat_attackWalker(walkerId, Data_Grid_walkerIds[w->gridOffset]);
	w->previousTileX = oldX;
	w->previousTileY = oldY;
}

void WalkerMovement_initRoaming(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	w->progressOnTile = 15;
	w->roamChooseDestination = 0;
	w->roamTicksUntilNextTurn = -1;
	w->roamTurnDirection = 2;
	int roamDir = b->walkerRoamDirection;
	b->walkerRoamDirection += 2;
	if (b->walkerRoamDirection > 6) {
		b->walkerRoamDirection = 0;
	}
	int x = b->x;
	int y = b->y;
	switch (roamDir) {
		case 0: y -= 8; break;
		case 2: x += 8; break;
		case 4: y += 8; break;
		case 6: x -= 8; break;
	}
	BoundToMap(x, y);
	int xRoad, yRoad;
	if (Terrain_getClosestRoadWithinRadius(x, y, 1, 6, &xRoad, &yRoad)) {
		w->destinationX = xRoad;
		w->destinationY = yRoad;
	} else {
		w->roamChooseDestination = 1;
	}
}

static void roamSetDirection(struct Data_Walker *w)
{
	int gridOffset = GridOffset(w->x, w->y);
	int direction = Routing_getGeneralDirection(w->x, w->y, w->destinationX, w->destinationY);
	if (direction >= 8) {
		direction = 0;
	}
	int roadOffsetDir1 = 0;
	int roadDir1;
	for (int i = 0, dir = direction; i < 8; i++) {
		if (dir % 2 == 0 && Data_Grid_terrain[gridOffset + Constant_DirectionGridOffsets[dir]] & Terrain_Road) {
			roadDir1 = dir;
			break;
		}
		dir++;
		if (dir > 7) dir = 0;
		roadOffsetDir1++;
	}
	int roadOffsetDir2 = 0;
	int roadDir2;
	for (int i = 0, dir = direction; i < 8; i++) {
		if (dir % 2 == 0 && Data_Grid_terrain[gridOffset + Constant_DirectionGridOffsets[dir]] & Terrain_Road) {
			roadDir2 = dir;
			break;
		}
		dir--;
		if (dir < 0) dir = 7;
		roadOffsetDir2++;
	}
	if (roadOffsetDir1 <= roadOffsetDir2) {
		w->direction = roadDir1;
		w->roamTurnDirection = 2;
	} else {
		w->direction = roadDir2;
		w->roamTurnDirection = -2;
	}
	w->roamTicksUntilNextTurn = 5;
}

void WalkerMovement_roamTicks(int walkerId, int numTicks)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	if (w->roamChooseDestination == 0) {
		roamingEnabled = 1;
		WalkerMovement_walkTicks(walkerId, numTicks);
		if (w->direction == 8) {
			w->roamChooseDestination = 1;
			w->roamLength = 0;
		} else if (w->direction == 9 || w->direction == 10) {
			w->roamChooseDestination = 1;
		}
		if (w->roamChooseDestination) {
			w->roamTicksUntilNextTurn = 100;
			w->direction = w->previousTileDirection;
		} else {
			return;
		}
	}
	// no destination: walk to end of tile and pick a direction
	while (numTicks > 0) {
		numTicks--;
		w->progressOnTile++;
		if (w->progressOnTile < 15) {
			WalkerMovement_advanceTick(w);
		} else {
			w->progressOnTile = 15;
			w->roamRandomCounter++;
			int cameFromDirection = (w->previousTileDirection + 4) % 8;
			if (Walker_provideServiceCoverage(walkerId)) {
				return;
			}
			int roadTiles[8];
			int adjacentRoadTiles = Terrain_getAdjacentRoadTilesForRoaming(w->gridOffset, roadTiles);
			if (adjacentRoadTiles == 3 && Terrain_getSurroundingRoadTilesForRoaming(w->gridOffset, roadTiles) >= 5) {
				// go in the straight direction of a double-wide road
				adjacentRoadTiles = 2;
				if (cameFromDirection == 0 || cameFromDirection == 4) {
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
			if (adjacentRoadTiles == 4 && Terrain_getSurroundingRoadTilesForRoaming(w->gridOffset, roadTiles) >= 8) {
				// go straight on when all surrounding tiles are road
				adjacentRoadTiles = 2;
				if (cameFromDirection == 0 || cameFromDirection == 4) {
					roadTiles[2] = roadTiles[6] = 0;
				} else {
					roadTiles[0] = roadTiles[4] = 0;
				}
			}
			if (adjacentRoadTiles <= 0) {
				w->roamLength = w->maxRoamLength; // end roaming walk
				return;
			}
			if (adjacentRoadTiles == 1) {
				int dir = 0;
				do {
					w->direction = 2 * dir;
				} while (!roadTiles[w->direction] && dir++ < 4);
			} else if (adjacentRoadTiles == 2) {
				if (w->roamTicksUntilNextTurn == -1) {
					roamSetDirection(w);
					cameFromDirection = -1;
				}
				// 1. continue in the same direction
				// 2. turn in the direction given by roamTurnDirection
				int dir = 0;
				do {
					if (roadTiles[w->direction] && w->direction != cameFromDirection) {
						break;
					}
					w->direction += w->roamTurnDirection;
					if (w->direction > 6) w->direction = 0;
					if (w->direction < 0) w->direction = 6;
				} while (dir++ < 4);
			} else { // > 2 road tiles
				w->direction = (w->roamRandomCounter + Data_Grid_random[w->gridOffset]) & 6;
				if (!roadTiles[w->direction] || w->direction == cameFromDirection) {
					w->roamTicksUntilNextTurn--;
					if (w->roamTicksUntilNextTurn <= 0) {
						roamSetDirection(w);
						cameFromDirection = -1;
					}
					int dir = 0;
					do {
						if (roadTiles[w->direction] && w->direction != cameFromDirection) {
							break;
						}
						w->direction += w->roamTurnDirection;
						if (w->direction > 6) w->direction = 0;
						if (w->direction < 0) w->direction = 6;
					} while (dir++ < 4);
				}
			}
			w->routingPathCurrentTile++;
			w->previousTileDirection = w->direction;
			w->progressOnTile = 0;
			walkerMoveToNextTile(walkerId, w);
			WalkerMovement_advanceTick(w);
		}
	}
}

static void walkerSetNextRouteTileDirection(int walkerId, struct Data_Walker *w)
{
	if (w->routingPathId > 0) {
		if (w->routingPathCurrentTile < w->routingPathLength) {
			w->direction = Data_Routes.directionPaths[w->routingPathId][w->routingPathCurrentTile];
		} else {
			WalkerRoute_remove(walkerId);
			w->direction = 8;
		}
	} else { // should be at destination
		w->direction = Routing_getGeneralDirection(w->x, w->y, w->destinationX, w->destinationY);
		if (w->direction != 8) {
			w->direction = 10;
		}
	}
}

static void walkerAdvanceRouteTile(struct Data_Walker *w)
{
	if (w->direction >= 8) {
		return;
	}
	int targetGridOffset = w->gridOffset + Constant_DirectionGridOffsets[w->direction];
	int targetTerrain = Data_Grid_terrain[targetGridOffset] & 0xc75f;
	if (w->isBoat) {
		if (!(targetTerrain & Terrain_Water)) {
			w->direction = 9;
		}
	} else if (w->terrainUsage == WalkerTerrainUsage_Enemy) {
		int groundType = Data_Grid_routingLandNonCitizen[targetGridOffset];
		if (groundType < 0) {
			w->direction = 9;
		} else if (groundType > 0 && groundType != 5) {
			w->attackDirection = w->direction;
			w->direction = 11;
			int damage;
			switch (groundType) {
				case 1: damage = 10; break;
				case 2: damage = 10; break;
				case 3: damage = 200; break;
				default: damage = 150; break;
			}
			if (!(Data_CityInfo_Extra.gameTimeTick & 3)) {
				Building_increaseDamageByEnemy(targetGridOffset, damage);
			}
		}
	} else if (w->terrainUsage == WalkerTerrainUsage_Walls) {
		if (Data_Grid_routingWalls[targetGridOffset] < 0) {
			w->direction = 9;
		}
	} else if (targetTerrain & (Terrain_Road | Terrain_AccessRamp)) {
		if (roamingEnabled && targetTerrain & Terrain_Building) {
			if (Data_Buildings[Data_Grid_buildingIds[targetGridOffset]].type == Building_Gatehouse) {
				// do not allow roaming through gatehouse
				w->direction = 9;
			}
		}
	} else if (targetTerrain & Terrain_Building) {
		int type = Data_Buildings[Data_Grid_buildingIds[targetGridOffset]].type;
		switch (type) {
			case Building_Warehouse:
			case Building_Granary:
			case Building_TriumphalArch:
			case Building_FortGround:
				break; // OK to walk
			default:
				w->direction = 9;
		}
	} else if (targetTerrain) {
		w->direction = 9;
	}
}

void WalkerMovement_walkTicks(int walkerId, int numTicks)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	while (numTicks > 0) {
		numTicks--;
		w->progressOnTile++;
		if (w->progressOnTile < 15) {
			WalkerMovement_advanceTick(w);
		} else {
			Walker_provideServiceCoverage(walkerId);
			w->progressOnTile = 15;
			if (w->routingPathId <= 0) {
				WalkerRoute_add(walkerId);
			}
			walkerSetNextRouteTileDirection(walkerId, w);
			walkerAdvanceRouteTile(w);
			if (w->direction >= 8) {
				break;
			}
			w->routingPathCurrentTile++;
			w->previousTileDirection = w->direction;
			w->progressOnTile = 0;
			walkerMoveToNextTile(walkerId, w);
			WalkerMovement_advanceTick(w);
		}
	}
	roamingEnabled = 0;
}

void WalkerMovement_followTicks(int walkerId, int leaderWalkerId, int numTicks)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	if (w->x == w->sourceX && w->y == w->sourceY) {
		w->isGhost = 1;
	}
	while (numTicks > 0) {
		numTicks--;
		w->progressOnTile++;
		if (w->progressOnTile < 15) {
			WalkerMovement_advanceTick(w);
		} else {
			w->progressOnTile = 15;
			w->direction = Routing_getGeneralDirection(w->x, w->y,
				Data_Walkers[leaderWalkerId].previousTileX,
				Data_Walkers[leaderWalkerId].previousTileY);
			if (w->direction < 8) {
				w->previousTileDirection = w->direction;
				w->progressOnTile = 0;
				walkerMoveToNextTile(walkerId, w);
				WalkerMovement_advanceTick(w);
			}
		}
	}
}

void WalkerMovement_walkTicksTowerSentry(int walkerId, int numTicks)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	while (numTicks > 0) {
		numTicks--;
		w->progressOnTile++;
		if (w->progressOnTile < 15) {
			WalkerMovement_advanceTick(w);
		} else {
			w->progressOnTile = 15;
		}
	}
}

void WalkerMovement_crossCountrySetDirection(int walkerId, int xSrc, int ySrc, int xDst, int yDst, int isProjectile)
{
	// all x/y are in 1/15th of a tile
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->ccDestinationX = xDst;
	w->ccDestinationY = yDst;
	w->ccDeltaX = (xSrc > xDst) ? (xSrc - xDst) : (xDst - xSrc);
	w->ccDeltaY = (ySrc > yDst) ? (ySrc - yDst) : (yDst - ySrc);
	if (w->ccDeltaX < w->ccDeltaY) {
		w->ccDeltaXY = 2 * w->ccDeltaX - w->ccDeltaY;
	} else if (w->ccDeltaY < w->ccDeltaX) {
		w->ccDeltaXY = 2 * w->ccDeltaY - w->ccDeltaX;
	} else { // equal
		w->ccDeltaXY = 0;
	}
	if (isProjectile) {
		w->direction = Routing_getDirectionForMissile(xSrc, ySrc, xDst, yDst);
	} else {
		w->direction = Routing_getGeneralDirection(xSrc, ySrc, xDst, yDst);
		if (w->ccDeltaY > 2 * w->ccDeltaX) {
			switch (w->direction) {
				case 1: case 7: w->direction = 0; break;
				case 3: case 5: w->direction = 4; break;
			}
		}
		if (w->ccDeltaX > 2 * w->ccDeltaY) {
			switch (w->direction) {
				case 1: case 3: w->direction = 2; break;
				case 5: case 7: w->direction = 6; break;
			}
		}
	}
	if (w->ccDeltaX >= w->ccDeltaY) {
		w->ccDirection = 1;
	} else {
		w->ccDirection = 2;
	}
}

static void crossCountryUpdateDelta(struct Data_Walker *w)
{
	if (w->ccDirection == 1) { // x
		if (w->ccDeltaXY >= 0) {
			w->ccDeltaXY += 2 * (w->ccDeltaY - w->ccDeltaX);
		} else {
			w->ccDeltaXY += 2 * w->ccDeltaY;
		}
		w->ccDeltaX--;
	} else { // y
		if (w->ccDeltaXY >= 0) {
			w->ccDeltaXY += 2 * (w->ccDeltaX - w->ccDeltaY);
		} else {
			w->ccDeltaXY += 2 * w->ccDeltaX;
		}
		w->ccDeltaY--;
	}
}

static void crossCountryAdvanceX(struct Data_Walker *w)
{
	if (w->crossCountryX < w->ccDestinationX) {
		w->crossCountryX++;
	} else if (w->crossCountryX > w->ccDestinationX) {
		w->crossCountryX--;
	}
}

static void crossCountryAdvanceY(struct Data_Walker *w)
{
	if (w->crossCountryY < w->ccDestinationY) {
		w->crossCountryY++;
	} else if (w->crossCountryY > w->ccDestinationY) {
		w->crossCountryY--;
	}
}

static void crossCountryAdvance(struct Data_Walker *w)
{
	crossCountryUpdateDelta(w);
	if (w->ccDirection == 2) { // y
		crossCountryAdvanceY(w);
		if (w->ccDeltaXY >= 0) {
			w->ccDeltaX--;
			crossCountryAdvanceX(w);
		}
	} else {
		crossCountryAdvanceX(w);
		if (w->ccDeltaXY >= 0) {
			w->ccDeltaY--;
			crossCountryAdvanceY(w);
		}
	}
}

int WalkerMovement_crossCountryWalkTicks(int walkerId, int numTicks)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	Walker_removeFromTileList(walkerId);
	int isAtDestination = 0;
	while (numTicks > 0) {
		numTicks--;
		if (w->missileDamage <= 0) {
			w->missileDamage = 0;
		} else {
			w->missileDamage--;
		}
		if (w->ccDeltaX + w->ccDeltaY <= 0) {
			isAtDestination = 1;
			break;
		}
		crossCountryAdvance(w);
	}
	w->x = w->crossCountryX / 15;
	w->y = w->crossCountryY / 15;
	w->gridOffset = GridOffset(w->x, w->y);
	if (Data_Grid_terrain[w->gridOffset] & Terrain_Building) {
		w->inBuildingWaitTicks = 8;
	} else if (w->inBuildingWaitTicks) {
		w->inBuildingWaitTicks--;
	}
	Walker_addToTileList(walkerId);
	return isAtDestination;
}

int WalkerMovement_canLaunchCrossCountryMissile(int xSrc, int ySrc, int xDst, int yDst)
{
	int height = 0;
	struct Data_Walker *w = &Data_Walkers[0];
	w->crossCountryX = 15 * xSrc;
	w->crossCountryY = 15 * ySrc;
	if (Data_Grid_terrain[GridOffset(xSrc, ySrc)] & (Terrain_Wall | Terrain_Gatehouse)) {
		height = 6;
	}
	WalkerMovement_crossCountrySetDirection(0, 15 * xSrc, 15 * ySrc, 15 * xDst, 15 * yDst, 0);

	for (int guard = 0; guard < 1000; guard++) {
		for (int i = 0; i < 8; i++) {
			if (w->ccDeltaX + w->ccDeltaY <= 0) {
				return 1;
			}
			crossCountryAdvance(w);
		}
		w->x = w->crossCountryX / 15;
		w->y = w->crossCountryY / 15;
		if (height) {
			height--;
		} else {
			int gridOffset = GridOffset(w->x, w->y);
			if (Data_Grid_terrain[gridOffset] & (Terrain_Wall | Terrain_Gatehouse | Terrain_Tree | Terrain_Building)) {
				break;
			}
			if (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
				break;
			}
		}
	}
	return 0;
}
