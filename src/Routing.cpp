//#include "Routing.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Settings.h"
#include "Data/Walker.h"

#include <string.h>

#define MAX_QUEUE 26243

static struct {
	int items[MAX_QUEUE];
	int head;
	int tail;
} queue;

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
				// TODO
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
	++Data_CityInfo_Extra.routingTotalRoutesCalculated;
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
	++Data_CityInfo_Extra.routingTotalRoutesCalculated;
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
	++Data_CityInfo_Extra.routingTotalRoutesCalculated;
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
	++Data_CityInfo_Extra.routingTotalRoutesCalculated;
	++Data_CityInfo_Extra.routingEnemyRoutesCalculated;
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
	++Data_CityInfo_Extra.routingTotalRoutesCalculated;
	ROUTE_QUEUE(sourceOffset, destOffset,
	{
		if (Data_Grid_routingLandNonCitizen[nextOffset] >= 0) {
			SET_DIST_AND_ENQUEUE();
		}
	});
	return Data_Grid_routingDistance[destOffset] != 0;
}
