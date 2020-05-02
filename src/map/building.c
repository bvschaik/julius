#include "building.h"

#include "building/building.h"
#include "core/config.h"
#include "map/grid.h"

static grid_u16 buildings_grid;
static grid_u8 damage_grid;
static grid_u8 rubble_type_grid;
static grid_u8 highlight_grid;

int map_building_at(int grid_offset)
{
    return map_grid_is_valid_offset(grid_offset) ? buildings_grid.items[grid_offset] : 0;
}

void map_building_set(int grid_offset, int building_id)
{
    buildings_grid.items[grid_offset] = building_id;
}

void map_building_damage_clear(int grid_offset)
{
    damage_grid.items[grid_offset] = 0;
}

void map_highlight_set(int grid_offset)
{
    highlight_grid.items[grid_offset] = 1;
}

void map_highlight_clear(int grid_offset)
{
    highlight_grid.items[grid_offset] = 0;
}

int map_is_highlighted(int grid_offset)
{
    if (config_get(CONFIG_UI_WALKER_WAYPOINTS)) {
        return highlight_grid.items[grid_offset];
    }
    else {    
        return 0;
    }
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

void map_building_clear(void)
{
    map_grid_clear_u16(buildings_grid.items);
    map_grid_clear_u8(damage_grid.items);
    map_grid_clear_u8(rubble_type_grid.items);
}

void map_clear_highlights(void)
{
    map_grid_clear_u8(highlight_grid.items);
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

int map_building_is_reservoir(int x, int y)
{
    if (!map_grid_is_inside(x, y, 3)) {
        return 0;
    }
    int grid_offset = map_grid_offset(x, y);
    int building_id = map_building_at(grid_offset);
    if (!building_id || building_get(building_id)->type != BUILDING_RESERVOIR) {
        return 0;
    }
    for (int dy = 0; dy < 3; dy++) {
        for (int dx = 0; dx < 3; dx++) {
            if (building_id != map_building_at(grid_offset + map_grid_delta(dx, dy))) {
                return 0;
            }
        }
    }
    return 1;
}
