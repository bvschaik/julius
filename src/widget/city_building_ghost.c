#include "city_building_ghost.h"

#include "building/construction.h"
#include "building/count.h"
#include "building/industry.h"
#include "building/properties.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/view.h"
#include "figure/formation.h"
#include "graphics/image.h"
#include "input/scroll.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/image_context.h"
#include "map/orientation.h"
#include "map/road_aqueduct.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/water.h"
#include "widget/city_bridge.h"

#define MAX_TILES 25

static const int X_VIEW_OFFSETS[MAX_TILES] = {
    0,
    -30, 30, 0,
    -60, 60, -30, 30, 0,
    -90, 90, -60, 60, -30, 30, 0,
    -120, 120, -90, 90, -60, 60, -30, 30, 0
};

static const int Y_VIEW_OFFSETS[MAX_TILES] = {
    0,
    15, 15, 30,
    30, 30, 45, 45, 60,
    45, 45, 60, 60, 75, 75, 90,
    60, 60, 75, 75, 90, 90, 105, 105, 120
};

static const int TILE_GRID_OFFSETS[4][MAX_TILES] = {
    {0,
    162, 1, 163,
    324, 2, 325, 164, 326,
    486, 3, 487, 165, 488, 327, 489,
    648, 4, 649, 166, 650, 328, 651, 490, 652},
    {0,
    -1, 162, 161,
    -2, 324, 160, 323, 322,
    -3, 486, 159, 485, 321, 484, 483,
    -4, 648, 158, 647, 320, 646, 482, 645, 644},
    {0,
    -162, -1, -163,
    -324, -2, -325, -164, -326,
    -486, -3, -487, -165, -488, -327, -489,
    -648, -4, -649, -166, -650, -328, -651, -490, -652},
    {0,
    1, -162, -161,
    2, -324, -160, -323, -322,
    3, -486, -159, -485, -321, -484, -483,
    4, -648, -158, -647, -320, -646, -482, -645, -644},
};

static const int FORT_GROUND_GRID_OFFSETS[4] = {-159, -158, 4, 3};
static const int FORT_GROUND_X_VIEW_OFFSETS[4] = {120, 90, -120, -90};
static const int FORT_GROUND_Y_VIEW_OFFSETS[4] = {30, -75, -60, 45};

static const int HIPPODROME_X_VIEW_OFFSETS[4] = {150, 150, -150, -150};
static const int HIPPODROME_Y_VIEW_OFFSETS[4] = {75, -75, -75, 75};

static void draw_flat_tile(int x, int y, color_t color_mask)
{
    image_draw_blend(image_group(GROUP_TERRAIN_FLAT_TILE), x, y, color_mask);
}

static int is_blocked_for_building(int grid_offset, int num_tiles, int *blocked_tiles)
{
    int orientation_index = city_view_orientation() / 2;
    int blocked = 0;
    for (int i = 0; i < num_tiles; i++) {
        int tile_offset = grid_offset + TILE_GRID_OFFSETS[orientation_index][i];
        int tile_blocked = 0;
        if (map_terrain_is(tile_offset, TERRAIN_NOT_CLEAR)) {
            tile_blocked = 1;
        }
        if (map_has_figure_at(tile_offset)) {
            tile_blocked = 1;
        }
        blocked_tiles[i] = tile_blocked;
        blocked += tile_blocked;
    }
    return blocked;
}

static void draw_partially_blocked(int x, int y, int fully_blocked, int num_tiles, int *blocked_tiles)
{
    for (int i = 0; i < num_tiles; i++) {
        int x_offset = x + X_VIEW_OFFSETS[i];
        int y_offset = y + Y_VIEW_OFFSETS[i];
        if (fully_blocked || blocked_tiles[i]) {
            draw_flat_tile(x_offset, y_offset, COLOR_MASK_RED);
        } else {
            draw_flat_tile(x_offset, y_offset, COLOR_MASK_GREEN);
        }
    }
}

static void draw_building(int image_id, int x, int y)
{
    image_draw_isometric_footprint(image_id, x, y, COLOR_MASK_GREEN);
    image_draw_isometric_top(image_id, x, y, COLOR_MASK_GREEN);
}

static void draw_regular_building(building_type type, int image_id, int x, int y)
{
    if (building_is_farm(type)) {
        draw_building(image_id, x, y);
        // fields
        for (int i = 4; i < 9; i++) {
            image_draw_isometric_footprint(image_id + 1, x + X_VIEW_OFFSETS[i], y + Y_VIEW_OFFSETS[i], COLOR_MASK_GREEN);
        }
    } else if (type == BUILDING_WAREHOUSE) {
        draw_building(image_id, x, y);
        image_draw_masked(image_group(GROUP_BUILDING_WAREHOUSE) + 17, x - 4, y - 42, COLOR_MASK_GREEN);
        int image_id_space = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY);
        for (int i = 1; i < 9; i++) {
            draw_building(image_id_space, x + X_VIEW_OFFSETS[i], y + Y_VIEW_OFFSETS[i]);
        }
    } else if (type == BUILDING_GRANARY) {
        image_draw_isometric_footprint(image_id, x, y, COLOR_MASK_GREEN);
        const image *img = image_get(image_id + 1);
        image_draw_masked(image_id + 1, x + img->sprite_offset_x - 32, y + img->sprite_offset_y - 64, COLOR_MASK_GREEN);
    } else if (type == BUILDING_HOUSE_VACANT_LOT) {
        draw_building(image_group(GROUP_BUILDING_HOUSE_VACANT_LOT), x, y);
    } else if (type == BUILDING_TRIUMPHAL_ARCH) {
        draw_building(image_id, x, y);
        const image *img = image_get(image_id + 1);
        if (image_id == image_group(GROUP_BUILDING_TRIUMPHAL_ARCH)) {
            image_draw_masked(image_id + 1, x + img->sprite_offset_x + 4, y + img->sprite_offset_y - 51, COLOR_MASK_GREEN);
        } else {
            image_draw_masked(image_id + 1, x + img->sprite_offset_x - 33, y + img->sprite_offset_y - 56, COLOR_MASK_GREEN);
        }
    } else if (type != BUILDING_CLEAR_LAND) {
        draw_building(image_id, x, y);
    }
}

static int get_building_image_id(int map_x, int map_y, building_type type, const building_properties *props)
{
    int image_id = image_group(props->image_group) + props->image_offset;
    if (type == BUILDING_GATEHOUSE) {
        int orientation = map_orientation_for_gatehouse(map_x, map_y);
        int image_offset;
        if (orientation == 2) {
            image_offset = 1;
        } else if (orientation == 1) {
            image_offset = 0;
        } else {
            image_offset = building_construction_road_orientation() == 2 ? 1 : 0;
        }
        int map_orientation = city_view_orientation();
        if (map_orientation == DIR_6_LEFT || map_orientation == DIR_2_RIGHT) {
            image_offset = 1 - image_offset;
        }
        image_id += image_offset;
    } else if (type == BUILDING_TRIUMPHAL_ARCH) {
        int orientation = map_orientation_for_triumphal_arch(map_x, map_y);
        int image_offset;
        if (orientation == 2) {
            image_offset = 2;
        } else if (orientation == 1) {
            image_offset = 0;
        } else {
            image_offset = building_construction_road_orientation() == 2 ? 2 : 0;
        }
        int map_orientation = city_view_orientation();
        if (map_orientation == DIR_6_LEFT || map_orientation == DIR_2_RIGHT) {
            image_offset = 2 - image_offset;
        }
        image_id += image_offset;
    }
    return image_id;
}

static void get_building_base_xy(int map_x, int map_y, int building_size, int *x, int *y)
{
    switch (city_view_orientation()) {
        case DIR_0_TOP:
            *x = map_x;
            *y = map_y;
            break;
        case DIR_2_RIGHT:
            *x = map_x - building_size + 1;
            *y = map_y;
            break;
        case DIR_4_BOTTOM:
            *x = map_x - building_size + 1;
            *y = map_y - building_size + 1;
            break;
        case DIR_6_LEFT:
            *x = map_x;
            *y = map_y - building_size + 1;
            break;
        default:
            *x = *y = 0;
    }
}

static int is_fully_blocked(int map_x, int map_y, building_type type, int building_size, int grid_offset)
{
    // determine x and y offset
    int x = 0, y = 0;
    get_building_base_xy(map_x, map_y, building_size, &x, &y);

    if (!building_construction_can_place_on_terrain(x, y, 0)) {
        return 1;
    }
    if (type == BUILDING_SENATE_UPGRADED && city_buildings_has_senate()) {
        return 1;
    }
    if (type == BUILDING_BARRACKS && building_count_total(BUILDING_BARRACKS)) {
        return 1;
    }
    if (type == BUILDING_PLAZA && !map_terrain_is(grid_offset, TERRAIN_ROAD)) {
        return 1;
    }
    if (city_finance_out_of_money()) {
        return 1;
    }
    return 0;
}

static void draw_default(const map_tile *tile, int x_view, int y_view, building_type type)
{
    // update road required based on timer
    building_construction_update_road_orientation();

    const building_properties *props = building_properties_for_type(type);
    int building_size = type == BUILDING_WAREHOUSE ? 3 : props->size;

    // check if we can place building
    int grid_offset = tile->grid_offset;
    int fully_blocked = is_fully_blocked(tile->x, tile->y, type, building_size, grid_offset);
    int blocked = fully_blocked;

    int num_tiles = building_size * building_size;
    int blocked_tiles[MAX_TILES];
    int orientation_index = city_view_orientation() / 2;
    for (int i = 0; i < num_tiles; i++) {
        int tile_offset = grid_offset + TILE_GRID_OFFSETS[orientation_index][i];
        int forbidden_terrain = map_terrain_get(tile_offset) & TERRAIN_NOT_CLEAR;
        if (type == BUILDING_GATEHOUSE || type == BUILDING_TRIUMPHAL_ARCH || type == BUILDING_PLAZA) {
            forbidden_terrain &= ~TERRAIN_ROAD;
        }
        if (type == BUILDING_TOWER) {
            forbidden_terrain &= ~TERRAIN_WALL;
        }
        if (forbidden_terrain || map_has_figure_at(tile_offset)) {
            blocked_tiles[i] = blocked = 1;
        } else {
            blocked_tiles[i] = 0;
        }
    }
    if (blocked) {
        draw_partially_blocked(x_view, y_view, fully_blocked, num_tiles, blocked_tiles);
    } else {
        int image_id = get_building_image_id(tile->x, tile->y, type, props);
        draw_regular_building(type, image_id, x_view, y_view);
    }
}

static void draw_draggable_reservoir(const map_tile *tile, int x, int y)
{
    int map_x = tile->x - 1;
    int map_y = tile->y - 1;
    int blocked = 0;
    if (building_construction_in_progress()) {
        if (!building_construction_cost()) {
            blocked = 1;
        }
    } else {
        if (map_building_is_reservoir(map_x, map_y)) {
            blocked = 0;
        } else if (!map_tiles_are_clear(map_x, map_y, 3, TERRAIN_ALL)) {
            blocked = 1;
        }
    }
    if (city_finance_out_of_money()) {
        blocked = 1;
    }
    if (building_construction_in_progress()) {
        int x_start, y_start;
        building_construction_get_view_position(&x_start, &y_start);
        y_start -= 30;
        if (blocked) {
            for (int i = 0; i < 9; i++) {
                draw_flat_tile(x_start + X_VIEW_OFFSETS[i], y_start + Y_VIEW_OFFSETS[i], COLOR_MASK_RED);
            }
        } else {
            draw_building(image_group(GROUP_BUILDING_RESERVOIR), x_start, y_start);
        }
    }
    // mouse pointer = center tile of reservoir instead of north, correct here:
    y -= 30;
    if (blocked) {
        for (int i = 0; i < 9; i++) {
            draw_flat_tile(x + X_VIEW_OFFSETS[i], y + Y_VIEW_OFFSETS[i], COLOR_MASK_RED);
        }
    } else {
        int image_id = image_group(GROUP_BUILDING_RESERVOIR);
        draw_building(image_id, x, y);
        if (map_terrain_exists_tile_in_area_with_type(map_x - 1, map_y - 1, 5, TERRAIN_WATER)) {
            const image *img = image_get(image_id);
            int x_water = x - 58 + img->sprite_offset_x - 2;
            int y_water = y + img->sprite_offset_y - (img->height - 90);
            image_draw_masked(image_id + 1, x_water, y_water, COLOR_MASK_GREEN);
        }
    }
}

static void draw_aqueduct(const map_tile *tile, int x, int y)
{
    int grid_offset = tile->grid_offset;
    int blocked = 0;
    if (building_construction_in_progress()) {
        if (!building_construction_cost()) {
            blocked = 1;
        }
    } else {
        if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
            blocked = map_get_adjacent_road_tiles_for_aqueduct(grid_offset) == 2 ? 0 : 1;
        } else if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
            blocked = 1;
        }
    }
    if (city_finance_out_of_money()) {
        blocked = 1;
    }
    if (blocked) {
        draw_flat_tile(x, y, COLOR_MASK_RED);
    } else {
        int image_id = image_group(GROUP_BUILDING_AQUEDUCT);
        const terrain_image *image = map_image_context_get_aqueduct(grid_offset, 0);
        if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
            int group_offset = image->group_offset;
            if (!image->aqueduct_offset) {
                if (map_terrain_is(grid_offset - 162, TERRAIN_ROAD)) {
                    group_offset = 3;
                } else {
                    group_offset = 2;
                }
            }
            if (map_tiles_is_paved_road(grid_offset)) {
                image_id += group_offset + 13;
            } else {
                image_id += group_offset + 21;
            }
        } else {
            image_id += image->group_offset + 15;
        }
        draw_building(image_id, x, y);
    }
}

static void draw_fountain(const map_tile *tile, int x, int y)
{
    if (city_finance_out_of_money()) {
        draw_flat_tile(x, y, COLOR_MASK_RED);
    } else {
        int image_id = image_group(building_properties_for_type(BUILDING_FOUNTAIN)->image_group);
        draw_building(image_id, x, y);
        if (map_terrain_is(tile->grid_offset, TERRAIN_RESERVOIR_RANGE)) {
            const image *img = image_get(image_id);
            image_draw_masked(image_id + 1, x + img->sprite_offset_x, y + img->sprite_offset_y, COLOR_MASK_GREEN);
        }
    }
}

static void draw_bathhouse(const map_tile *tile, int x, int y)
{
    int grid_offset = tile->grid_offset;
    int num_tiles = 4;
    int blocked_tiles[4];
    int blocked = is_blocked_for_building(grid_offset, num_tiles, blocked_tiles);
    int fully_blocked = 0;
    if (city_finance_out_of_money()) {
        fully_blocked = 1;
        blocked = 1;
    }

    if (blocked) {
        draw_partially_blocked(x, y, fully_blocked, num_tiles, blocked_tiles);
    } else {
        int image_id = image_group(building_properties_for_type(BUILDING_BATHHOUSE)->image_group);
        int has_water = 0;
        int orientation_index = city_view_orientation() / 2;
        for (int i = 0; i < num_tiles; i++) {
            int tile_offset = grid_offset + TILE_GRID_OFFSETS[orientation_index][i];
            if (map_terrain_is(tile_offset, TERRAIN_RESERVOIR_RANGE)) {
                has_water = 1;
            }
        }
        draw_building(image_id, x, y);
        if (has_water) {
            const image *img = image_get(image_id);
            image_draw_masked(image_id - 1, x + img->sprite_offset_x, y + img->sprite_offset_y, COLOR_MASK_GREEN);
        }
    }
}

static void draw_bridge(const map_tile *tile, int x, int y, building_type type)
{
    int length, direction;
    int end_grid_offset = map_bridge_calculate_length_direction(tile->x, tile->y, &length, &direction);

    int dir = direction - city_view_orientation();
    if (dir < 0) {
        dir += 8;
    }
    int blocked = 0;
    if (type == BUILDING_SHIP_BRIDGE && length < 5) {
        blocked = 1;
    } else if (!end_grid_offset) {
        blocked = 1;
    }
    if (city_finance_out_of_money()) {
        blocked = 1;
    }
    int x_delta, y_delta;
    switch (dir) {
        case DIR_0_TOP: 
            x_delta = 29;
            y_delta = -15;
            break;
        case DIR_2_RIGHT: 
            x_delta = 29;
            y_delta = 15;
            break;
        case DIR_4_BOTTOM: 
            x_delta = -29;
            y_delta = 15;
            break;
        case DIR_6_LEFT: 
            x_delta = -29;
            y_delta = -15;
            break;
        default:
            return;
    }
    if (blocked) {
        draw_flat_tile(x, y, length > 0 ? COLOR_MASK_GREEN : COLOR_MASK_RED);
        if (length > 1) {
            draw_flat_tile(x + x_delta * (length - 1), y + y_delta * (length - 1), COLOR_MASK_RED);
        }
    } else {
        if (dir == DIR_0_TOP || dir == DIR_6_LEFT) {
            for (int i = length - 1; i >= 0; i--) {
                int sprite_id = map_bridge_get_sprite_id(i, length, dir, type == BUILDING_SHIP_BRIDGE);
                city_draw_bridge_tile(x + x_delta * i, y + y_delta * i, sprite_id, COLOR_MASK_GREEN);
            }
        } else {
            for (int i = 0; i < length; i++) {
                int sprite_id = map_bridge_get_sprite_id(i, length, dir, type == BUILDING_SHIP_BRIDGE);
                city_draw_bridge_tile(x + x_delta * i, y + y_delta * i, sprite_id, COLOR_MASK_GREEN);
            }
        }
    }
}

static void draw_fort(const map_tile *tile, int x, int y)
{
    int fully_blocked = 0;
    int blocked = 0;
    if (formation_get_num_legions_cached() >= MAX_LEGIONS || city_finance_out_of_money()) {
        fully_blocked = 1;
        blocked = 1;
    }

    int num_tiles_fort = building_properties_for_type(BUILDING_FORT)->size;
    num_tiles_fort *= num_tiles_fort;
    int num_tiles_ground = building_properties_for_type(BUILDING_FORT_GROUND)->size;
    num_tiles_ground *= num_tiles_ground;

    int orientation_index = city_view_orientation() / 2;
    int grid_offset_fort = tile->grid_offset;
    int grid_offset_ground = grid_offset_fort + FORT_GROUND_GRID_OFFSETS[orientation_index];
    int blocked_tiles_fort[MAX_TILES];
    int blocked_tiles_ground[MAX_TILES];
    
    blocked += is_blocked_for_building(grid_offset_fort, num_tiles_fort, blocked_tiles_fort);
    blocked += is_blocked_for_building(grid_offset_ground, num_tiles_ground, blocked_tiles_ground);

    int x_ground = x + FORT_GROUND_X_VIEW_OFFSETS[orientation_index];
    int y_ground = y + FORT_GROUND_Y_VIEW_OFFSETS[orientation_index];

    if (blocked) {
        draw_partially_blocked(x, y, fully_blocked, num_tiles_fort, blocked_tiles_fort);
        draw_partially_blocked(x_ground, y_ground, fully_blocked, num_tiles_ground, blocked_tiles_ground);
    } else {
        int image_id = image_group(GROUP_BUILDING_FORT);
        if (orientation_index == 0 || orientation_index == 3) {
            // draw fort first, then ground
            draw_building(image_id, x, y);
            draw_building(image_id + 1, x_ground, y_ground);
        } else {
            // draw ground first, then fort
            draw_building(image_id + 1, x_ground, y_ground);
            draw_building(image_id, x, y);
        }
    }
}

static void draw_hippodrome(const map_tile *tile, int x, int y)
{
    int fully_blocked = 0;
    int blocked = 0;
    if (city_buildings_has_hippodrome() || city_finance_out_of_money()) {
        fully_blocked = 1;
        blocked = 1;
    }
    int num_tiles = 25;
    int orientation_index = city_view_orientation() / 2;
    int grid_offset1 = tile->grid_offset;
    int grid_offset2 = grid_offset1 + map_grid_delta(5, 0);
    int grid_offset3 = grid_offset1 + map_grid_delta(10, 0);
    
    int blocked_tiles1[25];
    int blocked_tiles2[25];
    int blocked_tiles3[25];
    blocked += is_blocked_for_building(grid_offset1, num_tiles, blocked_tiles1);
    blocked += is_blocked_for_building(grid_offset2, num_tiles, blocked_tiles2);
    blocked += is_blocked_for_building(grid_offset3, num_tiles, blocked_tiles3);

    int x_part1 = x;
    int y_part1 = y;
    int x_part2 = x_part1 + HIPPODROME_X_VIEW_OFFSETS[orientation_index];
    int y_part2 = y_part1 + HIPPODROME_Y_VIEW_OFFSETS[orientation_index];
    int x_part3 = x_part2 + HIPPODROME_X_VIEW_OFFSETS[orientation_index];
    int y_part3 = y_part2 + HIPPODROME_Y_VIEW_OFFSETS[orientation_index];
    if (blocked) {
        draw_partially_blocked(x_part1, y_part1, fully_blocked, num_tiles, blocked_tiles1);
        draw_partially_blocked(x_part2, y_part2, fully_blocked, num_tiles, blocked_tiles2);
        draw_partially_blocked(x_part3, y_part3, fully_blocked, num_tiles, blocked_tiles3);
    } else {
        if (orientation_index == 0) {
            int image_id = image_group(GROUP_BUILDING_HIPPODROME_2);
            // part 1, 2, 3
            draw_building(image_id, x_part1, y_part1);
            draw_building(image_id + 2, x_part2, y_part2);
            draw_building(image_id + 4, x_part3, y_part3);
        } else if (orientation_index == 1) {
            int image_id = image_group(GROUP_BUILDING_HIPPODROME_1);
            // part 3, 2, 1
            draw_building(image_id, x_part3, y_part3);
            draw_building(image_id + 2, x_part2, y_part2);
            draw_building(image_id + 4, x_part1, y_part1);
        } else if (orientation_index == 2) {
            int image_id = image_group(GROUP_BUILDING_HIPPODROME_2);
            // part 1, 2, 3
            draw_building(image_id + 4, x_part1, y_part1);
            draw_building(image_id + 2, x_part2, y_part2);
            draw_building(image_id, x_part3, y_part3);
        } else if (orientation_index == 3) {
            int image_id = image_group(GROUP_BUILDING_HIPPODROME_1);
            // part 3, 2, 1
            draw_building(image_id + 4, x_part3, y_part3);
            draw_building(image_id + 2, x_part2, y_part2);
            draw_building(image_id, x_part1, y_part1);
        }
    }
}

static void draw_shipyard_wharf(const map_tile *tile, int x, int y, building_type type)
{
    int dir_absolute, dir_relative;
    int blocked = map_water_determine_orientation_size2(tile->x, tile->y, 1, &dir_absolute, &dir_relative);
    if (city_finance_out_of_money()) {
        blocked = 999;
    }
    if (blocked) {
        for (int i = 0; i < 4; i++) {
            draw_flat_tile(x + X_VIEW_OFFSETS[i], y + Y_VIEW_OFFSETS[i], COLOR_MASK_RED);
        }
    } else {
        const building_properties *props = building_properties_for_type(type);
        int image_id = image_group(props->image_group) + props->image_offset + dir_relative;
        draw_building(image_id, x, y);
    }
}

static void draw_dock(const map_tile *tile, int x, int y)
{
    int dir_absolute, dir_relative;
    int blocked = map_water_determine_orientation_size3(tile->x, tile->y, 1, &dir_absolute, &dir_relative);
    if (city_finance_out_of_money()) {
        blocked = 1;
    }
    if (blocked) {
        for (int i = 0; i < 9; i++) {
            draw_flat_tile(x + X_VIEW_OFFSETS[i], y + Y_VIEW_OFFSETS[i], COLOR_MASK_RED);
        }
    } else {
        int image_id;
        switch (dir_relative) {
            case 0: image_id = image_group(GROUP_BUILDING_DOCK_1); break;
            case 1: image_id = image_group(GROUP_BUILDING_DOCK_2); break;
            case 2: image_id = image_group(GROUP_BUILDING_DOCK_3); break;
            default:image_id = image_group(GROUP_BUILDING_DOCK_4); break;
        }
        draw_building(image_id, x, y);
    }
}

static void draw_road(const map_tile *tile, int x, int y)
{
    int grid_offset = tile->grid_offset;
    int blocked = 0;
    int image_id = 0;
    if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
        image_id = image_group(GROUP_BUILDING_AQUEDUCT);
        if (map_can_place_road_under_aqueduct(grid_offset)) {
            image_id += map_get_aqueduct_with_road_image(grid_offset);
        } else {
            blocked = 1;
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
        blocked = 1;
    } else {
        image_id = image_group(GROUP_TERRAIN_ROAD);
        if (!map_terrain_has_adjacent_x_with_type(grid_offset, TERRAIN_ROAD) &&
            map_terrain_has_adjacent_y_with_type(grid_offset, TERRAIN_ROAD)) {
            image_id++;
        }
    }
    if (city_finance_out_of_money()) {
        blocked = 1;
    }
    if (blocked) {
        draw_flat_tile(x, y, COLOR_MASK_RED);
    } else {
        draw_building(image_id, x, y);
    }
}

void city_building_ghost_draw(const map_tile *tile)
{
    if (!tile->grid_offset || scroll_in_progress()) {
        return;
    }
    building_type type = building_construction_type();
    if (building_construction_draw_as_constructing() || type == BUILDING_NONE) {
        return;
    }
    int x, y;
    city_view_get_selected_tile_pixels(&x, &y);
    switch (type) {
        case BUILDING_DRAGGABLE_RESERVOIR:
            draw_draggable_reservoir(tile, x, y);
            break;
        case BUILDING_AQUEDUCT:
            draw_aqueduct(tile, x, y);
            break;
        case BUILDING_FOUNTAIN:
            draw_fountain(tile, x, y);
            break;
        case BUILDING_BATHHOUSE:
            draw_bathhouse(tile, x, y);
            break;
        case BUILDING_LOW_BRIDGE:
        case BUILDING_SHIP_BRIDGE:
            draw_bridge(tile, x, y, type);
            break;
        case BUILDING_FORT_LEGIONARIES:
        case BUILDING_FORT_JAVELIN:
        case BUILDING_FORT_MOUNTED:
            draw_fort(tile, x, y);
            break;
        case BUILDING_HIPPODROME:
            draw_hippodrome(tile, x, y);
            break;
        case BUILDING_SHIPYARD:
        case BUILDING_WHARF:
            draw_shipyard_wharf(tile, x, y, type);
            break;
        case BUILDING_DOCK:
            draw_dock(tile, x, y);
            break;
        case BUILDING_ROAD:
            draw_road(tile, x, y);
            break;
        default:
            draw_default(tile, x, y, type);
            break;
    }
}
