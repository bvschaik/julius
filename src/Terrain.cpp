#include "Terrain.h"

#include "Routing.h"
#include "TerrainGraphics.h"
#include "Util.h"

#include "Data/Building.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Settings.h"

static const int tilesAroundBuildingGridOffsets[][20] = {
	{0},
	{-162, 1, 162, -1, 0},
	{-162, -161, 2, 164, 325, 324, 161, -1, 0},
	{-162, -161, -160, 3, 165, 327, 488, 487, 486, 323, 161, -1, 0},
	{-162, -161, -160, -159, 4, 166, 328, 490, 651, 650, 649, 628, 485, 323, 161, -1, 0},
	{-162, -161, -160, -159, -158, 5, 167, 329, 491, 653, 814, 813, 812, 811, 810, 647, 485, 323, 161, -1},
};

static const int tileEdgeSizeOffsets[5][5] = {
	{0, 1, 2, 3, 4},
	{8, 9, 10, 11, 12},
	{16, 17, 18, 19, 20},
	{24, 25, 26, 27, 28},
	{32, 33, 34, 35, 36},
};

struct RingTile {
	int x;
	int y;
	int gridOffset;
} ringTiles[1080];

// ringIndex[SIZE][DIST]
int ringIndex[6][7];

#define RING_SIZE(s,d) (4 * ((s) - 1) + 8 * (d))

#define FOR_XY_ADJACENT(block) \
	int baseOffset = GridOffset(x, y);\
	for (int i = 0; i < 20; i++) {\
		if (!tilesAroundBuildingGridOffsets[size][i]) break;\
		int gridOffset = baseOffset + tilesAroundBuildingGridOffsets[size][i];\
		block;\
	}

#define STORE_XY_ADJACENT(xTile,yTile) \
	*(xTile) = x + (tilesAroundBuildingGridOffsets[size][i] + 172) % 162 - 10;\
	*(yTile) = y + (tilesAroundBuildingGridOffsets[size][i] + 162) / 161 - 1;

#define FOR_XY_RADIUS(block) \
	int xMin = x - radius;\
	int yMin = y - radius;\
	int xMax = x + size + radius - 1;\
	int yMax = y + size + radius - 1;\
	if (xMin < 0) xMin = 0;\
	if (yMin < 0) yMin = 0;\
	if (xMax >= Data_Settings_Map.width) xMax = Data_Settings_Map.width - 1;\
	if (yMax >= Data_Settings_Map.height) yMax = Data_Settings_Map.height - 1;\
	int gridOffset = GridOffset(xMin, yMin);\
	for (int yy = yMin; yy <= yMax; yy++) {\
		for (int xx = xMin; xx <= xMax; xx++) {\
			block;\
			++gridOffset;\
		}\
		gridOffset += 162 - (xMax - xMin + 1);\
	}

#define STORE_XY_RADIUS(xTile,yTile) \
	*(xTile) = xx; *(yTile) = yy;

void Terrain_addBuildingToGrids(int buildingId, int x, int y, int size, int graphicId, int terrain)
{
	if (IsOutsideMap(x, y, size)) {
		return;
	}
	int xLeftmost, yLeftmost;
	switch (Data_Settings_Map.orientation) {
		case Direction_Top:
			xLeftmost = 0;
			yLeftmost = size - 1;
			break;
		case Direction_Right:
			xLeftmost = yLeftmost = 0;
			break;
		case Direction_Bottom:
			xLeftmost = size - 1;
			yLeftmost = 0;
			break;
		case Direction_Left:
			xLeftmost = yLeftmost = size - 1;
			break;
		default:
			return;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = GridOffset(x + dx, y + dy);
			Data_Grid_terrain[gridOffset] &= Terrain_2e80;
			Data_Grid_terrain[gridOffset] |= terrain;
			Data_Grid_buildingIds[gridOffset] = buildingId;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			switch (size) {
				case 1: Data_Grid_bitfields[gridOffset] |= Bitfield_Size1; break;
				case 2: Data_Grid_bitfields[gridOffset] |= Bitfield_Size2; break;
				case 3: Data_Grid_bitfields[gridOffset] |= Bitfield_Size3; break;
				case 4: Data_Grid_bitfields[gridOffset] |= Bitfield_Size4; break;
				case 5: Data_Grid_bitfields[gridOffset] |= Bitfield_Size5; break;
			}
			Data_Grid_graphicIds[gridOffset] = graphicId;
			Data_Grid_edge[gridOffset] = tileEdgeSizeOffsets[dy][dx];
			if (dx == xLeftmost && dy == yLeftmost) {
				Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
			}
		}
	}
}

static int getNorthTileGridOffset(int x, int y, int *size)
{
	int gridOffset = GridOffset(x, y);
	switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
		default:
		case Bitfield_Size1: *size = 1; break;
		case Bitfield_Size2: *size = 2; break;
		case Bitfield_Size3: *size = 3; break;
		case Bitfield_Size4: *size = 4; break;
		case Bitfield_Size5: *size = 5; break;
	}
	for (int i = 0; i < *size && (Data_Grid_edge[gridOffset] & Edge_MaskX); i++) {
		gridOffset--;
	}
	for (int i = 0; i < *size && (Data_Grid_edge[gridOffset] & Edge_MaskY); i++) {
		gridOffset -= GRID_SIZE;
	}
	return gridOffset;
}

void Terrain_removeBuildingFromGrids(int buildingId, int x, int y)
{
	if (IsOutsideMap(x, y, 1)) {
		return;
	}
	int size;
	int baseGridOffset = getNorthTileGridOffset(x, y, &size);
	x = GridOffsetToX(baseGridOffset);
	y = GridOffsetToY(baseGridOffset);
	if (Data_Grid_terrain[baseGridOffset] == Terrain_Rock) {
		return;
	}
	if (buildingId) {
		if (BuildingIsFarm(Data_Buildings[buildingId].type)) {
			size = 3;
		}
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = GridOffset(x + dx, y + dy);
			if (buildingId && Data_Grid_buildingIds[gridOffset] != buildingId) {
				continue;
			}
			if (buildingId && Data_Buildings[buildingId].type != Building_BurningRuin) {
				Data_Grid_rubbleBuildingType[gridOffset] = (unsigned char) Data_Buildings[buildingId].type;
			}
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			Data_Grid_edge[gridOffset] &= Edge_NativeLand;
			Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
			Data_Grid_aqueducts[gridOffset] = 0;
			Data_Grid_buildingIds[gridOffset] = 0;
			Data_Grid_buildingDamage[gridOffset] = 0;
			Data_Grid_spriteOffsets[gridOffset] = 0;
			if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
				Data_Grid_terrain[gridOffset] &= Terrain_Water;
				TerrainGraphics_setTileWater(x + dx, y + dy);
			} else {
				Data_Grid_graphicIds[gridOffset] =
					GraphicId(ID_Graphic_TerrainUglyGrass) +
					(Data_Grid_random[gridOffset] & 7);
				Data_Grid_terrain[gridOffset] &= Terrain_2e80;
			}
		}
	}
	TerrainGraphics_updateRegionEmptyLand(x, y, x + size, y + size);
	TerrainGraphics_updateRegionMeadow(x, y, x + size, y + size);
	TerrainGraphics_updateRegionRubble(x, y, x + size, y + size);
}

void Terrain_addWatersideBuildingToGrids(int buildingId, int x, int y, int size, int graphicId)
{
	if (IsOutsideMap(x, y, size)) {
		return;
	}
	int xLeftmost;
	int yLeftmost;
	switch (Data_Settings_Map.orientation) {
		case Direction_Top:
			xLeftmost = 0;
			yLeftmost = size - 1;
			break;
		case Direction_Right:
			xLeftmost = yLeftmost = 0;
			break;
		case Direction_Bottom:
			xLeftmost = size - 1;
			yLeftmost = 0;
			break;
		case Direction_Left:
			xLeftmost = yLeftmost = size - 1;
			break;
		default:
			return;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = GridOffset(x + dx, y + dy);
			Data_Grid_terrain[gridOffset] |= Terrain_Building;
			if (!(Data_Grid_terrain[gridOffset] & Terrain_Water)) {
				Data_Grid_terrain[gridOffset] &= Terrain_2e80;
				Data_Grid_terrain[gridOffset] |= Terrain_Building;
			}
			Data_Grid_buildingIds[gridOffset] = buildingId;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
			Data_Grid_bitfields[gridOffset] |= Bitfield_Size2;
			Data_Grid_graphicIds[gridOffset] = graphicId;
			Data_Grid_edge[gridOffset] = EdgeXY(dx, dy);
			if (dx == xLeftmost && dy == yLeftmost) {
				Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
			}
		}
	}
}

#define ADD_ROAD(g) \
	if (!(Data_Grid_terrain[g] & Terrain_NotClear)) \
		Data_Grid_terrain[g] |= Terrain_Road;

void Terrain_addRoadsForGatehouse(int x, int y, int orientation)
{
	// roads under gatehouse
	Data_Grid_terrain[GridOffset(x,y)] |= Terrain_Road;
	Data_Grid_terrain[GridOffset(x+1,y)] |= Terrain_Road;
	Data_Grid_terrain[GridOffset(x,y+1)] |= Terrain_Road;
	Data_Grid_terrain[GridOffset(x+1,y+1)] |= Terrain_Road;

	// free roads before/after gate
	if (orientation == 1) {
		ADD_ROAD(GridOffset(x, y-1));
		ADD_ROAD(GridOffset(x+1, y-1));
		ADD_ROAD(GridOffset(x, y+2));
		ADD_ROAD(GridOffset(x+1, y+2));
	} else if (orientation == 2) {
		ADD_ROAD(GridOffset(x-1, y));
		ADD_ROAD(GridOffset(x-1, y+1));
		ADD_ROAD(GridOffset(x+2, y));
		ADD_ROAD(GridOffset(x+2, y+1));
	}
}

void Terrain_addRoadsForTriumphalArch(int x, int y, int orientation)
{
	if (orientation == 1) {
		// road in the middle
		Data_Grid_terrain[GridOffset(x+1,y)] |= Terrain_Road;
		Data_Grid_terrain[GridOffset(x+1,y+1)] |= Terrain_Road;
		Data_Grid_terrain[GridOffset(x+1,y+2)] |= Terrain_Road;
		// no roads on other tiles
		Data_Grid_terrain[GridOffset(x,y)] &= ~Terrain_Road;
		Data_Grid_terrain[GridOffset(x,y+1)] &= ~Terrain_Road;
		Data_Grid_terrain[GridOffset(x,y+2)] &= ~Terrain_Road;
		Data_Grid_terrain[GridOffset(x+2,y)] &= ~Terrain_Road;
		Data_Grid_terrain[GridOffset(x+2,y+1)] &= ~Terrain_Road;
		Data_Grid_terrain[GridOffset(x+2,y+2)] &= ~Terrain_Road;
	} else if (orientation == 2) {
		// road in the middle
		Data_Grid_terrain[GridOffset(x,y+1)] |= Terrain_Road;
		Data_Grid_terrain[GridOffset(x+1,y+1)] |= Terrain_Road;
		Data_Grid_terrain[GridOffset(x+2,y+1)] |= Terrain_Road;
		// no roads on other tiles
		Data_Grid_terrain[GridOffset(x,y)] &= ~Terrain_Road;
		Data_Grid_terrain[GridOffset(x+1,y)] &= ~Terrain_Road;
		Data_Grid_terrain[GridOffset(x+2,y)] &= ~Terrain_Road;
		Data_Grid_terrain[GridOffset(x,y+2)] &= ~Terrain_Road;
		Data_Grid_terrain[GridOffset(x+1,y+2)] &= ~Terrain_Road;
		Data_Grid_terrain[GridOffset(x+2,y+2)] &= ~Terrain_Road;
	}
}

int Terrain_hasRoadAccess(int x, int y, int size, int *roadX, int *roadY)
{
	int minValue = 12;
	int minGridOffset = GridOffset(x, y);
	FOR_XY_ADJACENT(
		if (!(Data_Grid_terrain[gridOffset] & Terrain_Building) ||
			Data_Buildings[Data_Grid_buildingIds[gridOffset]].type != Building_Gatehouse) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
				// TODO something with determining minValue
				return 1;
			}
		}
	);
	if (minValue < 12) {
		if (roadX && roadY) {
			*roadX = GridOffsetToX(minGridOffset);
			*roadY = GridOffsetToY(minGridOffset);
		}
		return 1;
	}
	return 0;
}

int Terrain_hasRoadAccessHippodrome(int x, int y, int *roadX, int *roadY)
{
	// TODO
	return 0;
}

int Terrain_hasRoadAccessGranary(int x, int y, int *roadX, int *roadY)
{
	// TODO
	return 1;
}

#define Delta(x, y) ((y) * GRID_SIZE + (x))
int Terrain_getOrientationGatehouse(int x, int y)
{
	switch (Data_Settings_Map.orientation) {
		case Direction_Right: x--; break;
		case Direction_Bottom: x--; y--; break;
		case Direction_Left: y--; break;
	}
	int gridOffset = GridOffset(x, y);
	int numRoadTilesTop = 0;
	int numRoadTilesRight = 0;
	int numRoadTilesBottom = 0;
	int numRoadTilesLeft = 0;
	int numRoadTilesWithin = 0;
	int roadTilesWithin = 0;
	// tiles within gate, flags:
	// 1  2
	// 4  8
	if (Data_Grid_terrain[GridOffset(x, y)] & Terrain_Road) {
		roadTilesWithin |= 1;
		numRoadTilesWithin++;
	}
	if (Data_Grid_terrain[gridOffset + Delta(1, 0)] & Terrain_Road) {
		roadTilesWithin |= 2;
		numRoadTilesWithin++;
	}
	if (Data_Grid_terrain[gridOffset + Delta(0, 1)] & Terrain_Road) {
		roadTilesWithin |= 4;
		numRoadTilesWithin++;
	}
	if (Data_Grid_terrain[gridOffset + Delta(1, 1)] & Terrain_Road) {
		roadTilesWithin |= 8;
		numRoadTilesWithin++;
	}

	if (numRoadTilesWithin != 2 && numRoadTilesWithin != 4) {
		return 0;
	}
	if (numRoadTilesWithin == 2) {
		if (roadTilesWithin == 6 || roadTilesWithin == 9) { // diagonals
			return 0;
		}
		if (roadTilesWithin == 5 || roadTilesWithin == 10) { // top to bottom
			return 1;
		}
		if (roadTilesWithin == 3 || roadTilesWithin == 12) { // left to right
			return 2;
		}
		return 0;
	}
	// all 4 tiles are road: check adjacent roads
	// top
	if (Data_Grid_terrain[gridOffset + Delta(0, -1)] & Terrain_Road) {
		numRoadTilesTop++;
	}
	if (Data_Grid_terrain[gridOffset + Delta(1, -1)] & Terrain_Road) {
		numRoadTilesTop++;
	}
	// bottom
	if (Data_Grid_terrain[gridOffset + Delta(0, 2)] & Terrain_Road) {
		numRoadTilesBottom++;
	}
	if (Data_Grid_terrain[gridOffset + Delta(1, 2)] & Terrain_Road) {
		numRoadTilesBottom++;
	}
	// left
	if (Data_Grid_terrain[gridOffset + Delta(-1, 0)] & Terrain_Road) {
		numRoadTilesLeft++;
	}
	if (Data_Grid_terrain[gridOffset + Delta(-1, 1)] & Terrain_Road) {
		numRoadTilesLeft++;
	}
	// right
	if (Data_Grid_terrain[gridOffset + Delta(2, 0)] & Terrain_Road) {
		numRoadTilesRight++;
	}
	if (Data_Grid_terrain[gridOffset + Delta(2, 1)] & Terrain_Road) {
		numRoadTilesRight++;
	}
	// determine direction
	if (numRoadTilesTop || numRoadTilesBottom) {
		if (numRoadTilesLeft || numRoadTilesRight) {
			return 0;
		}
		return 1;
	} else if (numRoadTilesLeft || numRoadTilesRight) {
		return 2;
	}
	return 0;
}

int Terrain_getOrientationTriumphalArch(int x, int y)
{
	switch (Data_Settings_Map.orientation) {
		case Direction_Right: x -= 2; break;
		case Direction_Bottom: x -= 2; y -= 2; break;
		case Direction_Left: y -= 2; break;
	}
	int numRoadTilesTopBottom = 0;
	int numRoadTilesLeftRight = 0;
	int numBlockedTiles = 0;

	int gridOffset = GridOffset(x, y);
	// check corner tiles
	if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	if (Data_Grid_terrain[gridOffset + Delta(2, 0)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	if (Data_Grid_terrain[gridOffset + Delta(0, 2)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	if (Data_Grid_terrain[gridOffset + Delta(2, 2)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	// road tiles top to bottom
	if ((Data_Grid_terrain[gridOffset + Delta(1, 0)] & Terrain_NotClear) == Terrain_Road) {
		numRoadTilesTopBottom++;
	} else if (Data_Grid_terrain[gridOffset + Delta(1, 0)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	if ((Data_Grid_terrain[gridOffset + Delta(1, 2)] & Terrain_NotClear) == Terrain_Road) {
		numRoadTilesTopBottom++;
	} else if (Data_Grid_terrain[gridOffset + Delta(1, 2)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	// road tiles left to right
	if ((Data_Grid_terrain[gridOffset + Delta(0, 1)] & Terrain_NotClear) == Terrain_Road) {
		numRoadTilesLeftRight++;
	} else if (Data_Grid_terrain[gridOffset + Delta(0, 1)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	if ((Data_Grid_terrain[gridOffset + Delta(2, 1)] & Terrain_NotClear) == Terrain_Road) {
		numRoadTilesLeftRight++;
	} else if (Data_Grid_terrain[gridOffset + Delta(2, 1)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	// center tile
	if ((Data_Grid_terrain[gridOffset + Delta(2, 1)] & Terrain_NotClear) == Terrain_Road) {
		// do nothing
	} else if (Data_Grid_terrain[gridOffset + Delta(2, 1)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	// judgement time
	if (numBlockedTiles) {
		return 0;
	}
	if (!numRoadTilesLeftRight && !numRoadTilesTopBottom) {
		return 0; // don't care about direction
	}
	if (numRoadTilesTopBottom == 2 && !numRoadTilesLeftRight) {
		return 1;
	}
	if (numRoadTilesLeftRight == 2 && !numRoadTilesTopBottom) {
		return 2;
	}
	return 0;
}

static int getRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile);
int Terrain_getClosestRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile)
{
	for (int r = 1; r <= radius; r++) {
		if (getRoadWithinRadius(x, y, size, r, xTile, yTile)) {
			return 1;
		}
	}
	return 0;
}

static int getRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile)
{
	FOR_XY_RADIUS(
		if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
			if (xTile && yTile) {
				STORE_XY_RADIUS(xTile, yTile);
			}
			return 1;
		}
	);
	return 0;
}

int Terrain_isClear(int x, int y, int size, int disallowedTerrain, int graphicSet)
{
	if (IsOutsideMap(x, y, size)) {
		return 0;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = GridOffset(x + dx, y + dy);
			if (Data_Grid_terrain[gridOffset] & Terrain_NotClear & disallowedTerrain) {
				return 0;
			} else if (Data_Grid_walkerIds[gridOffset]) {
				return 0;
			} else if (graphicSet && Data_Grid_graphicIds[gridOffset] != 0) {
				return 0;
			}
		}
	}
	return 1;
}

int Terrain_isAdjacentToWall(int x, int y, int size)
{
	FOR_XY_ADJACENT(
		if (Data_Grid_terrain[gridOffset] & Terrain_Wall) {
			return 1;
		}
	);
	return 0;
}

int Terrain_isAdjacentToWater(int x, int y, int size)
{
	FOR_XY_ADJACENT(
		if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
			return 1;
		}
	);
	return 0;
}

int Terrain_isAdjacentToOpenWater(int x, int y, int size)
{
	FOR_XY_ADJACENT(
		if ((Data_Grid_terrain[gridOffset] & Terrain_Water) &&
			Routing_getCalculatedDistance(gridOffset)) {
			return 1;
		}
	);
	return 0;
}

int Terrain_getAdjacentRoadOrClearLand(int x, int y, int size, int *xTile, int *yTile)
{
	FOR_XY_ADJACENT(
		if ((Data_Grid_terrain[gridOffset] & Terrain_Road) ||
			!(Data_Grid_terrain[gridOffset] & Terrain_NotClear)) {
			STORE_XY_ADJACENT(xTile, yTile);
			return 1;
		}
	);
	return 0;
}

void Terrain_setWithRadius(int x, int y, int size, int radius, unsigned short typeToAdd)
{
	FOR_XY_RADIUS(
		Data_Grid_terrain[gridOffset] |= typeToAdd;
	);
}

void Terrain_clearWithRadius(int x, int y, int size, int radius, unsigned short typeToKeep)
{
	FOR_XY_RADIUS(
		Data_Grid_terrain[gridOffset] &= typeToKeep;
	);
}

int Terrain_existsTileWithinAreaWithType(int x, int y, int size, unsigned short type)
{
	return Terrain_existsTileWithinRadiusWithType(x, y, size, 0, type);
}

int Terrain_existsTileWithinRadiusWithType(int x, int y, int size, int radius, unsigned short type)
{
	FOR_XY_RADIUS(
		if (type & Data_Grid_terrain[gridOffset]) {
			return 1;
		}
	);
	return 0;
}

int Terrain_existsClearTileWithinRadius(int x, int y, int size, int radius, int exceptGridOffset)
{
	FOR_XY_RADIUS(
		if (gridOffset != exceptGridOffset && !Data_Grid_terrain[gridOffset]) {
			return 1;
		}
	);
	return 0;
}

int Terrain_allTilesWithinRadiusHaveType(int x, int y, int size, int radius, unsigned short type)
{
	FOR_XY_RADIUS(
		if (!(type & Data_Grid_terrain[gridOffset])) {
			return 0;
		}
	);
	return 1;
}

void Terrain_markBuildingsWithinWellRadius(int buildingId, int radius)
{
	int x = Data_Buildings[buildingId].x;
	int y = Data_Buildings[buildingId].y;
	int size = 1;
	FOR_XY_RADIUS(
		if (Data_Grid_buildingIds[gridOffset]) {
			Data_Buildings[Data_Grid_buildingIds[gridOffset]].hasWellAccess = 1;
		}
	);
}

int Terrain_allHousesWithinWellRadiusHaveFountain(int buildingId, int radius)
{
	int numHouses = 0;
	int x = Data_Buildings[buildingId].x;
	int y = Data_Buildings[buildingId].y;
	int size = 1;
	FOR_XY_RADIUS(
		int buildingId = Data_Grid_buildingIds[gridOffset];
		if (buildingId > 0 && Data_Buildings[buildingId].houseSize) {
			numHouses++;
			if (!(Data_Grid_terrain[gridOffset] & Terrain_FountainRange)) {
				return 0;
			}
		}
	);
	return numHouses ? 1 : 2;
}

void Terrain_markNativeLand(int x, int y, int size, int radius)
{
	FOR_XY_RADIUS(
		Data_Grid_edge[gridOffset] |= 0x80;
	);
}

int Terrain_hasBuildingOnNativeLand(int x, int y, int size, int radius)
{
	FOR_XY_RADIUS(
		int buildingId = Data_Grid_buildingIds[gridOffset];
		if (buildingId > 0) {
			int type = Data_Buildings[buildingId].type;
			if (type != Building_MissionPost &&
				type != Building_NativeHut &&
				type != Building_NativeMeeting &&
				type != Building_NativeCrops) {
				return 1;
			}
		}
	);
	return 0;
}

void Terrain_initDistanceRing()
{
	int index = 0;
	int x, y;
	for (int s = 1; s <= 5; s++) {
		for (int d = 1; d <= 6; d++) {
			ringIndex[s][d] = index;
			// top row, from x=0
			for (y = -d, x = 0; x < s + d; x++, index++) {
				ringTiles[index].x = x;
				ringTiles[index].y = y;
			}
			// right row down
			for (x = s + d - 1, y = -d + 1; y < s + d; y++, index++) {
				ringTiles[index].x = x;
				ringTiles[index].y = y;
			}
			// bottom row to the left
			for (y = s + d - 1, x = s + d - 2; x >= -d; x--, index++) {
				ringTiles[index].x = x;
				ringTiles[index].y = y;
			}
			// left row up
			for (x = -d, y = s + d - 2; y >= -d; y--, index++) {
				ringTiles[index].x = x;
				ringTiles[index].y = y;
			}
			// top row up to x=0
			for (y = -d, x = -d + 1; x < 0; x++, index++) {
				ringTiles[index].x = x;
				ringTiles[index].y = y;
			}
		}
	}
	for (int i = 0; i < index; i++) {
		ringTiles[i].gridOffset = ringTiles[i].x + GRID_SIZE * ringTiles[i].y;
	}
}

static int isInsideMap(int x, int y)
{
	return x >= 0 && x < Data_Settings_Map.width &&
		y >= 0 && y < Data_Settings_Map.height;
}

int Terrain_isAllRockAndTreesAtDistanceRing(int x, int y, int distance)
{
	int start = ringIndex[1][distance];
	int end = start + RING_SIZE(1,distance);
	int baseOffset = GridOffset(x, y);
	for (int i = start; i < end; i++) {
		if (isInsideMap(x + ringTiles[i].x, y + ringTiles[i].y)) {
			int terrain = Data_Grid_terrain[baseOffset + ringTiles[i].gridOffset];
			if (!(terrain & Terrain_Rock) || !(terrain & Terrain_Tree)) {
				return 0;
			}
		}
	}
	return 1;
}

int Terrain_isAllMeadowAtDistanceRing(int x, int y, int distance)
{
	int start = ringIndex[1][distance];
	int end = start + RING_SIZE(1,distance);
	int baseOffset = GridOffset(x, y);
	for (int i = start; i < end; i++) {
		if (isInsideMap(x + ringTiles[i].x, y + ringTiles[i].y)) {
			int terrain = Data_Grid_terrain[baseOffset + ringTiles[i].gridOffset];
			if (!(terrain & Terrain_Meadow)) {
				return 0;
			}
		}
	}
	return 1;
}

static void Terrain_addDesirabilityDistanceRing(int x, int y, int size, int distance, int desirability)
{
	int isPartiallyOutsideMap = 0;
	if (x - distance < -1 || x + distance + size - 1 > Data_Settings_Map.width) {
		isPartiallyOutsideMap = 1;
	}
	if (y - distance < -1 || y + distance + size - 1 > Data_Settings_Map.height) {
		isPartiallyOutsideMap = 1;
	}
	int start = ringIndex[size][distance];
	int end = start + RING_SIZE(size,distance);
	int baseOffset = GridOffset(x, y);

	if (isPartiallyOutsideMap) {
		for (int i = start; i < end; i++) {
			if (isInsideMap(x + ringTiles[i].x, y + ringTiles[i].y)) {
				Data_Grid_desirability[baseOffset + ringTiles[i].gridOffset] += desirability;
				BOUND(Data_Grid_desirability[baseOffset], -100, 100); // BUGFIX: bounding on wrong tile
			}
		}
	} else {
		for (int i = start; i < end; i++) {
			Data_Grid_desirability[baseOffset + ringTiles[i].gridOffset] += desirability;
			BOUND(Data_Grid_desirability[baseOffset + ringTiles[i].gridOffset], -100, 100);
		}
	}
}

void Terrain_addDesirability(int x, int y, int size, int desBase, int desStep, int desStepSize, int desRange)
{
	if (size > 0) {
		if (desRange > 6) desRange = 6;
		int tilesWithinStep = 0;
		int distance = 1;
		while (desRange > 0) {
			Terrain_addDesirabilityDistanceRing(x, y, size, distance, desBase);
			distance++;
			desRange--;
			tilesWithinStep++;
			if (tilesWithinStep >= desStep) {
				desBase += desStepSize;
				tilesWithinStep = 0;
			}
		}
	}
}
