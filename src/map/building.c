#include "building.h"

#include "map/grid.h"

static grid_u16 buildings_grid;
static grid_u8 damage_grid;
static grid_u8 rubble_type_grid;

int map_building_at(int grid_offset)
{
    return buildings_grid.items[grid_offset];
}

void map_building_set(int grid_offset, int building_id)
{
    buildings_grid.items[grid_offset] = building_id;
}

void map_building_damage_clear(int grid_offset)
{
    damage_grid.items[grid_offset] = 0;
}

int map_building_damage_increase(int grid_offset)
{
    return ++damage_grid.items[grid_offset];
}

int map_rubble_building_type(int grid_offset)
{
    return rubble_type_grid.items[grid_offset];
}

void map_set_rubble_building_type(int grid_offset, building_type type)
{
    rubble_type_grid.items[grid_offset] = type;
}

void map_building_clear()
{
    map_grid_clear_u16(buildings_grid.items);
    map_grid_clear_u8(damage_grid.items);
    map_grid_clear_u8(rubble_type_grid.items);
}

void map_building_save_state(buffer *buildings, buffer *damage)
{
    map_grid_save_state_u16(buildings_grid.items, buildings);
    map_grid_save_state_u8(damage_grid.items, damage);
}

void map_building_load_state(buffer *buildings, buffer *damage)
{
    map_grid_load_state_u16(buildings_grid.items, buildings);
    map_grid_load_state_u8(damage_grid.items, damage);
}
