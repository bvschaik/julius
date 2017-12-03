#ifndef ROUTING_H
#define ROUTING_H

#include <stdint.h>

int Routing_getPath(uint8_t *path, int xSrc, int ySrc, int xDst, int yDst, int numDirections);

typedef enum
{
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
