#include "construction.h"

#include "assets/assets.h"
#include "building/building.h"
#include "building/building_variant.h"
#include "building/connectable.h"
#include "building/construction_building.h"
#include "building/construction_clear.h"
#include "building/construction_routed.h"
#include "building/construction_warning.h"
#include "building/count.h"
#include "building/image.h"
#include "building/model.h"
#include "building/monument.h"
#include "building/properties.h"
#include "building/rotation.h"
#include "building/warehouse.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/resource.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/image.h"
#include "figure/formation.h"
#include "game/undo.h"
#include "graphics/window.h"
#include "map/aqueduct.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/point.h"
#include "map/property.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/water.h"

#define BUILDING_CYCLES 4
#define MAX_CYCLE_SIZE 10

struct reservoir_info {
    int cost;
    int place_reservoir_at_start;
    int place_reservoir_at_end;
};

struct cycle {
    int size;
    int rotations_to_next;
    int array[MAX_CYCLE_SIZE];
};

enum {
    PLACE_RESERVOIR_BLOCKED = -1,
    PLACE_RESERVOIR_NO = 0,
    PLACE_RESERVOIR_YES = 1,
    PLACE_RESERVOIR_EXISTS = 2
};

static struct {
    building_type type;
    building_type sub_type;
    int in_progress;
    map_tile start;
    map_tile end;
    int cost_preview;
    struct {
        int meadow;
        int rock;
        int tree;
        int water;
        int wall;
        int distant_water;
    } required_terrain;
    int draw_as_constructing;
    int start_offset_x_view;
    int start_offset_y_view;
    int cycle_step;
} data;

static int last_items_cleared;

static const int FORT_X_OFFSET[4][4] = { {3,4,4,3},{-1,0,0,-1},{-4,-3,-3,4},{0,1,1,0} };
static const int FORT_Y_OFFSET[4][4] = { {-1,-1,0,0},{-4,-4,-3,-3},{0,0,1,1},{3,3,4,4} };

const static struct cycle building_cycles[BUILDING_CYCLES] = {
    { 5, 1, { BUILDING_SMALL_TEMPLE_CERES, BUILDING_SMALL_TEMPLE_NEPTUNE, BUILDING_SMALL_TEMPLE_MERCURY,
      BUILDING_SMALL_TEMPLE_MARS,  BUILDING_SMALL_TEMPLE_VENUS }},
    { 5, 1, {BUILDING_LARGE_TEMPLE_CERES, BUILDING_LARGE_TEMPLE_NEPTUNE, BUILDING_LARGE_TEMPLE_MERCURY,
      BUILDING_LARGE_TEMPLE_MARS,  BUILDING_LARGE_TEMPLE_VENUS}},
    { 9, 2, {BUILDING_GARDEN_PATH, BUILDING_DATE_PATH, BUILDING_ELM_PATH,  BUILDING_FIG_PATH,  BUILDING_FIR_PATH,
      BUILDING_OAK_PATH,  BUILDING_PALM_PATH, BUILDING_PINE_PATH, BUILDING_PLUM_PATH}},
    { 8, 1, {BUILDING_DATE_TREE, BUILDING_ELM_TREE,  BUILDING_FIG_TREE,  BUILDING_FIR_TREE,
      BUILDING_OAK_TREE,  BUILDING_PALM_TREE, BUILDING_PINE_TREE, BUILDING_PLUM_TREE }},
};

int building_construction_type_can_cycle(building_type type)
{
    for (int i = 0; i < BUILDING_CYCLES; i++) {
        int size = building_cycles[i].size;
        for (int j = 0; j < size; j++) {
            if (building_cycles[i].array[j] == type) {
                return 1;
            }
        }
    }
    return 0;
}

int building_construction_type_num_cycles(building_type type)
{
    for (int i = 0; i < BUILDING_CYCLES; i++) {
        int size = building_cycles[i].size;
        for (int j = 0; j < size; j++) {
            if (building_cycles[i].array[j] == type) {
                return size * building_cycles[i].rotations_to_next;
            }
        }
    }
    return 1;
}

int building_construction_cycle_forward(void)
{
    if (data.type == BUILDING_NONE) {
        return 0;
    }

    for (int i = 0; i < BUILDING_CYCLES; i++) {
        int size = building_cycles[i].size;
        for (int j = 0; j < size; j++) {
            if (building_cycles[i].array[j] == building_construction_type()) {
                data.cycle_step += 1;
                if (data.cycle_step < building_cycles[i].rotations_to_next) {
                    return 0;
                }
                data.cycle_step = 0;
                int new_type;
                if (j + 1 >= size) { // If last element of the list, the next one is the first one
                    new_type = building_cycles[i].array[0];
                } else { // Otherwise pick the first one
                    new_type = building_cycles[i].array[j + 1];
                }
                if (building_cycles[i].array[j] == data.type) {
                    data.type = new_type;
                } else {
                    data.sub_type = new_type;
                }

                return 1;
            }
        }
    }
    return 0;
}

int building_construction_cycle_back(void)
{
    if (data.type == BUILDING_NONE) {
        return 0;
    }

    for (int i = 0; i < BUILDING_CYCLES; i++) {
        int size = building_cycles[i].size;
        for (int j = 0; j < size; j++) {
            if (building_cycles[i].array[j] == building_construction_type()) {
                data.cycle_step -= 1;
                if (data.cycle_step >= 0) {
                    return 0;
                }
                data.cycle_step = building_cycles[i].rotations_to_next - 1;
                int new_type;
                if (j - 1 < 0) { // If first element of the list, pick the last element
                    new_type = building_cycles[i].array[size - 1];
                } else { // Otherwise pick the previous element
                    new_type = building_cycles[i].array[j - 1];
                }
                if (building_cycles[i].array[j] == data.type) {
                    data.type = new_type;
                } else {
                    data.sub_type = new_type;
                }

                return 1;
            }
        }
    }
    return 0;
}

static void mark_construction(int x, int y, int size, int terrain, int absolute_xy)
{
    if (map_building_tiles_mark_construction(x, y, size, terrain, absolute_xy)) {
        data.draw_as_constructing = 1;
    }
}

static int place_houses(int measure_only, int x_start, int y_start, int x_end, int y_end)
{
    int x_min, x_max, y_min, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);

    int needs_road_warning = 0;
    int items_placed = 0;
    game_undo_restore_building_state();
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
                continue;
            }
            if (measure_only) {
                map_property_mark_constructing(grid_offset);
                items_placed++;
            } else {
                building *b = building_create(BUILDING_HOUSE_VACANT_LOT, x, y);
                game_undo_add_building(b);
                if (b->id > 0) {
                    items_placed++;
                    map_building_tiles_add(b->id, x, y, 1,
                        image_group(GROUP_BUILDING_HOUSE_VACANT_LOT), TERRAIN_BUILDING);
                    if (!map_terrain_exists_tile_in_radius_with_type(x, y, 1, 2, TERRAIN_ROAD)) {
                        needs_road_warning = 1;
                    }
                }
            }
        }
    }
    if (!measure_only) {
        building_construction_warning_check_food_stocks(BUILDING_HOUSE_VACANT_LOT);
        if (needs_road_warning) {
            city_warning_show(WARNING_HOUSE_TOO_FAR_FROM_ROAD, NEW_WARNING_SLOT);
        }
        map_routing_update_land();
        window_invalidate();
    }
    return items_placed;
}

static int place_plaza(int x_start, int y_start, int x_end, int y_end)
{
    int x_min, y_min, x_max, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);
    game_undo_restore_map(1);

    int items_placed = 0;
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (map_terrain_is(grid_offset, TERRAIN_ROAD) &&
                !map_terrain_is(grid_offset, TERRAIN_WATER | TERRAIN_BUILDING | TERRAIN_AQUEDUCT)) {
                if (!map_property_is_plaza_or_earthquake(grid_offset)) {
                    items_placed++;
                }
                map_image_set(grid_offset, 0);
                map_property_mark_plaza_or_earthquake(grid_offset);
                map_property_set_multi_tile_size(grid_offset, 1);
                map_property_mark_draw_tile(grid_offset);
            }
        }
    }
    map_tiles_update_all_plazas();
    return items_placed;
}

static int place_garden(int x_start, int y_start, int x_end, int y_end)
{
    game_undo_restore_map(1);

    int x_min, y_min, x_max, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);

    int items_placed = 0;
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (!map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
                items_placed++;
                map_terrain_add(grid_offset, TERRAIN_GARDEN);
            }
        }
    }
    map_tiles_update_all_gardens();
    return items_placed;
}

static int plot_draggable_building(int x_start, int y_start, int x_end, int y_end, int allow_roads)
{
    int x_min, y_min, x_max, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);
    map_image_restore();
    map_image_backup();

    int terrain = TERRAIN_NOT_CLEAR;
    if (allow_roads) {
        terrain = TERRAIN_NOT_CLEAR_EXCEPT_ROAD;
    }

    int items_placed = 0;
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (!map_terrain_is(grid_offset, terrain)) {
                map_property_mark_constructing(grid_offset);
                items_placed++;
                continue;
            }
        }
    }
    return items_placed;
}

static int place_draggable_building(int x_start, int y_start, int x_end, int y_end, int building_type, int rotation)
{
    int x_min, y_min, x_max, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);
    map_image_restore();

    int items_placed = 0;
    int gates_placed = 0;
    int gate_type = building_connectable_gate_type(building_type);

    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (!map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
                items_placed++;
                building *b = building_create(building_type, x, y);
                if (building_variant_has_variants(building_type)) {
                    b->variant = building_rotation_get_rotation_with_limit(building_variant_get_number_of_variants(b->type));
                } else {
                    b->subtype.orientation = rotation;
                }
                game_undo_add_building(b);
                map_building_tiles_add(b->id, b->x, b->y, b->size, building_image_get(b), TERRAIN_BUILDING);
            } else if (!map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR_EXCEPT_ROAD)) {
                if (gate_type) {
                    items_placed++;
                    building *b = building_create(gate_type, x, y);
                    if (building_variant_has_variants(gate_type)) {
                        b->variant = building_rotation_get_rotation_with_limit(building_variant_get_number_of_variants(b->type));
                    } else {
                        b->subtype.orientation = rotation;
                    }
                    game_undo_add_building(b);
                    map_building_tiles_add(b->id, b->x, b->y, b->size, building_image_get(b), TERRAIN_BUILDING);
                    map_terrain_add(grid_offset, TERRAIN_ROAD);
                }
            }
        }
    }

    if (building_is_connectable(building_type)) {
        map_property_clear_constructing_and_deleted();
        building_connectable_update_connections_for_type(building_type);
        if (gates_placed) {
            map_tiles_update_all_roads();
        }
    }

    map_routing_update_land();
    return items_placed;
}


static int place_reservoir_and_aqueducts(int measure_only, int x_start, int y_start,
    int x_end, int y_end, struct reservoir_info *info)
{
    info->cost = 0;
    info->place_reservoir_at_start = PLACE_RESERVOIR_NO;
    info->place_reservoir_at_end = PLACE_RESERVOIR_NO;

    game_undo_restore_map(0);

    int distance = calc_maximum_distance(x_start, y_start, x_end, y_end);
    if (measure_only && !data.in_progress) {
        distance = 0;
    }
    if (distance > 0) {
        if (map_building_is_reservoir(x_start - 1, y_start - 1)) {
            info->place_reservoir_at_start = PLACE_RESERVOIR_EXISTS;
        } else if (map_tiles_are_clear(x_start - 1, y_start - 1, 3, TERRAIN_ALL)) {
            info->place_reservoir_at_start = PLACE_RESERVOIR_YES;
        } else {
            info->place_reservoir_at_start = PLACE_RESERVOIR_BLOCKED;
        }
    }
    if (map_building_is_reservoir(x_end - 1, y_end - 1)) {
        info->place_reservoir_at_end = PLACE_RESERVOIR_EXISTS;
    } else if (map_tiles_are_clear(x_end - 1, y_end - 1, 3, TERRAIN_ALL)) {
        info->place_reservoir_at_end = PLACE_RESERVOIR_YES;
    } else {
        info->place_reservoir_at_end = PLACE_RESERVOIR_BLOCKED;
    }
    if (info->place_reservoir_at_start == PLACE_RESERVOIR_BLOCKED
        || info->place_reservoir_at_end == PLACE_RESERVOIR_BLOCKED) {
        return 0;
    }
    if (info->place_reservoir_at_start == PLACE_RESERVOIR_YES
        && info->place_reservoir_at_end == PLACE_RESERVOIR_YES && distance < 3) {
        return 0;
    }
    if (!distance) {
        if (info->place_reservoir_at_end == PLACE_RESERVOIR_YES) {
            info->cost = model_get_building(BUILDING_RESERVOIR)->cost;
        }
        return 1;
    }
    if (!map_routing_calculate_distances_for_building(ROUTED_BUILDING_AQUEDUCT, x_start, y_start)) {
        return 0;
    }
    if (info->place_reservoir_at_start != PLACE_RESERVOIR_NO) {
        map_routing_block(x_start - 1, y_start - 1, 3);
        mark_construction(x_start - 1, y_start - 1, 3, TERRAIN_ALL, 1);
    }
    if (info->place_reservoir_at_end != PLACE_RESERVOIR_NO) {
        map_routing_block(x_end - 1, y_end - 1, 3);
        mark_construction(x_end - 1, y_end - 1, 3, TERRAIN_ALL, 1);
    }
    const int aqueduct_offsets_x[] = { 0, 2, 0, -2 };
    const int aqueduct_offsets_y[] = { -2, 0, 2, 0 };
    int min_dist = 10000;
    int min_dir_start = 0, min_dir_end = 0;
    for (int dir_start = 0; dir_start < 4; dir_start++) {
        int dx_start = aqueduct_offsets_x[dir_start];
        int dy_start = aqueduct_offsets_y[dir_start];
        for (int dir_end = 0; dir_end < 4; dir_end++) {
            int dx_end = aqueduct_offsets_x[dir_end];
            int dy_end = aqueduct_offsets_y[dir_end];
            int dist;
            if (building_construction_place_aqueduct_for_reservoir(1,
                x_start + dx_start, y_start + dy_start, x_end + dx_end, y_end + dy_end, &dist)) {
                if (dist && dist < min_dist) {
                    min_dist = dist;
                    min_dir_start = dir_start;
                    min_dir_end = dir_end;
                }
            }
        }
    }
    if (min_dist == 10000) {
        return 0;
    }
    int x_aq_start = aqueduct_offsets_x[min_dir_start];
    int y_aq_start = aqueduct_offsets_y[min_dir_start];
    int x_aq_end = aqueduct_offsets_x[min_dir_end];
    int y_aq_end = aqueduct_offsets_y[min_dir_end];
    int aq_items;
    building_construction_place_aqueduct_for_reservoir(0, x_start + x_aq_start, y_start + y_aq_start,
        x_end + x_aq_end, y_end + y_aq_end, &aq_items);
    if (info->place_reservoir_at_start == PLACE_RESERVOIR_YES) {
        info->cost += model_get_building(BUILDING_RESERVOIR)->cost;
    }
    if (info->place_reservoir_at_end == PLACE_RESERVOIR_YES) {
        info->cost += model_get_building(BUILDING_RESERVOIR)->cost;
    }
    if (aq_items) {
        info->cost += aq_items * model_get_building(BUILDING_AQUEDUCT)->cost;
    }
    return 1;
}

void building_construction_set_cost(int cost)
{
    data.cost_preview = cost;
}

int building_construction_can_rotate(void)
{
    return building_rotation_type_has_rotations(data.type);
}

void building_construction_set_type(building_type type)
{
    if (type != data.type) {
        building_rotation_remove_rotation();
    }
    data.type = type;
    data.sub_type = BUILDING_NONE;
    data.in_progress = 0;
    data.start.x = 0;
    data.start.y = 0;
    data.end.x = 0;
    data.end.y = 0;
    data.cost_preview = 0;

    if (type != BUILDING_NONE) {
        data.required_terrain.wall = 0;
        data.required_terrain.water = 0;
        data.required_terrain.tree = 0;
        data.required_terrain.rock = 0;
        data.required_terrain.meadow = 0;
        data.required_terrain.distant_water = 0;
        data.start.grid_offset = 0;

        switch (type) {
            case BUILDING_WHEAT_FARM:
            case BUILDING_VEGETABLE_FARM:
            case BUILDING_FRUIT_FARM:
            case BUILDING_OLIVE_FARM:
            case BUILDING_VINES_FARM:
            case BUILDING_PIG_FARM:
                data.required_terrain.meadow = 1;
                break;
            case BUILDING_MARBLE_QUARRY:
            case BUILDING_IRON_MINE:
                data.required_terrain.rock = 1;
                break;
            case BUILDING_TIMBER_YARD:
                data.required_terrain.tree = 1;
                break;
            case BUILDING_CLAY_PIT:
                data.required_terrain.water = 1;
                break;
            case BUILDING_TOWER:
                data.required_terrain.wall = 1;
                break;
            case BUILDING_MENU_SMALL_TEMPLES:
                data.sub_type = BUILDING_SMALL_TEMPLE_CERES;
                break;
            case BUILDING_MENU_LARGE_TEMPLES:
                data.sub_type = BUILDING_LARGE_TEMPLE_CERES;
                break;
            case BUILDING_LIGHTHOUSE:
                data.required_terrain.distant_water = 1;
            default:
                break;
        }
    }
    if (building_construction_can_rotate()) {
        building_rotation_setup_rotation();
    }
}

void building_construction_clear_type(void)
{
    data.cost_preview = 0;
    data.sub_type = BUILDING_NONE;
    data.type = BUILDING_NONE;
    building_rotation_remove_rotation();
}

building_type building_construction_type(void)
{
    return data.sub_type ? data.sub_type : data.type;
}

int building_construction_cost(void)
{
    return data.cost_preview;
}

int building_construction_size(int *x, int *y)
{
    if (!config_get(CONFIG_UI_SHOW_CONSTRUCTION_SIZE) ||
        !building_construction_is_updatable() || !data.in_progress ||
        (data.type != BUILDING_CLEAR_LAND && !data.cost_preview)) {
        return 0;
    }
    int size_x = data.end.x - data.start.x;
    int size_y = data.end.y - data.start.y;
    if (size_x < 0) {
        size_x = -size_x;
    }
    if (size_y < 0) {
        size_y = -size_y;
    }
    size_x++;
    size_y++;
    *x = size_x;
    *y = size_y;
    return 1;
}

int building_construction_in_progress(void)
{
    return data.in_progress;
}

void building_construction_start(int x, int y, int grid_offset)
{
    if (data.type == BUILDING_HIGHWAY) {
        building_construction_offset_start_from_orientation(&x, &y, 2);
        grid_offset = map_grid_offset(x, y);
    }
    data.start.grid_offset = grid_offset;
    data.start.x = data.end.x = x;
    data.start.y = data.end.y = y;

    if (game_undo_start_build(data.type)) {
        data.in_progress = 1;
        int can_start = 1;
        switch (data.type) {
            case BUILDING_ROAD:
                can_start = map_routing_calculate_distances_for_building(
                    ROUTED_BUILDING_ROAD, data.start.x, data.start.y);
                break;
            case BUILDING_AQUEDUCT:
            case BUILDING_DRAGGABLE_RESERVOIR:
                can_start = map_routing_calculate_distances_for_building(
                    ROUTED_BUILDING_AQUEDUCT, data.start.x, data.start.y);
                break;
            case BUILDING_WALL:
                can_start = map_routing_calculate_distances_for_building(
                    ROUTED_BUILDING_WALL, data.start.x, data.start.y);
                break;
            case BUILDING_HIGHWAY:
                can_start = map_routing_calculate_distances_for_building(
                    ROUTED_BUILDING_HIGHWAY, data.start.x, data.start.y);
            default:
                break;
        }
        if (!can_start) {
            building_construction_cancel();
        }
    }
}

int building_construction_is_updatable(void)
{
    switch (data.type) {
        case BUILDING_CLEAR_LAND:
        case BUILDING_ROAD:
        case BUILDING_AQUEDUCT:
        case BUILDING_DRAGGABLE_RESERVOIR:
        case BUILDING_PINE_TREE:
        case BUILDING_FIR_TREE:
        case BUILDING_OAK_TREE:
        case BUILDING_ELM_TREE:
        case BUILDING_FIG_TREE:
        case BUILDING_PLUM_TREE:
        case BUILDING_PALM_TREE:
        case BUILDING_DATE_TREE:
        case BUILDING_PINE_PATH:
        case BUILDING_FIR_PATH:
        case BUILDING_OAK_PATH:
        case BUILDING_ELM_PATH:
        case BUILDING_FIG_PATH:
        case BUILDING_PLUM_PATH:
        case BUILDING_PALM_PATH:
        case BUILDING_DATE_PATH:
        case BUILDING_HEDGE_DARK:
        case BUILDING_HEDGE_LIGHT:
        case BUILDING_COLONNADE:
        case BUILDING_GARDEN_PATH:
        case BUILDING_GARDEN_WALL:
        case BUILDING_ROOFED_GARDEN_WALL:
        case BUILDING_DECORATIVE_COLUMN:
        case BUILDING_WALL:
        case BUILDING_PLAZA:
        case BUILDING_GARDENS:
        case BUILDING_HOUSE_VACANT_LOT:
        case BUILDING_PALISADE:
        case BUILDING_HIGHWAY:
            return 1;
        default:
            return 0;
    }
}

void building_construction_cancel(void)
{
    map_property_clear_constructing_and_deleted();
    if (data.in_progress && building_construction_is_updatable()) {
        game_undo_restore_building_state();
        game_undo_restore_map(1);
        data.in_progress = 0;
        data.cost_preview = 0;
    } else {
        building_construction_clear_type();
    }
    building_rotation_reset_rotation();
}

void building_construction_update(int x, int y, int grid_offset)
{
    building_type type = building_construction_type();
    if (grid_offset) {
        if (type == BUILDING_HIGHWAY) {
            building_construction_offset_start_from_orientation(&x, &y, 2);
            grid_offset = map_grid_offset(x, y);
        }
        data.end.x = x;
        data.end.y = y;
        data.end.grid_offset = grid_offset;
    } else {
        x = data.end.x;
        y = data.end.y;
        grid_offset = data.end.grid_offset;
    }
    if (!type || city_finance_out_of_money()) {
        data.cost_preview = 0;
        return;
    }

    map_property_clear_constructing_and_deleted();
    int current_cost = model_get_building(type)->cost;

    if (type == BUILDING_CLEAR_LAND) {
        int items_placed = last_items_cleared = building_construction_clear_land(1, data.start.x, data.start.y, x, y);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_WALL) {
        int items_placed = building_construction_place_wall(1, data.start.x, data.start.y, x, y);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_ROAD) {
        int items_placed = building_construction_place_road(1, data.start.x, data.start.y, x, y);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_HIGHWAY) {
        int items_placed = building_construction_place_highway(1, data.start.x, data.start.y, x, y);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_PLAZA) {
        int items_placed = place_plaza(data.start.x, data.start.y, x, y);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_GARDENS) {
        int items_placed = place_garden(data.start.x, data.start.y, x, y);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type >= BUILDING_PINE_TREE && type <= BUILDING_DATE_TREE) {
        int items_placed = plot_draggable_building(data.start.x, data.start.y, x, y, 0);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type >= BUILDING_PINE_PATH && type <= BUILDING_DATE_PATH) {
        int items_placed = plot_draggable_building(data.start.x, data.start.y, x, y, 0);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type >= BUILDING_SMALL_STATUE_ALT && type <= BUILDING_SMALL_STATUE_ALT_B) {
        int items_placed = plot_draggable_building(data.start.x, data.start.y, x, y, 0);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_HEDGE_DARK || type == BUILDING_HEDGE_LIGHT) {
        int items_placed = plot_draggable_building(data.start.x, data.start.y, x, y, 1);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_COLONNADE) {
        int items_placed = plot_draggable_building(data.start.x, data.start.y, x, y, 0);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_GARDEN_PATH) {
        int items_placed = plot_draggable_building(data.start.x, data.start.y, x, y, 0);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_GARDEN_WALL) {
        int items_placed = plot_draggable_building(data.start.x, data.start.y, x, y, 1);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_ROOFED_GARDEN_WALL) {
        int items_placed = plot_draggable_building(data.start.x, data.start.y, x, y, 1);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_DECORATIVE_COLUMN) {
            int items_placed = plot_draggable_building(data.start.x, data.start.y, x, y, 0);
            if (items_placed >= 0) {
                current_cost *= items_placed;
            }
    } else if (type == BUILDING_PALISADE) {
        int items_placed = plot_draggable_building(data.start.x, data.start.y, x, y, 1);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_LOW_BRIDGE || type == BUILDING_SHIP_BRIDGE) {
        int length = map_bridge_building_length();
        if (length > 1) {
            current_cost *= length;
        }
    } else if (type == BUILDING_AQUEDUCT) {
        building_construction_place_aqueduct(data.start.x, data.start.y, x, y, &current_cost);
        map_tiles_update_all_aqueducts(0);
    } else if (type == BUILDING_DRAGGABLE_RESERVOIR) {
        struct reservoir_info info;
        place_reservoir_and_aqueducts(1, data.start.x, data.start.y, x, y, &info);
        current_cost = info.cost;
        map_tiles_update_all_aqueducts(1);
        data.draw_as_constructing = 0;
    } else if (type == BUILDING_HOUSE_VACANT_LOT) {
        int items_placed = place_houses(1, data.start.x, data.start.y, x, y);
        if (items_placed >= 0) {
            current_cost *= items_placed;
        }
    } else if (type == BUILDING_GATEHOUSE) {
        mark_construction(x, y, 2, ~TERRAIN_ROAD, 0);
    } else if (type == BUILDING_ROADBLOCK) {
        // Don't show the footprint for roadblocks
    } else if (type == BUILDING_TRIUMPHAL_ARCH) {
        mark_construction(x, y, 3, ~TERRAIN_ROAD, 0);
    } else if (type == BUILDING_WAREHOUSE) {
        mark_construction(x, y, 3, TERRAIN_ALL, 0);
    } else if (building_is_fort(type)) {
        if (formation_get_num_legions_cached() < formation_get_max_legions()) {
            int rotation = building_rotation_get_rotation();
            int orientation = city_view_orientation() / 2;
            int x_offset = FORT_X_OFFSET[rotation][orientation];
            int y_offset = FORT_Y_OFFSET[rotation][orientation];
            if (map_building_tiles_are_clear(x, y, 3, TERRAIN_ALL) &&
                map_building_tiles_are_clear(x + x_offset, y + y_offset, 4, TERRAIN_ALL) &&
                city_buildings_has_mess_hall()) {
                mark_construction(x, y, 3, TERRAIN_ALL, 0);
            }
        }
    } else if (type == BUILDING_HIPPODROME) {
        int x_offset_1, y_offset_1;
        building_rotation_get_offset_with_rotation(5, building_rotation_get_rotation(), &x_offset_1, &y_offset_1);
        int x_offset_2, y_offset_2;
        building_rotation_get_offset_with_rotation(10, building_rotation_get_rotation(), &x_offset_2, &y_offset_2);
        if (map_building_tiles_are_clear(x, y, 5, TERRAIN_ALL) &&
            map_building_tiles_are_clear(x + x_offset_1, y + y_offset_1, 5, TERRAIN_ALL) &&
            map_building_tiles_are_clear(x + x_offset_2, y + y_offset_2, 5, TERRAIN_ALL) &&
            !city_buildings_has_hippodrome()) {
            mark_construction(x, y, 5, TERRAIN_ALL, 0);
        }
    } else if (type == BUILDING_SHIPYARD || type == BUILDING_WHARF) {
        if (!map_water_determine_orientation_size2(x, y, 1, 0, 0)) {
            data.draw_as_constructing = 1;
        }
    } else if (type == BUILDING_DOCK) {
        if (!map_water_determine_orientation_size3(x, y, 1, 0, 0)) {
            data.draw_as_constructing = 1;
        }
    } else if (data.required_terrain.meadow || data.required_terrain.rock || data.required_terrain.tree ||
        data.required_terrain.water || data.required_terrain.wall || data.required_terrain.distant_water) {
        // never mark as constructing
    } else {
        if (!(type == BUILDING_SENATE_UPGRADED && city_buildings_has_senate()) &&
            !(type == BUILDING_BARRACKS && city_buildings_has_barracks() &&
            !config_get(CONFIG_GP_CH_MULTIPLE_BARRACKS)) &&
            !(type == BUILDING_DISTRIBUTION_CENTER_UNUSED && city_buildings_has_distribution_center()) &&
            !(type == BUILDING_MESS_HALL && city_buildings_has_mess_hall()) &&
            !building_monument_get_id(type) &&
            !(building_monument_is_grand_temple(type) &&
            building_monument_count_grand_temples() >= config_get(CONFIG_GP_CH_MAX_GRAND_TEMPLES))) {
            int size = building_properties_for_type(type)->size;
            mark_construction(x, y, size, TERRAIN_ALL, 0);
        }
    }
    data.cost_preview = current_cost;
}

static figure_type nearby_enemy_type(int x_start, int y_start, int x_end, int y_end)
{
    for (int i = 1; i < figure_count(); i++) {
        figure *f = figure_get(i);
        if (config_get(CONFIG_GP_CH_WOLVES_BLOCK)) {
            if (f->state != FIGURE_STATE_ALIVE || (!figure_is_enemy(f) && f->type != FIGURE_WOLF)) {
                continue;
            }
        } else if (f->state != FIGURE_STATE_ALIVE || !figure_is_enemy(f)) {
            continue;
        }
        int distance = f->type == FIGURE_WOLF ? 6 : 12;
        int dx = (f->x > x_start) ? (f->x - x_start) : (x_start - f->x);
        int dy = (f->y > y_start) ? (f->y - y_start) : (y_start - f->y);
        if (dx <= distance && dy <= distance) {
            return f->type;
        }
        dx = (f->x > x_end) ? (f->x - x_end) : (x_end - f->x);
        dy = (f->y > y_end) ? (f->y - y_end) : (y_end - f->y);
        if (dx <= distance && dy <= distance) {
            return f->type;
        }
    }
    return FIGURE_NONE;
}

void building_construction_offset_start_from_orientation(int *x, int *y, int size)
{
    switch (city_view_orientation()) {
        case DIR_2_RIGHT: *x = *x - size + 1; break;
        case DIR_4_BOTTOM: *x = *x - size + 1; *y = *y - size + 1; break;
        case DIR_6_LEFT: *y = *y - size + 1; break;
    }
}

void building_construction_place(void)
{
    data.cost_preview = 0;
    data.in_progress = 0;
    int x_start = data.start.x;
    int y_start = data.start.y;
    int x_end = data.end.x;
    int y_end = data.end.y;
    building_type type = building_construction_type();
    building_construction_warning_reset();
    if (!type) {
        return;
    }
    if (city_finance_out_of_money()) {
        map_property_clear_constructing_and_deleted();
        city_warning_show(WARNING_OUT_OF_MONEY, NEW_WARNING_SLOT);
        return;
    }

    figure_type enemy_type = nearby_enemy_type(x_start, y_start, x_end, y_end);

    if (type != BUILDING_CLEAR_LAND && enemy_type != FIGURE_NONE) {
        if (type == BUILDING_WALL || type == BUILDING_ROAD || type == BUILDING_AQUEDUCT) {
            game_undo_restore_map(0);
        } else if (type == BUILDING_PLAZA || type == BUILDING_GARDENS || building_is_connectable(type)) {
            game_undo_restore_map(1);
        } else if (type == BUILDING_LOW_BRIDGE || type == BUILDING_SHIP_BRIDGE) {
            map_bridge_reset_building_length();
        } else {
            map_property_clear_constructing_and_deleted();
        }
        city_warning_show(enemy_type == FIGURE_WOLF ? WARNING_WOLF_NEARBY : WARNING_ENEMY_NEARBY, NEW_WARNING_SLOT);
        return;
    }

    int placement_cost = model_get_building(type)->cost;
    if (type == BUILDING_CLEAR_LAND) {
        // BUG in original (keep this behaviour): if confirmation has to be asked (bridge/fort),
        // the previous cost is deducted from treasury and if user chooses 'no', they still pay for removal.
        // If we don't do it this way, the user doesn't pay for the removal at all since we don't come back
        // here when the user says yes.
        int items_placed = building_construction_clear_land(0, x_start, y_start, x_end, y_end);
        if (items_placed < 0) {
            items_placed = last_items_cleared;
        }
        placement_cost *= items_placed;
        map_property_clear_constructing_and_deleted();
    } else if (type == BUILDING_WALL) {
        placement_cost *= building_construction_place_wall(0, x_start, y_start, x_end, y_end);
    } else if (type == BUILDING_ROAD) {
        placement_cost *= building_construction_place_road(0, x_start, y_start, x_end, y_end);
    } else if (type == BUILDING_HIGHWAY) {
        placement_cost *= building_construction_place_highway(0, x_start, y_start, x_end, y_end);
    } else if (type == BUILDING_PLAZA) {
        placement_cost *= place_plaza(x_start, y_start, x_end, y_end);
    } else if (type == BUILDING_GARDENS) {
        placement_cost *= place_garden(x_start, y_start, x_end, y_end);
        map_routing_update_land();
    } else if (type == BUILDING_LOW_BRIDGE) {
        int length = map_bridge_add(x_end, y_end, 0);
        if (length <= 1) {
            city_warning_show(WARNING_SHORE_NEEDED, NEW_WARNING_SLOT);
            return;
        }
        placement_cost *= length;
    } else if (type == BUILDING_SHIP_BRIDGE) {
        int length = map_bridge_add(x_end, y_end, 1);
        if (length <= 1) {
            city_warning_show(WARNING_SHORE_NEEDED, NEW_WARNING_SLOT);
            return;
        }
        placement_cost *= length;
    } else if (type == BUILDING_AQUEDUCT) {
        int cost;
        if (!building_construction_place_aqueduct(x_start, y_start, x_end, y_end, &cost)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED, NEW_WARNING_SLOT);
            return;
        }
        placement_cost = cost;
        map_tiles_update_all_aqueducts(0);
        map_routing_update_land();
    } else if (type == BUILDING_DRAGGABLE_RESERVOIR) {
        struct reservoir_info info;
        if (!place_reservoir_and_aqueducts(0, x_start, y_start, x_end, y_end, &info)) {
            map_property_clear_constructing_and_deleted();
            city_warning_show(WARNING_CLEAR_LAND_NEEDED, NEW_WARNING_SLOT);
            return;
        }
        if (info.place_reservoir_at_start == PLACE_RESERVOIR_YES) {
            building *reservoir = building_create(BUILDING_RESERVOIR, x_start - 1, y_start - 1);
            game_undo_add_building(reservoir);
            map_building_tiles_add(reservoir->id, x_start - 1, y_start - 1, 3,
                image_group(GROUP_BUILDING_RESERVOIR), TERRAIN_BUILDING);
            map_aqueduct_remove(map_grid_offset(x_start - 1, y_start - 1));
        }
        if (info.place_reservoir_at_end == PLACE_RESERVOIR_YES) {
            building *reservoir = building_create(BUILDING_RESERVOIR, x_end - 1, y_end - 1);
            game_undo_add_building(reservoir);
            map_building_tiles_add(reservoir->id, x_end - 1, y_end - 1, 3,
                image_group(GROUP_BUILDING_RESERVOIR), TERRAIN_BUILDING);
            map_aqueduct_remove(map_grid_offset(x_end - 1, y_end - 1));
            if (!map_terrain_exists_tile_in_area_with_type(x_start - 2, y_start - 2, 5, TERRAIN_WATER)
                && info.place_reservoir_at_start == PLACE_RESERVOIR_NO) {
                building_construction_warning_check_reservoir(BUILDING_RESERVOIR);
            }
        }
        placement_cost = info.cost;
        map_tiles_update_all_aqueducts(0);
        map_routing_update_land();
    } else if (type >= BUILDING_PINE_TREE && type <= BUILDING_DATE_TREE) {
        placement_cost *= place_draggable_building(x_start, y_start, x_end, y_end, type, 0);
    } else if (type >= BUILDING_PINE_PATH && type <= BUILDING_DATE_PATH) {
        int rotation = building_rotation_get_rotation_with_limit(BUILDING_CONNECTABLE_ROTATION_LIMIT_PATHS);
        placement_cost *= place_draggable_building(x_start, y_start, x_end, y_end, type, rotation);
    } else if (type >= BUILDING_SMALL_STATUE_ALT && type <= BUILDING_SMALL_STATUE_ALT_B) {
        int rotation = building_rotation_get_rotation();
        placement_cost *= place_draggable_building(x_start, y_start, x_end, y_end, type, rotation % 2);
    } else if (type >= BUILDING_HEDGE_DARK && type <= BUILDING_HEDGE_LIGHT) {
        int rotation = building_rotation_get_rotation_with_limit(BUILDING_CONNECTABLE_ROTATION_LIMIT_HEDGES);
        placement_cost *= place_draggable_building(x_start, y_start, x_end, y_end, type, rotation);
    } else if (type == BUILDING_COLONNADE) {
        int rotation = building_rotation_get_rotation_with_limit(BUILDING_CONNECTABLE_ROTATION_LIMIT_HEDGES);
        placement_cost *= place_draggable_building(x_start, y_start, x_end, y_end, type, rotation);
    } else if (type == BUILDING_GARDEN_PATH) {
        int rotation = building_rotation_get_rotation_with_limit(BUILDING_CONNECTABLE_ROTATION_LIMIT_PATHS);
        placement_cost *= place_draggable_building(x_start, y_start, x_end, y_end, type, rotation);
    } else if (type == BUILDING_GARDEN_WALL) {
        int rotation = building_rotation_get_rotation_with_limit(BUILDING_CONNECTABLE_ROTATION_LIMIT_PATHS);
        placement_cost *= place_draggable_building(x_start, y_start, x_end, y_end, type, rotation);
    } else if (type == BUILDING_ROOFED_GARDEN_WALL) {
        int rotation = building_rotation_get_rotation_with_limit(BUILDING_CONNECTABLE_ROTATION_LIMIT_PATHS);
        placement_cost *= place_draggable_building(x_start, y_start, x_end, y_end, type, rotation);
    } else if (type == BUILDING_DECORATIVE_COLUMN) {
        int variant_numbers = building_variant_get_number_of_variants(type);
        int rotation = building_rotation_get_rotation_with_limit(variant_numbers);
        placement_cost *= place_draggable_building(x_start, y_start, x_end, y_end, type, rotation);
    } else if (type == BUILDING_PALISADE) {
        int rotation = building_rotation_get_rotation_with_limit(BUILDING_CONNECTABLE_ROTATION_LIMIT_HEDGES);
        placement_cost *= place_draggable_building(x_start, y_start, x_end, y_end, type, rotation);
    } else if (type == BUILDING_HOUSE_VACANT_LOT) {
        placement_cost *= place_houses(0, x_start, y_start, x_end, y_end);
    } else if (!building_construction_place_building(type, x_end, y_end)) {
        return;
    }

    if (data.type == BUILDING_MENU_SMALL_TEMPLES || data.type == BUILDING_MENU_LARGE_TEMPLES) {
        building_rotation_rotate_forward();
    }
    formation_move_herds_away(x_end, y_end);
    city_finance_process_construction(placement_cost);
    game_undo_finish_build(placement_cost);
}

static void set_warning(int *warning_id, int warning)
{
    if (warning_id) {
        *warning_id = warning;
    }
}

int building_construction_can_place_on_terrain(int x, int y, int *warning_id)
{
    if (data.required_terrain.meadow) {
        if (!map_terrain_exists_tile_in_radius_with_type(x, y, 3, 1, TERRAIN_MEADOW)) {
            set_warning(warning_id, WARNING_MEADOW_NEEDED);
            return 0;
        }
    } else if (data.required_terrain.rock) {
        if (!map_terrain_exists_rock_in_radius(x, y, 2, 1)) {
            set_warning(warning_id, WARNING_ROCK_NEEDED);
            return 0;
        }
    } else if (data.required_terrain.tree) {
        if (!map_terrain_exists_tile_in_radius_with_type(x, y, 2, 1, TERRAIN_SHRUB | TERRAIN_TREE)) {
            set_warning(warning_id, WARNING_TREE_NEEDED);
            return 0;
        }
    } else if (data.required_terrain.water) {
        if (!map_terrain_exists_tile_in_radius_with_type(x, y, 2, 3, TERRAIN_WATER)) {
            set_warning(warning_id, WARNING_WATER_NEEDED);
            return 0;
        }
    } else if (data.required_terrain.wall) {
        if (!map_terrain_all_tiles_in_radius_are(x, y, 2, 0, TERRAIN_WALL)) {
            set_warning(warning_id, WARNING_WALL_NEEDED);
            return 0;
        }
    } else if (data.required_terrain.distant_water) {
        if (!map_terrain_exists_tile_in_radius_with_type(x, y, 3, 9, TERRAIN_WATER)) {
            set_warning(warning_id, WARNING_WATER_NEEDED_FOR_LIGHTHOUSE);
            return 0;
        }
    }
    return 1;
}

void building_construction_record_view_position(int view_x, int view_y, int grid_offset)
{
    if (grid_offset == data.start.grid_offset) {
        data.start_offset_x_view = view_x;
        data.start_offset_y_view = view_y;
    }
}

void building_construction_get_view_position(int *view_x, int *view_y)
{
    *view_x = data.start_offset_x_view;
    *view_y = data.start_offset_y_view;
}

int building_construction_get_start_grid_offset(void)
{
    return data.start.grid_offset;
}

void building_construction_reset_draw_as_constructing(void)
{
    data.draw_as_constructing = 0;
}

int building_construction_draw_as_constructing(void)
{
    return data.draw_as_constructing;
}
