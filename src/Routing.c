#include "Routing.h"

#include "TerrainGraphics.h"

#include "Data/Building.h"
#include "Data/Constants.h"
#include "Data/Grid.h"
#include "Data/Routes.h"
#include "Data/Figure.h"
#include "Data/State.h"

#include "core/calc.h"
#include "core/random.h"
#include "graphics/image.h"
#include "map/grid.h"

static int directionPath[500];

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
			case RoutedBUILDING_ROAD:
				*items += TerrainGraphics_setTileRoad(xDst, yDst);
				break;
			case RoutedBUILDING_WALL:
				*items += TerrainGraphics_setTileWall(xDst, yDst);
				break;
			case RoutedBuilding_Aqueduct:
				*items += TerrainGraphics_setTileAqueductTerrain(xDst, yDst);
				break;
			case RoutedBuilding_AqueductWithoutGraphic:
				*items += 1;
				break;
		}
		int direction = calc_general_direction(xDst, yDst, xSrc, ySrc);
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

int Routing_getPath(uint8_t *path, int xSrc, int ySrc, int xDst, int yDst, int numDirections)
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
		int generalDirection = calc_general_direction(x, y, xSrc, ySrc);
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
		path[i] = directionPath[numTiles - i - 1];
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
		int generalDirection = calc_general_direction(x, y, xSrc, ySrc);
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

int Routing_getPathOnWater(uint8_t *path, int xSrc, int ySrc, int xDst, int yDst, int isFlotsam)
{
	int rand = random_byte() & 3;
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
		path[i] = directionPath[numTiles - i - 1];
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
