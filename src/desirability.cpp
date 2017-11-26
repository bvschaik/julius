#include "desirability.h"

#include "grid.h"
#include "terrain.h"

#include "data/building.hpp"
#include "data/grid.hpp"
#include "data/settings.hpp"

#include "building/model.h"

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
    for (int i = 1; i <= Data_Buildings_Extra.highestBuildingIdInUse; i++)
    {
        if (BuildingIsInUse(i))
        {
            const model_building *model = model_get_building((building_type)Data_Buildings[i].type);
            Terrain_addDesirability(
                Data_Buildings[i].x, Data_Buildings[i].y,
                Data_Buildings[i].size,
                model->desirability_value,
                model->desirability_step,
                model->desirability_step_size,
                model->desirability_range);
        }
    }
}

static void updateTerrain()
{
    int gridOffset = Data_Settings_Map.gridStartOffset;
    for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize)
    {
        for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++)
        {
            int terrain = Data_Grid_terrain[gridOffset];
            if (Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake)
            {
                int type;
                if (terrain & Terrain_Road)
                {
                    type = BUILDING_PLAZA;
                }
                else if (terrain & Terrain_Rock)
                {
                    // earthquake fault line: slight negative
                    type = BUILDING_HOUSE_VACANT_LOT;
                }
                else
                {
                    // invalid plaza/earthquake flag
                    Data_Grid_bitfields[gridOffset] &= ~Bitfield_PlazaOrEarthquake;
                    continue;
                }
                const model_building *model = model_get_building((building_type)type);
                Terrain_addDesirability(x, y, 1,
                                        model->desirability_value,
                                        model->desirability_step,
                                        model->desirability_step_size,
                                        model->desirability_range);
            }
            else if (terrain & Terrain_Garden)
            {
                const model_building *model = model_get_building(BUILDING_GARDENS);
                Terrain_addDesirability(x, y, 1,
                                        model->desirability_value,
                                        model->desirability_step,
                                        model->desirability_step_size,
                                        model->desirability_range);
            }
            else if (terrain & Terrain_Rubble)
            {
                Terrain_addDesirability(x, y, 1, -2, 1, 1, 2);
            }
        }
    }
}
