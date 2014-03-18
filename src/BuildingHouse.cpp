#include "BuildingHouse.h"

#include "Data/Building.h"
#include "Data/Grid.h"

static int directionGridOffsets[] = {0, -163, -1, -162};
static int directionOffsetX[] = { 0, -1, -1, 0 };
static int directionOffsetY[] = { 0, -1, 0, -1 };
static int tileGridOffsets[] = {
	0, 1, 162, 163, // 2x2
	2, 164, 326, 325, 324, // 3x3
	3, 165, 327, 489, 488, 487, 486 // 4x4
};

static int mergeX, mergeY;


void BuildingHouse_checkMerge(int buildingId)
{
	// TODO
}

int BuildingHouse_canExpand(int buildingId, int numTiles)
{
	// merge with other houses
	for (int dir = 0; dir < 4; dir++) {
		int baseOffset = directionGridOffsets[dir] = Data_Buildings[buildingId].gridOffset;
		int okTiles = 0;
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = baseOffset + tileGridOffsets[i];
			if (Data_Grid_terrain[tileOffset] & Terrain_Building) {
				int b = Data_Grid_buildingIds[tileOffset];
				if (b == buildingId) {
					okTiles++;
				} else if (Data_Buildings[b].inUse == 1 && Data_Buildings[i].houseSize) {
					if (Data_Buildings[b].subtype.houseLevel <= Data_Buildings[buildingId].subtype.houseLevel) {
						okTiles++;
					}
				}
			}
		}
		if (okTiles == numTiles) {
			mergeX = Data_Buildings[buildingId].x + directionOffsetX[dir];
			mergeY = Data_Buildings[buildingId].y + directionOffsetY[dir];
			return 1;
		}
	}
	// merge with houses and empty terrain
	for (int dir = 0; dir < 4; dir++) {
		int baseOffset = directionGridOffsets[dir] = Data_Buildings[buildingId].gridOffset;
		int okTiles = 0;
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = baseOffset + tileGridOffsets[i];
			if ((Data_Grid_terrain[tileOffset] & Terrain_NotClear) == 0) {
				okTiles++;
			} else if (Data_Grid_terrain[tileOffset] & Terrain_Building) {
				int b = Data_Grid_buildingIds[tileOffset];
				if (b == buildingId) {
					okTiles++;
				} else if (Data_Buildings[b].inUse == 1 && Data_Buildings[i].houseSize) {
					if (Data_Buildings[b].subtype.houseLevel <= Data_Buildings[buildingId].subtype.houseLevel) {
						okTiles++;
					}
				}
			}
		}
		if (okTiles == numTiles) {
			mergeX = Data_Buildings[buildingId].x + directionOffsetX[dir];
			mergeY = Data_Buildings[buildingId].y + directionOffsetY[dir];
			return 1;
		}
	}
	// merge with houses, empty terrain and gardens
	for (int dir = 0; dir < 4; dir++) {
		int baseOffset = directionGridOffsets[dir] = Data_Buildings[buildingId].gridOffset;
		int okTiles = 0;
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = baseOffset + tileGridOffsets[i];
			if ((Data_Grid_terrain[tileOffset] & Terrain_NotClear) == 0) {
				okTiles++;
			} else if (Data_Grid_terrain[tileOffset] & Terrain_Building) {
				int b = Data_Grid_buildingIds[tileOffset];
				if (b == buildingId) {
					okTiles++;
				} else if (Data_Buildings[b].inUse == 1 && Data_Buildings[i].houseSize) {
					if (Data_Buildings[b].subtype.houseLevel <= Data_Buildings[buildingId].subtype.houseLevel) {
						okTiles++;
					}
				}
			} else if (Data_Grid_terrain[tileOffset] & Terrain_Garden) {
				okTiles++;
			}
		}
		if (okTiles == numTiles) {
			mergeX = Data_Buildings[buildingId].x + directionOffsetX[dir];
			mergeY = Data_Buildings[buildingId].y + directionOffsetY[dir];
			return 1;
		}
	}
	Data_Buildings[buildingId].data.house.noSpaceToExpand = 1;
	return 0;
}

void BuildingHouse_split(int buildingId, int tiles)
{
	// TODO
}

// TODO split + storeInventory to BuildingHouse internal
void BuildingHouse_expandToLargeInsula(int buildingId)
{
	// TODO
}
void BuildingHouse_expandToLargeVilla(int buildingId)
{
	// TODO
}
void BuildingHouse_expandToLargePalace(int buildingId)
{
	// TODO
}

void BuildingHouse_splitFromLargeInsula(int buildingId)
{
	// TODO
}
void BuildingHouse_splitFromLargeVilla(int buildingId)
{
	// TODO
}
void BuildingHouse_splitFromLargePalace(int buildingId)
{
	// TODO
}

void BuildingHouse_changeTo(int buildingId, int buildingType)
{
	// TODO
}
void BuildingHouse_changeToVacantLot(int buildingId)
{
	// TODO
}
