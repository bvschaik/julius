#include "TerrainGraphics.h"

#include "Data/Building.h"
#include "Data/Grid.h"
#include "Data/State.h"

#include "map/building.h"
#include "map/property.h"
#include "map/terrain.h"

#define MAX_TILES 8

struct TerrainGraphicsContext {
	unsigned char tiles[MAX_TILES];
	unsigned char offsetForOrientation[4];
	unsigned char aqueductOffset;
	unsigned char maxItemOffset;
	unsigned char currentItemOffset;
};

// 0 = no match
// 1 = match
// 2 = don't care

static struct TerrainGraphicsContext terrainGraphicsWater[48] = {
	{{1, 2, 1, 2, 1, 2, 1, 2}, {79, 79, 79, 79}, 0, 1},
	{{1, 2, 1, 2, 1, 2, 0, 2}, {47, 46, 45, 44}, 0, 1},
	{{0, 2, 1, 2, 1, 2, 1, 2}, {44, 47, 46, 45}, 0, 1},
	{{1, 2, 0, 2, 1, 2, 1, 2}, {45, 44, 47, 46}, 0, 1},
	{{1, 2, 1, 2, 0, 2, 1, 2}, {46, 45, 44, 47}, 0, 1},
	{{1, 2, 0, 2, 1, 2, 0, 2}, {40, 42, 40, 42}, 0, 2},
	{{0, 2, 1, 2, 0, 2, 1, 2}, {42, 40, 42, 40}, 0, 2},
	{{1, 2, 1, 2, 0, 0, 0, 2}, {32, 28, 24, 36}, 0, 4},
	{{0, 2, 1, 2, 1, 2, 0, 0}, {36, 32, 28, 24}, 0, 4},
	{{0, 0, 0, 2, 1, 2, 1, 2}, {24, 36, 32, 28}, 0, 4},
	{{1, 2, 0, 0, 0, 2, 1, 2}, {28, 24, 36, 32}, 0, 4},
	{{1, 2, 1, 2, 0, 1, 0, 2}, {77, 76, 75, 78}, 0, 1},
	{{0, 2, 1, 2, 1, 2, 0, 1}, {78, 77, 76, 75}, 0, 1},
	{{0, 1, 0, 2, 1, 2, 1, 2}, {75, 78, 77, 76}, 0, 1},
	{{1, 2, 0, 1, 0, 2, 1, 2}, {76, 75, 78, 77}, 0, 1},
	{{1, 2, 0, 0, 0, 0, 0, 2}, {16, 12, 8, 20}, 0, 4},
	{{0, 2, 1, 2, 0, 0, 0, 0}, {20, 16, 12, 8}, 0, 4},
	{{0, 0, 0, 2, 1, 2, 0, 0}, {8, 20, 16, 12}, 0, 4},
	{{0, 0, 0, 0, 0, 2, 1, 2}, {12, 8, 20, 16}, 0, 4},
	{{1, 2, 0, 1, 0, 0, 0, 2}, {69, 66, 63, 72}, 0, 1},
	{{0, 2, 1, 2, 0, 1, 0, 0}, {72, 69, 66, 63}, 0, 1},
	{{0, 0, 0, 2, 1, 2, 0, 1}, {63, 72, 69, 66}, 0, 1},
	{{0, 1, 0, 0, 0, 2, 1, 2}, {66, 63, 72, 69}, 0, 1},
	{{1, 2, 0, 0, 0, 1, 0, 2}, {70, 67, 64, 73}, 0, 1},
	{{0, 2, 1, 2, 0, 0, 0, 1}, {73, 70, 67, 64}, 0, 1},
	{{0, 1, 0, 2, 1, 2, 0, 0}, {64, 73, 70, 67}, 0, 1},
	{{0, 0, 0, 1, 0, 2, 1, 2}, {67, 64, 73, 70}, 0, 1},
	{{1, 2, 0, 1, 0, 1, 0, 2}, {71, 68, 65, 74}, 0, 1},
	{{0, 2, 1, 2, 0, 1, 0, 1}, {74, 71, 68, 65}, 0, 1},
	{{0, 1, 0, 2, 1, 2, 0, 1}, {65, 74, 71, 68}, 0, 1},
	{{0, 1, 0, 1, 0, 2, 1, 2}, {68, 65, 74, 71}, 0, 1},
	{{0, 1, 0, 1, 0, 1, 0, 1}, {62, 62, 62, 62}, 0, 1},
	{{0, 1, 0, 1, 0, 1, 0, 0}, {60, 59, 58, 61}, 0, 1},
	{{0, 0, 0, 1, 0, 1, 0, 1}, {61, 60, 59, 58}, 0, 1},
	{{0, 1, 0, 0, 0, 1, 0, 1}, {58, 61, 60, 59}, 0, 1},
	{{0, 1, 0, 1, 0, 0, 0, 1}, {59, 58, 61, 60}, 0, 1},
	{{0, 1, 0, 0, 0, 1, 0, 0}, {48, 49, 48, 49}, 0, 1},
	{{0, 0, 0, 1, 0, 0, 0, 1}, {49, 48, 49, 48}, 0, 1},
	{{0, 1, 0, 1, 0, 0, 0, 0}, {56, 55, 54, 57}, 0, 1},
	{{0, 0, 0, 1, 0, 1, 0, 0}, {57, 56, 55, 54}, 0, 1},
	{{0, 0, 0, 0, 0, 1, 0, 1}, {54, 57, 56, 55}, 0, 1},
	{{0, 1, 0, 0, 0, 0, 0, 1}, {55, 54, 57, 56}, 0, 1},
	{{0, 1, 0, 0, 0, 0, 0, 0}, {52, 51, 50, 53}, 0, 1},
	{{0, 0, 0, 1, 0, 0, 0, 0}, {53, 52, 51, 50}, 0, 1},
	{{0, 0, 0, 0, 0, 1, 0, 0}, {50, 53, 52, 51}, 0, 1},
	{{0, 0, 0, 0, 0, 0, 0, 1}, {51, 50, 53, 52}, 0, 1},
	{{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0}, 0, 6},
	{{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0},
};

static struct TerrainGraphicsContext terrainGraphicsWall[48] = {
	{{1, 2, 1, 2, 1, 2, 1, 2}, {26, 26, 26, 26}, 0, 1},
	{{1, 2, 1, 2, 1, 2, 0, 2}, {15, 10, 5, 16}, 0, 1},
	{{0, 2, 1, 2, 1, 2, 1, 2}, {16, 15, 10, 5}, 0, 1},
	{{1, 2, 0, 2, 1, 2, 1, 2}, {5, 16, 15, 10}, 0, 1},
	{{1, 2, 1, 2, 0, 2, 1, 2}, {10, 5, 16, 15}, 0, 1},
	{{1, 2, 0, 2, 1, 2, 0, 2}, {1, 4, 1, 4}, 0, 1},
	{{0, 2, 1, 2, 0, 2, 1, 2}, {4, 1, 4, 1}, 0, 1},
	{{1, 2, 1, 2, 0, 0, 0, 2}, {10, 7, 5, 12}, 0, 1},
	{{0, 2, 1, 2, 1, 2, 0, 0}, {12, 10, 7, 5}, 0, 1},
	{{0, 0, 0, 2, 1, 2, 1, 2}, {5, 12, 10, 7}, 0, 1},
	{{1, 2, 0, 0, 0, 2, 1, 2}, {7, 5, 12, 10}, 0, 1},
	{{1, 2, 1, 2, 0, 1, 0, 2}, {10, 22, 5, 12}, 0, 1},
	{{0, 2, 1, 2, 1, 2, 0, 1}, {12, 10, 22, 5}, 0, 1},
	{{0, 1, 0, 2, 1, 2, 1, 2}, {5, 12, 10, 22}, 0, 1},
	{{1, 2, 0, 1, 0, 2, 1, 2}, {22, 5, 12, 10}, 0, 1},
	{{1, 2, 0, 0, 0, 0, 0, 2}, {3, 2, 1, 4}, 0, 1},
	{{0, 2, 1, 2, 0, 0, 0, 0}, {4, 3, 2, 1}, 0, 1},
	{{0, 0, 0, 2, 1, 2, 0, 0}, {1, 4, 3, 2}, 0, 1},
	{{0, 0, 0, 0, 0, 2, 1, 2}, {2, 1, 4, 3}, 0, 1},
	{{1, 2, 0, 1, 0, 0, 0, 2}, {22, 24, 1, 4}, 0, 1},
	{{0, 2, 1, 2, 0, 1, 0, 0}, {4, 22, 24, 1}, 0, 1},
	{{0, 0, 0, 2, 1, 2, 0, 1}, {1, 4, 22, 24}, 0, 1},
	{{0, 1, 0, 0, 0, 2, 1, 2}, {24, 1, 4, 22}, 0, 1},
	{{1, 2, 0, 0, 0, 1, 0, 2}, {25, 22, 1, 4}, 0, 1},
	{{0, 2, 1, 2, 0, 0, 0, 1}, {4, 25, 22, 1}, 0, 1},
	{{0, 1, 0, 2, 1, 2, 0, 0}, {1, 4, 25, 22}, 0, 1},
	{{0, 0, 0, 1, 0, 2, 1, 2}, {22, 1, 4, 25}, 0, 1},
	{{1, 2, 0, 1, 0, 1, 0, 2}, {22, 22, 1, 4}, 0, 1},
	{{0, 2, 1, 2, 0, 1, 0, 1}, {4, 22, 22, 1}, 0, 1},
	{{0, 1, 0, 2, 1, 2, 0, 1}, {1, 4, 22, 22}, 0, 1},
	{{0, 1, 0, 1, 0, 2, 1, 2}, {22, 1, 4, 22}, 0, 1},
	{{0, 1, 0, 1, 0, 1, 0, 1}, {22, 22, 22, 22}, 0, 1},
	{{0, 1, 0, 1, 0, 1, 0, 0}, {22, 22, 23, 22}, 0, 1},
	{{0, 0, 0, 1, 0, 1, 0, 1}, {22, 22, 22, 23}, 0, 1},
	{{0, 1, 0, 0, 0, 1, 0, 1}, {23, 22, 22, 22}, 0, 1},
	{{0, 1, 0, 1, 0, 0, 0, 1}, {22, 23, 22, 22}, 0, 1},
	{{0, 1, 0, 0, 0, 1, 0, 0}, {17, 18, 17, 18}, 0, 1},
	{{0, 0, 0, 1, 0, 0, 0, 1}, {18, 17, 18, 17}, 0, 1},
	{{0, 1, 0, 1, 0, 0, 0, 0}, {22, 21, 19, 22}, 0, 1},
	{{0, 0, 0, 1, 0, 1, 0, 0}, {22, 22, 21, 19}, 0, 1},
	{{0, 0, 0, 0, 0, 1, 0, 1}, {19, 22, 22, 21}, 0, 1},
	{{0, 1, 0, 0, 0, 0, 0, 1}, {21, 19, 22, 22}, 0, 1},
	{{0, 1, 0, 0, 0, 0, 0, 0}, {21, 20, 19, 22}, 0, 1},
	{{0, 0, 0, 1, 0, 0, 0, 0}, {22, 21, 20, 19}, 0, 1},
	{{0, 0, 0, 0, 0, 1, 0, 0}, {19, 22, 21, 20}, 0, 1},
	{{0, 0, 0, 0, 0, 0, 0, 1}, {20, 19, 22, 21}, 0, 1},
	{{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0}, 0, 1},
	{{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0},
};

static struct TerrainGraphicsContext terrainGraphicsWallGatehouse[10] = {
	{{1, 2, 0, 2, 0, 2, 0, 2}, {16, 15, 10, 5}, 0, 1},
	{{0, 2, 1, 2, 0, 2, 0, 2}, {5, 16, 15, 10}, 0, 1},
	{{0, 2, 0, 2, 1, 2, 0, 2}, {10, 5, 16, 15}, 0, 1},
	{{0, 2, 0, 2, 0, 2, 1, 2}, {15, 10, 5, 16}, 0, 1},
	{{1, 2, 1, 2, 0, 2, 0, 2}, {27, 12, 28, 22}, 0, 1},
	{{0, 2, 1, 2, 1, 2, 0, 2}, {22, 27, 12, 28}, 0, 1},
	{{0, 2, 0, 2, 1, 2, 1, 2}, {28, 22, 27, 12}, 0, 1},
	{{1, 2, 0, 2, 0, 2, 1, 2}, {12, 28, 22, 27}, 0, 1},
	{{1, 2, 0, 2, 1, 2, 0, 2}, {31, 32, 31, 32}, 0, 1},
	{{0, 2, 1, 2, 0, 2, 1, 2}, {32, 31, 32, 31}, 0, 1},
};

static struct TerrainGraphicsContext terrainGraphicsElevation[14] = {
	{{1, 1, 1, 1, 1, 1, 1, 1}, {44, 44, 44, 44}, 2, 1},
	{{1, 1, 1, 1, 1, 0, 1, 1}, {30, 18, 28, 22}, 4, 2},
	{{1, 1, 1, 1, 1, 1, 1, 0}, {22, 30, 18, 28}, 4, 2},
	{{1, 0, 1, 1, 1, 1, 1, 1}, {28, 22, 30, 18}, 4, 2},
	{{1, 1, 1, 0, 1, 1, 1, 1}, {18, 28, 22, 30}, 4, 2},
	{{1, 1, 1, 2, 2, 2, 1, 1}, {0, 8, 12, 4}, 4, 4},
	{{1, 1, 1, 1, 1, 2, 2, 2}, {4, 0, 8, 12}, 4, 4},
	{{2, 2, 1, 1, 1, 1, 1, 2}, {12, 4, 0, 8}, 4, 4},
	{{1, 2, 2, 2, 1, 1, 1, 1}, {8, 12, 4, 0}, 4, 4},
	{{1, 1, 1, 2, 2, 2, 2, 2}, {24, 16, 26, 20}, 4, 2},
	{{2, 2, 1, 1, 1, 2, 2, 2}, {20, 24, 16, 26}, 4, 2},
	{{2, 2, 2, 2, 1, 1, 1, 2}, {26, 20, 24, 16}, 4, 2},
	{{1, 2, 2, 2, 2, 2, 1, 1}, {16, 26, 20, 24}, 4, 2},
	{{2, 2, 2, 2, 2, 2, 2, 2}, {32, 32, 32, 32}, 4, 4},
};

static struct TerrainGraphicsContext terrainGraphicsEarthquake[17] = {
	{{1, 2, 1, 2, 1, 2, 1, 2}, {29, 29, 29, 29}, 0, 1},
	{{1, 2, 1, 2, 1, 2, 0, 2}, {25, 28, 27, 26}, 0, 1},
	{{0, 2, 1, 2, 1, 2, 1, 2}, {26, 25, 28, 27}, 0, 1},
	{{1, 2, 0, 2, 1, 2, 1, 2}, {27, 26, 25, 28}, 0, 1},
	{{1, 2, 1, 2, 0, 2, 1, 2}, {28, 27, 26, 25}, 0, 1},
	{{1, 2, 1, 2, 0, 2, 0, 2}, {8, 14, 12, 10}, 0, 2},
	{{0, 2, 1, 2, 1, 2, 0, 2}, {10, 8, 14, 12}, 0, 2},
	{{0, 2, 0, 2, 1, 2, 1, 2}, {12, 10, 8, 14}, 0, 2},
	{{1, 2, 0, 2, 0, 2, 1, 2}, {14, 12, 10, 8}, 0, 2},
	{{1, 2, 0, 2, 1, 2, 0, 2}, {0, 4, 0, 4}, 0, 4},
	{{0, 2, 1, 2, 0, 2, 1, 2}, {4, 0, 4, 0}, 0, 4},
	{{1, 2, 0, 2, 0, 2, 0, 2}, {16, 22, 18, 20}, 0, 2},
	{{0, 2, 1, 2, 0, 2, 0, 2}, {20, 16, 22, 18}, 0, 2},
	{{0, 2, 0, 2, 1, 2, 0, 2}, {18, 20, 16, 22}, 0, 2},
	{{0, 2, 0, 2, 0, 2, 1, 2}, {22, 18, 20, 16}, 0, 2},
	{{0, 2, 0, 2, 0, 2, 0, 2}, {24, 24, 24, 24}, 0, 1},
	{{2, 2, 2, 2, 2, 2, 2, 2}, {24, 24, 24, 24}, 0, 1},
};

static struct TerrainGraphicsContext terrainGraphicsDirtRoad[17] = {
	{{1, 2, 1, 2, 1, 2, 1, 2}, {17, 17, 17, 17}, 0, 1},
	{{1, 2, 1, 2, 1, 2, 0, 2}, {13, 16, 15, 14}, 0, 1},
	{{0, 2, 1, 2, 1, 2, 1, 2}, {14, 13, 16, 15}, 0, 1},
	{{1, 2, 0, 2, 1, 2, 1, 2}, {15, 14, 13, 16}, 0, 1},
	{{1, 2, 1, 2, 0, 2, 1, 2}, {16, 15, 14, 13}, 0, 1},
	{{1, 2, 1, 2, 0, 2, 0, 2}, {4, 7, 6, 5}, 0, 1},
	{{0, 2, 1, 2, 1, 2, 0, 2}, {5, 4, 7, 6}, 0, 1},
	{{0, 2, 0, 2, 1, 2, 1, 2}, {6, 5, 4, 7}, 0, 1},
	{{1, 2, 0, 2, 0, 2, 1, 2}, {7, 6, 5, 4}, 0, 1},
	{{1, 2, 0, 2, 1, 2, 0, 2}, {0, 1, 0, 1}, 0, 1},
	{{0, 2, 1, 2, 0, 2, 1, 2}, {1, 0, 1, 0}, 0, 1},
	{{1, 2, 0, 2, 0, 2, 0, 2}, {8, 11, 10, 9}, 0, 1},
	{{0, 2, 1, 2, 0, 2, 0, 2}, {9, 8, 11, 10}, 0, 1},
	{{0, 2, 0, 2, 1, 2, 0, 2}, {10, 9, 8, 11}, 0, 1},
	{{0, 2, 0, 2, 0, 2, 1, 2}, {11, 10, 9, 8}, 0, 1},
	{{0, 2, 0, 2, 0, 2, 0, 2}, {12, 12, 12, 12}, 0, 1},
	{{2, 2, 2, 2, 2, 2, 2, 2}, {12, 12, 12, 12}, 0, 1},
};

static struct TerrainGraphicsContext terrainGraphicsPavedRoad[48] = {
	{{1, 0, 1, 0, 1, 0, 1, 0}, {17, 17, 17, 17}, 0, 1},
	{{1, 0, 1, 0, 1, 2, 0, 2}, {13, 16, 15, 14}, 0, 1},
	{{1, 1, 1, 1, 1, 2, 0, 2}, {18, 21, 20, 19}, 0, 1},
	{{1, 0, 1, 1, 1, 2, 0, 2}, {26, 33, 32, 31}, 0, 1},
	{{1, 1, 1, 0, 1, 2, 0, 2}, {30, 29, 28, 27}, 0, 1},
	{{0, 2, 1, 0, 1, 0, 1, 2}, {14, 13, 16, 15}, 0, 1},
	{{0, 2, 1, 1, 1, 1, 1, 2}, {19, 18, 21, 20}, 0, 1},
	{{0, 2, 1, 0, 1, 1, 1, 2}, {31, 26, 33, 32}, 0, 1},
	{{0, 2, 1, 1, 1, 0, 1, 2}, {27, 30, 29, 28}, 0, 1},
	{{1, 2, 0, 2, 1, 0, 1, 0}, {15, 14, 13, 16}, 0, 1},
	{{1, 2, 0, 2, 1, 1, 1, 1}, {20, 19, 18, 21}, 0, 1},
	{{1, 2, 0, 2, 1, 0, 1, 1}, {32, 31, 26, 33}, 0, 1},
	{{1, 2, 0, 2, 1, 1, 1, 0}, {28, 27, 30, 29}, 0, 1},
	{{1, 0, 1, 2, 0, 2, 1, 0}, {16, 15, 14, 13}, 0, 1},
	{{1, 1, 1, 2, 0, 2, 1, 1}, {21, 20, 19, 18}, 0, 1},
	{{1, 1, 1, 2, 0, 2, 1, 0}, {33, 32, 31, 26}, 0, 1},
	{{1, 0, 1, 2, 0, 2, 1, 1}, {29, 28, 27, 30}, 0, 1},
	{{1, 1, 1, 2, 0, 0, 0, 2}, {22, 25, 24, 23}, 0, 1},
	{{0, 2, 1, 1, 1, 2, 0, 0}, {23, 22, 25, 24}, 0, 1},
	{{0, 0, 0, 2, 1, 1, 1, 2}, {24, 23, 22, 25}, 0, 1},
	{{1, 2, 0, 0, 0, 2, 1, 1}, {25, 24, 23, 22}, 0, 1},
	{{1, 0, 1, 0, 1, 1, 1, 1}, {34, 37, 36, 35}, 0, 1},
	{{1, 1, 1, 0, 1, 0, 1, 1}, {35, 34, 37, 36}, 0, 1},
	{{1, 1, 1, 1, 1, 0, 1, 0}, {36, 35, 34, 37}, 0, 1},
	{{1, 0, 1, 1, 1, 1, 1, 0}, {37, 36, 35, 34}, 0, 1},
	{{1, 0, 1, 0, 1, 0, 1, 1}, {38, 41, 40, 39}, 0, 1},
	{{1, 1, 1, 0, 1, 0, 1, 0}, {39, 38, 41, 40}, 0, 1},
	{{1, 0, 1, 1, 1, 0, 1, 0}, {40, 39, 38, 41}, 0, 1},
	{{1, 0, 1, 0, 1, 1, 1, 0}, {41, 40, 39, 38}, 0, 1},
	{{1, 1, 1, 1, 1, 0, 1, 1}, {42, 45, 44, 43}, 0, 1},
	{{1, 1, 1, 1, 1, 1, 1, 0}, {43, 42, 45, 44}, 0, 1},
	{{1, 0, 1, 1, 1, 1, 1, 1}, {44, 43, 42, 45}, 0, 1},
	{{1, 1, 1, 0, 1, 1, 1, 1}, {45, 44, 43, 42}, 0, 1},
	{{1, 1, 1, 0, 1, 1, 1, 0}, {46, 47, 46, 47}, 0, 1},
	{{1, 0, 1, 1, 1, 0, 1, 1}, {47, 46, 47, 46}, 0, 1},
	{{1, 2, 1, 2, 0, 2, 0, 2}, {4, 7, 6, 5}, 0, 1},
	{{0, 2, 1, 2, 1, 2, 0, 2}, {5, 4, 7, 6}, 0, 1},
	{{0, 2, 0, 2, 1, 2, 1, 2}, {6, 5, 4, 7}, 0, 1},
	{{1, 2, 0, 2, 0, 2, 1, 2}, {7, 6, 5, 4}, 0, 1},
	{{1, 2, 0, 2, 1, 2, 0, 2}, {0, 1, 0, 1}, 0, 1},
	{{0, 2, 1, 2, 0, 2, 1, 2}, {1, 0, 1, 0}, 0, 1},
	{{1, 2, 0, 2, 0, 2, 0, 2}, {8, 11, 10, 9}, 0, 1},
	{{0, 2, 1, 2, 0, 2, 0, 2}, {9, 8, 11, 10}, 0, 1},
	{{0, 2, 0, 2, 1, 2, 0, 2}, {10, 9, 8, 11}, 0, 1},
	{{0, 2, 0, 2, 0, 2, 1, 2}, {11, 10, 9, 8}, 0, 1},
	{{0, 0, 0, 0, 0, 0, 0, 0}, {12, 12, 12, 12}, 0, 1},
	{{1, 1, 1, 1, 1, 1, 1, 1}, {48, 48, 48, 48}, 0, 1},
	{{2, 2, 2, 2, 2, 2, 2, 2}, {12, 12, 12, 12}, 0, 1},
};

static struct TerrainGraphicsContext terrainGraphicsAqueduct[16] = {
	{{1, 2, 1, 2, 0, 2, 0, 2}, {4, 7, 6, 5}, 7, 1},
	{{0, 2, 1, 2, 1, 2, 0, 2}, {5, 4, 7, 6}, 8, 1},
	{{0, 2, 0, 2, 1, 2, 1, 2}, {6, 5, 4, 7}, 9, 1},
	{{1, 2, 0, 2, 0, 2, 1, 2}, {7, 6, 5, 4}, 10, 1},
	{{1, 2, 0, 2, 1, 2, 0, 2}, {2, 3, 2, 3}, 5, 1},
	{{0, 2, 1, 2, 0, 2, 1, 2}, {3, 2, 3, 2}, 6, 1},
	{{1, 2, 0, 2, 0, 2, 0, 2}, {2, 3, 2, 3}, 1, 1},
	{{0, 2, 1, 2, 0, 2, 0, 2}, {3, 2, 3, 2}, 2, 1},
	{{0, 2, 0, 2, 1, 2, 0, 2}, {2, 3, 2, 3}, 3, 1},
	{{0, 2, 0, 2, 0, 2, 1, 2}, {3, 2, 3, 2}, 4, 1},
	{{1, 2, 1, 2, 1, 2, 0, 2}, {10, 13, 12, 11}, 11, 1},
	{{0, 2, 1, 2, 1, 2, 1, 2}, {11, 10, 13, 12}, 12, 1},
	{{1, 2, 0, 2, 1, 2, 1, 2}, {12, 11, 10, 13}, 13, 1},
	{{1, 2, 1, 2, 0, 2, 1, 2}, {13, 12, 11, 10}, 14, 1},
	{{1, 2, 1, 2, 1, 2, 1, 2}, {14, 14, 14, 14}, 15, 1},
	{{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2}, 0, 1},
};

static struct TerrainGraphicsContext *contextPointers[] = {
	terrainGraphicsWater, // Water
	terrainGraphicsWall, // Wall
	terrainGraphicsWallGatehouse, // WallGatehouse
	terrainGraphicsElevation, // Elevation
	terrainGraphicsEarthquake, // Earthquake
	terrainGraphicsDirtRoad, // DirtRoad
	terrainGraphicsPavedRoad, // PavedRoad
	terrainGraphicsAqueduct, // Aqueduct
};

static const int contextSizes[] = {
	48, // Water
	48, // Wall
	10, // WallGatehouse
	14, // Elevation
	17, // Earthquake
	17, // DirtRoad
	48, // PavedRoad
	16, // Aqueduct
};

static const int contextTileOffsets[] = {
	-162, -161, 1, 163, 162, 161, -1, -163
};

static void clearCurrentOffset(struct TerrainGraphicsContext *items, int numItems)
{
	for (int i = 0; i < numItems; i++) {
		items[i].currentItemOffset = 0;
	}
}

void TerrainGraphicsContext_init()
{
	for (int i = 0; i < TerrainGraphicsContext_NumItems; i++) {
		clearCurrentOffset(contextPointers[i], contextSizes[i]);
	}
}

static int contextMatchesTiles(struct TerrainGraphicsContext *context, int tiles[MAX_TILES])
{
	for (int i = 0; i < MAX_TILES; i++) {
		if (context->tiles[i] != 2 && tiles[i] != context->tiles[i]) {
			return 0;
		}
	}
	return 1;
}

const TerrainGraphic *TerrainGraphicsContext_getGraphic(int group, int tiles[MAX_TILES])
{
	static TerrainGraphic result;

	result.isValid = 0;
	struct TerrainGraphicsContext *context = contextPointers[group];
	int numItems = contextSizes[group];
	for (int i = 0; i < numItems; i++) {
		if (contextMatchesTiles(&context[i], tiles)) {
			context[i].currentItemOffset++;
			if (context[i].currentItemOffset >= context[i].maxItemOffset) {
				context[i].currentItemOffset = 0;
			}
			result.isValid = 1;
			result.groupOffset = context[i].offsetForOrientation[Data_State.map.orientation / 2];
			result.itemOffset = context[i].currentItemOffset;
			result.aqueductOffset = context[i].aqueductOffset;
			break;
		}
	}
	return &result;
}

const TerrainGraphic *TerrainGraphicsContext_getElevation(int gridOffset, int height)
{
	int tiles[MAX_TILES];
	for (int i = 0; i < MAX_TILES; i++) {
		tiles[i] = Data_Grid_elevation[gridOffset + contextTileOffsets[i]] >= height ? 1 : 0;
	}
	return TerrainGraphicsContext_getGraphic(TerrainGraphicsContext_Elevation, tiles);
}

const TerrainGraphic *TerrainGraphicsContext_getEarthquake(int gridOffset)
{
	int tiles[MAX_TILES];
	for (int i = 0; i < MAX_TILES; i++) {
		int offset = gridOffset + contextTileOffsets[i];
		tiles[i] = (map_terrain_is(offset, TERRAIN_ROCK) &&
			map_property_is_plaza_or_earthquake(gridOffset)) ? 1 : 0;
	}
	return TerrainGraphicsContext_getGraphic(TerrainGraphicsContext_Earthquake, tiles);
}

static void getTerrainMatch(int gridOffset, int terrain, int match, int noMatch, int tiles[MAX_TILES])
{
	for (int i = 0; i < MAX_TILES; i++) {
		tiles[i] = map_terrain_is(gridOffset + contextTileOffsets[i], terrain) ? match : noMatch;
	}
}

const TerrainGraphic *TerrainGraphicsContext_getShore(int gridOffset)
{
	int tiles[MAX_TILES];
	getTerrainMatch(gridOffset, TERRAIN_WATER, 0, 1, tiles);
	return TerrainGraphicsContext_getGraphic(TerrainGraphicsContext_Water, tiles);
}

const TerrainGraphic *TerrainGraphicsContext_getWall(int gridOffset)
{
	int tiles[MAX_TILES];
	getTerrainMatch(gridOffset, TERRAIN_WALL, 0, 1, tiles);
	return TerrainGraphicsContext_getGraphic(TerrainGraphicsContext_Wall, tiles);
}

const TerrainGraphic *TerrainGraphicsContext_getWallGatehouse(int gridOffset)
{
	int tiles[MAX_TILES] = {0,0,0,0,0,0,0,0};
	for (int i = 0; i < MAX_TILES; i += 2) {
		tiles[i] = map_terrain_is(gridOffset + contextTileOffsets[i], TERRAIN_WALL_OR_GATEHOUSE) ? 1 : 0;
	}
	return TerrainGraphicsContext_getGraphic(TerrainGraphicsContext_WallGatehouse, tiles);
}

static void setTilesRoad(int gridOffset, int tiles[MAX_TILES])
{
	getTerrainMatch(gridOffset, TERRAIN_ROAD, 1, 0, tiles);
	for (int i = 0; i < MAX_TILES; i += 2) {
		int offset = gridOffset + contextTileOffsets[i];
		if (map_terrain_is(offset, TERRAIN_GATEHOUSE)) {
			int buildingId = map_building_at(offset);
			if (Data_Buildings[buildingId].type == BUILDING_GATEHOUSE &&
				Data_Buildings[buildingId].subtype.orientation == 1 + ((i / 2) & 1)) { // 1,2,1,2
				tiles[i] = 1;
			}
		}
	}
}

const TerrainGraphic *TerrainGraphicsContext_getDirtRoad(int gridOffset)
{
	int tiles[MAX_TILES];
	setTilesRoad(gridOffset, tiles);
	return TerrainGraphicsContext_getGraphic(TerrainGraphicsContext_DirtRoad, tiles);
}

const TerrainGraphic *TerrainGraphicsContext_getPavedRoad(int gridOffset)
{
	int tiles[MAX_TILES];
	setTilesRoad(gridOffset, tiles);
	return TerrainGraphicsContext_getGraphic(TerrainGraphicsContext_PavedRoad, tiles);
}

static void setTerrainReservoir(int gridOffset, int index, int edgeMask, int tiles[MAX_TILES])
{
	int offset = gridOffset + contextTileOffsets[index];
	if (map_terrain_is(offset, TERRAIN_BUILDING)) {
		int buildingId = map_building_at(offset);
		if (Data_Buildings[buildingId].type == BUILDING_RESERVOIR &&
			map_property_multi_tile_xy(offset) == edgeMask) {
			tiles[index] = 1;
		}
	}
}

const TerrainGraphic *TerrainGraphicsContext_getAqueduct(int gridOffset, int includeOverlay)
{
	int tiles[MAX_TILES] = {0,0,0,0,0,0,0,0};
	int hasRoad = map_terrain_is(gridOffset, TERRAIN_ROAD) ? 1 : 0;
	for (int i = 0; i < MAX_TILES; i += 2) {
		int offset = gridOffset + contextTileOffsets[i];
		if (map_terrain_is(offset, TERRAIN_AQUEDUCT)) {
			if (hasRoad) {
				if (!map_terrain_is(offset, TERRAIN_ROAD)) {
					tiles[i] = 1;
				}
			} else {
				tiles[i] = 1;
			}
		}
	}
	setTerrainReservoir(gridOffset, 0, Edge_X1Y2, tiles);
	setTerrainReservoir(gridOffset, 2, Edge_X0Y1, tiles);
	setTerrainReservoir(gridOffset, 4, Edge_X1Y0, tiles);
	setTerrainReservoir(gridOffset, 6, Edge_X2Y1, tiles);
	if (includeOverlay) {
		for (int i = 0; i < MAX_TILES; i += 2) {
			if (map_property_is_constructing(gridOffset + contextTileOffsets[i])) {
				tiles[i] = 1;
			}
		}
	}
	return TerrainGraphicsContext_getGraphic(TerrainGraphicsContext_Aqueduct, tiles);
}

int TerrainGraphicsContext_getNumWaterTiles(int gridOffset)
{
	int amount = 0;
	for (int i = 0; i < MAX_TILES; i++) {
		if (map_terrain_is(gridOffset + contextTileOffsets[i], TERRAIN_WATER)) {
			amount++;
		}
	}
	return amount;
}
