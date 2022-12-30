#include "city_draw_highway.h"

#include "assets/assets.h"
#include "building/building.h"
#include "city/view.h"
#include "graphics/image.h"
#include "map/aqueduct.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/terrain.h"
#include "map/tiles.h"

static int highway_wall_direction_offsets[4] = { 1, -GRID_SIZE, -1, GRID_SIZE };

static int has_adjacent_road(int adjacent_grid_offset, int direction_index)
{
    int right_direction = highway_wall_direction_offsets[(direction_index + 3) % 4];
    int left_direction = highway_wall_direction_offsets[(direction_index + 1) % 4];
    int left_has_road = map_terrain_is(adjacent_grid_offset + left_direction, TERRAIN_ROAD);
    int right_has_road = map_terrain_is(adjacent_grid_offset + right_direction, TERRAIN_ROAD);
    if (left_has_road && right_has_road) {
        return 1;
    } else if (left_has_road && map_terrain_is(adjacent_grid_offset + left_direction * 2, TERRAIN_ROAD)) {
        return 1;
    } else if (right_has_road && map_terrain_is(adjacent_grid_offset + right_direction * 2, TERRAIN_ROAD)) {
        return 1;
    }
    return 0;
}

static int is_highway_access(int grid_offset, int direction_index)
{
    if (map_terrain_is(grid_offset, TERRAIN_HIGHWAY | TERRAIN_GATEHOUSE | TERRAIN_ACCESS_RAMP)) {
        return 1;
    }
    if (map_terrain_is(grid_offset, TERRAIN_ROAD) && !has_adjacent_road(grid_offset, direction_index)) {
        return 1;
    }
    if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        const building *b = building_get(map_building_at(grid_offset));
        if (b->type == BUILDING_GRANARY) {
            return grid_offset == b->grid_offset + map_grid_delta(1, 0) ||
                grid_offset == b->grid_offset + map_grid_delta(0, 1) ||
                grid_offset == b->grid_offset + map_grid_delta(2, 1) ||
                grid_offset == b->grid_offset + map_grid_delta(1, 2);
        }
    }
    return 0;
}

static void draw_wall_image(int grid_offset, int direction_index, int x, int y, float scale)
{
    int direction = highway_wall_direction_offsets[direction_index];

    int direction_offset = grid_offset + direction;
    if (is_highway_access(direction_offset, direction_index)) {
        return;
    }

    int last_direction_index = (direction_index + 3) % 4;
    int last_direction_offset = grid_offset + highway_wall_direction_offsets[last_direction_index];
    // last barrier was a corner and will handle the rendering
    if (!is_highway_access(last_direction_offset, last_direction_index)) {
        return;
    }

    int wall_offset = (direction_index + city_view_orientation() / 2) % 4;
    int next_direction_index = (direction_index + 1) % 4;
    int next_direction_offset = grid_offset + highway_wall_direction_offsets[next_direction_index];
    // is this a corner?
    if (!is_highway_access(next_direction_offset, next_direction_index)) {
        // increment by 4 to get the corner image
        wall_offset += 4;
    }
    int wall_image_id = assets_lookup_image_id(ASSET_HIGHWAY_BARRIER_START) + wall_offset;
    image_draw_isometric_footprint_from_draw_tile(wall_image_id, x, y, 0, scale);
}

void city_draw_highway_footprint(int x, int y, float scale, int grid_offset)
{
    int random_offset = map_random_get(grid_offset) & 15;
    int base_image_id = assets_lookup_image_id(ASSET_HIGHWAY_BASE_START) + random_offset;
    image_draw_isometric_footprint_from_draw_tile(base_image_id, x, y, 0, scale);
    draw_wall_image(grid_offset, 1, x, y, scale);
    draw_wall_image(grid_offset, 2, x, y, scale);
    if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
        int aqueduct_image_id = map_tiles_highway_get_aqueduct_image(grid_offset);
        image_draw_isometric_footprint_from_draw_tile(aqueduct_image_id, x, y, 0, scale);
    }
    draw_wall_image(grid_offset, 0, x, y, scale);
    draw_wall_image(grid_offset, 3, x, y, scale);
}
