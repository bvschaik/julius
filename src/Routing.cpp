//#include "Routing.h"

#include "Calc.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Random.h"
#include "Data/Routes.h"
#include "Data/Settings.h"
#include "Data/Walker.h"

#include <string.h>

#define MAX_QUEUE 26243

static struct {
	int items[MAX_QUEUE];
	int head;
	int tail;
} queue;

static int directionPath[500];

static char tmpGrid[GRID_SIZE * GRID_SIZE];

#define GRID_OFFSET(x,y) (Data_Settings_Map.gridStartOffset + (x) + (y) * GRID_SIZE)

#define SET_DIST_AND_ENQUEUE() \
	Data_Grid_routingDistance[nextOffset] = dist;\
	queue.items[queue.tail++] = nextOffset; \
	if (queue.tail > MAX_QUEUE) queue.tail = 0;

#define ROUTE_QUEUE(source, dest, block) \
	memset(Data_Grid_routingDistance, 0, GRID_SIZE * GRID_SIZE * 2);\
	Data_Grid_routingDistance[source] = 1;\
	queue.items[0] = source;\
	queue.head = 0;\
	queue.tail = 1;\
	while (queue.head != queue.tail) {\
		int offset = queue.items[queue.head];\
		if (offset == dest) break;\
		int dist = 1 + Data_Grid_routingDistance[offset];\
		int nextOffset = offset - 162;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset + 1;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset + 162;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset - 1;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		if (++queue.head > MAX_QUEUE) queue.head = 0;\
	}

#define ROUTE_QUEUE_MAX(source, dest, max, block) \
	memset(Data_Grid_routingDistance, 0, GRID_SIZE * GRID_SIZE * 2);\
	Data_Grid_routingDistance[source] = 1;\
	queue.items[0] = source;\
	queue.head = 0;\
	queue.tail = 1;\
	int tiles = 0;\
	while (queue.head != queue.tail) {\
		int offset = queue.items[queue.head];\
		if (offset == dest) break;\
		if (++tiles > max) break;\
		int dist = 1 + Data_Grid_routingDistance[offset];\
		int nextOffset = offset - 162;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset + 1;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset + 162;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset - 1;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		if (++queue.head > MAX_QUEUE) queue.head = 0;\
	}

#define ROUTE_QUEUE_BOAT(source, block) \
	memset(Data_Grid_routingDistance, 0, GRID_SIZE * GRID_SIZE * 2);\
	memset(tmpGrid, 0, GRID_SIZE * GRID_SIZE);\
	Data_Grid_routingDistance[source] = 1;\
	queue.items[0] = source;\
	queue.head = 0;\
	queue.tail = 1;\
	int tiles = 0;\
	while (queue.head != queue.tail) {\
		int offset = queue.items[queue.head];\
		if (++tiles > 50000) break;\
		int drag = Data_Grid_routingWater[offset] == -2 ? 4 : 0;\
		if (drag && tmpGrid[offset]++ < drag) {\
			queue.items[queue.tail++] = offset; \
			if (queue.tail > MAX_QUEUE) queue.tail = 0;\
		} else {\
			int dist = 1 + Data_Grid_routingDistance[offset];\
			int nextOffset = offset - 162;\
			if (!Data_Grid_routingDistance[nextOffset]) {\
				block;\
			}\
			nextOffset = offset + 1;\
			if (!Data_Grid_routingDistance[nextOffset]) {\
				block;\
			}\
			nextOffset = offset + 162;\
			if (!Data_Grid_routingDistance[nextOffset]) {\
				block;\
			}\
			nextOffset = offset - 1;\
			if (!Data_Grid_routingDistance[nextOffset]) {\
				block;\
			}\
		}\
		if (++queue.head > MAX_QUEUE) queue.head = 0;\
	}

#define ROUTE_QUEUE_DIR8(source, block) \
	memset(Data_Grid_routingDistance, 0, GRID_SIZE * GRID_SIZE * 2);\
	Data_Grid_routingDistance[source] = 1;\
	queue.items[0] = source;\
	queue.head = 0;\
	queue.tail = 1;\
	int tiles = 0;\
	while (queue.head != queue.tail) {\
		if (++tiles > 50000) break;\
		int offset = queue.items[queue.head];\
		int dist = 1 + Data_Grid_routingDistance[offset];\
		int nextOffset = offset - 162;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset + 1;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset + 162;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset - 1;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset - 161;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset + 163;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset + 161;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		nextOffset = offset - 163;\
		if (!Data_Grid_routingDistance[nextOffset]) {\
			block;\
		}\
		if (++queue.head > MAX_QUEUE) queue.head = 0;\
	}

void Routing_determineLandCitizen()
{
	memset(Data_Grid_routingLandCitizen, -1, GRID_SIZE * GRID_SIZE);
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
				Data_Grid_routingLandCitizen[gridOffset] = 0;
			} else if (Data_Grid_terrain[gridOffset] & 0x1420) { // rubble, access ramp, garden
				Data_Grid_routingLandCitizen[gridOffset] = 2;
			} else if (Data_Grid_terrain[gridOffset] & (Terrain_Building | Terrain_Gatehouse)) {
				int buildingId = Data_Grid_buildingIds[gridOffset];
				if (!buildingId) {
					// shouldn't happen - correct
					Data_Grid_routingLandNonCitizen[gridOffset] = 4; // BUGFIX - should be citizen?
					Data_Grid_terrain[gridOffset] &= 0xfff7; // remove 8 = building
					Data_Grid_graphicIds[gridOffset] = (Data_Grid_random[gridOffset] & 7) + GraphicId(ID_Graphic_TerrainGrass1);
					Data_Grid_edge[gridOffset] = Edge_LeftmostTile;
					Data_Grid_bitfields[gridOffset] &= 0xf0; // remove sizes
					continue;
				}
				int land = -1;
				switch (Data_Buildings[buildingId].type) {
					case Building_Warehouse:
					case Building_Gatehouse:
						land = 0;
						break;
					case Building_FortGround:
						land = 2;
						break;
					case Building_TriumphalArch:
						if (Data_Buildings[buildingId].subtype.orientation == 3) {
							switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
								case Edge_X0Y1:
								case Edge_X1Y1:
								case Edge_X2Y1:
									land = 0;
									break;
							}
						} else {
							switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
								case Edge_X1Y0:
								case Edge_X1Y1:
								case Edge_X1Y2:
									land = 0;
									break;
							}
						}
						break;
					case Building_Granary:
						switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
							case Edge_X1Y0:
							case Edge_X0Y1:
							case Edge_X1Y1:
							case Edge_X2Y1:
							case Edge_X1Y2:
								land = 0;
								break;
						}
						break;
					case Building_Reservoir:
						switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
							case Edge_X1Y0:
							case Edge_X0Y1:
							case Edge_X2Y1:
							case Edge_X1Y2:
								land = -4; // aqueduct connect points
								break;
						}
						break;
				}
				Data_Grid_routingLandCitizen[gridOffset] = land;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
				int graphicId = Data_Grid_graphicIds[gridOffset] - GraphicId(ID_Graphic_Aqueduct);
				int land;
				if (graphicId <= 3) {
					land = -3;
				} else if (graphicId <= 7) {
					land = -1;
				} else if (graphicId <= 9) {
					land = -3;
				} else if (graphicId <= 14) {
					land = -1;
				} else if (graphicId <= 18) {
					land = -3;
				} else if (graphicId <= 22) {
					land = -1;
				} else if (graphicId <= 24) {
					land = -3;
				} else {
					land = -1;
				}
				Data_Grid_routingLandCitizen[gridOffset] = land;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
				Data_Grid_routingLandCitizen[gridOffset] = -1;
			} else {
				Data_Grid_routingLandCitizen[gridOffset] = 4;
			}
		}
	}
}

void Routing_determineLandNonCitizen()
{
	memset(Data_Grid_routingLandNonCitizen, -1, GRID_SIZE * GRID_SIZE);
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			int terrain = Data_Grid_terrain[gridOffset] & Terrain_NotClear;
			if (Data_Grid_terrain[gridOffset] & Terrain_Gatehouse) {
				Data_Grid_routingLandNonCitizen[gridOffset] = 4;
			} else if (terrain & Terrain_Road) {
				Data_Grid_routingLandNonCitizen[gridOffset] = 0;
			} else if (terrain & 0x1420) {
				Data_Grid_routingLandNonCitizen[gridOffset] = 2;
			} else if (terrain & Terrain_Building) {
				int land = 1;
				switch (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type) {
					case Building_Warehouse:
					case Building_FortGround:
						land = 0;
						break;
					case Building_BurningRuin:
					case Building_NativeHut:
					case Building_NativeMeeting:
					case Building_NativeCrops:
						land = -1;
						break;
					case Building_FortGround__:
						land = 5;
						break;
					case Building_Granary:
						switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
							case Edge_X1Y0:
							case Edge_X0Y1:
							case Edge_X1Y1:
							case Edge_X2Y1:
							case Edge_X1Y2:
								land = 0;
								break;
						}
						break;
				}
				Data_Grid_routingLandNonCitizen[gridOffset] = land;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
				Data_Grid_routingLandNonCitizen[gridOffset] = 2;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_Wall) {
				Data_Grid_routingLandNonCitizen[gridOffset] = 3;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
				Data_Grid_routingLandNonCitizen[gridOffset] = -1;
			} else {
				Data_Grid_routingLandNonCitizen[gridOffset] = 0;
			}
		}
	}
}

void Routing_determineWater()
{
	memset(Data_Grid_routingWalls, -1, GRID_SIZE * GRID_SIZE);
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
				if ((Data_Grid_terrain[gridOffset - 162] & Terrain_Water) &&
					(Data_Grid_terrain[gridOffset - 1] & Terrain_Water) &&
					(Data_Grid_terrain[gridOffset + 1] & Terrain_Water) &&
					(Data_Grid_terrain[gridOffset + 162] & Terrain_Water)) {
					if (x > 0 && x < Data_Settings_Map.width - 1 &&
						y > 0 && y < Data_Settings_Map.height - 1) {
						switch (Data_Grid_spriteOffsets[gridOffset]) {
							case 5:
							case 6:
								Data_Grid_routingWater[gridOffset] = -3;
								break;
							case 13:
								Data_Grid_routingWater[gridOffset] = -1;
								break;
							default:
								Data_Grid_routingWater[gridOffset] = 0;
								break;
						}
					} else {
						Data_Grid_routingWater[gridOffset] = -2;
					}
				} else {
					Data_Grid_routingWater[gridOffset] = -1;
				}
			} else {
				Data_Grid_routingWater[gridOffset] = -1;
			}
		}
	}
}

#define WALL_GATE 0xc000
void Routing_determineWalls()
{
	memset(Data_Grid_routingWalls, -1, GRID_SIZE * GRID_SIZE);
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Wall) {
				int adjacent = 0;
				switch (Data_Settings_Map.orientation) {
					case Direction_Top:
						if (Data_Grid_terrain[gridOffset + 162] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset + 163] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset + 1] & WALL_GATE) {
							adjacent++;
						}
						break;
					case Direction_Right:
						if (Data_Grid_terrain[gridOffset + 162] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset + 161] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset - 1] & WALL_GATE) {
							adjacent++;
						}
						break;
					case Direction_Bottom:
						if (Data_Grid_terrain[gridOffset - 162] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset - 163] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset - 1] & WALL_GATE) {
							adjacent++;
						}
						break;
					case Direction_Left:
						if (Data_Grid_terrain[gridOffset - 162] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset - 161] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset + 1] & WALL_GATE) {
							adjacent++;
						}
						break;
				}
				if (adjacent == 3) {
					Data_Grid_routingWalls[gridOffset] = 0;
				} else {
					Data_Grid_routingWalls[gridOffset] = 0;
				}
			} else if (Data_Grid_terrain[gridOffset] & Terrain_Gatehouse) {
				Data_Grid_routingWalls[gridOffset] = 0;
			} else {
				Data_Grid_routingWalls[gridOffset] = -1;
			}
		}
	}
}

void Routing_clearLandTypeCitizen()
{
	int gridOffset = 0;
	for (int y = 0; y < GRID_SIZE; y++) {
		for (int x = 0; x < GRID_SIZE; x++) {
			Data_Grid_routingLandCitizen[gridOffset++] = -1;
		}
	}
}

void Routing_getDistance(int x, int y)
{
	int sourceOffset = GRID_OFFSET(x, y);
	ROUTE_QUEUE(sourceOffset, -1,
	{
		if (Data_Grid_routingLandCitizen >= 0) {
			SET_DIST_AND_ENQUEUE();
		}
	});
}

void Routing_deleteClosestWallOrAqueduct(int x, int y)
{
	int sourceOffset = GRID_OFFSET(x, y);
	ROUTE_QUEUE(sourceOffset, -1,
	{
		if (Data_Grid_routingLandCitizen[nextOffset] < 0) {
			if (Data_Grid_terrain[nextOffset] & (Terrain_Aqueduct | Terrain_Wall)) {
				Data_Grid_terrain[nextOffset] &= 0x2e80;
				return;
			}
		} else {
			SET_DIST_AND_ENQUEUE();
		}
	});
}

static int hasFightingFriendly(int gridOffset)
{
	int walkerId = Data_Grid_walkerIds[gridOffset];
	if (walkerId > 0) {
		while (walkerId) {
			if (Data_Walkers[walkerId].isFriendly &&
				Data_Walkers[walkerId].actionState == WalkerActionState_150_Attacking) {
				return 1;
			}
		}
	}
	return 0;
}

static int hasFightingEnemy(int gridOffset)
{
	int walkerId = Data_Grid_walkerIds[gridOffset];
	if (walkerId > 0) {
		while (walkerId) {
			if (Data_Walkers[walkerId].isFriendly &&
				Data_Walkers[walkerId].actionState == WalkerActionState_150_Attacking) {
				return 1;
			}
		}
	}
	return 0;
}

int Routing_canTravelOverLandCitizen(int xSrc, int ySrc, int xDst, int yDst)
{
	int sourceOffset = GRID_OFFSET(xSrc, ySrc);
	int destOffset = GRID_OFFSET(xDst, yDst);
	++Data_Routes.totalRoutesCalculated;
	ROUTE_QUEUE(sourceOffset, destOffset,
	{
		if (Data_Grid_routingLandCitizen[nextOffset] >= 0 && !hasFightingFriendly(nextOffset)) {
			SET_DIST_AND_ENQUEUE();
		}
	});
	return Data_Grid_routingDistance[destOffset] != 0;
}

int Routing_canTravelOverRoadGardenCitizen(int xSrc, int ySrc, int xDst, int yDst)
{
	int sourceOffset = GRID_OFFSET(xSrc, ySrc);
	int destOffset = GRID_OFFSET(xDst, yDst);
	++Data_Routes.totalRoutesCalculated;
	ROUTE_QUEUE(sourceOffset, destOffset,
	{
		if (Data_Grid_routingLandCitizen[nextOffset] >= 0 &&
			Data_Grid_routingLandCitizen[nextOffset] <= 2) {
			SET_DIST_AND_ENQUEUE();
		}
	});
	return Data_Grid_routingDistance[destOffset] != 0;
}

int Routing_canTravelOverWalls(int xSrc, int ySrc, int xDst, int yDst)
{
	int sourceOffset = GRID_OFFSET(xSrc, ySrc);
	int destOffset = GRID_OFFSET(xDst, yDst);
	++Data_Routes.totalRoutesCalculated;
	ROUTE_QUEUE(sourceOffset, destOffset,
	{
		if (Data_Grid_routingWalls[nextOffset] >= 0 &&
			Data_Grid_routingWalls[nextOffset] <= 2) {
			SET_DIST_AND_ENQUEUE();
		}
	});
	return Data_Grid_routingDistance[destOffset] != 0;
}

int Routing_canTravelOverLandNonCitizen(int xSrc, int ySrc, int xDst, int yDst, int onlyThroughBuildingId, int maxTiles)
{
	int sourceOffset = GRID_OFFSET(xSrc, ySrc);
	int destOffset = GRID_OFFSET(xDst, yDst);
	++Data_Routes.totalRoutesCalculated;
	++Data_Routes.enemyRoutesCalculated;
	if (onlyThroughBuildingId) {
		ROUTE_QUEUE(sourceOffset, destOffset,
		{
			if (!hasFightingEnemy(nextOffset)) {
				if (Data_Grid_routingLandNonCitizen[nextOffset] == 0 ||
					Data_Grid_routingLandNonCitizen[nextOffset] == 2 ||
					(Data_Grid_routingLandNonCitizen[nextOffset] == 1 && Data_Grid_buildingIds[nextOffset] == onlyThroughBuildingId)) {
					SET_DIST_AND_ENQUEUE();
				}
			}
		});
	} else {
		ROUTE_QUEUE_MAX(sourceOffset, destOffset, maxTiles,
		{
			if (!hasFightingEnemy(nextOffset)) {
				if (Data_Grid_routingLandNonCitizen[nextOffset] >= 0 &&
					Data_Grid_routingLandNonCitizen[nextOffset] < 5) {
					SET_DIST_AND_ENQUEUE();
				}
			}
		});
	}
	return Data_Grid_routingDistance[destOffset] != 0;
}

int Routing_canTravelThroughEverythingNonCitizen(int xSrc, int ySrc, int xDst, int yDst)
{
	int sourceOffset = GRID_OFFSET(xSrc, ySrc);
	int destOffset = GRID_OFFSET(xDst, yDst);
	++Data_Routes.totalRoutesCalculated;
	ROUTE_QUEUE(sourceOffset, destOffset,
	{
		if (Data_Grid_routingLandNonCitizen[nextOffset] >= 0) {
			SET_DIST_AND_ENQUEUE();
		}
	});
	return Data_Grid_routingDistance[destOffset] != 0;
}

int Routing_canPlaceRoadUnderAqueduct(int gridOffset)
{
	int graphic = Data_Grid_graphicIds[gridOffset] - GraphicId(ID_Graphic_Aqueduct);
	int checkRoadY;
	switch (graphic) {
		case 0:
		case 2:
		case 8:
		case 15:
		case 17:
		case 23:
			checkRoadY = 1;
			break;
		case 1:
		case 3:
		case 9:
		case 16:
		case 18:
		case 24:
			checkRoadY = 0;
			break;
		default: // not a straight aqueduct
			return 0;
	}
	if (Data_Settings_Map.orientation == Direction_Left || Data_Settings_Map.orientation == Direction_Right) {
		checkRoadY = !checkRoadY;
	}
	if (checkRoadY) {
		if ((Data_Grid_terrain[gridOffset - 162] & Terrain_Road) ||
			Data_Grid_routingDistance[gridOffset - 162] > 0) {
			return 0;
		}
		if ((Data_Grid_terrain[gridOffset + 162] & Terrain_Road) ||
			Data_Grid_routingDistance[gridOffset + 162] > 0) {
			return 0;
		}
	} else {
		if ((Data_Grid_terrain[gridOffset - 1] & Terrain_Road) ||
			Data_Grid_routingDistance[gridOffset - 1] > 0) {
			return 0;
		}
		if ((Data_Grid_terrain[gridOffset + 1] & Terrain_Road) ||
			Data_Grid_routingDistance[gridOffset + 1] > 0) {
			return 0;
		}
	}
	return 1;
}

int Routing_getAqueductGraphicIdWithRoad(int gridOffset)
{
	int graphic = Data_Grid_graphicIds[gridOffset] - GraphicId(ID_Graphic_Aqueduct);
	switch (graphic) {
		case 2:
			return 8;
		case 3:
			return 9;
		case 0:
		case 1:
		case 8:
		case 9:
		case 15:
		case 16:
		case 17:
		case 18:
		case 23:
		case 24:
			// unchanged
			return graphic;
		default:
			// shouldn't happen
			return 8;
	}
}

static int canPlaceAqueductOnRoad(int gridOffset)
{
	int graphic = Data_Grid_graphicIds[gridOffset] - GraphicId(ID_Graphic_Road);
	if (graphic != 0 && graphic != 1 && graphic != 49 && graphic != 50) {
		return 0;
	}
	int checkRoadY = graphic == 0 || graphic == 49;
	if (Data_Settings_Map.orientation == Direction_Left || Data_Settings_Map.orientation == Direction_Right) {
		checkRoadY = !checkRoadY;
	}
	if (checkRoadY) {
		if (Data_Grid_routingDistance[gridOffset - 162] > 0 ||
			Data_Grid_routingDistance[gridOffset + 162] > 0) {
			return 0;
		}
	} else {
		if (Data_Grid_routingDistance[gridOffset - 1] > 0 ||
			Data_Grid_routingDistance[gridOffset + 1] > 0) {
			return 0;
		}
	}
	return 1;
}

void Routing_getDistanceForBuildingWall(int x, int y)
{
	int sourceOffset = GRID_OFFSET(x, y);
	ROUTE_QUEUE(sourceOffset, -1,
	{
		if (Data_Grid_routingLandCitizen[nextOffset] == 4) {
			SET_DIST_AND_ENQUEUE();
		}
	});
}

int Routing_getGeneralDirection(int xSrc, int ySrc, int xDst, int yDst)
{
	if (xSrc < xDst) {
		if (ySrc > yDst) {
			return Direction_TopRight;
		} else if (ySrc == yDst) {
			return Direction_Right;
		} else if (ySrc < yDst) {
			return Direction_BottomRight;
		}
	} else if (xSrc == xDst) {
		if (ySrc > yDst) {
			return Direction_Top;
		} else if (ySrc < yDst) {
			return Direction_Bottom;
		}
	} else if (xSrc > xDst) {
		if (ySrc > yDst) {
			return Direction_TopLeft;
		} else if (ySrc == yDst) {
			return Direction_Left;
		} else if (ySrc < yDst) {
			return Direction_BottomLeft;
		}
	}
	return Direction_None;
}

int Routing_getDirection(int xSrc, int ySrc, int xDst, int yDst)
{
	int dx = xSrc > xDst ? xSrc - xDst : xDst - xSrc;
	int dy = ySrc > yDst ? ySrc - yDst : yDst - ySrc;
	int percentage;
	if (dx > dy) {
		percentage = Calc_getPercentage(dx, dy);
	} else if (dx == dy) {
		percentage = 100;
	} else {
		percentage = -Calc_getPercentage(dy, dx);
	}
	if (xSrc == xDst) {
		if (ySrc > yDst) {
			return 0;
		} else {
			return 4;
		}
	} else if (xSrc > xDst) {
		if (ySrc == yDst) {
			return 6;
		} else if (ySrc > yDst) {
			if (percentage >= 400) {
				return 6;
			} else if (percentage > -400) {
				return 7;
			} else {
				return 0;
			}
		} else {
			if (percentage >= 400) {
				return 6;
			} else if (percentage > -400) {
				return 5;
			} else {
				return 4;
			}
		}
	} else { // xSrc < xDst
		if (ySrc == yDst) {
			return 2;
		} else if (ySrc > yDst) {
			if (percentage >= 400) {
				return 2;
			} else if (percentage > -400) {
				return 1;
			} else {
				return 0;
			}
		} else {
			if (percentage >= 400) {
				return 2;
			} else if (percentage > -400) {
				return 3;
			} else {
				return 4;
			}
		}
	}
}

int Routing_getDirectionForProjectile(int xSrc, int ySrc, int xDst, int yDst)
{
	int dx = xSrc > xDst ? xSrc - xDst : xDst - xSrc;
	int dy = ySrc > yDst ? ySrc - yDst : yDst - ySrc;
	int percentage;
	if (dx > dy) {
		percentage = Calc_getPercentage(dx, dy);
	} else if (dx == dy) {
		percentage = 100;
	} else {
		percentage = -Calc_getPercentage(dy, dx);
	}
	if (xSrc == xDst) {
		if (ySrc < yDst) {
			return 8;
		} else {
			return 0;
		}
	} else if (xSrc > xDst) {
		if (ySrc == yDst) {
			return 12;
		} else if (ySrc > yDst) {
			if (percentage >= 500) {
				return 12;
			} else if (percentage >= 200) {
				return 13;
			} else if (percentage > -200) {
				return 14;
			} else if (percentage > -500) {
				return 15;
			} else {
				return 0;
			}
		} else {
			if (percentage >= 500) {
				return 12;
			} else if (percentage >= 200) {
				return 11;
			} else if (percentage > -200) {
				return 10;
			} else if (percentage > -500) {
				return 9;
			} else {
				return 8;
			}
        }
	} else { // xSrc < xDst
		if (ySrc == yDst) {
			return 4;
		} else if (ySrc > yDst) {
			if (percentage >= 500) {
				return 4;
			} else if (percentage >= 200) {
				return 3;
			} else if (percentage > -200) {
				return 2;
			} else if (percentage > -500) {
				return 1;
			} else {
				return 0;
			}
		} else {
			if (percentage >= 500) {
				return 4;
			} else if (percentage >= 200) {
				return 5;
			} else if (percentage > -200) {
				return 6;
			} else if (percentage > -500) {
				return 7;
			} else {
				return 8;
			}
		}
	}
}

static void updateXYGridOffsetForDirection(int direction, int *x, int *y, int *gridOffset)
{
	switch (direction) {
		case 0:
			--(*y);
			(*gridOffset) -= 162;
			break;
		case 1:
			++(*x);
			--(*y);
			(*gridOffset) -= 161;
			break;
		case 2:
			++(*x);
			++(*gridOffset);
		case 3:
			++(*x);
			++(*y);
			(*gridOffset) += 163;
			break;
		case 4:
			++(*y);
			(*gridOffset) += 162;
			break;
		case 5:
			--(*x);
			++(*y);
			(*gridOffset) += 161;
			break;
		case 6:
			--(*x);
			--(*gridOffset);
			break;
		case 7:
			--(*x);
			--(*y);
			(*gridOffset) -= 163;
			break;
	}
}

void Routing_getDistanceWaterBoat(int x, int y)
{
	int sourceGridOffset = GRID_OFFSET(x, y);
	if (Data_Grid_routingWater[sourceGridOffset] == -1) {
		return;
	}
	ROUTE_QUEUE_BOAT(sourceGridOffset,
	{
		if (Data_Grid_routingWater[nextOffset] != -1 && Data_Grid_routingWater[nextOffset] != -3) {
			SET_DIST_AND_ENQUEUE();
			if (Data_Grid_routingWater[nextOffset] == -2) {
				Data_Grid_routingDistance[nextOffset] += 4;
			}
		}
	});
}

void Routing_getDistanceWater(int x, int y)
{
	int sourceGridOffset = GRID_OFFSET(x, y);
	if (Data_Grid_routingWater[sourceGridOffset] == -1) {
		return;
	}
	ROUTE_QUEUE_DIR8(sourceGridOffset,
	{
		if (Data_Grid_routingWater[nextOffset] >= 0) {
			SET_DIST_AND_ENQUEUE();
		}
	});
}

int Routing_getPathOnWater(int routingPathId, int xSrc, int ySrc, int xDst, int yDst, int isFlotsam)
{
	int rand = Data_Random.random1_7bit & 3;
	int dstGridOffset = GRID_OFFSET(xDst, yDst);
	int distance = Data_Grid_routingDistance[dstGridOffset];
	if (distance <= 0 || distance >= 998) {
		return 0;
	}

	int numTiles = 0;
	int lastDirection = -1;
	int x = xDst;
	int y = yDst;
	int gridOffset = dstGridOffset;
	while (distance > 1) {
		int currentRand = rand;
		distance = Data_Grid_routingDistance[gridOffset];
		if (isFlotsam) {
			currentRand = Data_Grid_random[gridOffset] & 3;
		}
		int direction = -1;
		for (int d = 0; d < 8; d++) {
			if (d != lastDirection) {
				int nextOffset = gridOffset = Constant_DirectionGridOffsets[d];
				int nextDistance = Data_Grid_routingDistance[nextOffset];
				if (nextDistance) {
					if (nextDistance < distance) {
						distance = nextDistance;
						direction = d;
					} else if (nextDistance == distance && rand == currentRand) {
						// allow flotsam to wander
						distance = nextDistance;
						direction = d;
					}
				}
			}
		}
		if (direction == -1) {
			return 0;
		}
		updateXYGridOffsetForDirection(direction, &x, &y, &gridOffset);
		int forwardDirection = (direction + 4) % 8;
		directionPath[numTiles++] = forwardDirection;
		lastDirection = forwardDirection;
		if (numTiles >= 500) {
			return 0;
		}
	}
	for (int i = 0; i < numTiles; i++) {
		Data_Routes.directionPaths[routingPathId][i] = directionPath[numTiles - i - 1];
	}
	return numTiles;
}
