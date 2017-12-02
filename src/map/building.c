#include "building.h"

#include "map/grid.h"

#include "Data/Grid.h"

static grid_u16 buildings;

int map_building_at(int grid_offset)
{
    return buildings.items[grid_offset];
}

void map_building_set(int grid_offset, int building_id)
{
    buildings.items[grid_offset] = building_id;
}

void map_building_clear()
{
    map_grid_clear_u16(buildings.items);
}

void map_building_save_state(buffer *buf)
{
    map_grid_save_state_u16(buildings.items, buf);
}

void map_building_load_state(buffer *buf)
{
    map_grid_load_state_u16(buildings.items, buf);
}
