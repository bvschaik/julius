#include "water.h"

#include "building/building.h"
#include "city/view.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/point.h"
#include "map/property.h"
#include "map/terrain.h"

#define OFFSET(x,y) (x + GRID_SIZE * y)

void map_water_add_building(int building_id, int x, int y, int size, int image_id)
{
    if (!map_grid_is_inside(x, y, size)) {
        return;
    }
    map_point leftmost;
    switch (city_view_orientation()) {
        case DIR_0_TOP:
            leftmost.x = 0;
            leftmost.y = size - 1;
            break;
        case DIR_2_RIGHT:
            leftmost.x = leftmost.y = 0;
            break;
        case DIR_4_BOTTOM:
            leftmost.x = size - 1;
            leftmost.y = 0;
            break;
        case DIR_6_LEFT:
            leftmost.x = leftmost.y = size - 1;
            break;
        default:
            return;
    }
    for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
            int grid_offset = map_grid_offset(x + dx, y + dy);
            map_terrain_add(grid_offset, TERRAIN_BUILDING);
            if (!map_terrain_is(grid_offset, TERRAIN_WATER)) {
                map_terrain_remove(grid_offset, TERRAIN_CLEARABLE);
                map_terrain_add(grid_offset, TERRAIN_BUILDING);
            }
            map_building_set(grid_offset, building_id);
            map_property_clear_constructing(grid_offset);
            map_property_set_multi_tile_size(grid_offset, size);
            map_image_set(grid_offset, image_id);
            map_property_set_multi_tile_xy(grid_offset, dx, dy,
                dx == leftmost.x && dy == leftmost.y);
        }
    }
}

static int blocked_land_terrain(void)
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER |
        TERRAIN_BUILDING | TERRAIN_SHRUB | TERRAIN_GARDEN |
        TERRAIN_ROAD | TERRAIN_ELEVATION | TERRAIN_RUBBLE;
}

int map_water_determine_orientation_size2(int x, int y, int adjust_xy,
                                          int *orientation_absolute, int *orientation_relative)
{
    if (adjust_xy == 1) {
        switch (city_view_orientation()) {
            case DIR_0_TOP: break;
            case DIR_2_RIGHT: x--; break;
            case DIR_6_LEFT: y--; break;
            case DIR_4_BOTTOM: x--; y--; break;
        }
    }
    if (!map_grid_is_inside(x, y, 2)) {
        return 999;
    }

    int base_offset = map_grid_offset(x, y);
    int tile_offsets[] = {OFFSET(0,0), OFFSET(1,0), OFFSET(0,1), OFFSET(1,1)};
    const int should_be_water[4][4] = {{1, 1, 0, 0}, {0, 1, 0, 1}, {0, 0, 1, 1}, {1, 0, 1, 0}};
    for (int dir = 0; dir < 4; dir++) {
        int ok_tiles = 0;
        int blocked_tiles = 0;
        for (int i = 0; i < 4; i++) {
            int grid_offset = base_offset + tile_offsets[i];
            if (should_be_water[dir][i]) {
                if (!map_terrain_is(grid_offset, TERRAIN_WATER)) {
                    break;
                }
                ok_tiles++;
                if (map_terrain_is(grid_offset, TERRAIN_ROCK | TERRAIN_ROAD)) {
                    // bridge or map edge
                    blocked_tiles++;
                }
            } else {
                if (map_terrain_is(grid_offset, TERRAIN_WATER)) {
                    break;
                }
                ok_tiles++;
                if (map_terrain_is(grid_offset, blocked_land_terrain())) {
                    blocked_tiles++;
                }
            }
        }
        // check six water tiles in front
        const int tiles_to_check[4][6] = {
            {OFFSET(-1,0), OFFSET(-1,-1), OFFSET(0,-1), OFFSET(1,-1), OFFSET(2,-1), OFFSET(2,0)},
            {OFFSET(1,-1), OFFSET(2,-1), OFFSET(2,0), OFFSET(2,1), OFFSET(2,2), OFFSET(1,2)},
            {OFFSET(2,1), OFFSET(2,2), OFFSET(1,2), OFFSET(0,2), OFFSET(-1,2), OFFSET(-1,1)},
            {OFFSET(0,2), OFFSET(-1,2), OFFSET(-1,1), OFFSET(-1,0), OFFSET(-1,-1), OFFSET(0,-1)},
        };
        for (int i = 0; i < 6; i++) {
            if (!map_terrain_is(base_offset + tiles_to_check[dir][i], TERRAIN_WATER)) {
                ok_tiles = 0;
            }
        }
        if (ok_tiles == 4) {
            // water/land is OK in this orientation
            if (orientation_absolute) {
                *orientation_absolute = dir;
            }
            if (orientation_relative) {
                *orientation_relative = (4 + dir - city_view_orientation() / 2) % 4;
            }
            return blocked_tiles;
        }
    }
    return 999;
}

int map_water_determine_orientation_size3(int x, int y, int adjust_xy,
                                          int *orientation_absolute, int *orientation_relative)
{
    if (adjust_xy == 1) {
        switch (city_view_orientation()) {
            case DIR_0_TOP: break;
            case DIR_2_RIGHT: x -= 2; break;
            case DIR_6_LEFT: y -= 2; break;
            case DIR_4_BOTTOM: x -= 2; y -= 2; break;
        }
    }
    if (!map_grid_is_inside(x, y, 3)) {
        return 999;
    }

    int base_offset = map_grid_offset(x, y);
    int tile_offsets[] = {
        OFFSET(0,0), OFFSET(1,0), OFFSET(2,0),
        OFFSET(0,1), OFFSET(1,1), OFFSET(2,1),
        OFFSET(0,2), OFFSET(1,2), OFFSET(2,2)
    };
    int should_be_water[4][9] = {
        {1, 1, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 1, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 1, 1, 1},
        {1, 0, 0, 1, 0, 0, 1, 0, 0}
    };
    for (int dir = 0; dir < 4; dir++) {
        int ok_tiles = 0;
        int blocked_tiles = 0;
        for (int i = 0; i < 9; i++) {
            int grid_offset = base_offset + tile_offsets[i];
            if (should_be_water[dir][i]) {
                if (!map_terrain_is(grid_offset, TERRAIN_WATER)) {
                    break;
                }
                ok_tiles++;
                if (map_terrain_is(grid_offset, TERRAIN_ROCK | TERRAIN_ROAD)) {
                    // bridge or map edge
                    blocked_tiles++;
                }
            } else {
                if (map_terrain_is(grid_offset, TERRAIN_WATER)) {
                    break;
                }
                ok_tiles++;
                if (map_terrain_is(grid_offset, blocked_land_terrain())) {
                    blocked_tiles++;
                }
            }
        }
        // check two water tiles at the side
        const int tiles_to_check[4][2] = {
            {OFFSET(-1,0), OFFSET(3,0)}, {OFFSET(2,-1), OFFSET(2,3)},
            {OFFSET(3,2), OFFSET(-1,2)}, {OFFSET(0,-1), OFFSET(0,3)}
        };
        for (int i = 0; i < 2; i++) {
            if (!map_terrain_is(base_offset + tiles_to_check[dir][i], TERRAIN_WATER)) {
                ok_tiles = 0;
            }
        }
        if (ok_tiles == 9) {
            // water/land is OK in this orientation
            if (orientation_absolute) {
                *orientation_absolute = dir;
            }
            if (orientation_relative) {
                *orientation_relative = (4 + dir - city_view_orientation() / 2) % 4;
            }
            return blocked_tiles;
        }
    }
    return 999;
}

int map_water_get_wharf_for_new_fishing_boat(figure *boat, map_point *tile)
{
    building *wharf = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_WHARF) {
            int wharf_boat_id = b->data.industry.fishing_boat_id;
            if (!wharf_boat_id || wharf_boat_id == boat->id) {
                wharf = b;
                break;
            }
        }
    }
    if (!wharf) {
        return 0;
    }
    int dx, dy;
    switch (wharf->data.industry.orientation) {
        case 0: dx = 1; dy = -1; break;
        case 1: dx = 2; dy = 1; break;
        case 2: dx = 1; dy = 2; break;
        default: dx = -1; dy = 1; break;
    }
    map_point_store_result(wharf->x + dx, wharf->y + dy, tile);
    return wharf->id;
}

int map_water_find_alternative_fishing_boat_tile(figure *boat, map_point *tile)
{
    if (map_figure_at(boat->grid_offset) == boat->id) {
        return 0;
    }
    for (int radius = 1; radius <= 5; radius++) {
        int x_min, y_min, x_max, y_max;
        map_grid_get_area(boat->x, boat->y, 1, radius, &x_min, &y_min, &x_max, &y_max);

        for (int yy = y_min; yy <= y_max; yy++) {
            for (int xx = x_min; xx <= x_max; xx++) {
                int grid_offset = map_grid_offset(xx, yy);
                if (!map_has_figure_at(grid_offset) && map_terrain_is(grid_offset, TERRAIN_WATER)) {
                    map_point_store_result(xx, yy, tile);
                    return 1;
                }
            }
        }
    }
    return 0;
}

int map_water_find_shipwreck_tile(figure *wreck, map_point *tile)
{
    if (map_terrain_is(wreck->grid_offset, TERRAIN_WATER) && map_figure_at(wreck->grid_offset) == wreck->id) {
        return 0;
    }
    for (int radius = 1; radius <= 5; radius++) {
        int x_min, y_min, x_max, y_max;
        map_grid_get_area(wreck->x, wreck->y, 1, radius, &x_min, &y_min, &x_max, &y_max);

        for (int yy = y_min; yy <= y_max; yy++) {
            for (int xx = x_min; xx <= x_max; xx++) {
                int grid_offset = map_grid_offset(xx, yy);
                if (!map_has_figure_at(grid_offset) || map_figure_at(grid_offset) == wreck->id) {
                    if (map_terrain_is(grid_offset, TERRAIN_WATER) &&
                        map_terrain_is(map_grid_offset(xx, yy - 2), TERRAIN_WATER) &&
                        map_terrain_is(map_grid_offset(xx, yy + 2), TERRAIN_WATER) &&
                        map_terrain_is(map_grid_offset(xx - 2, yy), TERRAIN_WATER) &&
                        map_terrain_is(map_grid_offset(xx + 2, yy), TERRAIN_WATER)) {
                        map_point_store_result(xx, yy, tile);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

static int num_surrounding_water_tiles(int grid_offset)
{
    int amount = 0;
    for (int i = 0; i < DIR_8_NONE; i++) {
        if (map_terrain_is(grid_offset + map_grid_direction_delta(i), TERRAIN_WATER)) {
            amount++;
        }
    }
    return amount;
}

int map_water_can_spawn_fishing_boat(int x, int y, int size, map_point *tile)
{
    int base_offset = map_grid_offset(x, y);
    for (const int *tile_delta = map_grid_adjacent_offsets(size); *tile_delta; tile_delta++) {
        int grid_offset = base_offset + *tile_delta;
        if (map_terrain_is(grid_offset, TERRAIN_WATER)) {
            if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
                if (num_surrounding_water_tiles(grid_offset) >= 8) {
                    map_point_store_result(map_grid_offset_to_x(grid_offset), map_grid_offset_to_y(grid_offset), tile);
                    return 1;
                }
            }
        }
    }
    return 0;
}
