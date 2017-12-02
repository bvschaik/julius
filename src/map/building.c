#include "building.h"

#include "map/grid.h"

#include "Data/Grid.h"

int map_building_at(int grid_offset)
{
    return Data_Grid_buildingIds[grid_offset];
}

void map_building_set(int grid_offset, int building_id)
{
    Data_Grid_buildingIds[grid_offset] = building_id;
}

void map_building_clear()
{
    map_grid_clear_u16(Data_Grid_buildingIds);
}

