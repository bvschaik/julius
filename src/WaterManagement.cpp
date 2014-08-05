#include "Grid.h"
#include "Terrain.h"
#include "TerrainGraphics.h"

#include "Data/Building.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Random.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"

#include <string.h>

static int spreadDirection;

void WaterManagement_updateHouseWaterAccess()
{
	Data_BuildingList.small.size = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse != 1) {
			continue;
		}
		if (Data_Buildings[i].type == Building_Well) {
			DATA_BUILDINGLIST_SMALL_ENQUEUE(i);
		} else if (Data_Buildings[i].houseSize) {
			Data_Buildings[i].hasWaterAccess = 0;
			Data_Buildings[i].hasWellAccess = 0;
			if (Terrain_existsTileWithinAreaWithType(
				Data_Buildings[i].x, Data_Buildings[i].y,
				Data_Buildings[i].size, Terrain_FountainRange)) {
				Data_Buildings[i].hasWaterAccess = 1;
			}
		}
	}
	DATA_BUILDINGLIST_SMALL_FOREACH(
		Terrain_markBuildingsWithinWellRadius(item, 2);
	);
}


static void setAllAqueductsToNoWater()
{
	int graphicId = GraphicId(ID_Graphic_Aqueduct) + 15;
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
				Data_Grid_aqueducts[gridOffset] = 0;
				if (Data_Grid_graphicIds[gridOffset] < graphicId) {
					Data_Grid_graphicIds[gridOffset] += 15;
				}
			}
		}
	}
}

static void fillAqueductsFromOffset(int gridOffset)
{
	// TODO
}

void WaterManagement_updateReservoirFountain()
{
	Grid_andShortGrid(Data_Grid_terrain, ~(Terrain_FountainRange | Terrain_ReservoirRange));
	// reservoirs
	setAllAqueductsToNoWater();
	memset(Data_BuildingList.large.items, 0, MAX_BUILDINGS * sizeof(short));
	Data_BuildingList.large.size = 0;
	for (int i = 0; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse == 1 && Data_Buildings[i].type == Building_Reservoir) {
			Data_BuildingList.large.items[Data_BuildingList.large.size++] = i;
			if (Terrain_existsTileWithinAreaWithType(
				Data_Buildings[i].x - 1, Data_Buildings[i].y - 1, 5, Terrain_Water)) {
				Data_Buildings[i].hasWaterAccess = 2;
			} else {
				Data_Buildings[i].hasWaterAccess = 0;
			}
		}
	}
	int changed = 1;
	static const int connectorOffsets[] = {-161, 165, 487, 161};
	while (changed == 1) {
		for (int i = 0; i < Data_BuildingList.large.size; i++) {
			int buildingId = Data_BuildingList.large.items[i];
			if (Data_Buildings[buildingId].hasWaterAccess == 2) {
				Data_Buildings[buildingId].hasWaterAccess = 1;
				changed = 1;
				for (int d = 0; d < 4; d++) {
					fillAqueductsFromOffset(Data_Buildings[buildingId].gridOffset + connectorOffsets[d]);
				}
			}
		}
	}
	for (int i = 0; i < Data_BuildingList.large.size; i++) {
		int buildingId = Data_BuildingList.large.items[i];
		if (Data_Buildings[buildingId].hasWaterAccess) {
			Terrain_setWithRadius(
				Data_Buildings[buildingId].x, Data_Buildings[buildingId].y,
				3, 10, Terrain_ReservoirRange);
		}
	}
	// fountains
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (b->inUse != 1 || b->type != Building_Fountain) {
			continue;
		}
		int des = Data_Grid_desirability[b->gridOffset];
		int graphicId;
		if (des > 60) {
			graphicId = GraphicId(ID_Graphic_Fountain4);
		} else if (des > 40) {
			graphicId = GraphicId(ID_Graphic_Fountain3);
		} else if (des > 20) {
			graphicId = GraphicId(ID_Graphic_Fountain2);
		} else {
			graphicId = GraphicId(ID_Graphic_Fountain1);
		}
		Terrain_addBuildingToGrids(i, b->x, b->y, 1, graphicId, Terrain_Building);
		if ((Data_Grid_terrain[b->gridOffset] & Terrain_ReservoirRange) && b->numWorkers) {
			b->hasWaterAccess = 1;
			Terrain_setWithRadius(b->x, b->y, 1,
				Data_Scenario.climate == Climate_Desert ? 3 : 4,
				Terrain_FountainRange);
		} else {
			b->hasWaterAccess = 0;
		}
	}
}

void WaterManagement_updateFireSpreadDirection()
{
	spreadDirection = Data_Random.random1_7bit & 7;
}
