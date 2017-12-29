#include "road_aqueduct.h"

#include "building/building.h"
#include "core/direction.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"

#include "Data/State.h"

int map_can_place_road_under_aqueduct(int grid_offset)
{
    int image = map_image_at(grid_offset) - image_group(GROUP_BUILDING_AQUEDUCT);
    int check_y;
    switch (image) {
        case 0:
        case 2:
        case 8:
        case 15:
        case 17:
        case 23:
            check_y = 1;
            break;
        case 1:
        case 3:
        case 9: case 10: case 11: case 12: case 13: case 14:
        case 16:
        case 18:
        case 24: case 25: case 26: case 27: case 28: case 29:
            check_y = 0;
            break;
        default: // not a straight aqueduct
            return 0;
    }
    if (Data_State.map.orientation == DIR_6_LEFT || Data_State.map.orientation == DIR_2_RIGHT) {
        check_y = !check_y;
    }
    if (check_y) {
        int dy_up = map_grid_delta(0, -1);
        int dy_down = map_grid_delta(0, 1);
        if (map_terrain_is(grid_offset + dy_up, TERRAIN_ROAD) ||
            map_routing_distance(grid_offset + dy_up) > 0) {
            return 0;
        }
        if (map_terrain_is(grid_offset + dy_down, TERRAIN_ROAD) ||
            map_routing_distance(grid_offset + dy_down) > 0) {
            return 0;
        }
    } else {
        int dx_left = map_grid_delta(-1, 0);
        int dx_right = map_grid_delta(1, 0);
        if (map_terrain_is(grid_offset + dx_left, TERRAIN_ROAD) ||
            map_routing_distance(grid_offset + dx_left) > 0) {
            return 0;
        }
        if (map_terrain_is(grid_offset + dx_right, TERRAIN_ROAD) ||
            map_routing_distance(grid_offset + dx_right) > 0) {
            return 0;
        }
    }
    return 1;
}

int map_can_place_aqueduct_on_road(int grid_offset)
{
    int image = map_image_at(grid_offset) - image_group(GROUP_TERRAIN_ROAD);
    if (image != 0 && image != 1 && image != 49 && image != 50) {
        return 0;
    }
    int check_y = image == 0 || image == 49;
    if (Data_State.map.orientation == DIR_6_LEFT || Data_State.map.orientation == DIR_2_RIGHT) {
        check_y = !check_y;
    }
    if (check_y) {
        if (map_routing_distance(grid_offset + map_grid_delta(0, -1)) > 0 ||
            map_routing_distance(grid_offset + map_grid_delta(0, 1)) > 0) {
            return 0;
        }
    } else {
        if (map_routing_distance(grid_offset + map_grid_delta(-1, 0)) > 0 ||
            map_routing_distance(grid_offset + map_grid_delta(1, 0)) > 0) {
            return 0;
        }
    }
    return 1;
}

int map_get_aqueduct_with_road_image(int grid_offset)
{
    int image = map_image_at(grid_offset) - image_group(GROUP_BUILDING_AQUEDUCT);
    switch (image) {
        case 2:
            return 8;
        case 17:
            return 23;
        case 3:
            return 9;
        case 18:
            return 24;
        case 0:
        case 1:
        case 8:
        case 9:
        case 15:
        case 16:
        case 23:
        case 24:
            // unchanged
            return image;
        default:
            // shouldn't happen
            return 8;
    }
}


static int get_road_tile_for_aqueduct(int grid_offset, int gate_orientation)
{
    int is_road = map_terrain_is(grid_offset, TERRAIN_ROAD) ? 1 : 0;
    if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        building *b = building_get(map_building_at(grid_offset));
        if (b->type == BUILDING_GATEHOUSE) {
            if (b->subtype.orientation == gate_orientation) {
                is_road = 1;
            }
        } else if (b->type == BUILDING_GRANARY) {
            if (map_routing_citizen_is_road(grid_offset)) {
                is_road = 1;
            }
        }
    }
    return is_road;
}

int map_get_adjacent_road_tiles_for_aqueduct(int grid_offset)
{
    int road_tiles = 0;
    road_tiles += get_road_tile_for_aqueduct(grid_offset + map_grid_delta(0, -1), 1);
    road_tiles += get_road_tile_for_aqueduct(grid_offset + map_grid_delta(1, 0), 2);
    road_tiles += get_road_tile_for_aqueduct(grid_offset + map_grid_delta(0, 1), 1);
    road_tiles += get_road_tile_for_aqueduct(grid_offset + map_grid_delta(-1, 0), 2);
    if (road_tiles == 4) {
        if (building_get(map_building_at(grid_offset))->type == BUILDING_GRANARY) {
            road_tiles = 2;
        }
    }
    return road_tiles;
}
