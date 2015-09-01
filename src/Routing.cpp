#include "Routing.h"

#include "Calc.h"
#include "Grid.h"
#include "TerrainGraphics.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Debug.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Random.h"
#include "Data/Routes.h"
#include "Data/Settings.h"
#include "Data/Walker.h"

#include <string.h>

#define MAX_QUEUE 26244

static struct {
	int head;
	int tail;
	int items[MAX_QUEUE];
} queue;

static struct {
	int throughBuildingId;
	int isAqueduct;
} state;

static int directionPath[500];

static char tmpGrid[GRID_SIZE * GRID_SIZE];

static void setDistAndEnqueue(int nextOffset, int dist)
{
	Data_Grid_routingDistance[nextOffset] = dist;
	queue.items[queue.tail++] = nextOffset;
	if (queue.tail >= MAX_QUEUE) queue.tail = 0;
}

static void routeQueue(int source, int dest, void (*callback)(int nextOffset, int dist))
{
	Grid_clearShortGrid(Data_Grid_routingDistance);
	Data_Grid_routingDistance[source] = 1;
	queue.items[0] = source;
	queue.head = 0;
	queue.tail = 1;
	while (queue.head != queue.tail) {
		int offset = queue.items[queue.head];
		if (offset == dest) break;
		int dist = 1 + Data_Grid_routingDistance[offset];
		int nextOffset = offset - 162;
		if (nextOffset >= 0 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset + 1;
		if (nextOffset < 162 * 162 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset + 162;
		if (nextOffset < 162 * 162 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset - 1;
		if (nextOffset >= 0 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		if (++queue.head >= MAX_QUEUE) queue.head = 0;
	}
}

static void routeQueueWhileTrue(int source, int (*callback)(int nextOffset, int dist))
{
	Grid_clearShortGrid(Data_Grid_routingDistance);
	Data_Grid_routingDistance[source] = 1;
	queue.items[0] = source;
	queue.head = 0;
	queue.tail = 1;
	while (queue.head != queue.tail) {
		int offset = queue.items[queue.head];
		int dist = 1 + Data_Grid_routingDistance[offset];
		int nextOffset = offset - 162;
		if (nextOffset >= 0 && !Data_Grid_routingDistance[nextOffset]) {
			if (!callback(nextOffset, dist)) break;
		}
		nextOffset = offset + 1;
		if (nextOffset < 162 * 162 && !Data_Grid_routingDistance[nextOffset]) {
			if (!callback(nextOffset, dist)) break;
		}
		nextOffset = offset + 162;
		if (nextOffset < 162 * 162 && !Data_Grid_routingDistance[nextOffset]) {
			if (!callback(nextOffset, dist)) break;
		}
		nextOffset = offset - 1;
		if (nextOffset >= 0 && !Data_Grid_routingDistance[nextOffset]) {
			if (!callback(nextOffset, dist)) break;
		}
		if (++queue.head >= MAX_QUEUE) queue.head = 0;
	}
}

static void routeQueueMax(int source, int dest, int maxTiles, void (*callback)(int, int))
{
	Grid_clearShortGrid(Data_Grid_routingDistance);
	Data_Grid_routingDistance[source] = 1;
	queue.items[0] = source;
	queue.head = 0;
	queue.tail = 1;
	int tiles = 0;
	while (queue.head != queue.tail) {
		int offset = queue.items[queue.head];
		if (offset == dest) break;
		if (++tiles > maxTiles) break;
		int dist = 1 + Data_Grid_routingDistance[offset];
		int nextOffset = offset - 162;
		if (nextOffset >= 0 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset + 1;
		if (nextOffset < 162 * 162 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset + 162;
		if (nextOffset < 162 * 162 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset - 1;
		if (nextOffset >= 0 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		if (++queue.head >= MAX_QUEUE) queue.head = 0;
	}
}

static void routeQueueBoat(int source, void (*callback)(int, int))
{
	Grid_clearShortGrid(Data_Grid_routingDistance);
	Grid_clearByteGrid(tmpGrid);
	Data_Grid_routingDistance[source] = 1;
	queue.items[0] = source;
	queue.head = 0;
	queue.tail = 1;
	int tiles = 0;
	while (queue.head != queue.tail) {
		int offset = queue.items[queue.head];
		if (++tiles > 50000) break;
		int drag = Data_Grid_routingWater[offset] == Routing_Water_m2_MapEdge ? 4 : 0;
		if (drag && tmpGrid[offset]++ < drag) {
			queue.items[queue.tail++] = offset;
			if (queue.tail >= MAX_QUEUE) queue.tail = 0;
		} else {
			int dist = 1 + Data_Grid_routingDistance[offset];
			int nextOffset = offset - 162;
			if (nextOffset >= 0 && !Data_Grid_routingDistance[nextOffset]) {
				callback(nextOffset, dist);
			}
			nextOffset = offset + 1;
			if (nextOffset < 162 * 162 && !Data_Grid_routingDistance[nextOffset]) {
				callback(nextOffset, dist);
			}
			nextOffset = offset + 162;
			if (nextOffset < 162 * 162 && !Data_Grid_routingDistance[nextOffset]) {
				callback(nextOffset, dist);
			}
			nextOffset = offset - 1;
			if (nextOffset >= 0 && !Data_Grid_routingDistance[nextOffset]) {
				callback(nextOffset, dist);
			}
		}
		if (++queue.head >= MAX_QUEUE) queue.head = 0;
	}
}

static void routeQueueDir8(int source, void (*callback)(int, int))
{
	Grid_clearShortGrid(Data_Grid_routingDistance);
	Data_Grid_routingDistance[source] = 1;
	queue.items[0] = source;
	queue.head = 0;
	queue.tail = 1;
	int tiles = 0;
	while (queue.head != queue.tail) {
		if (++tiles > 50000) break;
		int offset = queue.items[queue.head];
		int dist = 1 + Data_Grid_routingDistance[offset];
		int nextOffset = offset - 162;
		if (nextOffset >= 0 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset + 1;
		if (nextOffset < 162 * 162 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset + 162;
		if (nextOffset < 162 * 162 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset - 1;
		if (nextOffset >= 0 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset - 161;
		if (nextOffset >= 0 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset + 163;
		if (nextOffset < 162 * 162 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset + 161;
		if (nextOffset < 162 * 162 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		nextOffset = offset - 163;
		if (nextOffset >= 0 && !Data_Grid_routingDistance[nextOffset]) {
			callback(nextOffset, dist);
		}
		if (++queue.head >= MAX_QUEUE) queue.head = 0;
	}
}

void Routing_determineLandCitizen()
{
	memset(Data_Grid_routingLandCitizen, -1, GRID_SIZE * GRID_SIZE);
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
				Data_Grid_routingLandCitizen[gridOffset] = Routing_Citizen_0_Road;
			} else if (Data_Grid_terrain[gridOffset] & (Terrain_Rubble | Terrain_AccessRamp | Terrain_Garden)) {
				Data_Grid_routingLandCitizen[gridOffset] = Routing_Citizen_2_PassableTerrain;
			} else if (Data_Grid_terrain[gridOffset] & (Terrain_Building | Terrain_Gatehouse)) {
				int buildingId = Data_Grid_buildingIds[gridOffset];
				if (!buildingId) {
					// shouldn't happen
					Data_Grid_routingLandNonCitizen[gridOffset] = 4; // BUGFIX - should be citizen?
					Data_Grid_terrain[gridOffset] &= ~Terrain_Building; // remove 8 = building
					Data_Grid_graphicIds[gridOffset] = (Data_Grid_random[gridOffset] & 7) + GraphicId(ID_Graphic_TerrainGrass1);
					Data_Grid_edge[gridOffset] = Edge_LeftmostTile;
					Data_Grid_bitfields[gridOffset] &= 0xf0; // remove sizes
					continue;
				}
				int land = Routing_Citizen_m1_Blocked;
				switch (Data_Buildings[buildingId].type) {
					case Building_Warehouse:
					case Building_Gatehouse:
						land = Routing_Citizen_0_Road;
						break;
					case Building_FortGround:
						land = Routing_Citizen_2_PassableTerrain;
						break;
					case Building_TriumphalArch:
						if (Data_Buildings[buildingId].subtype.orientation == 3) {
							switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
								case Edge_X0Y1:
								case Edge_X1Y1:
								case Edge_X2Y1:
									land = Routing_Citizen_0_Road;
									break;
							}
						} else {
							switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
								case Edge_X1Y0:
								case Edge_X1Y1:
								case Edge_X1Y2:
									land = Routing_Citizen_0_Road;
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
								land = Routing_Citizen_0_Road;
								break;
						}
						break;
					case Building_Reservoir:
						switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
							case Edge_X1Y0:
							case Edge_X0Y1:
							case Edge_X2Y1:
							case Edge_X1Y2:
								land = Routing_Citizen_m4_ReservoirConnector; // aqueduct connect points
								break;
						}
						break;
				}
				Data_Grid_routingLandCitizen[gridOffset] = land;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
				int graphicId = Data_Grid_graphicIds[gridOffset] - GraphicId(ID_Graphic_Aqueduct);
				int land;
				if (graphicId <= 3) {
					land = Routing_Citizen_m3_Aqueduct;
				} else if (graphicId <= 7) {
					land = Routing_Citizen_m1_Blocked;
				} else if (graphicId <= 9) {
					land = Routing_Citizen_m3_Aqueduct;
				} else if (graphicId <= 14) {
					land = Routing_Citizen_m1_Blocked;
				} else if (graphicId <= 18) {
					land = Routing_Citizen_m3_Aqueduct;
				} else if (graphicId <= 22) {
					land = Routing_Citizen_m1_Blocked;
				} else if (graphicId <= 24) {
					land = Routing_Citizen_m3_Aqueduct;
				} else {
					land = Routing_Citizen_m1_Blocked;
				}
				Data_Grid_routingLandCitizen[gridOffset] = land;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
				Data_Grid_routingLandCitizen[gridOffset] = Routing_Citizen_m1_Blocked;
			} else {
				Data_Grid_routingLandCitizen[gridOffset] = Routing_Citizen_4_ClearTerrain;
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
				Data_Grid_routingLandNonCitizen[gridOffset] = Routing_NonCitizen_4_Gatehouse;
			} else if (terrain & Terrain_Road) {
				Data_Grid_routingLandNonCitizen[gridOffset] = Routing_NonCitizen_0_Passable;
			} else if (terrain & (Terrain_Garden | Terrain_AccessRamp | Terrain_Rubble)) {
				Data_Grid_routingLandNonCitizen[gridOffset] = Routing_NonCitizen_2_Clearable;
			} else if (terrain & Terrain_Building) {
				int land = Routing_NonCitizen_1_Building;
				switch (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type) {
					case Building_Warehouse:
					case Building_FortGround:
						land = Routing_NonCitizen_0_Passable;
						break;
					case Building_BurningRuin:
					case Building_NativeHut:
					case Building_NativeMeeting:
					case Building_NativeCrops:
						land = Routing_NonCitizen_m1_Blocked;
						break;
					case Building_FortGround__:
						land = Routing_NonCitizen_5_Fort;
						break;
					case Building_Granary:
						switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
							case Edge_X1Y0:
							case Edge_X0Y1:
							case Edge_X1Y1:
							case Edge_X2Y1:
							case Edge_X1Y2:
								land = Routing_NonCitizen_0_Passable;
								break;
						}
						break;
				}
				Data_Grid_routingLandNonCitizen[gridOffset] = land;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
				Data_Grid_routingLandNonCitizen[gridOffset] = Routing_NonCitizen_2_Clearable;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_Wall) {
				Data_Grid_routingLandNonCitizen[gridOffset] = Routing_NonCitizen_3_Wall;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
				Data_Grid_routingLandNonCitizen[gridOffset] = Routing_NonCitizen_m1_Blocked;
			} else {
				Data_Grid_routingLandNonCitizen[gridOffset] = Routing_NonCitizen_0_Passable;
			}
		}
	}
}

void Routing_determineWater()
{
	memset(Data_Grid_routingWater, -1, GRID_SIZE * GRID_SIZE);
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
							case 6: // low bridge middle section
								Data_Grid_routingWater[gridOffset] = Routing_Water_m3_LowBridge;
								break;
							case 13: // ship bridge pillar
								Data_Grid_routingWater[gridOffset] = Routing_Water_m1_Blocked;
								break;
							default:
								Data_Grid_routingWater[gridOffset] = Routing_Water_0_Passable;
								break;
						}
					} else {
						Data_Grid_routingWater[gridOffset] = Routing_Water_m2_MapEdge;
					}
				} else {
					Data_Grid_routingWater[gridOffset] = Routing_Water_m1_Blocked;
				}
			} else {
				Data_Grid_routingWater[gridOffset] = Routing_Water_m1_Blocked;
			}
		}
	}
}

void Routing_determineWalls()
{
	memset(Data_Grid_routingWalls, -1, GRID_SIZE * GRID_SIZE);
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Wall) {
				int adjacent = 0;
				switch (Data_Settings_Map.orientation) {
					case Dir_0_Top:
						if (Data_Grid_terrain[gridOffset + 162] & Terrain_WallOrGatehouse) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset + 163] & Terrain_WallOrGatehouse) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset + 1] & Terrain_WallOrGatehouse) {
							adjacent++;
						}
						break;
					case Dir_2_Right:
						if (Data_Grid_terrain[gridOffset + 162] & Terrain_WallOrGatehouse) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset + 161] & Terrain_WallOrGatehouse) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset - 1] & Terrain_WallOrGatehouse) {
							adjacent++;
						}
						break;
					case Dir_4_Bottom:
						if (Data_Grid_terrain[gridOffset - 162] & Terrain_WallOrGatehouse) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset - 163] & Terrain_WallOrGatehouse) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset - 1] & Terrain_WallOrGatehouse) {
							adjacent++;
						}
						break;
					case Dir_6_Left:
						if (Data_Grid_terrain[gridOffset - 162] & Terrain_WallOrGatehouse) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset - 161] & Terrain_WallOrGatehouse) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset + 1] & Terrain_WallOrGatehouse) {
							adjacent++;
						}
						break;
				}
				if (adjacent == 3) {
					Data_Grid_routingWalls[gridOffset] = Routing_Wall_0_Passable;
				} else {
					Data_Grid_routingWalls[gridOffset] = Routing_Wall_m1_Blocked;
				}
			} else if (Data_Grid_terrain[gridOffset] & Terrain_Gatehouse) {
				Data_Grid_routingWalls[gridOffset] = Routing_Wall_0_Passable;
			} else {
				Data_Grid_routingWalls[gridOffset] = Routing_Wall_m1_Blocked;
			}
		}
	}
}

void Routing_clearLandTypeCitizen()
{
	int gridOffset = 0;
	for (int y = 0; y < GRID_SIZE; y++) {
		for (int x = 0; x < GRID_SIZE; x++) {
			Data_Grid_routingLandCitizen[gridOffset++] = Routing_Citizen_m1_Blocked;
		}
	}
}

static void callbackGetDistance(int nextOffset, int dist)
{
	if (Data_Grid_routingLandCitizen[nextOffset] >= Routing_Citizen_0_Road) {
		setDistAndEnqueue(nextOffset, dist);
	}
}

void Routing_getDistance(int x, int y)
{
	int sourceOffset = GridOffset(x, y);
	++Data_Routes.totalRoutesCalculated;
	routeQueue(sourceOffset, -1, callbackGetDistance);
}

int Routing_getCalculatedDistance(int gridOffset)
{
	return Data_Grid_routingDistance[gridOffset];
}

static int callbackDeleteClosestWallOrAqueduct(int nextOffset, int dist)
{
	if (Data_Grid_routingLandCitizen[nextOffset] < Routing_Citizen_0_Road) {
		if (Data_Grid_terrain[nextOffset] & (Terrain_Aqueduct | Terrain_Wall)) {
			Data_Grid_terrain[nextOffset] &= Terrain_2e80;
			return 0;
		}
	} else {
		setDistAndEnqueue(nextOffset, dist);
	}
	return 1;
}

void Routing_deleteClosestWallOrAqueduct(int x, int y)
{
	int sourceOffset = GridOffset(x, y);
	++Data_Routes.totalRoutesCalculated;
	routeQueueWhileTrue(sourceOffset, callbackDeleteClosestWallOrAqueduct);
}

static int hasFightingFriendly(int gridOffset)
{
	int walkerId = Data_Grid_walkerIds[gridOffset];
	if (walkerId > 0) {
		while (walkerId) {
			if (Data_Walkers[walkerId].isFriendly &&
				Data_Walkers[walkerId].actionState == WalkerActionState_150_Attack) {
				return 1;
			}
			walkerId = Data_Walkers[walkerId].nextWalkerIdOnSameTile;
		}
	}
	return 0;
}

static int hasFightingEnemy(int gridOffset)
{
	int walkerId = Data_Grid_walkerIds[gridOffset];
	if (walkerId > 0) {
		while (walkerId) {
			if (!Data_Walkers[walkerId].isFriendly &&
				Data_Walkers[walkerId].actionState == WalkerActionState_150_Attack) {
				return 1;
			}
			walkerId = Data_Walkers[walkerId].nextWalkerIdOnSameTile;
		}
	}
	return 0;
}

static void callbackCanTravelOverLandCitizen(int nextOffset, int dist)
{
	if (Data_Grid_routingLandCitizen[nextOffset] >= 0 && !hasFightingFriendly(nextOffset)) {
		setDistAndEnqueue(nextOffset, dist);
	}
}

int Routing_canTravelOverLandCitizen(int xSrc, int ySrc, int xDst, int yDst)
{
	int sourceOffset = GridOffset(xSrc, ySrc);
	int destOffset = GridOffset(xDst, yDst);
	++Data_Routes.totalRoutesCalculated;
	routeQueue(sourceOffset, destOffset, callbackCanTravelOverLandCitizen);
	return Data_Grid_routingDistance[destOffset] != 0;
}

static void callbackCanTravelOverRoadGardenCitizen(int nextOffset, int dist)
{
	if (Data_Grid_routingLandCitizen[nextOffset] >= Routing_Citizen_0_Road &&
		Data_Grid_routingLandCitizen[nextOffset] <= Routing_Citizen_2_PassableTerrain) {
		setDistAndEnqueue(nextOffset, dist);
	}
}

int Routing_canTravelOverRoadGardenCitizen(int xSrc, int ySrc, int xDst, int yDst)
{
	int sourceOffset = GridOffset(xSrc, ySrc);
	int destOffset = GridOffset(xDst, yDst);
	++Data_Routes.totalRoutesCalculated;
	routeQueue(sourceOffset, destOffset, callbackCanTravelOverRoadGardenCitizen);
	return Data_Grid_routingDistance[destOffset] != 0;
}

static void callbackCanTravelOverWalls(int nextOffset, int dist)
{
	if (Data_Grid_routingWalls[nextOffset] >= Routing_Wall_0_Passable &&
		Data_Grid_routingWalls[nextOffset] <= 2) {
		setDistAndEnqueue(nextOffset, dist);
	}
}

int Routing_canTravelOverWalls(int xSrc, int ySrc, int xDst, int yDst)
{
	int sourceOffset = GridOffset(xSrc, ySrc);
	int destOffset = GridOffset(xDst, yDst);
	++Data_Routes.totalRoutesCalculated;
	routeQueue(sourceOffset, destOffset, callbackCanTravelOverWalls);
	return Data_Grid_routingDistance[destOffset] != 0;
}

static void callbackCanTravelOverLandNonCitizenThroughBuilding(int nextOffset, int dist)
{
	if (!hasFightingEnemy(nextOffset)) {
		if (Data_Grid_routingLandNonCitizen[nextOffset] == Routing_NonCitizen_0_Passable ||
			Data_Grid_routingLandNonCitizen[nextOffset] == Routing_NonCitizen_2_Clearable ||
			(Data_Grid_routingLandNonCitizen[nextOffset] == Routing_NonCitizen_1_Building &&
				Data_Grid_buildingIds[nextOffset] == state.throughBuildingId)) {
			setDistAndEnqueue(nextOffset, dist);
		}
	}
}

static void callbackCanTravelOverLandNonCitizen(int nextOffset, int dist)
{
	if (!hasFightingEnemy(nextOffset)) {
		if (Data_Grid_routingLandNonCitizen[nextOffset] >= Routing_NonCitizen_0_Passable &&
			Data_Grid_routingLandNonCitizen[nextOffset] < Routing_NonCitizen_5_Fort) {
			setDistAndEnqueue(nextOffset, dist);
		}
	}
}

int Routing_canTravelOverLandNonCitizen(int xSrc, int ySrc, int xDst, int yDst, int onlyThroughBuildingId, int maxTiles)
{
	int sourceOffset = GridOffset(xSrc, ySrc);
	int destOffset = GridOffset(xDst, yDst);
	++Data_Routes.totalRoutesCalculated;
	++Data_Routes.enemyRoutesCalculated;
	if (onlyThroughBuildingId) {
		state.throughBuildingId = onlyThroughBuildingId;
		routeQueue(sourceOffset, destOffset, callbackCanTravelOverLandNonCitizenThroughBuilding);
	} else {
		routeQueueMax(sourceOffset, destOffset, maxTiles, callbackCanTravelOverLandNonCitizen);
	}
	return Data_Grid_routingDistance[destOffset] != 0;
}

static void callbackCanTravelThroughEverythingNonCitizen(int nextOffset, int dist)
{
	if (Data_Grid_routingLandNonCitizen[nextOffset] >= Routing_NonCitizen_0_Passable) {
		setDistAndEnqueue(nextOffset, dist);
	}
}

int Routing_canTravelThroughEverythingNonCitizen(int xSrc, int ySrc, int xDst, int yDst)
{
	int sourceOffset = GridOffset(xSrc, ySrc);
	int destOffset = GridOffset(xDst, yDst);
	++Data_Routes.totalRoutesCalculated;
	routeQueue(sourceOffset, destOffset, callbackCanTravelThroughEverythingNonCitizen);
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
		case 9: case 10: case 11: case 12: case 13: case 14:
		case 16:
		case 18:
		case 24: case 25: case 26: case 27: case 28: case 29:
			checkRoadY = 0;
			break;
		default: // not a straight aqueduct
			return 0;
	}
	if (Data_Settings_Map.orientation == Dir_6_Left || Data_Settings_Map.orientation == Dir_2_Right) {
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

int Routing_getAqueductGraphicOffsetWithRoad(int gridOffset)
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
	if (Data_Settings_Map.orientation == Dir_6_Left || Data_Settings_Map.orientation == Dir_2_Right) {
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

static int canPlaceInitialRoadOrAqueduct(int gridOffset, int isAqueduct)
{
	if (Data_Grid_routingLandCitizen[gridOffset] == Routing_Citizen_m1_Blocked) {
		// not open land, can only if:
		// - aqueduct should be placed, and:
		// - land is a reservoir building OR an aqueduct
		if (!isAqueduct) {
			return 0;
		}
		if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
			return 1;
		}
		if (Data_Grid_terrain[gridOffset] & Terrain_Building) {
			if (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type == Building_Reservoir) {
				return 1;
			}
		}
		return 0;
	} else if (Data_Grid_routingLandCitizen[gridOffset] == Routing_Citizen_2_PassableTerrain) {
		// rubble, access ramp, garden
		return 0;
	} else if (Data_Grid_routingLandCitizen[gridOffset] == Routing_Citizen_m3_Aqueduct) {
		if (isAqueduct) {
			return 0;
		}
		if (Routing_canPlaceRoadUnderAqueduct(gridOffset)) {
			return 1;
		}
		return 0;
	} else {
		return 1;
	}
}

static void callbackGetDistanceForBuildingRoadOrAqueduct(int nextOffset, int dist)
{
	int blocked = 0;
	switch (Data_Grid_routingLandCitizen[nextOffset]) {
		case Routing_Citizen_m3_Aqueduct:
			if (state.isAqueduct) {
				blocked = 1;
			} else if (!Routing_canPlaceRoadUnderAqueduct(nextOffset)) {
				Data_Grid_routingDistance[nextOffset] = -1;
				blocked = 1;
			}
			break;
		case Routing_Citizen_2_PassableTerrain: // rubble, garden, access ramp
		case Routing_Citizen_m1_Blocked: // non-empty land
			blocked = 1;
			break;
		default:
			if (Data_Grid_terrain[nextOffset] & Terrain_Building) {
				if (Data_Grid_routingLandCitizen[nextOffset] != Routing_Citizen_m4_ReservoirConnector ||
					!state.isAqueduct) {
					blocked = 1;
				}
			}
			break;
	}
	if (Data_Grid_terrain[nextOffset] & Terrain_Road) {
		if (state.isAqueduct && !canPlaceAqueductOnRoad(nextOffset)) {
			Data_Grid_routingDistance[nextOffset] = -1;
			blocked = 1;
		}
	}
	if (!blocked) {
		setDistAndEnqueue(nextOffset, dist);
	}
}

int Routing_getDistanceForBuildingRoadOrAqueduct(int x, int y, int isAqueduct)
{
	int sourceOffset = GridOffset(x, y);
	if (!canPlaceInitialRoadOrAqueduct(sourceOffset, isAqueduct)) {
		return 0;
	}
	if (Data_Grid_terrain[sourceOffset] & Terrain_Road &&
		isAqueduct && !canPlaceAqueductOnRoad(sourceOffset)) {
		return 0;
	}
	++Data_Routes.totalRoutesCalculated;
	state.isAqueduct = isAqueduct;
	routeQueue(sourceOffset, -1, callbackGetDistanceForBuildingRoadOrAqueduct);
	return 1;
}

static void callbackGetDistanceForBuildingWall(int nextOffset, int dist)
{
	if (Data_Grid_routingLandCitizen[nextOffset] == Routing_Citizen_4_ClearTerrain) {
		setDistAndEnqueue(nextOffset, dist);
	}
}

int Routing_getDistanceForBuildingWall(int x, int y)
{
	int sourceOffset = GridOffset(x, y);
	routeQueue(sourceOffset, -1, callbackGetDistanceForBuildingWall);
	return 1;
}

int Routing_placeRoutedBuilding(int xSrc, int ySrc, int xDst, int yDst, RoutedBuilding type, int *items)
{
	static const int directionIndices[8][4] = {
		{0, 2, 6, 4},
		{0, 2, 6, 4},
		{2, 4, 0, 6},
		{2, 4, 0, 6},
		{4, 6, 2, 0},
		{4, 6, 2, 0},
		{6, 0, 4, 2},
		{6, 0, 4, 2}
	};
	*items = 0;
	int gridOffset = GridOffset(xDst, yDst);
	int guard = 0;
	// reverse routing
	while (1) {
		if (++guard >= 400) {
			return 0;
		}
		int distance = Data_Grid_routingDistance[gridOffset];
		if (distance <= 0) {
			return 0;
		}
		switch (type) {
			default:
			case RoutedBuilding_Road:
				*items += TerrainGraphics_setTileRoad(xDst, yDst);
				break;
			case RoutedBuilding_Wall:
				*items += TerrainGraphics_setTileWall(xDst, yDst);
				break;
			case RoutedBuilding_Aqueduct:
				*items += TerrainGraphics_setTileAqueductTerrain(xDst, yDst);
				break;
			case RoutedBuilding_AqueductWithoutGraphic:
				*items += 1;
				break;
		}
		int direction = Routing_getGeneralDirection(xDst, yDst, xSrc, ySrc);
		if (direction == Dir_8_None) {
			return 1; // destination reached
		}
		int routed = 0;
		for (int i = 0; i < 4; i++) {
			int index = directionIndices[direction][i];
			int newGridOffset = gridOffset + Constant_DirectionGridOffsets[index];
			int newDist = Data_Grid_routingDistance[newGridOffset];
			if (newDist > 0 && newDist < distance) {
				gridOffset = newGridOffset;
				xDst = GridOffsetToX(gridOffset);
				yDst = GridOffsetToY(gridOffset);
				routed = 1;
				break;
			}
		}
		if (!routed) {
			return 0;
		}
	}
}

int Routing_getGeneralDirection(int xSrc, int ySrc, int xDst, int yDst)
{
	if (xSrc < xDst) {
		if (ySrc > yDst) {
			return Dir_1_TopRight;
		} else if (ySrc == yDst) {
			return Dir_2_Right;
		} else if (ySrc < yDst) {
			return Dir_3_BottomRight;
		}
	} else if (xSrc == xDst) {
		if (ySrc > yDst) {
			return Dir_0_Top;
		} else if (ySrc < yDst) {
			return Dir_4_Bottom;
		}
	} else if (xSrc > xDst) {
		if (ySrc > yDst) {
			return Dir_7_TopLeft;
		} else if (ySrc == yDst) {
			return Dir_6_Left;
		} else if (ySrc < yDst) {
			return Dir_5_BottomLeft;
		}
	}
	return Dir_8_None;
}

int Routing_getDirectionForMissileShooter(int xSrc, int ySrc, int xDst, int yDst)
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
			return 4;
		} else {
			return 0;
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

int Routing_getDirectionForMissile(int xSrc, int ySrc, int xDst, int yDst)
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
		case Dir_0_Top:
			--(*y);
			(*gridOffset) -= 162;
			break;
		case Dir_1_TopRight:
			++(*x);
			--(*y);
			(*gridOffset) -= 161;
			break;
		case Dir_2_Right:
			++(*x);
			++(*gridOffset);
			break;
		case Dir_3_BottomRight:
			++(*x);
			++(*y);
			(*gridOffset) += 163;
			break;
		case Dir_4_Bottom:
			++(*y);
			(*gridOffset) += 162;
			break;
		case Dir_5_BottomLeft:
			--(*x);
			++(*y);
			(*gridOffset) += 161;
			break;
		case Dir_6_Left:
			--(*x);
			--(*gridOffset);
			break;
		case Dir_7_TopLeft:
			--(*x);
			--(*y);
			(*gridOffset) -= 163;
			break;
	}
}

static void callbackGetDistanceWaterBoat(int nextOffset, int dist)
{
	if (Data_Grid_routingWater[nextOffset] != Routing_Water_m1_Blocked &&
		Data_Grid_routingWater[nextOffset] != Routing_Water_m3_LowBridge) {
		setDistAndEnqueue(nextOffset, dist);
		if (Data_Grid_routingWater[nextOffset] == Routing_Water_m2_MapEdge) {
			Data_Grid_routingDistance[nextOffset] += 4;
		}
	}
}

void Routing_getDistanceWaterBoat(int x, int y)
{
	int sourceGridOffset = GridOffset(x, y);
	if (Data_Grid_routingWater[sourceGridOffset] == Routing_Water_m1_Blocked) {
		return;
	}
	routeQueueBoat(sourceGridOffset, callbackGetDistanceWaterBoat);
}

static void callbackGetDistanceWaterFlotsam(int nextOffset, int dist)
{
	if (Data_Grid_routingWater[nextOffset] != Routing_Water_m1_Blocked) {
		setDistAndEnqueue(nextOffset, dist);
	}
}

void Routing_getDistanceWaterFlotsam(int x, int y)
{
	int sourceGridOffset = GridOffset(x, y);
	if (Data_Grid_routingWater[sourceGridOffset] == Routing_Water_m1_Blocked) {
		return;
	}
	routeQueueDir8(sourceGridOffset, callbackGetDistanceWaterFlotsam);
}

int Routing_getPath(int numDirections, int routingPathId, int xSrc, int ySrc, int xDst, int yDst)
{
	int dstGridOffset = GridOffset(xDst, yDst);
	int distance = Data_Grid_routingDistance[dstGridOffset];
	if (distance <= 0 || distance >= 998) {
		return 0;
	}

	int numTiles = 0;
	int lastDirection = -1;
	int x = xDst;
	int y = yDst;
	int gridOffset = dstGridOffset;
	int step = numDirections == 8 ? 1 : 2;

	while (distance > 1) {
		distance = Data_Grid_routingDistance[gridOffset];
		int direction = -1;
		int generalDirection = Routing_getGeneralDirection(x, y, xSrc, ySrc);
		for (int d = 0; d < 8; d += step) {
			if (d != lastDirection) {
				int nextOffset = gridOffset + Constant_DirectionGridOffsets[d];
				int nextDistance = Data_Grid_routingDistance[nextOffset];
				if (nextDistance) {
					if (nextDistance < distance) {
						distance = nextDistance;
						direction = d;
					} else if (nextDistance == distance && (d == generalDirection || direction == -1)) {
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

int Routing_getClosestXYWithinRange(int numDirections, int xSrc, int ySrc, int xDst, int yDst, int range, int *xOut, int *yOut)
{
	int dstGridOffset = GridOffset(xDst, yDst);
	int distance = Data_Grid_routingDistance[dstGridOffset];
	if (distance <= 0 || distance >= 998) {
		return 0;
	}

	int numTiles = 0;
	int lastDirection = -1;
	int x = xDst;
	int y = yDst;
	int gridOffset = dstGridOffset;
	int step = numDirections == 8 ? 1 : 2;

	while (distance > 1) {
		distance = Data_Grid_routingDistance[gridOffset];
		*xOut = x;
		*yOut = y;
		if (distance <= range) {
			return 1;
		}
		int direction = -1;
		int generalDirection = Routing_getGeneralDirection(x, y, xSrc, ySrc);
		for (int d = 0; d < 8; d += step) {
			if (d != lastDirection) {
				int nextOffset = gridOffset + Constant_DirectionGridOffsets[d];
				int nextDistance = Data_Grid_routingDistance[nextOffset];
				if (nextDistance) {
					if (nextDistance < distance) {
						distance = nextDistance;
						direction = d;
					} else if (nextDistance == distance && (d == generalDirection || direction == -1)) {
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
	return 0;
}

int Routing_getPathOnWater(int routingPathId, int xSrc, int ySrc, int xDst, int yDst, int isFlotsam)
{
	int rand = Data_Random.random1_7bit & 3;
	int dstGridOffset = GridOffset(xDst, yDst);
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
				int nextOffset = gridOffset + Constant_DirectionGridOffsets[d];
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

void Routing_block(int x, int y, int size)
{
	if (IsOutsideMap(x, y, size)) {
		return;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			Data_Grid_routingDistance[GridOffset(x+dx, y+dy)] = 0;
		}
	}
}
