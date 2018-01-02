#include "tiles.h"

#include "core/direction.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/desirability.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/image_context.h"
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

static void foreach_region_tile(int x_min, int y_min, int x_max, int y_max,
                                void (*callback)(int x, int y, int grid_offset))
{
    map_grid_bound_area(&x_min, &y_min, &x_max, &y_max);
    int grid_offset = map_grid_offset(x_min, y_min);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            callback(xx, yy, grid_offset);
            ++grid_offset;
        }
        grid_offset += GRID_SIZE - (x_max - x_min + 1);
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

static int get_gatehouse_building_id(int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_GATEHOUSE)) {
        return map_building_at(grid_offset);
    }
    return 0;
}

static int get_gatehouse_position(int grid_offset, int direction, int building_id)
{
    int result = 0;
    if (direction == DIR_0_TOP) {
        if (map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_GATEHOUSE) &&
                map_building_at(grid_offset + map_grid_delta(1, -1)) == building_id) {
            result = 1;
            if (!map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_WALL)) {
                result = 2;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        } else if (map_terrain_is(grid_offset + map_grid_delta(-1, -1), TERRAIN_GATEHOUSE) &&
                map_building_at(grid_offset + map_grid_delta(-1, -1)) == building_id) {
            result = 3;
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_WALL)) {
                result = 4;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        }
    } else if (direction == DIR_6_LEFT) {
        if (map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_GATEHOUSE) &&
                map_building_at(grid_offset + map_grid_delta(-1, 1)) == building_id) {
            result = 1;
            if (!map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_WALL)) {
                result = 2;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        } else if (map_terrain_is(grid_offset + map_grid_delta(-1, -1), TERRAIN_GATEHOUSE) &&
                map_building_at(grid_offset + map_grid_delta(-1, -1)) == building_id) {
            result = 3;
            if (!map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_WALL)) {
                result = 4;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        }
    } else if (direction == DIR_4_BOTTOM) {
        if (map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_GATEHOUSE) &&
                map_building_at(grid_offset + map_grid_delta(1, 1)) == building_id) {
            result = 1;
            if (!map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(-1, -1), TERRAIN_WALL)) {
                result = 2;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        } else if (map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_GATEHOUSE) &&
                map_building_at(grid_offset + map_grid_delta(-1, 1)) == building_id) {
            result = 3;
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_WALL)) {
                result = 4;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        }
    } else if (direction == DIR_2_RIGHT) {
        if (map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_GATEHOUSE) &&
                map_building_at(grid_offset + map_grid_delta(1, 1)) == building_id) {
            result = 1;
            if (!map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(-1, -1), TERRAIN_WALL)) {
                result = 2;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        } else if (map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_GATEHOUSE) &&
                map_building_at(grid_offset + map_grid_delta(1, -1)) == building_id) {
            result = 3;
            if (!map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WALL)) {
                result = 0;
            }
            if (map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WALL) &&
                map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_WALL)) {
                result = 4;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
            if (!map_terrain_is(grid_offset + map_grid_delta(-1, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
                result = 0;
            }
        }
    }
    return result;
}

static void set_wall_gatehouse_image_manually(int grid_offset)
{
    int gatehouse_up = get_gatehouse_building_id(grid_offset + map_grid_delta(0, -1));
    int gatehouse_left = get_gatehouse_building_id(grid_offset + map_grid_delta(-1, 0));
    int gatehouse_down = get_gatehouse_building_id(grid_offset + map_grid_delta(0, 1));
    int gatehouse_right = get_gatehouse_building_id(grid_offset + map_grid_delta(1, 0));
    int image_offset = 0;
    if (Data_State.map.orientation == DIR_0_TOP) {
        if (gatehouse_up && !gatehouse_left) {
            int pos = get_gatehouse_position(grid_offset, DIR_0_TOP, gatehouse_up);
            if (pos > 0) {
                if (pos <= 2) {
                    image_offset = 29;
                } else if (pos == 3) {
                    image_offset = 31;
                } else {
                    image_offset = 33;
                }
            }
        } else if (gatehouse_left && !gatehouse_up) {
            int pos = get_gatehouse_position(grid_offset, DIR_6_LEFT, gatehouse_left);
            if (pos > 0) {
                if (pos <= 2) {
                    image_offset = 30;
                } else if (pos == 3) {
                    image_offset = 32;
                } else {
                    image_offset = 33;
                }
            }
        }
    } else if (Data_State.map.orientation == DIR_2_RIGHT) {
        if (gatehouse_up && !gatehouse_right) {
            int pos = get_gatehouse_position(grid_offset, DIR_0_TOP, gatehouse_up);
            if (pos > 0) {
                if (pos == 1) {
                    image_offset = 32;
                } else if (pos == 2) {
                    image_offset = 33;
                } else {
                    image_offset = 30;
                }
            }
        } else if (gatehouse_right && !gatehouse_up) {
            int pos = get_gatehouse_position(grid_offset, DIR_2_RIGHT, gatehouse_right);
            if (pos > 0) {
                if (pos <= 2) {
                    image_offset = 29;
                } else if (pos == 3) {
                    image_offset = 31;
                } else {
                    image_offset = 33;
                }
            }
        }
    } else if (Data_State.map.orientation == DIR_4_BOTTOM) {
        if (gatehouse_down && !gatehouse_right) {
            int pos = get_gatehouse_position(grid_offset, DIR_4_BOTTOM, gatehouse_down);
            if (pos > 0) {
                if (pos == 1) {
                    image_offset = 31;
                } else if (pos == 2) {
                    image_offset = 33;
                } else {
                    image_offset = 29;
                }
            }
        } else if (gatehouse_right && !gatehouse_down) {
            int pos = get_gatehouse_position(grid_offset, DIR_2_RIGHT, gatehouse_right);
            if (pos > 0) {
                if (pos == 1) {
                    image_offset = 32;
                } else if (pos == 2) {
                    image_offset = 33;
                } else {
                    image_offset = 30;
                }
            }
        }
    } else if (Data_State.map.orientation == DIR_6_LEFT) {
        if (gatehouse_down && !gatehouse_left) {
            int pos = get_gatehouse_position(grid_offset, DIR_4_BOTTOM, gatehouse_down);
            if (pos > 0) {
                if (pos <= 2) {
                    image_offset = 30;
                } else if (pos == 3) {
                    image_offset = 32;
                } else {
                    image_offset = 33;
                }
            }
        } else if (gatehouse_left && !gatehouse_down) {
            int pos = get_gatehouse_position(grid_offset, DIR_6_LEFT, gatehouse_left);
            if (pos > 0) {
                if (pos == 1) {
                    image_offset = 31;
                } else if (pos == 2) {
                    image_offset = 33;
                } else {
                    image_offset = 29;
                }
            }
        }
    }
    if (image_offset) {
        map_image_set(grid_offset, image_group(GROUP_BUILDING_WALL) + image_offset);
    }
}

static void set_wall_image(int x, int y, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_WALL) ||
        map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        return;
    }
    const terrain_image *image = map_image_context_get_wall(grid_offset);
    map_image_set(grid_offset, image_group(GROUP_BUILDING_WALL) +
                  image->group_offset + image->item_offset);
    map_property_set_multi_tile_size(grid_offset, 1);
    map_property_mark_draw_tile(grid_offset);
    if (map_terrain_count_directly_adjacent_with_type(grid_offset, TERRAIN_GATEHOUSE) > 0) {
        image = map_image_context_get_wall_gatehouse(grid_offset);
        if (image->is_valid) {
            map_image_set(grid_offset, image_group(GROUP_BUILDING_WALL) +
                          image->group_offset + image->item_offset);
        } else {
            set_wall_gatehouse_image_manually(grid_offset);
        }
    }
}

void map_tiles_update_all_walls()
{
    foreach_map_tile(set_wall_image);
}

void map_tiles_update_area_walls(int x, int y, int size)
{
    foreach_region_tile(x - 1, y - 1, x + size - 2, y + size - 2, set_wall_image);
}

int map_tiles_set_wall(int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    int tile_set = 0;
    if (!map_terrain_is(grid_offset, TERRAIN_WALL)) {
        tile_set = 1;
    }
    map_terrain_set(grid_offset, TERRAIN_WALL);
    map_property_clear_constructing(grid_offset);

    foreach_region_tile(x - 1, y - 1, x + 1, y + 1, set_wall_image);
    return tile_set;
}

int map_tiles_is_paved_road(int grid_offset)
{
    int desirability = map_desirability_get(grid_offset);
    if (desirability > 4) {
        return 1;
    }
    if (desirability > 0 && map_terrain_is(grid_offset, TERRAIN_FOUNTAIN_RANGE)) {
        return 1;
    }
    return 0;
}

static void set_road_with_aqueduct_image(int grid_offset)
{
    int image_aqueduct = image_group(GROUP_BUILDING_AQUEDUCT);
    int water_offset;
    if (map_image_at(grid_offset) < image_aqueduct + 15) {
        water_offset = 0;
    } else {
        water_offset = 15;
    }
    const terrain_image *image = map_image_context_get_aqueduct(grid_offset, 0);
    int group_offset = image->group_offset;
    if (!image->aqueduct_offset) {
        if (map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_ROAD)) {
            group_offset = 3;
        } else {
            group_offset = 2;
        }
    }
    if (map_tiles_is_paved_road(grid_offset)) {
        map_image_set(grid_offset, image_aqueduct + water_offset + group_offset - 2);
    } else {
        map_image_set(grid_offset, image_aqueduct + water_offset + group_offset + 6);
    }
    map_property_set_multi_tile_size(grid_offset, 1);
    map_property_mark_draw_tile(grid_offset);
}

static void set_road_image(int x, int y, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_ROAD) || 
        map_terrain_is(grid_offset, TERRAIN_WATER | TERRAIN_BUILDING)) {
        return;
    }
    if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
        set_road_with_aqueduct_image(grid_offset);
        return;
    }
    if (map_property_is_plaza_or_earthquake(grid_offset)) {
        return;
    }
    if (map_tiles_is_paved_road(grid_offset)) {
        const terrain_image *image = map_image_context_get_paved_road(grid_offset);
        map_image_set(grid_offset, image_group(GROUP_TERRAIN_ROAD) +
                      image->group_offset + image->item_offset);
    } else {
        const terrain_image *image = map_image_context_get_dirt_road(grid_offset);
        map_image_set(grid_offset, image_group(GROUP_TERRAIN_ROAD) +
                      image->group_offset + image->item_offset + 49);
    }
    map_property_set_multi_tile_size(grid_offset, 1);
    map_property_mark_draw_tile(grid_offset);
}

void map_tiles_update_all_roads()
{
    foreach_map_tile(set_road_image);
}

void map_tiles_update_area_roads(int x, int y, int size)
{
    foreach_region_tile(x - 1, y - 1, x + size - 2, y + size - 2, set_road_image);
}

int map_tiles_set_road(int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    int tile_set = 0;
    if (!map_terrain_is(grid_offset, TERRAIN_ROAD)) {
        tile_set = 1;
    }
    map_terrain_add(grid_offset, TERRAIN_ROAD);
    map_property_clear_constructing(grid_offset);

    foreach_region_tile(x - 1, y - 1, x + 1, y + 1, set_road_image);
    return tile_set;
}
