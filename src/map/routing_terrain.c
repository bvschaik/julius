#include "routing_terrain.h"

#include "building/building.h"
#include "city/view.h"
#include "core/direction.h"
#include "core/image.h"
#include "map/building.h"
#include "map/data.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/routing_data.h"
#include "map/sprite.h"
#include "map/terrain.h"

static void map_routing_update_land_noncitizen(void);

void map_routing_update_all(void)
{
    map_routing_update_land();
    map_routing_update_water();
    map_routing_update_walls();
}

void map_routing_update_land(void)
{
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen();
}

static int get_land_type_citizen_building(int grid_offset)
{
    building *b = building_get(map_building_at(grid_offset));
    int terrain = map_terrain_get(grid_offset);
    int type = CITIZEN_N1_BLOCKED;
    switch (b->type) {
        default:
            return CITIZEN_N1_BLOCKED;
        case BUILDING_WAREHOUSE:
            type = CITIZEN_0_ROAD;
            break;
        case BUILDING_GATEHOUSE:
            if (terrain & TERRAIN_HIGHWAY) {
                type = CITIZEN_1_HIGHWAY;
            } else {
                type = CITIZEN_0_ROAD;
            }
            break;
        case BUILDING_ROADBLOCK:
            type = CITIZEN_0_ROAD;
            break;
        case BUILDING_FORT_GROUND:
            type = CITIZEN_2_PASSABLE_TERRAIN;
            break;
        case BUILDING_TRIUMPHAL_ARCH:
            if (b->subtype.orientation == 3) {
                switch (map_property_multi_tile_xy(grid_offset)) {
                    case EDGE_X0Y1:
                    case EDGE_X1Y1:
                    case EDGE_X2Y1:
                        type = CITIZEN_0_ROAD;
                        break;
                }
            } else {
                switch (map_property_multi_tile_xy(grid_offset)) {
                    case EDGE_X1Y0:
                    case EDGE_X1Y1:
                    case EDGE_X1Y2:
                        type = CITIZEN_0_ROAD;
                        break;
                }
            }
            break;
        case BUILDING_GRANARY:
            switch (map_property_multi_tile_xy(grid_offset)) {
                case EDGE_X1Y0:
                case EDGE_X0Y1:
                case EDGE_X1Y1:
                case EDGE_X2Y1:
                case EDGE_X1Y2:
                    type = CITIZEN_0_ROAD;
                    break;
            }
            break;
        case BUILDING_RESERVOIR:
            switch (map_property_multi_tile_xy(grid_offset)) {
                case EDGE_X1Y0:
                case EDGE_X0Y1:
                case EDGE_X2Y1:
                case EDGE_X1Y2:
                    type = CITIZEN_N4_RESERVOIR_CONNECTOR; // aqueduct connect points
                    break;
            }
            break;
    }
    return type;
}

static int get_land_type_citizen_aqueduct(int grid_offset)
{
    int image_id = map_image_at(grid_offset) - image_group(GROUP_BUILDING_AQUEDUCT);
    if (image_id <= 3) {
        return CITIZEN_N3_AQUEDUCT;
    } else if (image_id <= 7) {
        return CITIZEN_N1_BLOCKED;
    } else if (image_id <= 9) {
        return CITIZEN_N3_AQUEDUCT;
    } else if (image_id <= 14) {
        return CITIZEN_N1_BLOCKED;
    } else if (image_id <= 18) {
        return CITIZEN_N3_AQUEDUCT;
    } else if (image_id <= 22) {
        return CITIZEN_N1_BLOCKED;
    } else if (image_id <= 24) {
        return CITIZEN_N3_AQUEDUCT;
    } else {
        return CITIZEN_N1_BLOCKED;
    }
}

void map_routing_update_land_citizen(void)
{
    map_grid_init_i8(terrain_land_citizen.items, -1);
    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            int terrain = map_terrain_get(grid_offset);
            if (terrain & TERRAIN_ROAD) {
                terrain_land_citizen.items[grid_offset] = CITIZEN_0_ROAD;
            } else if (terrain & TERRAIN_HIGHWAY) {
                terrain_land_citizen.items[grid_offset] = CITIZEN_1_HIGHWAY;
            } else if (terrain & (TERRAIN_RUBBLE | TERRAIN_ACCESS_RAMP | TERRAIN_GARDEN)) {
                terrain_land_citizen.items[grid_offset] = CITIZEN_2_PASSABLE_TERRAIN;
            } else if (terrain & (TERRAIN_BUILDING | TERRAIN_GATEHOUSE)) {
                if (!map_building_at(grid_offset)) {
                    // shouldn't happen
                    terrain_land_noncitizen.items[grid_offset] = CITIZEN_4_CLEAR_TERRAIN; // BUG: should be citizen?
                    map_terrain_remove(grid_offset, TERRAIN_BUILDING);
                    map_image_set(grid_offset, (map_random_get(grid_offset) & 7) + image_group(GROUP_TERRAIN_GRASS_1));
                    map_property_mark_draw_tile(grid_offset);
                    map_property_set_multi_tile_size(grid_offset, 1);
                    continue;
                }
                terrain_land_citizen.items[grid_offset] = get_land_type_citizen_building(grid_offset);
            } else if (terrain & TERRAIN_AQUEDUCT) {
                terrain_land_citizen.items[grid_offset] = get_land_type_citizen_aqueduct(grid_offset);
            } else if (terrain & TERRAIN_NOT_CLEAR) {
                terrain_land_citizen.items[grid_offset] = CITIZEN_N1_BLOCKED;
            } else {
                terrain_land_citizen.items[grid_offset] = CITIZEN_4_CLEAR_TERRAIN;
            }
        }
    }
}

static int get_land_type_noncitizen(int grid_offset)
{
    int type = NONCITIZEN_1_BUILDING;
    switch (building_get(map_building_at(grid_offset))->type) {
        default:
            return NONCITIZEN_1_BUILDING;
        case BUILDING_WAREHOUSE:
        case BUILDING_FORT_GROUND:
            type = NONCITIZEN_0_PASSABLE;
            break;
        case BUILDING_BURNING_RUIN:
        case BUILDING_NATIVE_HUT:
        case BUILDING_NATIVE_MEETING:
        case BUILDING_NATIVE_CROPS:
            type = NONCITIZEN_N1_BLOCKED;
            break;
        case BUILDING_FORT:
            type = NONCITIZEN_5_FORT;
            break;
        case BUILDING_GRANARY:
            switch (map_property_multi_tile_xy(grid_offset)) {
                case EDGE_X1Y0:
                case EDGE_X0Y1:
                case EDGE_X1Y1:
                case EDGE_X2Y1:
                case EDGE_X1Y2:
                    type = NONCITIZEN_0_PASSABLE;
                    break;
            }
            break;
        case BUILDING_GARDEN_WALL_GATE:
        case BUILDING_ROADBLOCK:
        case BUILDING_HEDGE_GATE_DARK:
        case BUILDING_HEDGE_GATE_LIGHT:
            type = NONCITIZEN_0_PASSABLE;
            break;
    }
    return type;
}

static void map_routing_update_land_noncitizen(void)
{
    map_grid_init_i8(terrain_land_noncitizen.items, -1);
    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            int terrain = map_terrain_get(grid_offset);
            if (terrain & TERRAIN_GATEHOUSE) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_4_GATEHOUSE;
            } else if (terrain & TERRAIN_BUILDING) {
                terrain_land_noncitizen.items[grid_offset] = get_land_type_noncitizen(grid_offset);
            } else if (terrain & TERRAIN_ROAD) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_0_PASSABLE;
            } else if (terrain & TERRAIN_HIGHWAY) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_0_PASSABLE;
            } else if (terrain & (TERRAIN_GARDEN | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE)) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_2_CLEARABLE;
            } else if (terrain & TERRAIN_AQUEDUCT) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_2_CLEARABLE;
            } else if (terrain & TERRAIN_WALL) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_3_WALL;
            } else if (terrain & TERRAIN_NOT_CLEAR) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_N1_BLOCKED;
            } else {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_0_PASSABLE;
            }
        }
    }
}

static int is_surrounded_by_water(int grid_offset)
{
    return map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WATER) &&
        map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WATER) &&
        map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WATER) &&
        map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WATER);
}

void map_routing_update_water(void)
{
    map_grid_init_i8(terrain_water.items, -1);
    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            if (map_terrain_is(grid_offset, TERRAIN_WATER) && is_surrounded_by_water(grid_offset)) {
                if (x > 0 && x < map_data.width - 1 &&
                    y > 0 && y < map_data.height - 1) {
                    switch (map_sprite_bridge_at(grid_offset)) {
                        case 5:
                        case 6: // low bridge middle section
                            terrain_water.items[grid_offset] = WATER_N3_LOW_BRIDGE;
                            break;
                        case 13: // ship bridge pillar
                            terrain_water.items[grid_offset] = WATER_N1_BLOCKED;
                            break;
                        default:
                            terrain_water.items[grid_offset] = WATER_0_PASSABLE;
                            break;
                    }
                } else {
                    terrain_water.items[grid_offset] = WATER_N2_MAP_EDGE;
                }
            } else {
                terrain_water.items[grid_offset] = WATER_N1_BLOCKED;
            }
        }
    }
}

static int is_wall_tile(int grid_offset)
{
    return map_terrain_is(grid_offset, TERRAIN_WALL_OR_GATEHOUSE) ? 1 : 0;
}

static int count_adjacent_wall_tiles(int grid_offset)
{
    int adjacent = 0;
    switch (city_view_orientation()) {
        case DIR_0_TOP:
            adjacent += is_wall_tile(grid_offset + map_grid_delta(0, 1));
            adjacent += is_wall_tile(grid_offset + map_grid_delta(1, 1));
            adjacent += is_wall_tile(grid_offset + map_grid_delta(1, 0));
            break;
        case DIR_2_RIGHT:
            adjacent += is_wall_tile(grid_offset + map_grid_delta(0, 1));
            adjacent += is_wall_tile(grid_offset + map_grid_delta(-1, 1));
            adjacent += is_wall_tile(grid_offset + map_grid_delta(-1, 0));
            break;
        case DIR_4_BOTTOM:
            adjacent += is_wall_tile(grid_offset + map_grid_delta(0, -1));
            adjacent += is_wall_tile(grid_offset + map_grid_delta(-1, -1));
            adjacent += is_wall_tile(grid_offset + map_grid_delta(-1, 0));
            break;
        case DIR_6_LEFT:
            adjacent += is_wall_tile(grid_offset + map_grid_delta(0, -1));
            adjacent += is_wall_tile(grid_offset + map_grid_delta(1, -1));
            adjacent += is_wall_tile(grid_offset + map_grid_delta(1, 0));
            break;
    }
    return adjacent;
}

void map_routing_update_walls(void)
{
    map_grid_init_i8(terrain_walls.items, -1);
    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            if (map_terrain_is(grid_offset, TERRAIN_WALL)) {
                if (count_adjacent_wall_tiles(grid_offset) == 3) {
                    terrain_walls.items[grid_offset] = WALL_0_PASSABLE;
                } else {
                    terrain_walls.items[grid_offset] = WALL_N1_BLOCKED;
                }
            } else if (map_terrain_is(grid_offset, TERRAIN_GATEHOUSE)) {
                terrain_walls.items[grid_offset] = WALL_0_PASSABLE;
            } else {
                terrain_walls.items[grid_offset] = WALL_N1_BLOCKED;
            }
        }
    }
}

int map_routing_is_wall_passable(int grid_offset)
{
    return terrain_walls.items[grid_offset] == WALL_0_PASSABLE;
}

static int wall_tile_in_radius(int x, int y, int radius, int *x_wall, int *y_wall)
{
    int size = 1;
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, size, radius, &x_min, &y_min, &x_max, &y_max);

    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            if (map_routing_is_wall_passable(map_grid_offset(xx, yy))) {
                *x_wall = xx;
                *y_wall = yy;
                return 1;
            }
        }
    }
    return 0;
}

int map_routing_wall_tile_in_radius(int x, int y, int radius, int *x_wall, int *y_wall)
{
    for (int i = 1; i <= radius; i++) {
        if (wall_tile_in_radius(x, y, i, x_wall, y_wall)) {
            return 1;
        }
    }
    return 0;
}

int map_routing_citizen_is_passable(int grid_offset)
{
    return terrain_land_citizen.items[grid_offset] >= CITIZEN_0_ROAD ||
        terrain_land_citizen.items[grid_offset] <= CITIZEN_2_PASSABLE_TERRAIN;
}

int map_routing_citizen_is_road(int grid_offset)
{
    return terrain_land_citizen.items[grid_offset] == CITIZEN_0_ROAD;
}

int map_routing_citizen_is_highway(int grid_offset)
{
    return terrain_land_citizen.items[grid_offset] == CITIZEN_1_HIGHWAY;
}

int map_routing_citizen_is_passable_terrain(int grid_offset)
{
    return terrain_land_citizen.items[grid_offset] == CITIZEN_2_PASSABLE_TERRAIN;
}

int map_routing_noncitizen_is_passable(int grid_offset)
{
    return terrain_land_noncitizen.items[grid_offset] >= NONCITIZEN_0_PASSABLE;
}

int map_routing_is_destroyable(int grid_offset)
{
    return terrain_land_noncitizen.items[grid_offset] > NONCITIZEN_0_PASSABLE &&
        terrain_land_noncitizen.items[grid_offset] != NONCITIZEN_5_FORT;
}

int map_routing_get_destroyable(int grid_offset)
{
    switch (terrain_land_noncitizen.items[grid_offset]) {
        case NONCITIZEN_1_BUILDING:
            return DESTROYABLE_BUILDING;
        case NONCITIZEN_2_CLEARABLE:
            return DESTROYABLE_AQUEDUCT_GARDEN;
        case NONCITIZEN_3_WALL:
            return DESTROYABLE_WALL;
        case NONCITIZEN_4_GATEHOUSE:
            return DESTROYABLE_GATEHOUSE;
        default:
            return DESTROYABLE_NONE;
    }
}
