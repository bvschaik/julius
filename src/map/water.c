#include "water.h"

#include "building/building.h"
#include "building/image.h"
#include "city/view.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/point.h"
#include "map/property.h"
#include "map/terrain.h"

#define OFFSET(x,y) ((x) + GRID_SIZE * (y))

void map_water_add_building(int building_id, int x, int y, int size)
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
            map_image_set(grid_offset, building_image_get(building_get(building_id)));
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

static int is_blocked_tile(int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_WATER)) {
        return map_terrain_is(grid_offset, TERRAIN_ROCK | TERRAIN_ROAD | TERRAIN_BUILDING);
    }
    return map_terrain_is(grid_offset, blocked_land_terrain());
}

int map_water_determine_orientation(int x, int y, int size, int adjust_xy,
    int *orientation_absolute, int *orientation_relative, int check_water_in_front, int *blocked)
{
    int edge = size - 1;
    int square = size * size;
    int unadjusted_base_offset;
    if (adjust_xy == 1) {
        unadjusted_base_offset = map_grid_offset(x, y);
        switch (city_view_orientation()) {
            case DIR_0_TOP: break;
            case DIR_2_RIGHT: x -= edge; break;
            case DIR_6_LEFT: y -= edge; break;
            case DIR_4_BOTTOM: x -= edge; y -= edge; break;
        }
    } else {
        switch (city_view_orientation()) {
            case DIR_0_TOP: unadjusted_base_offset = map_grid_offset(x, y); break;
            case DIR_2_RIGHT: unadjusted_base_offset = map_grid_offset(x + edge, y); break;
            case DIR_6_LEFT: unadjusted_base_offset = map_grid_offset(x, y + edge); break;
            case DIR_4_BOTTOM: unadjusted_base_offset = map_grid_offset(x + edge, y + edge); break;
        }
    }
    if (!map_grid_is_inside(x, y, size)) {
        return square;
    }

    int base_offset = map_grid_offset(x, y);
    int water_line_x[4] = { -1, edge, -1, 0 };
    int water_line_y[4] = { 0, -1, edge, -1 };

    for (int dir = 0; dir < 4; dir++) {
        int ok_tiles = 0;
        int blocked_tiles = 0;
        int index = 0;
        int column = 0;
        int row = 0;

        while (index < square) {
            for (column = 0; column < row; column++) {

                int should_be_water = water_line_y[dir] == row || water_line_x[dir] == column;
                ok_tiles += should_be_water == map_terrain_is(base_offset + OFFSET(column, row), TERRAIN_WATER);

                should_be_water = water_line_y[dir] == column || water_line_x[dir] == row;
                ok_tiles += should_be_water == map_terrain_is(base_offset + OFFSET(row, column), TERRAIN_WATER);

                index += 2;
            }
            int should_be_water = water_line_y[dir] == row || water_line_x[dir] == row;
            ok_tiles += should_be_water == map_terrain_is(base_offset + OFFSET(row, row), TERRAIN_WATER);

            index++;
            row++;
        }

        if (ok_tiles < square) {
            continue;
        }

        // water/land is OK in this orientation
        if (orientation_absolute) {
            *orientation_absolute = dir;
        }
        if (orientation_relative) {
            *orientation_relative = (4 + dir - city_view_orientation() / 2) % 4;
        }

        // Check for blocked tiles
        static const struct {
            int x;
            int y;
        } steps[4] = { { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } };

        int orientation = city_view_orientation() / 2;
        index = 0;
        column = 0;
        row = 0;
        int *x_offset = orientation & 1 ? &row : &column;
        int *y_offset = orientation & 1 ? &column : &row;

        while (index < square) {
            for (column = 0; column < row; column++) {
                int is_blocked[2];
                is_blocked[0] = is_blocked_tile(unadjusted_base_offset +
                    OFFSET(*x_offset * steps[orientation].x, *y_offset * steps[orientation].y));
                is_blocked[1] = is_blocked_tile(unadjusted_base_offset +
                    OFFSET(*y_offset * steps[orientation].x, *x_offset * steps[orientation].y));

                blocked_tiles += is_blocked[0] + is_blocked[1];
                if (blocked) {
                    blocked[index] = is_blocked[0];
                    blocked[index + 1] = is_blocked[1];
                }
                index += 2;
            }
            int is_blocked = is_blocked_tile(unadjusted_base_offset +
                OFFSET(row * steps[orientation].x, row * steps[orientation].y));
            blocked_tiles += is_blocked;

            if (blocked) {
                blocked[index] = is_blocked;
            }

            index++;
            row++;
        }

        if (!check_water_in_front ||
            map_water_has_water_in_front(x, y, 0, map_water_get_waterside_tile_loop(dir, size), 0)) {
            return blocked_tiles;
        }
    }
    // If the waterside building isn't properly positioned next to a shore, block everything
    if (blocked) {
        for (int i = 0; i < square; i++) {
            blocked[i] = 1;
        }
    }
    return square;
}

const waterside_tile_loop *map_water_get_waterside_tile_loop(int direction, int size)
{
    static waterside_tile_loop base_loops[4] = {
        { { -1,  0 }, {  0, -1 }, {  1,  0 } },
        { {  2, -1 }, {  1,  0 }, {  0,  1 } },
        { {  3,  2 }, {  0,  1 }, { -1,  0 } },
        { {  0,  3 }, { -1,  0 }, {  0, -1 } }
    };
    base_loops[direction].inner_length = size + 2;
    switch (direction) {
        case 1:
            base_loops[1].start.x = size - 1;
            return &base_loops[1];
        case 2:
            base_loops[2].start.x = size;
            base_loops[2].start.y = size - 1;
            return &base_loops[2];
        case 3:
            base_loops[3].start.y = size;
            return &base_loops[3];
        default:
            return &base_loops[0];
    }
}

int map_water_has_water_in_front(int x, int y, int adjust_xy, const waterside_tile_loop *loop, int *land_tiles)
{
    if (adjust_xy == 1) {
        int edge = loop->inner_length - 3;
        switch (city_view_orientation()) {
            case DIR_0_TOP: break;
            case DIR_2_RIGHT: x -= edge; break;
            case DIR_6_LEFT: y -= edge; break;
            case DIR_4_BOTTOM: x -= edge; y -= edge; break;
        }
    }
    int base_offset = map_grid_offset(x, y);

    // check three rows of water tiles in front
    int dx = loop->start.x;
    int dy = loop->start.y;
    int water_ok = 1;
    int index = 0;
    int outer_corners[2];
    for (int outer = 0; outer < MAP_WATER_WATERSIDE_ROWS_NEEDED; outer++) {
        for (int inner = 0; inner < loop->inner_length; inner++) {
            if (outer == MAP_WATER_WATERSIDE_ROWS_NEEDED - 1 && (inner == 0 || inner == loop->inner_length - 1)) {
                outer_corners[inner != 0] = !map_terrain_is(base_offset + OFFSET(dx, dy), TERRAIN_WATER);
            } else if (!map_terrain_is(base_offset + OFFSET(dx, dy), TERRAIN_WATER)) {
                water_ok = 0;
                if (land_tiles) {
                    land_tiles[index] = 1;
                } else {
                    return 0;
                }
            } else if (land_tiles) {
                land_tiles[index] = 0;
            }
            dx += loop->inner_step.x;
            dy += loop->inner_step.y;
            index++;
        }
        if (loop->outer_step.y) {
            dx = loop->start.x;
            dy += loop->outer_step.y;
        } else {
            dy = loop->start.y;
            dx += loop->outer_step.x;
        }
    }
    // Check outer corners: at least one of them must be water for the building to be placable
    if (water_ok && outer_corners[0] && outer_corners[1]) {
        water_ok = 0;
    }
    if (!water_ok && land_tiles) {
        land_tiles[index - 1] = outer_corners[1];
        land_tiles[index - loop->inner_length] = outer_corners[0];
    }
    return water_ok;
}

int map_water_get_wharf_for_new_fishing_boat(figure *boat, map_point *tile)
{
    building *wharf = 0;
    for (building *b = building_first_of_type(BUILDING_WHARF); b; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE) {
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
