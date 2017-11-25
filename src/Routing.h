#ifndef ROUTING_H
#define ROUTING_H

#include <stdint.h>

int Routing_getPath(uint8_t *path, int xSrc, int ySrc, int xDst, int yDst, int numDirections);

int Routing_getPathOnWater(uint8_t *path, int xSrc, int ySrc, int xDst, int yDst, int isFlotsam);

int Routing_getClosestXYWithinRange(int numDirections, int xSrc, int ySrc, int xDst, int yDst, int range, int *xOut, int *yOut);


#endif
