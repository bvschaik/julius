#ifndef ROUTING_H
#define ROUTING_H

#include <stdint.h>
#include "map/routing.h"

void Routing_determineLandCitizen();
void Routing_determineLandNonCitizen();
void Routing_determineWater();
void Routing_determineWalls();

void Routing_clearLandTypeCitizen();

void Routing_getDistance(int x, int y);

void Routing_deleteClosestWallOrAqueduct(int x, int y);

int Routing_canTravelOverLandCitizen(int xSrc, int ySrc, int xDst, int yDst);
int Routing_canTravelOverRoadGardenCitizen(int xSrc, int ySrc, int xDst, int yDst);
int Routing_canTravelOverWalls(int xSrc, int ySrc, int xDst, int yDst);
int Routing_canTravelOverLandNonCitizen(int xSrc, int ySrc, int xDst, int yDst, int onlyThroughBuildingId, int maxTiles);
int Routing_canTravelThroughEverythingNonCitizen(int xSrc, int ySrc, int xDst, int yDst);

int Routing_getPath(uint8_t *path, int xSrc, int ySrc, int xDst, int yDst, int numDirections);

int Routing_canPlaceRoadUnderAqueduct(int gridOffset);
int Routing_getAqueductGraphicOffsetWithRoad(int gridOffset);

int Routing_getDistanceForBuildingRoadOrAqueduct(int x, int y, int isAqueduct);
int Routing_getDistanceForBuildingWall(int x, int y);

typedef enum
{
    RoutedBUILDING_ROAD = 0,
    RoutedBUILDING_WALL = 1,
    RoutedBuilding_Aqueduct = 2,
    RoutedBuilding_AqueductWithoutGraphic = 4,
} RoutedBuilding;

int Routing_placeRoutedBuilding(int xSrc, int ySrc, int xDst, int yDst, RoutedBuilding type, int *items);

void Routing_getDistanceWaterBoat(int x, int y);
void Routing_getDistanceWaterFlotsam(int x, int y);
int Routing_getPathOnWater(uint8_t *path, int xSrc, int ySrc, int xDst, int yDst, int isFlotsam);

int Routing_getClosestXYWithinRange(int numDirections, int xSrc, int ySrc, int xDst, int yDst, int range, int *xOut, int *yOut);

void Routing_block(int x, int y, int size);

#endif
