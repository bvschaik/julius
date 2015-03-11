#include "BuildingHouse.h"

#include "Building.h"
#include "Terrain.h"

#include "Data/Building.h"
#include "Data/Constants.h"
#include "Data/Debug.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Settings.h"

#define CREATE_HOUSE_TILE(tt, xx,yy)\
	{\
		int newBuildingId = Building_create((tt), (xx), (yy));\
		struct Data_Building *nb = &Data_Buildings[newBuildingId];\
		nb->housePopulation = populationPerTile;\
		for (int i = 0; i < 8; i++) {\
			nb->data.house.inventory.all[i] = inventoryPerTile[i];\
		}\
		nb->distanceFromEntry = 0;\
		Terrain_addBuildingToGrids(newBuildingId, nb->x, nb->y, 1,\
				graphicId + (Data_Grid_random[nb->gridOffset] & 1), Terrain_Building);\
	}


static void merge(int buildingId);
static void prepareForMerge(int buildingId, int numTiles);
static void splitMerged(int buildingId);
static void splitSize2(int buildingId);
static void splitSize3(int buildingId);

static const int directionGridOffsets[] = {0, -163, -1, -162};
static const int directionOffsetX[] = { 0, -1, -1, 0 };
static const int directionOffsetY[] = { 0, -1, 0, -1 };
static const int tileGridOffsets[] = {
	0, 1, 162, 163, // 2x2
	2, 164, 326, 325, 324, // 3x3
	3, 165, 327, 489, 488, 487, 486 // 4x4
};

static int houseGraphicGroup[20] = {
	26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 35
};

static int houseGraphicOffset[20] = {
	0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 1, 0, 1, 0, 1
};

static int houseGraphicNumTypes[20] = {
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1
};

static struct {
	int x;
	int y;
	int inventory[8];
	int population;
} mergeData;

int BuildingHouse_canExpand(int buildingId, int numTiles)
{
	// merge with other houses
	for (int dir = 0; dir < 4; dir++) {
		int baseOffset = directionGridOffsets[dir] + Data_Buildings[buildingId].gridOffset;
		int okTiles = 0;
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = baseOffset + tileGridOffsets[i];
			if (Data_Grid_terrain[tileOffset] & Terrain_Building) {
				int b = Data_Grid_buildingIds[tileOffset];
				if (b == buildingId) {
					okTiles++;
				} else if (BuildingIsInUse(b) && Data_Buildings[b].houseSize) {
					if (Data_Buildings[b].subtype.houseLevel <= Data_Buildings[buildingId].subtype.houseLevel) {
						okTiles++;
					}
				}
			}
		}
		if (okTiles == numTiles) {
			mergeData.x = Data_Buildings[buildingId].x + directionOffsetX[dir];
			mergeData.y = Data_Buildings[buildingId].y + directionOffsetY[dir];
			return 1;
		}
	}
	// merge with houses and empty terrain
	for (int dir = 0; dir < 4; dir++) {
		int baseOffset = directionGridOffsets[dir] + Data_Buildings[buildingId].gridOffset;
		int okTiles = 0;
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = baseOffset + tileGridOffsets[i];
			if ((Data_Grid_terrain[tileOffset] & Terrain_NotClear) == 0) {
				okTiles++;
			} else if (Data_Grid_terrain[tileOffset] & Terrain_Building) {
				int b = Data_Grid_buildingIds[tileOffset];
				if (b == buildingId) {
					okTiles++;
				} else if (BuildingIsInUse(b) && Data_Buildings[b].houseSize) {
					if (Data_Buildings[b].subtype.houseLevel <= Data_Buildings[buildingId].subtype.houseLevel) {
						okTiles++;
					}
				}
			}
		}
		if (okTiles == numTiles) {
			mergeData.x = Data_Buildings[buildingId].x + directionOffsetX[dir];
			mergeData.x = Data_Buildings[buildingId].y + directionOffsetY[dir];
			return 1;
		}
	}
	// merge with houses, empty terrain and gardens
	for (int dir = 0; dir < 4; dir++) {
		int baseOffset = directionGridOffsets[dir] + Data_Buildings[buildingId].gridOffset;
		int okTiles = 0;
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = baseOffset + tileGridOffsets[i];
			if ((Data_Grid_terrain[tileOffset] & Terrain_NotClear) == 0) {
				okTiles++;
			} else if (Data_Grid_terrain[tileOffset] & Terrain_Building) {
				int b = Data_Grid_buildingIds[tileOffset];
				if (b == buildingId) {
					okTiles++;
				} else if (BuildingIsInUse(b) && Data_Buildings[b].houseSize) {
					if (Data_Buildings[b].subtype.houseLevel <= Data_Buildings[buildingId].subtype.houseLevel) {
						okTiles++;
					}
				}
			} else if (Data_Grid_terrain[tileOffset] & Terrain_Garden) {
				okTiles++;
			}
		}
		if (okTiles == numTiles) {
			mergeData.x = Data_Buildings[buildingId].x + directionOffsetX[dir];
			mergeData.y = Data_Buildings[buildingId].y + directionOffsetY[dir];
			return 1;
		}
	}
	Data_Buildings[buildingId].data.house.noSpaceToExpand = 1;
	return 0;
}

void BuildingHouse_checkForCorruption(int buildingId)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	int houseGridOffset = b->gridOffset;
	int calcGridOffset = GridOffset(b->x, b->y);
	b->data.house.noSpaceToExpand = 0;
	if (houseGridOffset != calcGridOffset || Data_Grid_buildingIds[houseGridOffset] != buildingId) {
		++Data_Debug.incorrectHousePositions;
		int gridOffset = Data_Settings_Map.gridStartOffset;
		for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
			for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
				if (Data_Grid_buildingIds[gridOffset] == buildingId) {
					b->gridOffset = gridOffset;
					b->x = GridOffsetToX(gridOffset);
					b->y = GridOffsetToY(gridOffset);
					return;
				}
			}
		}
		++Data_Debug.unfixableHousePositions;
		b->state = BuildingState_Rubble;
	}
}

void BuildingHouse_checkMerge(int buildingId)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->houseIsMerged) {
		return;
	}
	if ((Data_Grid_random[b->gridOffset] & 7) >= 5) {
		return;
	}
	int numTiles = 0;
	for (int i = 0; i < 4; i++) {
		int otherGridOffset = b->gridOffset + tileGridOffsets[i];
		if (Data_Grid_terrain[otherGridOffset] & Terrain_Building) {
			int otherBuildingId = Data_Grid_buildingIds[otherGridOffset];
			if (otherBuildingId == buildingId) {
				numTiles++;
			} else if (BuildingIsInUse(otherBuildingId) &&
					Data_Buildings[otherBuildingId].houseSize &&
					Data_Buildings[otherBuildingId].subtype.houseLevel == b->subtype.houseLevel &&
					!Data_Buildings[otherBuildingId].houseIsMerged) {
				numTiles++;
			}
		}
	}
	if (numTiles == 4) {
		mergeData.x = b->x + directionOffsetX[0];
		mergeData.y = b->y + directionOffsetY[0];
		merge(buildingId);
	}
}

static void split(int buildingId, int numTiles)
{
	int gridOffset = GridOffset(mergeData.x, mergeData.y);
	for (int i = 0; i < numTiles; i++) {
		int tileOffset = gridOffset + tileGridOffsets[i];
		if (Data_Grid_terrain[tileOffset] & Terrain_Building) {
			int otherBuildingId = Data_Grid_buildingIds[tileOffset];
			if (otherBuildingId != buildingId && Data_Buildings[otherBuildingId].houseSize) {
				if (Data_Buildings[otherBuildingId].houseIsMerged == 1) {
					splitMerged(otherBuildingId);
				} else if (Data_Buildings[otherBuildingId].houseSize == 2) {
					splitSize2(buildingId);
				} else if (Data_Buildings[otherBuildingId].houseSize == 3) {
					splitSize3(buildingId);
				}
			}
		}
	}
}

static void prepareForMerge(int buildingId, int numTiles)
{
	for (int i = 0; i < 8; i++) {
		mergeData.inventory[i] = 0;
	}
	mergeData.population = 0;
	int gridOffset = GridOffset(mergeData.x, mergeData.y);
	for (int i = 0; i < numTiles; i++) {
		int tileOffset = gridOffset + tileGridOffsets[i];
		if (Data_Grid_terrain[tileOffset] & Terrain_Building) {
			int otherBuildingId = Data_Grid_buildingIds[tileOffset];
			if (otherBuildingId != buildingId && Data_Buildings[otherBuildingId].houseSize) {
				mergeData.population += Data_Buildings[otherBuildingId].housePopulation;
				for (int i = 0; i < 8; i++) {
					mergeData.inventory[i] += Data_Buildings[otherBuildingId].data.house.inventory.all[i];
					Data_Buildings[otherBuildingId].housePopulation = 0;
					Data_Buildings[otherBuildingId].state = BuildingState_DeletedByGame;
				}
			}
		}
	}
}

void BuildingHouse_expandToLargeInsula(int buildingId)
{
	split(buildingId, 4);
	prepareForMerge(buildingId, 4);

	struct Data_Building *b = &Data_Buildings[buildingId];
	b->type = Building_HouseLargeInsula;
	b->subtype.houseLevel = HouseLevel_LargeInsula;
	b->size = b->houseSize = 2;
	b->housePopulation += mergeData.population;
	for (int i = 0; i < 8; i++) {
		b->data.house.inventory.all[i] += mergeData.inventory[i];
	}
	int graphicId =
		GraphicId(houseGraphicGroup[b->subtype.houseLevel]) +
		houseGraphicOffset[b->subtype.houseLevel] +
		(Data_Grid_random[b->gridOffset] & 1);
	Terrain_removeBuildingFromGrids(buildingId, b->x, b->y);
	b->x = mergeData.x;
	b->y = mergeData.y;
	b->gridOffset = GridOffset(b->x, b->y);
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size, graphicId, Terrain_Building);
}

void BuildingHouse_expandToLargeVilla(int buildingId)
{
	split(buildingId, 9);
	prepareForMerge(buildingId, 9);

	struct Data_Building *b = &Data_Buildings[buildingId];
	b->type = Building_HouseLargeVilla;
	b->subtype.houseLevel = HouseLevel_LargeVilla;
	b->size = b->houseSize = 3;
	b->housePopulation += mergeData.population;
	for (int i = 0; i < 8; i++) {
		b->data.house.inventory.all[i] += mergeData.inventory[i];
	}
	int graphicId =
		GraphicId(houseGraphicGroup[b->subtype.houseLevel]) +
		houseGraphicOffset[b->subtype.houseLevel];
	Terrain_removeBuildingFromGrids(buildingId, b->x, b->y);
	b->x = mergeData.x;
	b->y = mergeData.y;
	b->gridOffset = GridOffset(b->x, b->y);
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size, graphicId, Terrain_Building);
}

void BuildingHouse_expandToLargePalace(int buildingId)
{
	split(buildingId, 16);
	prepareForMerge(buildingId, 16);

	struct Data_Building *b = &Data_Buildings[buildingId];
	b->type = Building_HouseLargePalace;
	b->subtype.houseLevel = HouseLevel_LargePalace;
	b->size = b->houseSize = 4;
	b->housePopulation += mergeData.population;
	for (int i = 0; i < 8; i++) {
		b->data.house.inventory.all[i] += mergeData.inventory[i];
	}
	int graphicId =
		GraphicId(houseGraphicGroup[b->subtype.houseLevel]) +
		houseGraphicOffset[b->subtype.houseLevel];
	Terrain_removeBuildingFromGrids(buildingId, b->x, b->y);
	b->x = mergeData.x;
	b->y = mergeData.y;
	b->gridOffset = GridOffset(b->x, b->y);
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size, graphicId, Terrain_Building);
}

static void merge(int buildingId)
{
	prepareForMerge(buildingId, 4);

	struct Data_Building *b = &Data_Buildings[buildingId];
	b->size = b->houseSize = 2;
	b->housePopulation += mergeData.population;
	for (int i = 0; i < 8; i++) {
		b->data.house.inventory.all[i] += mergeData.inventory[i];
	}
	int graphicId = GraphicId(houseGraphicGroup[b->subtype.houseLevel]) + 4;
	if (houseGraphicOffset[b->subtype.houseLevel]) {
		graphicId += 1;
	}
	
	Terrain_removeBuildingFromGrids(buildingId, b->x, b->y);
	b->x = mergeData.x;
	b->y = mergeData.y;
	b->gridOffset = GridOffset(b->x, b->y);
	b->houseIsMerged = 1;
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, 2, graphicId, Terrain_Building);
}

static void splitMerged(int buildingId)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	int inventoryPerTile[8];
	int inventoryRest[8];
	for (int i = 0; i < 8; i++) {
		inventoryPerTile[i] = b->data.house.inventory.all[i] / 4;
		inventoryRest[i] = b->data.house.inventory.all[i] % 4;
	}
	int populationPerTile = b->housePopulation / 4;
	int populationRest = b->housePopulation % 4;

	Terrain_removeBuildingFromGrids(buildingId, b->x, b->y);

	// main tile
	b->size = b->houseSize = 1;
	b->houseIsMerged = 0;
	b->housePopulation = populationPerTile + populationRest;
	for (int i = 0; i < 8; i++) {
		b->data.house.inventory.all[i] = inventoryPerTile[i] + inventoryRest[i];
	}
	b->distanceFromEntry = 0;

	int graphicId = GraphicId(houseGraphicGroup[b->subtype.houseLevel]) + houseGraphicOffset[b->subtype.houseLevel];
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
		graphicId + (Data_Grid_random[b->gridOffset] & 1), Terrain_Building);
	
	// the other tiles (new buildings, medium insula)
	CREATE_HOUSE_TILE(b->type, b->x + 1, b->y);
	CREATE_HOUSE_TILE(b->type, b->x, b->y + 1);
	CREATE_HOUSE_TILE(b->type, b->x + 1, b->y + 1);
}

static void splitSize2(int buildingId)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	int inventoryPerTile[8];
	int inventoryRest[8];
	for (int i = 0; i < 8; i++) {
		inventoryPerTile[i] = b->data.house.inventory.all[i] / 4;
		inventoryRest[i] = b->data.house.inventory.all[i] % 4;
	}
	int populationPerTile = b->housePopulation / 4;
	int populationRest = b->housePopulation % 4;

	Terrain_removeBuildingFromGrids(buildingId, b->x, b->y);

	// main tile
	b->type = Building_HouseMediumInsula;
	b->subtype.houseLevel = b->type - 10;
	b->size = b->houseSize = 1;
	b->houseIsMerged = 0;
	b->housePopulation = populationPerTile + populationRest;
	for (int i = 0; i < 8; i++) {
		b->data.house.inventory.all[i] = inventoryPerTile[i] + inventoryRest[i];
	}
	b->distanceFromEntry = 0;

	int graphicId = GraphicId(houseGraphicGroup[b->subtype.houseLevel]) + houseGraphicOffset[b->subtype.houseLevel];
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
		graphicId + (Data_Grid_random[b->gridOffset] & 1), Terrain_Building);

	// the other tiles (new buildings)
	CREATE_HOUSE_TILE(b->type, b->x + 1, b->y);
	CREATE_HOUSE_TILE(b->type, b->x, b->y + 1);
	CREATE_HOUSE_TILE(b->type, b->x + 1, b->y + 1);
}

static void splitSize3(int buildingId)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	int inventoryPerTile[8];
	int inventoryRest[8];
	for (int i = 0; i < 8; i++) {
		inventoryPerTile[i] = b->data.house.inventory.all[i] / 9;
		inventoryRest[i] = b->data.house.inventory.all[i] % 9;
	}
	int populationPerTile = b->housePopulation / 9;
	int populationRest = b->housePopulation % 9;

	Terrain_removeBuildingFromGrids(buildingId, b->x, b->y);

	// main tile
	b->type = Building_HouseMediumInsula;
	b->subtype.houseLevel = b->type - 10;
	b->size = b->houseSize = 1;
	b->houseIsMerged = 0;
	b->housePopulation = populationPerTile + populationRest;
	for (int i = 0; i < 8; i++) {
		b->data.house.inventory.all[i] = inventoryPerTile[i] + inventoryRest[i];
	}
	b->distanceFromEntry = 0;

	int graphicId = GraphicId(houseGraphicGroup[b->subtype.houseLevel]) + houseGraphicOffset[b->subtype.houseLevel];
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
		graphicId + (Data_Grid_random[b->gridOffset] & 1), Terrain_Building);

	// the other tiles (new buildings)
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 1, b->y);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 2, b->y);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x, b->y + 1);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 1, b->y + 1);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 2, b->y + 1);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x, b->y + 2);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 1, b->y + 2);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 2, b->y + 2);
}

void BuildingHouse_devolveFromLargeInsula(int buildingId)
{
	splitSize2(buildingId);
}

void BuildingHouse_devolveFromLargeVilla(int buildingId)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	int inventoryPerTile[8];
	int inventoryRest[8];
	for (int i = 0; i < 8; i++) {
		inventoryPerTile[i] = b->data.house.inventory.all[i] / 6;
		inventoryRest[i] = b->data.house.inventory.all[i] % 6;
	}
	int populationPerTile = b->housePopulation / 6;
	int populationRest = b->housePopulation % 6;

	Terrain_removeBuildingFromGrids(buildingId, b->x, b->y);

	// main tile
	b->type = Building_HouseMediumVilla;
	b->subtype.houseLevel = b->type - 10;
	b->size = b->houseSize = 2;
	b->houseIsMerged = 0;
	b->housePopulation = populationPerTile + populationRest;
	for (int i = 0; i < 8; i++) {
		b->data.house.inventory.all[i] = inventoryPerTile[i] + inventoryRest[i];
	}
	b->distanceFromEntry = 0;

	int graphicId = GraphicId(houseGraphicGroup[b->subtype.houseLevel]) + houseGraphicOffset[b->subtype.houseLevel];
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
		graphicId + (Data_Grid_random[b->gridOffset] & 1), Terrain_Building);

	// the other tiles (new buildings)
	graphicId = GraphicId(houseGraphicGroup[HouseLevel_MediumInsula]) + houseGraphicOffset[HouseLevel_MediumInsula];
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 2, b->y);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 2, b->y + 1);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x, b->y + 2);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 1, b->y + 2);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 2, b->y + 2);
}

void BuildingHouse_devolveFromLargePalace(int buildingId)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	int inventoryPerTile[8];
	int inventoryRest[8];
	for (int i = 0; i < 8; i++) {
		inventoryPerTile[i] = b->data.house.inventory.all[i] / 8;
		inventoryRest[i] = b->data.house.inventory.all[i] % 8;
	}
	int populationPerTile = b->housePopulation / 8;
	int populationRest = b->housePopulation % 8;

	Terrain_removeBuildingFromGrids(buildingId, b->x, b->y);

	// main tile
	b->type = Building_HouseMediumPalace;
	b->subtype.houseLevel = b->type - 10;
	b->size = b->houseSize = 3;
	b->houseIsMerged = 0;
	b->housePopulation = populationPerTile + populationRest;
	for (int i = 0; i < 8; i++) {
		b->data.house.inventory.all[i] = inventoryPerTile[i] + inventoryRest[i];
	}
	b->distanceFromEntry = 0;

	int graphicId = GraphicId(houseGraphicGroup[b->subtype.houseLevel]) + houseGraphicOffset[b->subtype.houseLevel];
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size, graphicId, Terrain_Building);

	// the other tiles (new buildings)
	graphicId = GraphicId(houseGraphicGroup[HouseLevel_MediumInsula]) + houseGraphicOffset[HouseLevel_MediumInsula];
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 3, b->y);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 3, b->y + 1);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 3, b->y + 2);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x, b->y + 3);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 1, b->y + 3);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 2, b->y + 3);
	CREATE_HOUSE_TILE(Building_HouseMediumInsula, b->x + 3, b->y + 3);
}

void BuildingHouse_changeTo(int buildingId, int buildingType)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	b->type = buildingType;
	b->subtype.houseLevel = b->type - 10;
	int graphicId = GraphicId(houseGraphicGroup[b->subtype.houseLevel]);
	if (b->houseIsMerged) {
		graphicId += 4;
		if (houseGraphicOffset[b->subtype.houseLevel]) {
			graphicId += 1;
		}
	} else {
		graphicId += houseGraphicOffset[b->subtype.houseLevel];
		graphicId += Data_Grid_random[b->gridOffset] & (houseGraphicNumTypes[b->subtype.houseLevel] - 1);
	}
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size, graphicId, Terrain_Building);
}

void BuildingHouse_changeToVacantLot(int buildingId)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	b->type = Building_HouseVacantLot;
	b->subtype.houseLevel = b->type - 10;
	int graphicId = GraphicId(ID_Graphic_HouseVacantLot);
	if (b->houseIsMerged) {
		Terrain_removeBuildingFromGrids(buildingId, b->x, b->y);
		b->houseIsMerged = 0;
		b->size = b->houseSize = 1;
		Terrain_addBuildingToGrids(buildingId, b->x, b->y, 1, graphicId, Terrain_Building);

		int b2 = Building_create(b->type, b->x + 1, b->y);
		Data_Buildings[b2].housePopulation = 0;
		Data_Buildings[b2].distanceFromEntry = 0;
		Terrain_addBuildingToGrids(b2, b->x + 1, b->y, 1, graphicId, Terrain_Building);

		int b3 = Building_create(b->type, b->x, b->y + 1);
		Data_Buildings[b3].housePopulation = 0;
		Data_Buildings[b3].distanceFromEntry = 0;
		Terrain_addBuildingToGrids(b3, b->x, b->y + 1, 1, graphicId, Terrain_Building);

		int b4 = Building_create(b->type, b->x + 1, b->y + 1);
		Data_Buildings[b4].housePopulation = 0;
		Data_Buildings[b4].distanceFromEntry = 0;
		Terrain_addBuildingToGrids(b4, b->x + 1, b->y + 1, 1, graphicId, Terrain_Building);
	} else {
		Data_Grid_graphicIds[b->gridOffset] = graphicId;
	}
}
