#include "city_water_ghost.h"

#include "building/building.h"
#include "building/construction.h"
#include "city/view.h"
#include "map/grid.h"
#include "map/water_supply.h"
#include "widget/city_building_ghost.h"

#include <string.h>

enum {
    WATER_ACCESS_NONE = 0b00,
    WATER_ACCESS_WELL = 0b01,
    WATER_ACCESS_FOUNTAIN = 0b10
};

static uint8_t has_water_access[GRID_SIZE * GRID_SIZE];
static building_type last_building_type = BUILDING_NONE;
static int last_well_count = 0;
static int last_fountain_count = 0;

static void set_well_access(int x, int y, int grid_offset)
{
    has_water_access[grid_offset] |= WATER_ACCESS_WELL;
}

static void set_fountain_access(int x, int y, int grid_offset)
{
    has_water_access[grid_offset] |= WATER_ACCESS_FOUNTAIN;
}

static void update_water_access(void)
{
    memset(has_water_access, 0, sizeof(uint8_t) * GRID_SIZE * GRID_SIZE);
    for (building *b = building_first_of_type(BUILDING_WELL); b; b = b->next_of_type) {
        city_view_foreach_tile_in_range(b->grid_offset, 1, map_water_supply_well_radius(), set_well_access);
    }
    for (building *b = building_first_of_type(BUILDING_FOUNTAIN); b; b = b->next_of_type) {
        city_view_foreach_tile_in_range(b->grid_offset, 1, map_water_supply_fountain_radius(), set_fountain_access);
    }
}

static void draw_water_access(int x, int y, int grid_offset)
{
    uint8_t water_access = has_water_access[grid_offset];
    if (water_access & WATER_ACCESS_FOUNTAIN) {
        city_building_ghost_draw_fountain_range(x, y, grid_offset);
    } else if (water_access & WATER_ACCESS_WELL) {
        city_building_ghost_draw_well_range(x, y, grid_offset);
    }
}

void city_water_ghost_draw_water_structure_ranges(void)
{
    building_type type = building_construction_type();
    // we're counting the number of buildings using the building linked list rather than the counts in building/counts.c
    // because the linked list counts update immediately so the outlines still update even when the game is paused
    int num_wells = 0;
    for (building *b = building_first_of_type(BUILDING_WELL); b; b = b->next_of_type) {
        num_wells++;
    }
    int num_fountains = 0;
    for (building *b = building_first_of_type(BUILDING_FOUNTAIN); b; b = b->next_of_type) {
        num_fountains++;
    }
    if (type != last_building_type || num_wells != last_well_count || num_fountains != last_fountain_count) {
        update_water_access();
    }
    last_building_type = type;
    last_well_count = num_wells;
    last_fountain_count = num_fountains;
    city_view_foreach_valid_map_tile(draw_water_access);
}