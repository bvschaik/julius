#include "desirability.h"

#include "building/building.h"
#include "building/model.h"
#include "building/monument.h"
#include "core/calc.h"
#include "map/data.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/ring.h"
#include "map/terrain.h"

static grid_i8 desirability_grid;

void map_desirability_clear(void)
{
    map_grid_clear_i8(desirability_grid.items);
}

static void add_desirability_at_distance(int x, int y, int size, int distance, int desirability)
{
    int partially_outside_map = 0;
    if (x - distance < -1 || x + distance + size - 1 > map_data.width) {
        partially_outside_map = 1;
    }
    if (y - distance < -1 || y + distance + size - 1 > map_data.height) {
        partially_outside_map = 1;
    }
    int base_offset = map_grid_offset(x, y);
    int start = map_ring_start(size, distance);
    int end = map_ring_end(size, distance);

    if (partially_outside_map) {
        for (int i = start; i < end; i++) {
            const ring_tile *tile = map_ring_tile(i);
            if (map_ring_is_inside_map(x + tile->x, y + tile->y)) {
                desirability_grid.items[base_offset + tile->grid_offset] =
                    calc_bound(desirability_grid.items[base_offset + tile->grid_offset] + desirability, -100, 100);
            }
        }
    } else {
        for (int i = start; i < end; i++) {
            const ring_tile *tile = map_ring_tile(i);
            desirability_grid.items[base_offset + tile->grid_offset] =
                calc_bound(desirability_grid.items[base_offset + tile->grid_offset] + desirability, -100, 100);
        }
    }
}

static void add_to_terrain(int x, int y, int size, int desirability, int step, int step_size, int range)
{
    if (size > 0) {
        if (range > 6) {
            range = 6;
        }
        int tiles_within_step = 0;
        int distance = 1;
        while (range > 0) {
            add_desirability_at_distance(x, y, size, distance, desirability);
            distance++;
            range--;
            tiles_within_step++;
            if (tiles_within_step >= step) {
                desirability += step_size;
                tiles_within_step = 0;
            }
        }
    }
}

static void update_buildings(void)
{
    int value;
    int value_bonus;
    int step;
    int step_size;
    int range;
    int venus_module2 = building_monument_gt_module_is_active(VENUS_MODULE_2_DESIRABILITY_ENTERTAINMENT);
    int venus_gt = building_monument_working(BUILDING_GRAND_TEMPLE_VENUS);
    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE) {

            const model_building *model = model_get_building(b->type);
            value = model->desirability_value;
            step = model->desirability_step;
            step_size = model->desirability_step_size;
            range = model->desirability_range;

            // Venus Module 2 House Desirability Bonus
            if (building_is_house(b->type) && b->data.house.temple_venus && venus_module2) {
                if (b->subtype.house_level >= HOUSE_SMALL_VILLA) {
                    value += 4;
                    range += 1;
                } else if (b->subtype.house_level <= HOUSE_LARGE_TENT) {
                    // tents normally confer -3, -2, -1, 0, 0, 0 (range=3)
                    // now this becomes -1, 0, 0, 0, 0, 0 (range=1)
                    value += 2;
                    range = 1;
                } else {
                    value += 2;
                }
            }

            if (building_monument_is_monument(b) && b->data.monument.phase != MONUMENT_FINISHED) {
                value = 0;
                step = 0;
                step_size = 0;
                range = 0;
            }

            // Venus GT Base Bonus
            if (building_is_statue_garden_temple(b->type) && venus_gt) {
                value_bonus = ((value / 4) > 1) ? (value / 4) : 1;
                value += value_bonus;
                step += 1;
                range += 1;
            }

            add_to_terrain(
                b->x, b->y, b->size,
                value,
                step,
                step_size,
                range);
        }
    }
}

static void update_terrain(void)
{
    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            int terrain = map_terrain_get(grid_offset);
            if (map_property_is_plaza_or_earthquake(grid_offset)) {
                int type;
                if (terrain & TERRAIN_ROAD) {
                    type = BUILDING_PLAZA;
                } else if (terrain & TERRAIN_ROCK) {
                    // earthquake fault line: slight negative
                    type = BUILDING_HOUSE_VACANT_LOT;
                } else {
                    // invalid plaza/earthquake flag
                    map_property_clear_plaza_or_earthquake(grid_offset);
                    continue;
                }
                const model_building *model = model_get_building(type);
                add_to_terrain(x, y, 1,
                    model->desirability_value,
                    model->desirability_step,
                    model->desirability_step_size,
                    model->desirability_range);
            } else if (terrain & TERRAIN_GARDEN) {
                const model_building *model = model_get_building(BUILDING_GARDENS);
                add_to_terrain(x, y, 1,
                    model->desirability_value,
                    model->desirability_step,
                    model->desirability_step_size,
                    model->desirability_range);
            } else if (terrain & TERRAIN_RUBBLE) {
                add_to_terrain(x, y, 1, -2, 1, 1, 2);
            } else if (terrain & TERRAIN_HIGHWAY) {
                const model_building *model = model_get_building(BUILDING_HIGHWAY);
                add_to_terrain(x, y, 1,
                    model->desirability_value,
                    model->desirability_step,
                    model->desirability_step_size,
                    model->desirability_range);
            }
        }
    }
}

void map_desirability_update(void)
{
    map_desirability_clear();
    update_buildings();
    update_terrain();
}

int map_desirability_get(int grid_offset)
{
    return desirability_grid.items[grid_offset];
}

int map_desirability_get_max(int x, int y, int size)
{
    if (size == 1) {
        return desirability_grid.items[map_grid_offset(x, y)];
    }
    int max = -9999;
    for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
            int grid_offset = map_grid_offset(x + dx, y + dy);
            if (desirability_grid.items[grid_offset] > max) {
                max = desirability_grid.items[grid_offset];
            }
        }
    }
    return max;
}

void map_desirability_save_state(buffer *buf)
{
    map_grid_save_state_i8(desirability_grid.items, buf);
}

void map_desirability_load_state(buffer *buf)
{
    map_grid_load_state_i8(desirability_grid.items, buf);
}
