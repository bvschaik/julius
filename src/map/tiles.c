#include "tiles.h"

#include "graphics/image.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"

#include "Data/State.h"

static void foreach_map_tile(void (*callback)(int x, int y, int grid_offset))
{
    int grid_offset = Data_State.map.gridStartOffset;
    for (int y = 0; y < Data_State.map.height; y++, grid_offset += Data_State.map.gridBorderSize) {
        for (int x = 0; x < Data_State.map.width; x++, grid_offset++) {
            callback(x, y, grid_offset);
        }
    }
}

static int is_all_terrain_in_area(int x, int y, int size, int terrain)
{
    if (!map_grid_is_inside(x, y, size)) {
        return 0;
    }
    for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
            int gridOffset = map_grid_offset(x + dx, y + dy);
            if ((map_terrain_get(gridOffset) & TERRAIN_NOT_CLEAR) != terrain) {
                return 0;
            }
            if (map_image_at(gridOffset) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

static void clear_rock_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ROCK) && !map_terrain_is(grid_offset, TERRAIN_RESERVOIR_RANGE | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
        map_image_set(grid_offset, 0);
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
    }
}

static void set_rock_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ROCK) && !map_terrain_is(grid_offset, TERRAIN_RESERVOIR_RANGE | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
        if (!map_image_at(grid_offset)) {
            if (is_all_terrain_in_area(x, y, 3, TERRAIN_ROCK)) {
                int graphicId = 12 + (map_random_get(grid_offset) & 1);
                if (map_terrain_exists_tile_in_radius_with_type(x, y, 3, 4, TERRAIN_ELEVATION)) {
                    graphicId += image_group(GROUP_TERRAIN_ELEVATION_ROCK);
                } else {
                    graphicId += image_group(GROUP_TERRAIN_ROCK);
                }
                map_building_tiles_add(0, x, y, 3, graphicId, TERRAIN_ROCK);
            } else if (is_all_terrain_in_area(x, y, 2, TERRAIN_ROCK)) {
                int graphicId = 8 + (map_random_get(grid_offset) & 3);
                if (map_terrain_exists_tile_in_radius_with_type(x, y, 2, 4, TERRAIN_ELEVATION)) {
                    graphicId += image_group(GROUP_TERRAIN_ELEVATION_ROCK);
                } else {
                    graphicId += image_group(GROUP_TERRAIN_ROCK);
                }
                map_building_tiles_add(0, x, y, 2, graphicId, TERRAIN_ROCK);
            } else {
                int graphicId = map_random_get(grid_offset) & 7;
                if (map_terrain_exists_tile_in_radius_with_type(x, y, 1, 4, TERRAIN_ELEVATION)) {
                    graphicId += image_group(GROUP_TERRAIN_ELEVATION_ROCK);
                } else {
                    graphicId += image_group(GROUP_TERRAIN_ROCK);
                }
                map_image_set(grid_offset, graphicId);
            }
        }
    }
}

void map_tiles_update_all_rocks()
{
    foreach_map_tile(clear_rock_image);
    foreach_map_tile(set_rock_image);
}

static void clear_garden_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_GARDEN) &&
        !map_terrain_is(grid_offset, TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
        map_image_set(grid_offset, 0);
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
    }
}

static void set_garden_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_GARDEN) && !map_terrain_is(grid_offset, TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
        if (!map_image_at(grid_offset)) {
            int image_id = image_group(GROUP_TERRAIN_GARDEN);
            if (is_all_terrain_in_area(x, y, 2, TERRAIN_GARDEN)) {
                switch (map_random_get(grid_offset) & 3) {
                    case 0: case 1:
                        image_id += 6;
                        break;
                    case 2:
                        image_id += 5;
                        break;
                    case 3:
                        image_id += 4;
                        break;
                }
                map_building_tiles_add(0, x, y, 2, image_id, TERRAIN_GARDEN);
            } else {
                if (y & 1) {
                    switch (x & 3) {
                        case 0: case 2:
                            image_id += 2;
                            break;
                        case 1: case 3:
                            image_id += 3;
                            break;
                    }
                } else {
                    switch (x & 3) {
                        case 1: case 3:
                            image_id += 1;
                            break;
                    }
                }
                map_image_set(grid_offset, image_id);
            }
        }
    }
}

static void remove_plaza_below_building(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ROAD) &&
        map_property_is_plaza_or_earthquake(grid_offset)) {
        if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
            map_property_clear_plaza_or_earthquake(grid_offset);
        }
    }
}

static void clear_plaza_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ROAD) &&
        map_property_is_plaza_or_earthquake(grid_offset)) {
        map_image_set(grid_offset, 0);
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
    }
}

static int is_tile_plaza(int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ROAD) &&
        map_property_is_plaza_or_earthquake(grid_offset) &&
        !map_terrain_is(grid_offset, TERRAIN_WATER | TERRAIN_BUILDING) &&
        !map_image_at(grid_offset)) {
        return 1;
    }
    return 0;
}

static int is_two_tile_square_plaza(int grid_offset)
{
    return
        is_tile_plaza(grid_offset + map_grid_delta(1, 0)) &&
        is_tile_plaza(grid_offset + map_grid_delta(0, 1)) &&
        is_tile_plaza(grid_offset + map_grid_delta(1, 1));
}

static void set_plaza_image(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_ROAD) &&
        map_property_is_plaza_or_earthquake(grid_offset) &&
        !map_image_at(grid_offset)) {
        int image_id = image_group(GROUP_TERRAIN_PLAZA);
        if (is_two_tile_square_plaza(grid_offset)) {
            if (map_random_get(grid_offset) & 1) {
                image_id += 7;
            } else {
                image_id += 6;
            }
            map_building_tiles_add(0, x, y, 2, image_id, TERRAIN_ROAD);
        } else {
            // single tile plaza
            switch ((x & 1) + (y & 1)) {
                case 2: image_id += 1; break;
                case 1: image_id += 2; break;
            }
            map_image_set(grid_offset, image_id);
        }
    }
}

void map_tiles_update_all_gardens()
{
    foreach_map_tile(clear_garden_image);
    foreach_map_tile(set_garden_image);
}

void map_tiles_update_all_plazas()
{
    foreach_map_tile(remove_plaza_below_building);
    foreach_map_tile(clear_plaza_image);
    foreach_map_tile(set_plaza_image);
}
