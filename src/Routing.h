#ifndef ROUTING_H
#define ROUTING_H

#include <stdint.h>

enum {
	Routing_Citizen_0_Road = 0,
	Routing_Citizen_2_PassableTerrain = 2,
	Routing_Citizen_4_ClearTerrain = 4,
	Routing_Citizen_m1_Blocked = -1,
	Routing_Citizen_m3_Aqueduct = -3,
	Routing_Citizen_m4_ReservoirConnector = -4,
	
	Routing_NonCitizen_0_Passable = 0,
	Routing_NonCitizen_1_Building = 1,
	Routing_NonCitizen_2_Clearable = 2,
	Routing_NonCitizen_3_Wall = 3,
	Routing_NonCitizen_4_Gatehouse = 4,
	Routing_NonCitizen_5_Fort = 5,
	Routing_NonCitizen_m1_Blocked = -1,
	
	Routing_Water_0_Passable = 0,
	Routing_Water_m1_Blocked = -1,
	Routing_Water_m2_MapEdge = -2,
	Routing_Water_m3_LowBridge = -3,
	
	Routing_Wall_0_Passable = 0,
	Routing_Wall_m1_Blocked = -1,
};

int Routing_getPath(uint8_t *path, int xSrc, int ySrc, int xDst, int yDst, int numDirections);

typedef enum {
	RoutedBUILDING_ROAD = 0,
	RoutedBUILDING_WALL = 1,
	RoutedBuilding_Aqueduct = 2,
	RoutedBuilding_AqueductWithoutGraphic = 4,
} RoutedBuilding;

int Routing_placeRoutedBuilding(int xSrc, int ySrc, int xDst, int yDst, RoutedBuilding type, int *items);

int Routing_getPathOnWater(uint8_t *path, int xSrc, int ySrc, int xDst, int yDst, int isFlotsam);

int Routing_getClosestXYWithinRange(int numDirections, int xSrc, int ySrc, int xDst, int yDst, int range, int *xOut, int *yOut);

void Routing_block(int x, int y, int size);

#endif
