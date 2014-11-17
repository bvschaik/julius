#include "Desirability.h"

#include "Grid.h"
#include "Terrain.h"

#include "Data/Building.h"
#include "Data/Grid.h"
#include "Data/Model.h"
#include "Data/Settings.h"

static void updateBuildings();
static void updateTerrain();

void Desirability_update()
{
	Grid_clearByteGrid(Data_Grid_desirability);
	updateBuildings();
	updateTerrain();
}

static void updateBuildings()
{
	for (int i = 1; i <= Data_Buildings_Extra.highestBuildingIdInUse; i++) {
		if (Data_Buildings[i].inUse == 1) {
			int type = Data_Buildings[i].type;
			Terrain_addDesirability(
				Data_Buildings[i].x, Data_Buildings[i].y,
				Data_Buildings[i].size,
				Data_Model_Buildings[type].desirabilityValue,
				Data_Model_Buildings[type].desirabilityStep,
				Data_Model_Buildings[type].desirabilityStepSize,
				Data_Model_Buildings[type].desirabilityRange);
		}
	}
}

static void updateTerrain()
{
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			int terrain = Data_Grid_terrain[gridOffset];
			if (Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake) {
				int type;
				if (terrain & Terrain_Road) {
					type = Building_Plaza;
				} else if (terrain & Terrain_Rock) {
					// earthquake fault line: slight negative
					type = Building_HouseVacantLot;
				} else {
					// invalid plaza/earthquake flag
					Data_Grid_bitfields[gridOffset] &= ~Bitfield_PlazaOrEarthquake;
					continue;
				}
				Terrain_addDesirability(x, y, 1,
					Data_Model_Buildings[type].desirabilityValue,
					Data_Model_Buildings[type].desirabilityStep,
					Data_Model_Buildings[type].desirabilityStepSize,
					Data_Model_Buildings[type].desirabilityRange);
			} else if (terrain & Terrain_Garden) {
				Terrain_addDesirability(x, y, 1,
					Data_Model_Buildings[Building_Gardens].desirabilityValue,
					Data_Model_Buildings[Building_Gardens].desirabilityStep,
					Data_Model_Buildings[Building_Gardens].desirabilityStepSize,
					Data_Model_Buildings[Building_Gardens].desirabilityRange);
			} else if (terrain & Terrain_Rubble) {
				Terrain_addDesirability(x, y, 1, -2, 1, 1, 2);
			}
		}
	}
}
