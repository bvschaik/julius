#include "routing_terrain.h"

#include "building/type.h"
#include "core/direction.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/routing_data.h"

#include "Data/Building.h"
#include "Data/State.h"

static void map_routing_update_land_noncitizen();

void map_routing_update_all()
{
    map_routing_update_land();
    map_routing_update_water();
    map_routing_update_walls();
}

void map_routing_update_land()
{
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen();
}

static int get_land_type_citizen_building(int grid_offset)
{
    int building_id = map_building_at(grid_offset);
    int type = CITIZEN_N1_BLOCKED;
    switch (Data_Buildings[building_id].type) {
        case BUILDING_WAREHOUSE:
        case BUILDING_GATEHOUSE:
            type = CITIZEN_0_ROAD;
            break;
        case BUILDING_FORT_GROUND:
            type = CITIZEN_2_PASSABLE_TERRAIN;
            break;
        case BUILDING_TRIUMPHAL_ARCH:
            if (Data_Buildings[building_id].subtype.orientation == 3) {
                switch (map_property_multi_tile_xy(grid_offset)) {
                    case Edge_X0Y1:
                    case Edge_X1Y1:
                    case Edge_X2Y1:
                        type = CITIZEN_0_ROAD;
                        break;
                }
            } else {
                switch (map_property_multi_tile_xy(grid_offset)) {
                    case Edge_X1Y0:
                    case Edge_X1Y1:
                    case Edge_X1Y2:
                        type = CITIZEN_0_ROAD;
                        break;
                }
            }
            break;
        case BUILDING_GRANARY:
            switch (map_property_multi_tile_xy(grid_offset)) {
                case Edge_X1Y0:
                case Edge_X0Y1:
                case Edge_X1Y1:
                case Edge_X2Y1:
                case Edge_X1Y2:
                    type = CITIZEN_0_ROAD;
                    break;
            }
            break;
        case BUILDING_RESERVOIR:
            switch (map_property_multi_tile_xy(grid_offset)) {
                case Edge_X1Y0:
                case Edge_X0Y1:
                case Edge_X2Y1:
                case Edge_X1Y2:
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

void map_routing_update_land_citizen()
{
    map_grid_init_i8(terrain_land_citizen.items, -1);
    int grid_offset = Data_State.map.gridStartOffset;
    for (int y = 0; y < Data_State.map.height; y++, grid_offset += Data_State.map.gridBorderSize) {
        for (int x = 0; x < Data_State.map.width; x++, grid_offset++) {
            if (Data_Grid_terrain[grid_offset] & Terrain_Road) {
                terrain_land_citizen.items[grid_offset] = CITIZEN_0_ROAD;
            } else if (Data_Grid_terrain[grid_offset] & (Terrain_Rubble | Terrain_AccessRamp | Terrain_Garden)) {
                terrain_land_citizen.items[grid_offset] = CITIZEN_2_PASSABLE_TERRAIN;
            } else if (Data_Grid_terrain[grid_offset] & (Terrain_Building | Terrain_Gatehouse)) {
                if (!map_building_at(grid_offset)) {
                    // shouldn't happen
                    terrain_land_noncitizen.items[grid_offset] = CITIZEN_4_CLEAR_TERRAIN; // BUG: should be citizen grid?
                    Data_Grid_terrain[grid_offset] &= ~Terrain_Building;
                    map_image_set(grid_offset, (map_random_get(grid_offset) & 7) + image_group(GROUP_TERRAIN_GRASS_1));
                    map_property_mark_draw_tile(grid_offset);
                    map_property_set_multi_tile_size(grid_offset, 1);
                    continue;
                }
                terrain_land_citizen.items[grid_offset] = get_land_type_citizen_building(grid_offset);
            } else if (Data_Grid_terrain[grid_offset] & Terrain_Aqueduct) {
                terrain_land_citizen.items[grid_offset] = get_land_type_citizen_aqueduct(grid_offset);
            } else if (Data_Grid_terrain[grid_offset] & Terrain_NotClear) {
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
    switch (Data_Buildings[map_building_at(grid_offset)].type) {
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
                case Edge_X1Y0:
                case Edge_X0Y1:
                case Edge_X1Y1:
                case Edge_X2Y1:
                case Edge_X1Y2:
                    type = NONCITIZEN_0_PASSABLE;
                    break;
            }
            break;
    }
    return type;
}

static void map_routing_update_land_noncitizen()
{
    map_grid_init_i8(terrain_land_noncitizen.items, -1);
    int grid_offset = Data_State.map.gridStartOffset;
    for (int y = 0; y < Data_State.map.height; y++, grid_offset += Data_State.map.gridBorderSize) {
        for (int x = 0; x < Data_State.map.width; x++, grid_offset++) {
            int terrain = Data_Grid_terrain[grid_offset] & Terrain_NotClear;
            if (Data_Grid_terrain[grid_offset] & Terrain_Gatehouse) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_4_GATEHOUSE;
            } else if (terrain & Terrain_Road) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_0_PASSABLE;
            } else if (terrain & (Terrain_Garden | Terrain_AccessRamp | Terrain_Rubble)) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_2_CLEARABLE;
            } else if (terrain & Terrain_Building) {
                terrain_land_noncitizen.items[grid_offset] = get_land_type_noncitizen(grid_offset);
            } else if (Data_Grid_terrain[grid_offset] & Terrain_Aqueduct) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_2_CLEARABLE;
            } else if (Data_Grid_terrain[grid_offset] & Terrain_Wall) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_3_WALL;
            } else if (Data_Grid_terrain[grid_offset] & Terrain_NotClear) {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_N1_BLOCKED;
            } else {
                terrain_land_noncitizen.items[grid_offset] = NONCITIZEN_0_PASSABLE;
            }
        }
    }
}

static int is_surrounded_by_water(int grid_offset)
{
    return (Data_Grid_terrain[grid_offset + map_grid_delta(0, -1)] & Terrain_Water) &&
        (Data_Grid_terrain[grid_offset + map_grid_delta(-1, 0)] & Terrain_Water) &&
        (Data_Grid_terrain[grid_offset + map_grid_delta(1, 0)] & Terrain_Water) &&
        (Data_Grid_terrain[grid_offset + map_grid_delta(0, 1)] & Terrain_Water);
}

void map_routing_update_water()
{
    map_grid_init_i8(terrain_water.items, -1);
    int grid_offset = Data_State.map.gridStartOffset;
    for (int y = 0; y < Data_State.map.height; y++, grid_offset += Data_State.map.gridBorderSize) {
        for (int x = 0; x < Data_State.map.width; x++, grid_offset++) {
            if (Data_Grid_terrain[grid_offset] & Terrain_Water && is_surrounded_by_water(grid_offset)) {
                if (x > 0 && x < Data_State.map.width - 1 &&
                    y > 0 && y < Data_State.map.height - 1) {
                    switch (Data_Grid_spriteOffsets[grid_offset]) {
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
    return (Data_Grid_terrain[grid_offset] & Terrain_WallOrGatehouse) ? 1 : 0;
}

static int count_adjacent_wall_tiles(int grid_offset)
{
    int adjacent = 0;
    switch (Data_State.map.orientation) {
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

void map_routing_update_walls()
{
    map_grid_init_i8(terrain_walls.items, -1);
    int grid_offset = Data_State.map.gridStartOffset;
    for (int y = 0; y < Data_State.map.height; y++, grid_offset += Data_State.map.gridBorderSize) {
        for (int x = 0; x < Data_State.map.width; x++, grid_offset++) {
            if (Data_Grid_terrain[grid_offset] & Terrain_Wall) {
                if (count_adjacent_wall_tiles(grid_offset) == 3) {
                    terrain_walls.items[grid_offset] = WALL_0_PASSABLE;
                } else {
                    terrain_walls.items[grid_offset] = WALL_N1_BLOCKED;
                }
            } else if (Data_Grid_terrain[grid_offset] & Terrain_Gatehouse) {
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

int map_routing_citizen_is_passable(int grid_offset)
{
    return terrain_land_citizen.items[grid_offset] == CITIZEN_0_ROAD ||
        terrain_land_citizen.items[grid_offset] == CITIZEN_2_PASSABLE_TERRAIN;
}

int map_routing_citizen_is_road(int grid_offset)
{
    return terrain_land_citizen.items[grid_offset] == CITIZEN_0_ROAD;
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
