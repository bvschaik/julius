#include "routing_terrain.h"

#include "building/type.h"
#include "core/direction.h"
#include "graphics/image.h"
#include "map/grid.h"
#include "map/routing_data.h"

#include "Data/Building.h"
#include "Data/Grid.h"
#include "Data/State.h"
#include "../Routing.h"

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
    int building_id = Data_Grid_buildingIds[grid_offset];
    int type = Routing_Citizen_m1_Blocked;
    switch (Data_Buildings[building_id].type) {
        case BUILDING_WAREHOUSE:
        case BUILDING_GATEHOUSE:
            type = Routing_Citizen_0_Road;
            break;
        case BUILDING_FORT_GROUND:
            type = Routing_Citizen_2_PassableTerrain;
            break;
        case BUILDING_TRIUMPHAL_ARCH:
            if (Data_Buildings[building_id].subtype.orientation == 3) {
                switch (Data_Grid_edge[grid_offset] & Edge_MaskXY) {
                    case Edge_X0Y1:
                    case Edge_X1Y1:
                    case Edge_X2Y1:
                        type = Routing_Citizen_0_Road;
                        break;
                }
            } else {
                switch (Data_Grid_edge[grid_offset] & Edge_MaskXY) {
                    case Edge_X1Y0:
                    case Edge_X1Y1:
                    case Edge_X1Y2:
                        type = Routing_Citizen_0_Road;
                        break;
                }
            }
            break;
        case BUILDING_GRANARY:
            switch (Data_Grid_edge[grid_offset] & Edge_MaskXY) {
                case Edge_X1Y0:
                case Edge_X0Y1:
                case Edge_X1Y1:
                case Edge_X2Y1:
                case Edge_X1Y2:
                    type = Routing_Citizen_0_Road;
                    break;
            }
            break;
        case BUILDING_RESERVOIR:
            switch (Data_Grid_edge[grid_offset] & Edge_MaskXY) {
                case Edge_X1Y0:
                case Edge_X0Y1:
                case Edge_X2Y1:
                case Edge_X1Y2:
                    type = Routing_Citizen_m4_ReservoirConnector; // aqueduct connect points
                    break;
            }
            break;
    }
    return type;
}

static int get_land_type_citizen_aqueduct(int grid_offset)
{
    int graphicId = Data_Grid_graphicIds[grid_offset] - image_group(GROUP_BUILDING_AQUEDUCT);
    if (graphicId <= 3) {
        return Routing_Citizen_m3_Aqueduct;
    } else if (graphicId <= 7) {
        return Routing_Citizen_m1_Blocked;
    } else if (graphicId <= 9) {
        return Routing_Citizen_m3_Aqueduct;
    } else if (graphicId <= 14) {
        return Routing_Citizen_m1_Blocked;
    } else if (graphicId <= 18) {
        return Routing_Citizen_m3_Aqueduct;
    } else if (graphicId <= 22) {
        return Routing_Citizen_m1_Blocked;
    } else if (graphicId <= 24) {
        return Routing_Citizen_m3_Aqueduct;
    } else {
        return Routing_Citizen_m1_Blocked;
    }
}

void map_routing_update_land_citizen()
{
    map_grid_init_i8(Data_Grid_routingLandCitizen, -1);
    int grid_offset = Data_State.map.gridStartOffset;
    for (int y = 0; y < Data_State.map.height; y++, grid_offset += Data_State.map.gridBorderSize) {
        for (int x = 0; x < Data_State.map.width; x++, grid_offset++) {
            if (Data_Grid_terrain[grid_offset] & Terrain_Road) {
                Data_Grid_routingLandCitizen[grid_offset] = Routing_Citizen_0_Road;
            } else if (Data_Grid_terrain[grid_offset] & (Terrain_Rubble | Terrain_AccessRamp | Terrain_Garden)) {
                Data_Grid_routingLandCitizen[grid_offset] = Routing_Citizen_2_PassableTerrain;
            } else if (Data_Grid_terrain[grid_offset] & (Terrain_Building | Terrain_Gatehouse)) {
                if (!Data_Grid_buildingIds[grid_offset]) {
                    // shouldn't happen
                    Data_Grid_routingLandNonCitizen[grid_offset] = Routing_Citizen_4_ClearTerrain; // BUG: should be citizen grid?
                    Data_Grid_terrain[grid_offset] &= ~Terrain_Building;
                    Data_Grid_graphicIds[grid_offset] = (Data_Grid_random[grid_offset] & 7) + image_group(GROUP_TERRAIN_GRASS_1);
                    Data_Grid_edge[grid_offset] = Edge_LeftmostTile;
                    Data_Grid_bitfields[grid_offset] &= 0xf0; // remove sizes
                    continue;
                }
                Data_Grid_routingLandCitizen[grid_offset] = get_land_type_citizen_building(grid_offset);
            } else if (Data_Grid_terrain[grid_offset] & Terrain_Aqueduct) {
                Data_Grid_routingLandCitizen[grid_offset] = get_land_type_citizen_aqueduct(grid_offset);
            } else if (Data_Grid_terrain[grid_offset] & Terrain_NotClear) {
                Data_Grid_routingLandCitizen[grid_offset] = Routing_Citizen_m1_Blocked;
            } else {
                Data_Grid_routingLandCitizen[grid_offset] = Routing_Citizen_4_ClearTerrain;
            }
        }
    }
}

static int get_land_type_noncitizen(int grid_offset)
{
    int type = Routing_NonCitizen_1_Building;
    switch (Data_Buildings[Data_Grid_buildingIds[grid_offset]].type) {
        case BUILDING_WAREHOUSE:
        case BUILDING_FORT_GROUND:
            type = Routing_NonCitizen_0_Passable;
            break;
        case BUILDING_BURNING_RUIN:
        case BUILDING_NATIVE_HUT:
        case BUILDING_NATIVE_MEETING:
        case BUILDING_NATIVE_CROPS:
            type = Routing_NonCitizen_m1_Blocked;
            break;
        case BUILDING_FORT:
            type = Routing_NonCitizen_5_Fort;
            break;
        case BUILDING_GRANARY:
            switch (Data_Grid_edge[grid_offset] & Edge_MaskXY) {
                case Edge_X1Y0:
                case Edge_X0Y1:
                case Edge_X1Y1:
                case Edge_X2Y1:
                case Edge_X1Y2:
                    type = Routing_NonCitizen_0_Passable;
                    break;
            }
            break;
    }
    return type;
}

static void map_routing_update_land_noncitizen()
{
    map_grid_init_i8(Data_Grid_routingLandNonCitizen, -1);
    int grid_offset = Data_State.map.gridStartOffset;
    for (int y = 0; y < Data_State.map.height; y++, grid_offset += Data_State.map.gridBorderSize) {
        for (int x = 0; x < Data_State.map.width; x++, grid_offset++) {
            int terrain = Data_Grid_terrain[grid_offset] & Terrain_NotClear;
            if (Data_Grid_terrain[grid_offset] & Terrain_Gatehouse) {
                Data_Grid_routingLandNonCitizen[grid_offset] = Routing_NonCitizen_4_Gatehouse;
            } else if (terrain & Terrain_Road) {
                Data_Grid_routingLandNonCitizen[grid_offset] = Routing_NonCitizen_0_Passable;
            } else if (terrain & (Terrain_Garden | Terrain_AccessRamp | Terrain_Rubble)) {
                Data_Grid_routingLandNonCitizen[grid_offset] = Routing_NonCitizen_2_Clearable;
            } else if (terrain & Terrain_Building) {
                Data_Grid_routingLandNonCitizen[grid_offset] = get_land_type_noncitizen(grid_offset);
            } else if (Data_Grid_terrain[grid_offset] & Terrain_Aqueduct) {
                Data_Grid_routingLandNonCitizen[grid_offset] = Routing_NonCitizen_2_Clearable;
            } else if (Data_Grid_terrain[grid_offset] & Terrain_Wall) {
                Data_Grid_routingLandNonCitizen[grid_offset] = Routing_NonCitizen_3_Wall;
            } else if (Data_Grid_terrain[grid_offset] & Terrain_NotClear) {
                Data_Grid_routingLandNonCitizen[grid_offset] = Routing_NonCitizen_m1_Blocked;
            } else {
                Data_Grid_routingLandNonCitizen[grid_offset] = Routing_NonCitizen_0_Passable;
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
    map_grid_init_i8(Data_Grid_routingWater, -1);
    int grid_offset = Data_State.map.gridStartOffset;
    for (int y = 0; y < Data_State.map.height; y++, grid_offset += Data_State.map.gridBorderSize) {
        for (int x = 0; x < Data_State.map.width; x++, grid_offset++) {
            if (Data_Grid_terrain[grid_offset] & Terrain_Water && is_surrounded_by_water(grid_offset)) {
                if (x > 0 && x < Data_State.map.width - 1 &&
                    y > 0 && y < Data_State.map.height - 1) {
                    switch (Data_Grid_spriteOffsets[grid_offset]) {
                        case 5:
                        case 6: // low bridge middle section
                            Data_Grid_routingWater[grid_offset] = Routing_Water_m3_LowBridge;
                            break;
                        case 13: // ship bridge pillar
                            Data_Grid_routingWater[grid_offset] = Routing_Water_m1_Blocked;
                            break;
                        default:
                            Data_Grid_routingWater[grid_offset] = Routing_Water_0_Passable;
                            break;
                    }
                } else {
                    Data_Grid_routingWater[grid_offset] = Routing_Water_m2_MapEdge;
                }
            } else {
                Data_Grid_routingWater[grid_offset] = Routing_Water_m1_Blocked;
            }
        }
    }
}

static int is_gatehouse_tile(int grid_offset)
{
    return (Data_Grid_terrain[grid_offset] & Terrain_WallOrGatehouse) ? 1 : 0;
}

static int count_adjacent_gatehouse_tiles(int grid_offset)
{
    int adjacent = 0;
    switch (Data_State.map.orientation) {
        case DIR_0_TOP:
            adjacent += is_gatehouse_tile(grid_offset + map_grid_delta(0, 1));
            adjacent += is_gatehouse_tile(grid_offset + map_grid_delta(1, 1));
            adjacent += is_gatehouse_tile(grid_offset + map_grid_delta(1, 0));
            break;
        case DIR_2_RIGHT:
            adjacent += is_gatehouse_tile(grid_offset + map_grid_delta(0, 1));
            adjacent += is_gatehouse_tile(grid_offset + map_grid_delta(-1, 1));
            adjacent += is_gatehouse_tile(grid_offset + map_grid_delta(-1, 0));
            break;
        case DIR_4_BOTTOM:
            adjacent += is_gatehouse_tile(grid_offset + map_grid_delta(0, -1));
            adjacent += is_gatehouse_tile(grid_offset + map_grid_delta(-1, -1));
            adjacent += is_gatehouse_tile(grid_offset + map_grid_delta(-1, 0));
            break;
        case DIR_6_LEFT:
            adjacent += is_gatehouse_tile(grid_offset + map_grid_delta(0, -1));
            adjacent += is_gatehouse_tile(grid_offset + map_grid_delta(1, -1));
            adjacent += is_gatehouse_tile(grid_offset + map_grid_delta(1, 0));
            break;
    }
    return adjacent;
}

void map_routing_update_walls()
{
    map_grid_init_i8(Data_Grid_routingWalls, -1);
    int grid_offset = Data_State.map.gridStartOffset;
    for (int y = 0; y < Data_State.map.height; y++, grid_offset += Data_State.map.gridBorderSize) {
        for (int x = 0; x < Data_State.map.width; x++, grid_offset++) {
            if (Data_Grid_terrain[grid_offset] & Terrain_Wall) {
                if (count_adjacent_gatehouse_tiles(grid_offset) == 3) {
                    Data_Grid_routingWalls[grid_offset] = Routing_Wall_0_Passable;
                } else {
                    Data_Grid_routingWalls[grid_offset] = Routing_Wall_m1_Blocked;
                }
            } else if (Data_Grid_terrain[grid_offset] & Terrain_Gatehouse) {
                Data_Grid_routingWalls[grid_offset] = Routing_Wall_0_Passable;
            } else {
                Data_Grid_routingWalls[grid_offset] = Routing_Wall_m1_Blocked;
            }
        }
    }
}

void map_routing_block(int x, int y, int size)
{
    if (IsOutsideMap(x, y, size)) {
        return;
    }
    for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
            Data_Grid_routingDistance[map_grid_offset(x+dx, y+dy)] = 0;
        }
    }
}

int map_routing_is_wall_passable(int grid_offset)
{
    return Data_Grid_routingWalls[grid_offset] == 0;
}

int map_routing_citizen_is_passable(int grid_offset)
{
    return Data_Grid_routingLandCitizen[grid_offset] == Routing_Citizen_0_Road ||
        Data_Grid_routingLandCitizen[grid_offset] == Routing_Citizen_2_PassableTerrain;
}

int map_routing_citizen_is_road(int grid_offset)
{
    return Data_Grid_routingLandCitizen[grid_offset] == Routing_Citizen_0_Road;
}

int map_routing_citizen_is_passable_terrain(int grid_offset)
{
    return Data_Grid_routingLandCitizen[grid_offset] == Routing_Citizen_2_PassableTerrain;
}

int map_routing_noncitizen_is_passable(int grid_offset)
{
    return Data_Grid_routingLandNonCitizen[grid_offset] >= Routing_NonCitizen_0_Passable;
}

int map_routing_is_destroyable(int grid_offset)
{
    return Data_Grid_routingLandNonCitizen[grid_offset] > Routing_NonCitizen_0_Passable &&
        Data_Grid_routingLandNonCitizen[grid_offset] != Routing_NonCitizen_5_Fort;
}

int map_routing_get_destroyable(int grid_offset)
{
    switch (Data_Grid_routingLandNonCitizen[grid_offset]) {
        case Routing_NonCitizen_1_Building:
            return DESTROYABLE_BUILDING;
        case Routing_NonCitizen_2_Clearable:
            return DESTROYABLE_AQUEDUCT_GARDEN;
        case Routing_NonCitizen_3_Wall:
            return DESTROYABLE_WALL;
        case Routing_NonCitizen_4_Gatehouse:
            return DESTROYABLE_GATEHOUSE;
        default:
            return DESTROYABLE_NONE;
    }
}
