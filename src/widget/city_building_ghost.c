#include "city_building_ghost.h"

#include "assets/assets.h"
#include "building/building_variant.h"
#include "building/connectable.h"
#include "building/construction.h"
#include "building/count.h"
#include "building/image.h"
#include "building/industry.h"
#include "building/market.h"
#include "building/monument.h"
#include "building/model.h"
#include "building/properties.h"
#include "building/rotation.h"
#include "building/type.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/view.h"
#include "core/config.h"
#include "core/config.h"
#include "core/log.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "figure/roamer_preview.h"
#include "figuretype/animal.h"
#include "game/state.h"
#include "graphics/image.h"
#include "graphics/renderer.h"
#include "input/scroll.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/image_context.h"
#include "map/orientation.h"
#include "map/property.h"
#include "map/road_aqueduct.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/water.h"
#include "map/water_supply.h"
#include "scenario/property.h"
#include "widget/city.h"
#include "widget/city_bridge.h"
#include "widget/city_water_ghost.h"

#include <stdlib.h>

#define RESERVOIR_RANGE_MAX_TILES 900

// Note: If we ever end up creating larger buildings than 7 * 7, we should update this
#define MAX_TILES (7 * 7)

#define GRID_OFFSET(x, y) ((x) + GRID_SIZE * (y))
#define X_VIEW_OFFSET(x, y) (((x) - (y)) * 30)
#define Y_VIEW_OFFSET(x, y) (((x) + (y)) * 15)

typedef struct {
    int x;
    int y;
} tile_offset;

enum farm_ghost_object {
    FARM_GHOST_NO_DRAW,
    FARM_GHOST_FARMHOUSE,
    FARM_GHOST_CROP
};

static const int FORT_GROUND_GRID_OFFSETS[4][4] = {
    { GRID_OFFSET(3, -1),  GRID_OFFSET(4, -1), GRID_OFFSET(4, 0),  GRID_OFFSET(3, 0)   },
    { GRID_OFFSET(-1, -4), GRID_OFFSET(0, -4), GRID_OFFSET(0, -3), GRID_OFFSET(-1, -3) },
    { GRID_OFFSET(-4, 0),  GRID_OFFSET(-3, 0), GRID_OFFSET(-3, 1), GRID_OFFSET(-4, 1)  },
    { GRID_OFFSET(0, 3),   GRID_OFFSET(1, 3),  GRID_OFFSET(1, 4),  GRID_OFFSET(0, 4)   }
};
static const int FORT_GROUND_X_VIEW_OFFSETS[4] = { 120, 90, -120, -90 };
static const int FORT_GROUND_Y_VIEW_OFFSETS[4] = { 30, -75, -60, 45 };

static const int RESERVOIR_GRID_OFFSETS[4] = {
    GRID_OFFSET(-1, -1), GRID_OFFSET(1, -1), GRID_OFFSET(1, 1), GRID_OFFSET(-1, 1)
};

static const int HIPPODROME_X_VIEW_OFFSETS[4] = { 150, 150, -150, -150 };
static const int HIPPODROME_Y_VIEW_OFFSETS[4] = { 75, -75, -75, 75 };

static const int FARM_TILES[4][9] = {
    {
        FARM_GHOST_FARMHOUSE, FARM_GHOST_NO_DRAW, FARM_GHOST_NO_DRAW,
        FARM_GHOST_NO_DRAW, FARM_GHOST_CROP, FARM_GHOST_CROP,
        FARM_GHOST_CROP, FARM_GHOST_CROP, FARM_GHOST_CROP
    },
    {
        FARM_GHOST_CROP, FARM_GHOST_FARMHOUSE, FARM_GHOST_CROP,
        FARM_GHOST_NO_DRAW, FARM_GHOST_NO_DRAW, FARM_GHOST_CROP,
        FARM_GHOST_NO_DRAW, FARM_GHOST_CROP, FARM_GHOST_CROP
    },
    {
        FARM_GHOST_CROP, FARM_GHOST_CROP, FARM_GHOST_CROP,
        FARM_GHOST_FARMHOUSE, FARM_GHOST_CROP, FARM_GHOST_CROP,
        FARM_GHOST_NO_DRAW, FARM_GHOST_NO_DRAW, FARM_GHOST_NO_DRAW
    },
    {
        FARM_GHOST_CROP, FARM_GHOST_CROP, FARM_GHOST_FARMHOUSE,
        FARM_GHOST_NO_DRAW, FARM_GHOST_CROP, FARM_GHOST_NO_DRAW,
        FARM_GHOST_CROP, FARM_GHOST_NO_DRAW, FARM_GHOST_CROP
    },
};

static struct {
    struct {
        int total;
        int save_offsets;
        int offsets[RESERVOIR_RANGE_MAX_TILES];
        int last_grid_offset;
        int blocked;
    } reservoir_range;
    building ghost_building;
    float scale;
    struct {
        int grid_offset;
        building_type type;
    } roamer_preview;
    tile_offset offsets[4][MAX_TILES];
} data = {
    .scale = SCALE_NONE
};

static inline int view_offset_x(int index)
{
    return X_VIEW_OFFSET(data.offsets[0][index].x, data.offsets[0][index].y);
}

static inline int view_offset_y(int index)
{
    return Y_VIEW_OFFSET(data.offsets[0][index].x, data.offsets[0][index].y);
}

static inline int tile_grid_offset(int orientation, int index)
{
    return GRID_OFFSET(data.offsets[orientation][index].x, data.offsets[orientation][index].y);
}

static int is_blocked_for_building(int grid_offset, int building_size, int *blocked_tiles)
{
    int orientation_index = city_view_orientation() / 2;
    int blocked = 0;
    int num_tiles = building_size * building_size;
    for (int i = 0; i < num_tiles; i++) {
        int tile_offset = grid_offset + tile_grid_offset(orientation_index, i);
        int tile_blocked = 0;
        if (map_terrain_is(tile_offset, TERRAIN_NOT_CLEAR)) {
            tile_blocked = 1;
        }
        if (map_has_figure_at(tile_offset)) {
            tile_blocked = 1;
            figure_animal_try_nudge_at(grid_offset, tile_offset, building_size);
        }
        blocked_tiles[i] = tile_blocked;
        blocked += tile_blocked;
    }
    return blocked;
}

static int has_blocked_tiles(int num_tiles, int *blocked_tiles)
{
    for (int i = 0; i < num_tiles; i++) {
        if (blocked_tiles[i]) {
            return 1;
        }
    }
    return 0;
}

static void draw_building_tiles(int x, int y, int num_tiles, int *blocked_tiles)
{
    for (int i = 0; i < num_tiles; i++) {
        int x_offset = x + view_offset_x(i);
        int y_offset = y + view_offset_y(i);
        if (blocked_tiles[i]) {
            image_blend_footprint_color(x_offset, y_offset, COLOR_MASK_RED, data.scale);
        } else {
            image_draw_isometric_footprint(image_group(GROUP_TERRAIN_FLAT_TILE),
                x_offset, y_offset, COLOR_MASK_FOOTPRINT_GHOST, data.scale);
        }
    }
}

static void draw_building(int image_id, int x, int y, color_t color)
{
    image_draw_isometric_footprint(image_id, x, y, color, data.scale);
    image_draw_isometric_top(image_id, x, y, color, data.scale);
}

void city_building_ghost_draw_well_range(int x, int y, int grid_offset)
{
    image_draw(image_group(GROUP_TERRAIN_FLAT_TILE), x, y, COLOR_MASK_DARK_BLUE, data.scale);
}

void city_building_ghost_draw_fountain_range(int x, int y, int grid_offset)
{
    image_draw(image_group(GROUP_TERRAIN_FLAT_TILE), x, y, COLOR_MASK_BLUE, data.scale);
}

static void image_draw_warehouse(int image_id, int x, int y, color_t color)
{
    int image_id_space = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY);
    int building_orientation = building_rotation_get_building_orientation(building_rotation_get_rotation());
    int corner = building_rotation_get_corner(building_orientation);
    for (int i = 0; i < 9; i++) {
        int x_offset = view_offset_x(i);
        int y_offset = view_offset_y(i);
        if (i == corner) {
            draw_building(image_id, x + x_offset, y + y_offset, color);
            image_draw(image_group(GROUP_BUILDING_WAREHOUSE) + 17,
                x + x_offset - 4, y + y_offset - 42, color, data.scale);
        } else {
            draw_building(image_id_space, x + x_offset, y + y_offset, color);
        }
    }
}

static void image_draw_farm(building_type type, int image_id, int x, int y, color_t color)
{
    // Custom draw order to properly draw isometric tops
    const int draw_order[9] = { 0, 2, 5, 1, 3, 7, 4, 6, 8 };
    int orientation_index = city_view_orientation() / 2;
    int crop_image = building_image_get_base_farm_crop(type);
    for (int i = 0; i < 9; i++) {
        int j = draw_order[i];
        int x_offset = view_offset_x(j);
        int y_offset = view_offset_y(j);
        switch (FARM_TILES[orientation_index][j]) {
            case FARM_GHOST_CROP:
                draw_building(crop_image, x + x_offset, y + y_offset, color);
                break;
            case FARM_GHOST_FARMHOUSE:
                draw_building(image_id, x + x_offset, y + y_offset, color);
                break;
            default:
                break;
        }
    }
}

static void draw_regular_building(building_type type, int image_id, int x, int y, int grid_offset,
    int num_tiles, int *blocked_tiles)
{
    color_t color = has_blocked_tiles(num_tiles, blocked_tiles) ?
        COLOR_MASK_BUILDING_GHOST_RED : COLOR_MASK_BUILDING_GHOST;
    if (building_is_farm(type)) {
        image_draw_farm(type, image_id, x, y, color);
    } else if (type == BUILDING_WAREHOUSE) {
        image_draw_warehouse(image_id, x, y, color);
    } else if (type == BUILDING_GRANARY) {
        image_draw_isometric_footprint(image_id, x, y, color, data.scale);
        image_draw(image_id + 1, x - 32, y - 64, color, data.scale);
    } else if (type == BUILDING_HOUSE_VACANT_LOT) {
        draw_building(image_group(GROUP_BUILDING_HOUSE_VACANT_LOT), x, y, color);
    } else if (type == BUILDING_TRIUMPHAL_ARCH) {
        draw_building(image_id, x, y, color);
        if (image_id == image_group(GROUP_BUILDING_TRIUMPHAL_ARCH)) {
            image_draw(image_id + 1, x + 4, y - 51, color, data.scale);
        } else {
            image_draw(image_id + 1, x - 33, y - 56, color, data.scale);
        }
    } else if (type != BUILDING_CLEAR_LAND) {
        draw_building(image_id, x, y, color);
    }
    draw_building_tiles(x, y, num_tiles, blocked_tiles);
}

static int get_building_image_id(int map_x, int map_y, building_type type, const building_properties *props)
{
    int image_id;
    image_id = image_group(props->image_group) + props->image_offset;

    if (type == BUILDING_GATEHOUSE) {
        int orientation = map_orientation_for_gatehouse(map_x, map_y);
        int image_offset;
        if (orientation == 2) {
            image_offset = 1;
        } else if (orientation == 1) {
            image_offset = 0;
        } else {
            image_offset = building_rotation_get_road_orientation() == 2 ? 1 : 0;
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
            image_offset = building_rotation_get_road_orientation() == 2 ? 2 : 0;
        }
        int map_orientation = city_view_orientation();
        if (map_orientation == DIR_6_LEFT || map_orientation == DIR_2_RIGHT) {
            image_offset = 2 - image_offset;
        }
        image_id += image_offset;
    }
    return image_id;
}

static int get_new_building_image_id(int map_x, int map_y, int grid_offset,
    building_type type, const building_properties *props)
{
    data.ghost_building.type = type;
    data.ghost_building.grid_offset = grid_offset;
    if (building_variant_has_variants(type)) {
        data.ghost_building.variant = building_rotation_get_rotation_with_limit(
            building_variant_get_number_of_variants(type));
    } else {
        data.ghost_building.variant = 0;
    }
    if (building_properties_for_type(type)->rotation_offset) {
        data.ghost_building.subtype.orientation = building_rotation_get_rotation();
    } else {
        data.ghost_building.subtype.orientation = 0;
    }
    return building_image_get(&data.ghost_building);
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
    if (type == BUILDING_CITY_MINT && (!city_buildings_has_senate() || city_buildings_has_city_mint())) {
        return 1;
    }
    if (type == BUILDING_CARAVANSERAI && city_buildings_has_caravanserai()) {
        return 1;
    }
    if (type == BUILDING_BARRACKS && city_buildings_has_barracks() && !config_get(CONFIG_GP_CH_MULTIPLE_BARRACKS)) {
        return 1;
    }
    if (type == BUILDING_MESS_HALL && city_buildings_has_mess_hall()) {
        return 1;
    }
    if (type == BUILDING_PLAZA && !map_terrain_is(grid_offset, TERRAIN_ROAD)) {
        return 1;
    }
    if (type == BUILDING_ROADBLOCK && !map_terrain_is(grid_offset, TERRAIN_ROAD)) {
        return 1;
    }
    if (!building_monument_type_is_mini_monument(type) && building_monument_get_id(type)) {
        return 1;
    }
    if (building_monument_is_grand_temple(type) &&
        building_monument_count_grand_temples() >= config_get(CONFIG_GP_CH_MAX_GRAND_TEMPLES)) {
        return 1;
    }
    if (city_finance_out_of_money()) {
        return 1;
    }
    return 0;
}

static void set_roamer_path(building_type type, int size, const map_tile *tile, int is_blocked)
{
    if (data.roamer_preview.grid_offset == tile->grid_offset && data.roamer_preview.type == type) {
        return;
    }
    figure_roamer_preview_reset(type);
    data.roamer_preview.type = type;
    data.roamer_preview.grid_offset = tile->grid_offset;
    int grid_x = tile->x;
    int grid_y = tile->y;
    building_construction_offset_start_from_orientation(&grid_x, &grid_y, size);
    
    if (!is_blocked) {
        figure_roamer_preview_create(type, grid_x, grid_y);
    } else {
        int building_id = map_building_at(tile->grid_offset);
        if (!building_id) {
            return;
        }
        building *b = building_main(building_get(building_id));
        if (b->type == type && b->x == grid_x && b->y == grid_y) {
            figure_roamer_preview_create(type, grid_x, grid_y);
        }
    }
}

static void draw_default(const map_tile *tile, int x_view, int y_view, building_type type)
{
    const building_properties *props = building_properties_for_type(type);
    int building_size = type == BUILDING_WAREHOUSE ? 3 : props->size;
    int image_id = 0;

    // check if we can place building
    int grid_offset = tile->grid_offset;
    int fully_blocked = is_fully_blocked(tile->x, tile->y, type, building_size, grid_offset);

    int num_tiles = building_size * building_size;
    int blocked_tiles[MAX_TILES];
    int orientation_index = city_view_orientation() / 2;

    if (building_connectable_gate_type(type) && map_terrain_get(grid_offset) & TERRAIN_ROAD) {
        type = building_connectable_gate_type(type);
    }

    for (int i = 0; i < num_tiles; i++) {
        int tile_offset = grid_offset + tile_grid_offset(orientation_index, i);
        int forbidden_terrain = map_terrain_get(tile_offset) & TERRAIN_NOT_CLEAR;
        if (!fully_blocked) {
            if (type == BUILDING_GATEHOUSE || type == BUILDING_TRIUMPHAL_ARCH ||
                type == BUILDING_PLAZA || building_type_is_roadblock(type)) {
                forbidden_terrain &= ~TERRAIN_ROAD;
            }
            if (type == BUILDING_GATEHOUSE) {
                forbidden_terrain &= ~TERRAIN_HIGHWAY;
            }
            if (type == BUILDING_TOWER) {
                forbidden_terrain &= ~TERRAIN_WALL;
            }
        }

        if (fully_blocked || forbidden_terrain) {
            blocked_tiles[i] = 1;
        } else if (map_has_figure_at(tile_offset) && type != BUILDING_PLAZA) {
            blocked_tiles[i] = 1;
            figure_animal_try_nudge_at(grid_offset, tile_offset, building_size);
        } else {
            blocked_tiles[i] = 0;
        }
    }
    if (type >= BUILDING_ROADBLOCK || type == BUILDING_LIBRARY || type == BUILDING_SMALL_STATUE || type == BUILDING_MEDIUM_STATUE) {
        image_id = get_new_building_image_id(tile->x, tile->y, grid_offset, type, props);
        draw_regular_building(type, image_id, x_view, y_view, grid_offset, num_tiles, blocked_tiles);
    } else {
        image_id = get_building_image_id(tile->x, tile->y, type, props);
        draw_regular_building(type, image_id, x_view, y_view, grid_offset, num_tiles, blocked_tiles);
    }
    set_roamer_path(type, building_size, tile, has_blocked_tiles(num_tiles, blocked_tiles));
}

static void draw_single_reservoir(int grid_offset, int x, int y, color_t color, int has_water, int draw_blocked)
{
    int image_id = image_group(GROUP_BUILDING_RESERVOIR);
    draw_building(image_id, x, y, color);
    if (has_water) {
        const image *img = image_get(image_id);
        if (img->animation) {
            int x_water = x - FOOTPRINT_WIDTH + img->animation->sprite_offset_x - 2;
            int top_height = img->top ? img->top->original.height : 0;
            int y_water = y + img->animation->sprite_offset_y - top_height + FOOTPRINT_HALF_HEIGHT * 3;
            image_draw(image_id + 1, x_water, y_water, color, data.scale);
        }
    }
    if (data.reservoir_range.blocked && draw_blocked) {
        for (int i = 0; i < 9; i++) {
            image_blend_footprint_color(x + view_offset_x(i), y + view_offset_y(i), COLOR_MASK_RED, data.scale);
        }
    }
    if (grid_offset)
    {
        int num_tiles = 9;
        int orientation_index = city_view_orientation() / 2;

        grid_offset += GRID_OFFSET(-1, -1);

        for (int i = 0; i < num_tiles; i++) {
            int tile_offset = grid_offset + tile_grid_offset(orientation_index, i);

            if (map_has_figure_at(tile_offset)) {
                figure_animal_try_nudge_at(grid_offset, tile_offset, 3);
            } 
        }
    }
}

static void draw_first_reservoir_range(int x, int y, int grid_offset)
{
    if (data.reservoir_range.save_offsets) {
        data.reservoir_range.offsets[data.reservoir_range.total] = grid_offset;
        data.reservoir_range.total++;
    }
    int color_mask = data.reservoir_range.blocked ? COLOR_MASK_GREY : COLOR_MASK_BLUE;
    image_draw(image_group(GROUP_TERRAIN_FLAT_TILE), x, y, color_mask, data.scale);
}

static void draw_second_reservoir_range(int x, int y, int grid_offset)
{
    for (int i = 0; i < data.reservoir_range.total; ++i) {
        if (data.reservoir_range.offsets[i] == grid_offset) {
            return;
        }
    }
    int color_mask = data.reservoir_range.blocked ? COLOR_MASK_GREY : COLOR_MASK_BLUE;
    image_draw(image_group(GROUP_TERRAIN_FLAT_TILE), x, y, color_mask, data.scale);
}

static void draw_draggable_reservoir(const map_tile *tile, int x, int y)
{
    int map_x = tile->x - 1;
    int map_y = tile->y - 1;
    int blocked = 0;
    int blocked_tiles[9];
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
    data.reservoir_range.blocked = blocked;
    color_t color = blocked ? COLOR_MASK_BUILDING_GHOST_RED : COLOR_MASK_BUILDING_GHOST;
    int draw_later = 0;
    int x_start, y_start, offset;
    int has_water = map_terrain_exists_tile_in_area_with_type(map_x - 1, map_y - 1, 5, TERRAIN_WATER);
    int orientation_index = city_view_orientation() / 2;
    int drawing_two_reservoirs = building_construction_in_progress();
    if (drawing_two_reservoirs) {
        building_construction_get_view_position(&x_start, &y_start);
        y_start -= 30;
        offset = building_construction_get_start_grid_offset();
        if (offset != data.reservoir_range.last_grid_offset) {
            data.reservoir_range.last_grid_offset = offset;
            data.reservoir_range.total = 0;
            data.reservoir_range.save_offsets = 1;
        } else {
            data.reservoir_range.save_offsets = 0;
        }
        if (offset == tile->grid_offset) {
            drawing_two_reservoirs = 0;
        } else {
            int map_x_start = map_grid_offset_to_x(offset) - 1;
            int map_y_start = map_grid_offset_to_y(offset) - 1;
            if (!has_water) {
                has_water = map_terrain_exists_tile_in_area_with_type(
                    map_x_start - 1, map_y_start - 1, 5, TERRAIN_WATER);
            }
            switch (city_view_orientation()) {
                case DIR_0_TOP:
                    draw_later = map_x_start > map_x || map_y_start > map_y;
                    break;
                case DIR_2_RIGHT:
                    draw_later = map_x_start < map_x || map_y_start > map_y;
                    break;
                case DIR_4_BOTTOM:
                    draw_later = map_x_start < map_x || map_y_start < map_y;
                    break;
                case DIR_6_LEFT:
                    draw_later = map_x_start > map_x || map_y_start < map_y;
                    break;
            }
            if (!draw_later) {
                if (config_get(CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE)) {
                    city_view_foreach_tile_in_range(offset + RESERVOIR_GRID_OFFSETS[orientation_index], 3,
                        map_water_supply_reservoir_radius(), draw_first_reservoir_range);
                    city_view_foreach_tile_in_range(tile->grid_offset + RESERVOIR_GRID_OFFSETS[orientation_index], 3,
                        map_water_supply_reservoir_radius(), draw_second_reservoir_range);
                }
                draw_single_reservoir(0, x_start, y_start, color, has_water, 1);
            }
        }
    }
    if (!drawing_two_reservoirs) {
        data.reservoir_range.last_grid_offset = -1;
        data.reservoir_range.total = 0;
        if (blocked) {
            int grid_offset = tile->grid_offset + RESERVOIR_GRID_OFFSETS[orientation_index];
            for (int i = 0; i < 9; i++) {
                int tile_offset = grid_offset + tile_grid_offset(orientation_index, i);
                blocked_tiles[i] = map_terrain_is(tile_offset, TERRAIN_NOT_CLEAR) || map_has_figure_at(tile_offset);
            }
        } else {
            for (int i = 0; i < 9; i++) {
                blocked_tiles[i] = 0;
            }
        }
    }
    // mouse pointer = center tile of reservoir instead of north, correct here:
    y -= 30;
    if (config_get(CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE) && (!building_construction_in_progress() || draw_later)) {
        if (draw_later) {
            city_view_foreach_tile_in_range(offset + RESERVOIR_GRID_OFFSETS[orientation_index], 3,
                map_water_supply_reservoir_radius(), draw_first_reservoir_range);
        }
        city_view_foreach_tile_in_range(tile->grid_offset + RESERVOIR_GRID_OFFSETS[orientation_index], 3,
            map_water_supply_reservoir_radius(), draw_second_reservoir_range);
    }
    draw_single_reservoir(tile->grid_offset, x, y, color, has_water, drawing_two_reservoirs);
    if (!drawing_two_reservoirs) {
        draw_building_tiles(x, y, 9, blocked_tiles);
    }
    if (draw_later) {
        draw_single_reservoir(0, x_start, y_start, color, has_water, 1);
    }
}

static void draw_aqueduct(const map_tile *tile, int x, int y)
{
    int grid_offset = tile->grid_offset;
    int blocked = city_finance_out_of_money();
    if (!blocked) {
        if (building_construction_in_progress()) {
            if (!building_construction_cost()) {
                blocked = 1;
            }
        } else {
            if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
                blocked = !map_is_straight_road_for_aqueduct(grid_offset);
                if (map_property_is_plaza_or_earthquake(grid_offset)) {
                    blocked = 1;
                }
                if (map_terrain_count_directly_adjacent_with_types(grid_offset, TERRAIN_ROAD | TERRAIN_AQUEDUCT)) {
                    blocked = 1;
                }
            } else if (!map_can_place_aqueduct_on_highway(grid_offset, 0)) {
                blocked = 1;
            } else if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR ^ TERRAIN_HIGHWAY)) {
                blocked = 1;
            }
        }
    }
    int image_id = image_group(GROUP_BUILDING_AQUEDUCT);
    const terrain_image *img = map_image_context_get_aqueduct(grid_offset, 1);
    if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
        int group_offset = img->group_offset;
        if (!img->aqueduct_offset) {
            if (map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_ROAD)) {
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
        image_id += img->group_offset + 15;
    }
    draw_building(image_id, x, y, blocked ? COLOR_MASK_BUILDING_GHOST_RED : COLOR_MASK_BUILDING_GHOST);
    draw_building_tiles(x, y, 1, &blocked);
}

static void draw_fountain(const map_tile *tile, int x, int y)
{
    color_t color_mask;
    int blocked = 0;
    if (city_finance_out_of_money() || is_blocked_for_building(tile->grid_offset, 1, &blocked)) {
        color_mask = COLOR_MASK_BUILDING_GHOST_RED;
    } else {
        color_mask = COLOR_MASK_BUILDING_GHOST;
    }
    int image_id = image_group(building_properties_for_type(BUILDING_FOUNTAIN)->image_group);
    if (config_get(CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE)) {
        city_water_ghost_draw_water_structure_ranges();
        city_view_foreach_tile_in_range(tile->grid_offset, 1, map_water_supply_fountain_radius(), city_building_ghost_draw_fountain_range);
    }
    draw_building(image_id, x, y, color_mask);
    if (map_terrain_is(tile->grid_offset, TERRAIN_RESERVOIR_RANGE)) {
        const image *img = image_get(image_id);
        if (img->animation) {
            image_draw(image_id + 1, x + img->animation->sprite_offset_x, y + img->animation->sprite_offset_y,
                color_mask, data.scale);
        }
        }
    draw_building_tiles(x, y, 1, &blocked);
}

static void draw_well(const map_tile *tile, int x, int y)
{
    color_t color_mask;
    int blocked = 0;
    if (city_finance_out_of_money() || is_blocked_for_building(tile->grid_offset, 1, &blocked)) {
        image_blend_footprint_color(x, y, COLOR_MASK_RED, data.scale);
        color_mask = COLOR_MASK_BUILDING_GHOST_RED;
    } else {
        color_mask = COLOR_MASK_BUILDING_GHOST;
    }
    int image_id = image_group(building_properties_for_type(BUILDING_WELL)->image_group);
    if (config_get(CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE)) {
        city_water_ghost_draw_water_structure_ranges();
        city_view_foreach_tile_in_range(tile->grid_offset, 1, map_water_supply_well_radius(), city_building_ghost_draw_well_range);
    }
    draw_building(image_id, x, y, color_mask);
    draw_building_tiles(x, y, 1, &blocked);
}

static void draw_bathhouse(const map_tile *tile, int x, int y)
{
    int grid_offset = tile->grid_offset;
    const int building_size = 2;
    const int num_tiles = 4;
    int blocked_tiles[4];
    int blocked = 0;
    color_t color;
    if (city_finance_out_of_money()) {
        blocked = 1;
        for (int i = 0; i < num_tiles; i++) {
            blocked_tiles[i] = 1;
        }
    } else {
        blocked = is_blocked_for_building(grid_offset, building_size, blocked_tiles);
    }
    int image_id = image_group(building_properties_for_type(BUILDING_BATHHOUSE)->image_group);

    if (blocked) {
        color = COLOR_MASK_BUILDING_GHOST_RED;
    } else {
        color = COLOR_MASK_BUILDING_GHOST;
    }

    int has_water = 0;
    int orientation_index = city_view_orientation() / 2;
    for (int i = 0; i < num_tiles; i++) {
        int tile_offset = grid_offset + tile_grid_offset(orientation_index, i);
        if (map_terrain_is(tile_offset, TERRAIN_RESERVOIR_RANGE)) {
            has_water = 1;
            break;
        }
    }
    draw_building(image_id, x, y, color);
    if (has_water) {
        image_draw(image_id - 1, x - 7, y + 6, color, data.scale);
    }
    draw_building_tiles(x, y, num_tiles, blocked_tiles);
    set_roamer_path(BUILDING_BATHHOUSE, building_size, tile, has_blocked_tiles(num_tiles, blocked_tiles));
}

static void draw_pond(const map_tile *tile, int x, int y, int type)
{
    int grid_offset = tile->grid_offset;
    int building_size = (type == BUILDING_LARGE_POND) ? 3 : 2;
    int num_tiles = building_size * building_size;
    int blocked_tiles[9];
    if (city_finance_out_of_money()) {
        for (int i = 0; i < num_tiles; i++) {
            blocked_tiles[i] = 1;
        }
    } else {
        is_blocked_for_building(grid_offset, building_size, blocked_tiles);
    }

    int has_water = 0;
    int orientation_index = city_view_orientation() / 2;

    for (int i = 0; i < num_tiles; i++) {
        int tile_offset = grid_offset + tile_grid_offset(orientation_index, i);
        if (map_terrain_is(tile_offset, TERRAIN_RESERVOIR_RANGE)) {
            has_water = 1;
        }
    }
    data.ghost_building.type = type;
    data.ghost_building.has_water_access = has_water;
    data.ghost_building.grid_offset = grid_offset;

    draw_regular_building(type, building_image_get(&data.ghost_building), x, y, grid_offset, num_tiles, blocked_tiles);
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
    color_t color_mask;
    if (blocked) {
        image_blend_footprint_color(x, y, length > 0 ? COLOR_MASK_GREEN : COLOR_MASK_RED, data.scale);
        if (length > 1) {
            image_blend_footprint_color(x + x_delta * (length - 1), y + y_delta * (length - 1),
                COLOR_MASK_RED, data.scale);
        }
        building_construction_set_cost(0);
        color_mask = COLOR_MASK_BUILDING_GHOST_RED;
    } else {
        color_mask = COLOR_MASK_BUILDING_GHOST;
    }
    if (dir == DIR_0_TOP || dir == DIR_6_LEFT) {
        for (int i = length - 1; i >= 0; i--) {
            int sprite_id = map_bridge_get_sprite_id(i, length, dir, type == BUILDING_SHIP_BRIDGE);
            city_draw_bridge_tile(x + x_delta * i, y + y_delta * i, data.scale, sprite_id, color_mask);
        }
    } else {
        for (int i = 0; i < length; i++) {
            int sprite_id = map_bridge_get_sprite_id(i, length, dir, type == BUILDING_SHIP_BRIDGE);
            city_draw_bridge_tile(x + x_delta * i, y + y_delta * i, data.scale, sprite_id, color_mask);
        }
    }
    building_construction_set_cost(model_get_building(type)->cost * length);
}

static void draw_fort(const map_tile *tile, int x, int y)
{
    int blocked = 0;

    int building_size_fort = building_properties_for_type(BUILDING_FORT)->size;
    int num_tiles_fort = building_size_fort * building_size_fort;
    int building_size_ground = building_properties_for_type(BUILDING_FORT_GROUND)->size;
    int num_tiles_ground = building_size_ground * building_size_ground;

    int grid_offset_fort = tile->grid_offset;
    int grid_offset_ground = grid_offset_fort +
        FORT_GROUND_GRID_OFFSETS[building_rotation_get_rotation()][city_view_orientation() / 2];
    int blocked_tiles_fort[9];
    int blocked_tiles_ground[16];

    if (formation_get_num_legions_cached() >= formation_get_max_legions() || city_finance_out_of_money()) {
        blocked = 1;
        for (int i = 0; i < num_tiles_fort; i++) {
            blocked_tiles_fort[i] = 1;
        }
        for (int i = 0; i < num_tiles_ground; i++) {
            blocked_tiles_ground[i] = 1;
        }
    } else {
        blocked |= is_blocked_for_building(grid_offset_fort, building_size_fort, blocked_tiles_fort);
        blocked |= is_blocked_for_building(grid_offset_ground, building_size_ground, blocked_tiles_ground);
    }

    int orientation_index = building_rotation_get_building_orientation(building_rotation_get_rotation()) / 2;
    int x_ground = x + FORT_GROUND_X_VIEW_OFFSETS[orientation_index];
    int y_ground = y + FORT_GROUND_Y_VIEW_OFFSETS[orientation_index];

    color_t color_mask = blocked ? COLOR_MASK_BUILDING_GHOST_RED : COLOR_MASK_BUILDING_GHOST;

    const building_properties *props = building_properties_for_type(BUILDING_FORT);
    int image_id = get_new_building_image_id(tile->x, tile->y, tile->grid_offset, BUILDING_FORT, props);
    int image_id_grounds = image_group(GROUP_BUILDING_FORT) + 1;
    if (orientation_index == 0 || orientation_index == 3) {
        // draw fort first, then ground
        draw_building(image_id, x, y, color_mask);
        draw_building_tiles(x, y, num_tiles_fort, blocked_tiles_fort);
        draw_building(image_id_grounds, x_ground, y_ground, color_mask);
        draw_building_tiles(x_ground, y_ground, num_tiles_ground, blocked_tiles_ground);
    } else {
        // draw ground first, then fort
        draw_building(image_id_grounds, x_ground, y_ground, color_mask);
        draw_building_tiles(x_ground, y_ground, num_tiles_ground, blocked_tiles_ground);
        draw_building(image_id, x, y, color_mask);
        draw_building_tiles(x, y, num_tiles_fort, blocked_tiles_fort);
    }
}

static void draw_hippodrome(const map_tile *tile, int x, int y)
{
    int blocked = 0;
    if (city_buildings_has_hippodrome() || city_finance_out_of_money()) {
        blocked = 1;
    }
    const int building_block_size = 5;
    const int num_tiles = 25;

    building_rotation_force_two_orientations();
    int orientation_index = building_rotation_get_building_orientation(building_rotation_get_rotation()) / 2;
    int grid_offset1 = tile->grid_offset;
    int grid_offset2 = grid_offset1 + building_rotation_get_delta_with_rotation(5);
    int grid_offset3 = grid_offset1 + building_rotation_get_delta_with_rotation(10);

    int blocked_tiles1[25];
    int blocked_tiles2[25];
    int blocked_tiles3[25];
    if (city_buildings_has_hippodrome() || city_finance_out_of_money()) {
        blocked = 1;
        for (int i = 0; i < num_tiles; i++) {
            blocked_tiles1[i] = 1;
            blocked_tiles2[i] = 1;
            blocked_tiles3[i] = 1;
        }
    } else {
        blocked |= is_blocked_for_building(grid_offset1, building_block_size, blocked_tiles1);
        blocked |= is_blocked_for_building(grid_offset2, building_block_size, blocked_tiles2);
        blocked |= is_blocked_for_building(grid_offset3, building_block_size, blocked_tiles3);
    }

    int x_part1 = x;
    int y_part1 = y;
    int x_part2 = x_part1 + HIPPODROME_X_VIEW_OFFSETS[orientation_index];
    int y_part2 = y_part1 + HIPPODROME_Y_VIEW_OFFSETS[orientation_index];
    int x_part3 = x_part2 + HIPPODROME_X_VIEW_OFFSETS[orientation_index];
    int y_part3 = y_part2 + HIPPODROME_Y_VIEW_OFFSETS[orientation_index];


    color_t color_mask;
    if (blocked) {
        color_mask = COLOR_MASK_BUILDING_GHOST_RED;
    } else {
        color_mask = COLOR_MASK_BUILDING_GHOST;
    }
    if (orientation_index == 0) {
        int image_id = image_group(GROUP_BUILDING_HIPPODROME_2);
        // part 1, 2, 3
        draw_building(image_id, x_part1, y_part1, color_mask);
        draw_building_tiles(x_part1, y_part1, num_tiles, blocked_tiles1);
        draw_building(image_id + 2, x_part2, y_part2, color_mask);
        draw_building_tiles(x_part2, y_part2, num_tiles, blocked_tiles2);
        draw_building(image_id + 4, x_part3, y_part3, color_mask);
        draw_building_tiles(x_part3, y_part3, num_tiles, blocked_tiles3);
    } else if (orientation_index == 1) {
        int image_id = image_group(GROUP_BUILDING_HIPPODROME_1);
        // part 3, 2, 1
        draw_building(image_id, x_part3, y_part3, color_mask);
        draw_building_tiles(x_part3, y_part3, num_tiles, blocked_tiles3);
        draw_building(image_id + 2, x_part2, y_part2, color_mask);
        draw_building_tiles(x_part2, y_part2, num_tiles, blocked_tiles2);
        draw_building(image_id + 4, x_part1, y_part1, color_mask);
        draw_building_tiles(x_part1, y_part1, num_tiles, blocked_tiles1);
    } else if (orientation_index == 2) {
        int image_id = image_group(GROUP_BUILDING_HIPPODROME_2);
        // part 1, 2, 3
        draw_building(image_id + 4, x_part1, y_part1, color_mask);
        draw_building_tiles(x_part1, y_part1, num_tiles, blocked_tiles1);
        draw_building(image_id + 2, x_part2, y_part2, color_mask);
        draw_building_tiles(x_part2, y_part2, num_tiles, blocked_tiles2);
        draw_building(image_id, x_part3, y_part3, color_mask);
        draw_building_tiles(x_part3, y_part3, num_tiles, blocked_tiles3);
    } else if (orientation_index == 3) {
        int image_id = image_group(GROUP_BUILDING_HIPPODROME_1);
        // part 3, 2, 1
        draw_building(image_id + 4, x_part3, y_part3, color_mask);
        draw_building_tiles(x_part3, y_part3, num_tiles, blocked_tiles3);
        draw_building(image_id + 2, x_part2, y_part2, color_mask);
        draw_building_tiles(x_part2, y_part2, num_tiles, blocked_tiles2);
        draw_building(image_id, x_part1, y_part1, color_mask);
        draw_building_tiles(x_part1, y_part1, num_tiles, blocked_tiles1);
    }
    int is_blocked = has_blocked_tiles(num_tiles, blocked_tiles1) || has_blocked_tiles(num_tiles, blocked_tiles2) ||
        has_blocked_tiles(num_tiles, blocked_tiles3);
    set_roamer_path(BUILDING_HIPPODROME, building_block_size, tile, is_blocked);
}

static void draw_waterside_building(const map_tile *tile, int x, int y, building_type type)
{
    int dir_absolute = 0;
    int dir_relative = 0;
    const building_properties *props = building_properties_for_type(type);
    int blocked_tiles[9];
    int blocked = map_water_determine_orientation(tile->x, tile->y, props->size, 1, &dir_absolute, &dir_relative,
        0, blocked_tiles);
    if (city_finance_out_of_money()) {
        blocked = 1;
    }
    int offset_multiplier = type == BUILDING_DOCK ? 12 : 1;
    int image_id = image_group(props->image_group) + props->image_offset + dir_relative * offset_multiplier;

    const waterside_tile_loop *loop = map_water_get_waterside_tile_loop(dir_absolute, props->size);
    int land_tiles[5 * MAP_WATER_WATERSIDE_ROWS_NEEDED];
    int has_water_in_front = map_water_has_water_in_front(tile->x, tile->y, 1, loop, land_tiles);

    color_t color = blocked || !has_water_in_front ? COLOR_MASK_BUILDING_GHOST_RED : COLOR_MASK_BUILDING_GHOST;

    if (!has_water_in_front) {
        for (int i = 0; i < props->size * props->size; i++) {
            blocked_tiles[i] = 1;
        }
    }

    draw_building(image_id, x, y, color);
    draw_building_tiles(x, y, props->size * props->size, blocked_tiles);

    if (blocked || has_water_in_front) {
        return;
    }

    loop = map_water_get_waterside_tile_loop(dir_relative, props->size);
    int dx = loop->start.x;
    int dy = loop->start.y;
    int index = 0;

    for (int outer = 0; outer < MAP_WATER_WATERSIDE_ROWS_NEEDED; outer++) {
        for (int inner = 0; inner < loop->inner_length; inner++) {
            // Don't highlight over the dock, which is already highlighted by the blocked tiles
            if (outer > 0 || inner == 0 || inner == loop->inner_length - 1) {
                if (land_tiles[index]) {
                    image_blend_footprint_color(x + X_VIEW_OFFSET(dx, dy), y + Y_VIEW_OFFSET(dx, dy),
                        COLOR_MASK_RED, data.scale);
                } else {
                    image_draw_isometric_footprint(image_group(GROUP_TERRAIN_FLAT_TILE),
                        x + X_VIEW_OFFSET(dx, dy), y + Y_VIEW_OFFSET(dx, dy), COLOR_MASK_FOOTPRINT_GHOST, data.scale);
                }
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
        image_blend_footprint_color(x, y, COLOR_MASK_RED, data.scale);
    } else {
        draw_building(image_id, x, y, COLOR_MASK_BUILDING_GHOST);
    }
}

static void draw_market_range(int x, int y, int grid_offset)
{
    image_draw(image_group(GROUP_TERRAIN_FLAT_TILE), x, y, COLOR_MASK_GREY, data.scale);
}

static void draw_market(const map_tile *tile, int x, int y)
{
    int grid_offset = tile->grid_offset;
    const int building_size = 2;
    const int num_tiles = 4;
    int blocked_tiles[4];
    int blocked = 0;
    if (city_finance_out_of_money()) {
        blocked = 1;
        for (int i = 0; i < num_tiles; i++) {
            blocked_tiles[i] = 1;
        }
    } else {
        blocked = is_blocked_for_building(grid_offset, building_size, blocked_tiles);
    }
    if (config_get(CONFIG_UI_SHOW_MARKET_RANGE)) {
        city_view_foreach_tile_in_range(tile->grid_offset, 2, MARKET_MAX_DISTANCE, draw_market_range);
    }
    int image_id = image_group(building_properties_for_type(BUILDING_MARKET)->image_group);
    color_t color = blocked ? COLOR_MASK_BUILDING_GHOST_RED : COLOR_MASK_BUILDING_GHOST;
    draw_building(image_id, x, y, color);
    draw_building_tiles(x, y, num_tiles, blocked_tiles);
    set_roamer_path(BUILDING_MARKET, building_size, tile, blocked);
}

static void draw_highway(const map_tile *tile, int x, int y)
{
    const building_properties *props = building_properties_for_type(BUILDING_HIGHWAY);

    // check if we can place building
    int grid_offset = tile->grid_offset;
    int fully_blocked = city_finance_out_of_money();

    int num_tiles = props->size * props->size;
    int blocked_tiles[MAX_TILES];
    int orientation_index = city_view_orientation() / 2;

    for (int i = 0; i < num_tiles; i++) {
        int tile_offset = grid_offset + tile_grid_offset(orientation_index, i);
        int terrain = map_terrain_get(tile_offset);
        int has_forbidden_terrain = terrain & TERRAIN_NOT_CLEAR & ~TERRAIN_HIGHWAY & ~TERRAIN_AQUEDUCT & ~TERRAIN_ROAD;
        if (fully_blocked || has_forbidden_terrain || !map_can_place_highway_under_aqueduct(tile_offset, 0)) {
            blocked_tiles[i] = 1;
        } else {
            blocked_tiles[i] = 0;
        }
    }

    int image_id = get_new_building_image_id(tile->x, tile->y, grid_offset, BUILDING_HIGHWAY, props);
    draw_regular_building(BUILDING_HIGHWAY, image_id, x, y, grid_offset, num_tiles, blocked_tiles);
}

static void draw_grand_temple_neptune_range(int x, int y, int grid_offset)
{
    image_draw(image_group(GROUP_TERRAIN_FLAT_TILE), x, y, COLOR_MASK_BLUE, data.scale);
}

static void draw_grand_temple_neptune(const map_tile *tile, int x, int y)
{
    const building_properties *props = building_properties_for_type(BUILDING_GRAND_TEMPLE_NEPTUNE);
    int num_tiles = props->size * props->size;
    int blocked[MAX_TILES];
    if (city_finance_out_of_money() || is_blocked_for_building(tile->grid_offset, props->size, blocked)) {
        image_blend_footprint_color(x, y, COLOR_MASK_RED, data.scale);
    }
    // need to add 2 for the bonus the Neptune GT will add
    int radius = map_water_supply_reservoir_radius() + 2;
    city_view_foreach_tile_in_range(tile->grid_offset, props->size, radius, draw_grand_temple_neptune_range);
    int image_id = get_new_building_image_id(tile->x, tile->y, tile->grid_offset, BUILDING_GRAND_TEMPLE_NEPTUNE, props);
    draw_regular_building(BUILDING_GRAND_TEMPLE_NEPTUNE, image_id, x, y, tile->grid_offset, num_tiles, blocked);
    set_roamer_path(BUILDING_GRAND_TEMPLE_NEPTUNE, props->size, tile, has_blocked_tiles(num_tiles, blocked));
}

int city_building_ghost_mark_deleting(const map_tile *tile)
{
    int construction_type = building_construction_type();
    if (!tile->grid_offset || building_construction_draw_as_constructing() ||
        scroll_in_progress() || construction_type != BUILDING_CLEAR_LAND) {
        return (construction_type == BUILDING_CLEAR_LAND);
    }
    if (!building_construction_in_progress()) {
        map_property_clear_constructing_and_deleted();
    }
    map_building_tiles_mark_deleting(tile->grid_offset);
    if (map_terrain_is(tile->grid_offset, TERRAIN_HIGHWAY) && !map_terrain_is(tile->grid_offset, TERRAIN_AQUEDUCT)) {
        map_tiles_clear_highway(tile->grid_offset, 1);
    }
    return 1;
}

static int is_water_building(void)
{
    switch (data.ghost_building.type) {
        case BUILDING_DOCK:
        case BUILDING_SHIPYARD:
        case BUILDING_WHARF:
        case BUILDING_LOW_BRIDGE:
        case BUILDING_SHIP_BRIDGE:
            return 1;
        default:
            return 0;
    }
}

static void draw_grid_tile(int x, int y, int grid_offset)
{
    static int image_id = 0;
    if (!image_id) {
        image_id = assets_get_image_id("UI", "Grid_Full");
    }
    if (map_terrain_is(grid_offset, TERRAIN_BUILDING) || map_terrain_is(grid_offset, TERRAIN_ROCK) ||
        map_terrain_is(grid_offset, TERRAIN_ACCESS_RAMP) || map_terrain_is(grid_offset, TERRAIN_ELEVATION) ||
        (map_terrain_is(grid_offset, TERRAIN_WATER) && !is_water_building())) {
        return;
    }
    image_draw(image_id, x, y, COLOR_GRID, data.scale);
}

static void draw_grid_around_building(int grid_offset, int size, int orientation, int x, int y)
{
    int num_tiles = size * size;
    city_view_foreach_tile_in_range(grid_offset, size, 2, draw_grid_tile);
    for (int i = 0; i < num_tiles; i++) {
        draw_grid_tile(x + view_offset_x(i), y + view_offset_y(i), grid_offset + tile_grid_offset(orientation, i));
    }
}

static void draw_partial_grid(int grid_offset, int x, int y, building_type type)
{
    int size = building_properties_for_type(type)->size;
    int orientation_index = city_view_orientation() / 2;
    if (building_is_farm(type) || type == BUILDING_DRAGGABLE_RESERVOIR || type == BUILDING_WAREHOUSE) {
        size = 3;
        if (type == BUILDING_DRAGGABLE_RESERVOIR) {
            grid_offset += RESERVOIR_GRID_OFFSETS[orientation_index];
        }
    }
    draw_grid_around_building(grid_offset, size, orientation_index, x, y);
    if (building_is_fort(type)) {
        grid_offset += FORT_GROUND_GRID_OFFSETS[building_rotation_get_rotation()][orientation_index];
        int ground_index = building_rotation_get_building_orientation(building_rotation_get_rotation()) / 2;
        int x_ground = x + FORT_GROUND_X_VIEW_OFFSETS[ground_index];
        int y_ground = y + FORT_GROUND_Y_VIEW_OFFSETS[ground_index];
        draw_grid_around_building(grid_offset, 4, ground_index, x_ground, y_ground);
    } else if (type == BUILDING_HIPPODROME) {
        building_rotation_force_two_orientations();
        orientation_index = building_rotation_get_building_orientation(building_rotation_get_rotation()) / 2;
        int grid_offset2 = grid_offset + building_rotation_get_delta_with_rotation(5);
        int grid_offset3 = grid_offset + building_rotation_get_delta_with_rotation(10);
        int x_part2 = x + HIPPODROME_X_VIEW_OFFSETS[orientation_index];
        int y_part2 = y + HIPPODROME_Y_VIEW_OFFSETS[orientation_index];
        int x_part3 = x_part2 + HIPPODROME_X_VIEW_OFFSETS[orientation_index];
        int y_part3 = y_part2 + HIPPODROME_Y_VIEW_OFFSETS[orientation_index];
        draw_grid_around_building(grid_offset2, size, orientation_index, x_part2, y_part2);
        draw_grid_around_building(grid_offset3, size, orientation_index, x_part3, y_part3);
    }
}

static void create_tile_offsets(void)
{
    if (data.offsets[0][MAX_TILES - 1].x) {
        return;
    }

    static const tile_offset steps[4] = { { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } };

    for (int dir = 0; dir < 4; dir++) {
        const tile_offset *step = &steps[dir];
        int index = 0;
        int column = 0;
        int row = 0;
        int *x = dir & 1 ? &row : &column;
        int *y = dir & 1 ? &column : &row;
        tile_offset *offset = data.offsets[dir];

        while (index < MAX_TILES) {
            for (column = 0; column < row; column++) {
                offset[index].x = *x * step->x;
                offset[index].y = *y * step->y;
                offset[index + 1].x = *y * step->x;
                offset[index + 1].y = *x * step->y;
                index += 2;
            }
            offset[index].x = row * step->x;
            offset[index].y = row * step->y;
            index++;
            row++;
        }
    }
}

void city_building_ghost_draw(const map_tile *tile)
{
    if (!tile->grid_offset || scroll_in_progress()) {
        return;
    }
    building_type type = building_construction_type();
    data.ghost_building.type = type;
    if (building_construction_draw_as_constructing() || type == BUILDING_NONE || type == BUILDING_CLEAR_LAND) {
        return;
    }
    create_tile_offsets();
    data.scale = city_view_get_scale() / 100.0f;
    int x, y;
    city_view_get_selected_tile_pixels(&x, &y);

    if (!config_get(CONFIG_UI_SHOW_GRID) && config_get(CONFIG_UI_SHOW_PARTIAL_GRID_AROUND_CONSTRUCTION)) {
        draw_partial_grid(tile->grid_offset, x, y, type);
    }

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
        case BUILDING_WELL:
            draw_well(tile, x, y);
            break;
        case BUILDING_BATHHOUSE:
            draw_bathhouse(tile, x, y);
            break;
        case BUILDING_SMALL_POND:
        case BUILDING_LARGE_POND:
            draw_pond(tile, x, y, type);
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
        case BUILDING_DOCK:
            draw_waterside_building(tile, x, y, type);
            break;
        case BUILDING_ROAD:
            draw_road(tile, x, y);
            break;
        case BUILDING_MARKET:
            draw_market(tile, x, y);
            break;
        case BUILDING_HOUSE_VACANT_LOT:
            if (config_get(CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE_HOUSES)) {
                city_water_ghost_draw_water_structure_ranges();
            }
            draw_default(tile, x, y, type);
            break;
        case BUILDING_HIGHWAY:
            draw_highway(tile, x, y);
            break;
        case BUILDING_GRAND_TEMPLE_NEPTUNE:
            draw_grand_temple_neptune(tile, x, y);
            break;
        default:
            draw_default(tile, x, y, type);
            break;
    }
}
