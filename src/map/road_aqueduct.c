#include "road_aqueduct.h"

#include "building/building.h"
#include "city/view.h"
#include "core/direction.h"
#include "core/image.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"

int map_can_place_road_under_aqueduct(int grid_offset)
{
    int image_id = map_image_at(grid_offset) - image_group(GROUP_BUILDING_AQUEDUCT);
    int check_y;
    switch (image_id) {
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
    if (city_view_orientation() == DIR_6_LEFT || city_view_orientation() == DIR_2_RIGHT) {
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
    int image_id = map_image_at(grid_offset) - image_group(GROUP_TERRAIN_ROAD);
    if (image_id != 0 && image_id != 1 && image_id != 49 && image_id != 50) {
        return 0;
    }
    if (map_terrain_count_directly_adjacent_with_types(grid_offset, TERRAIN_ROAD | TERRAIN_AQUEDUCT)) {
        return 0;
    }

    int check_y = image_id == 0 || image_id == 49;
    if (city_view_orientation() == DIR_6_LEFT || city_view_orientation() == DIR_2_RIGHT) {
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
    int image_id = map_image_at(grid_offset) - image_group(GROUP_BUILDING_AQUEDUCT);
    switch (image_id) {
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
            return image_id;
        default:
            // shouldn't happen
            return 8;
    }
}

static int is_road_tile_for_aqueduct(int grid_offset, int gate_orientation)
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

int map_is_straight_road_for_aqueduct(int grid_offset)
{
    int road_tiles_x =
        is_road_tile_for_aqueduct(grid_offset + map_grid_delta(1, 0), 2) +
        is_road_tile_for_aqueduct(grid_offset + map_grid_delta(-1, 0), 2);
    int road_tiles_y =
        is_road_tile_for_aqueduct(grid_offset + map_grid_delta(0, -1), 1) +
        is_road_tile_for_aqueduct(grid_offset + map_grid_delta(0, 1), 1);

    if (road_tiles_x == 2 && road_tiles_y == 0) {
        return 1;
    } else if (road_tiles_y == 2 && road_tiles_x == 0) {
        return 1;
    } else {
        return 0;
    }
}

static int is_highway(int x, int y, int check_routing)
{
    int grid_offset = map_grid_offset(x, y);
    if (!map_grid_is_valid_offset(grid_offset)) {
        return 0;
    } else if (map_terrain_is(grid_offset, TERRAIN_HIGHWAY)) {
        return 1;
    } else if (check_routing) {
        for (int xx = x - 1; xx <= x; xx++) {
            for (int yy = y - 1; yy <= y; yy++) {
                int routing_grid_offset = map_grid_offset(xx, yy);
                if (map_routing_distance(routing_grid_offset) > 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int is_aqueduct(int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    if (map_grid_is_valid_offset(grid_offset) && map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
        return 1;
    }
    return 0;
}

static int is_highway_and_aqueduct(int x, int y, int check_highway_routing)
{
    if (is_highway(x, y, check_highway_routing) && is_aqueduct(x, y)) {
        return 1;
    }
    return 0;
}

// check to see if placing an aqueduct here would create an aqueduct corner on a highway
// note: this tile does NOT need to be on a highway to create a corner on one
static int aqueduct_placement_creates_corner(int x, int y, int corner_x, int corner_y)
{
    if (!is_highway_and_aqueduct(corner_x, corner_y, 0)) {
        return 0;
    }
    int c1x, c1y, c2x, c2y;
    map_grid_get_corner_tiles(x, y, corner_x, corner_y, &c1x, &c1y, &c2x, &c2y);
    if (is_aqueduct(c1x, c1y) || is_aqueduct(c2x, c2y)) {
        return 1;
    }
    return 0;
}

// check to see if placing an aqueduct here would create a line (at least two aqueduct tiles) along a highway
static int aqueduct_highway_line(int x, int y, int is_check_x, int check_highway_routing)
{
    int x_offs = 1;
    int y_offs = 0;
    if (!is_check_x) {
        x_offs = 0;
        y_offs = 1;
    }
    int left_occupied = is_highway_and_aqueduct(x - x_offs, y - y_offs, check_highway_routing);
    int right_occupied = is_highway_and_aqueduct(x + x_offs, y + y_offs, check_highway_routing);
    if (left_occupied && right_occupied) {
        return 1;
    } else if (left_occupied && is_highway(x - x_offs * 2, y - y_offs * 2, check_highway_routing)) {
        return 1;
    } else if (right_occupied && is_highway(x + x_offs * 2, y + y_offs * 2, check_highway_routing)) {
        return 1;
    }
    return 0;
}

int map_can_place_aqueduct_on_highway(int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_HIGHWAY)) {
        return 1;
    }

    int x = map_grid_offset_to_x(grid_offset);
    int y = map_grid_offset_to_y(grid_offset);

    if (aqueduct_placement_creates_corner(x, y, x + 1, y)) {
        return 0;
    } else if (aqueduct_placement_creates_corner(x, y, x - 1, y)) {
        return 0;
    } else if (aqueduct_placement_creates_corner(x, y, x, y + 1)) {
        return 0;
    } else if (aqueduct_placement_creates_corner(x, y, x, y - 1)) {
        return 0;
    }

    if (aqueduct_highway_line(x, y, 1, 0)) {
        return 0;
    } else if (aqueduct_highway_line(x, y, 0, 0)) {
        return 0;
    }

    return 1;
}

int map_can_place_highway_under_aqueduct(int grid_offset, int check_highway_routing)
{
    if (!map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
        return 1;
    }

    int x = map_grid_offset_to_x(grid_offset);
    int y = map_grid_offset_to_y(grid_offset);
    if (is_aqueduct(x - 1, y) && is_aqueduct(x, y - 1)) {
        return 0;
    } else if (is_aqueduct(x, y - 1) && is_aqueduct(x + 1, y)) {
        return 0;
    } else if (is_aqueduct(x + 1, y) && is_aqueduct(x, y + 1)) {
        return 0;
    } else if (is_aqueduct(x, y + 1) && is_aqueduct(x - 1, y)) {
        return 0;
    }

    if (aqueduct_highway_line(x, y, 1, check_highway_routing)) {
        return 0;
    } else if (aqueduct_highway_line(x, y, 0, check_highway_routing)) {
        return 0;
    }

    return 1;
}
