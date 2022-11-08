#include "water_supply.h"

#include "assets/assets.h"
#include "building/building.h"
#include "building/image.h"
#include "building/monument.h"
#include "building/list.h"
#include "core/image.h"
#include "map/aqueduct.h"
#include "map/building_tiles.h"
#include "map/data.h"
#include "map/desirability.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "scenario/property.h"

#include <string.h>

#define OFFSET(x,y) (x + GRID_SIZE * y)

#define MAX_QUEUE 1000
#define RESERVOIR_RADIUS 10
#define WELL_RADIUS 2
#define FOUNTAIN_RADIUS 4

static const int ADJACENT_OFFSETS[] = { -GRID_SIZE, 1, GRID_SIZE, -1 };

static struct {
    int items[MAX_QUEUE];
    int head;
    int tail;
} queue;

static void mark_well_access(int well_id, int radius)
{
    building *well = building_get(well_id);
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(well->x, well->y, 1, radius, &x_min, &y_min, &x_max, &y_max);

    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int building_id = map_building_at(map_grid_offset(xx, yy));
            if (building_id) {
                building_get(building_id)->has_well_access = 1;
            }
        }
    }
}

void map_water_supply_update_houses(void)
{
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
                continue;
            }
            b->has_water_access = 0;
            b->has_well_access = 0;
            if (map_terrain_exists_tile_in_area_with_type(
                b->x, b->y, b->size, TERRAIN_FOUNTAIN_RANGE)) {
                b->has_water_access = 1;
            }
        }
    }
    for (building *b = building_first_of_type(BUILDING_WELL); b; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE) {
            mark_well_access(b->id, map_water_supply_well_radius());
        }
    }
}

static void set_all_aqueducts_to_no_water(void)
{
    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
                map_aqueduct_set_water_access(grid_offset, 0);
                int image_id = map_image_at(grid_offset);
                if (image_id < image_group(GROUP_BUILDING_AQUEDUCT_NO_WATER)) {
                    map_image_set(grid_offset, image_id + 15);
                } else if(map_terrain_is(grid_offset, TERRAIN_HIGHWAY)) {
                    image_id = map_tiles_highway_get_aqueduct_image(grid_offset);
                    map_image_set(grid_offset, image_id);
                }
            }
        }
    }
}

static void fill_aqueducts_from_offset(int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
        return;
    }
    memset(&queue, 0, sizeof(queue));
    int guard = 0;
    int next_offset;

    do {
        if (++guard >= GRID_SIZE * GRID_SIZE) {
            break;
        }
        map_aqueduct_set_water_access(grid_offset, 1);
        int image_id = map_image_at(grid_offset);
        if (map_terrain_is(grid_offset, TERRAIN_HIGHWAY)) {
            image_id = map_tiles_highway_get_aqueduct_image(grid_offset);
            map_image_set(grid_offset, image_id);
        } else if (image_id >= image_group(GROUP_BUILDING_AQUEDUCT_NO_WATER)) {
            map_image_set(grid_offset, image_id - 15);
        }
        next_offset = -1;
        for (int i = 0; i < 4; i++) {
            int new_offset = grid_offset + ADJACENT_OFFSETS[i];
            building *b = building_get(map_building_at(new_offset));
            if (b->id && b->type == BUILDING_RESERVOIR) {
                // check if aqueduct connects to reservoir --> doesn't connect to corner
                int xy = map_property_multi_tile_xy(new_offset);
                if (xy != EDGE_X0Y0 && xy != EDGE_X2Y0 && xy != EDGE_X0Y2 && xy != EDGE_X2Y2) {
                    if (!b->has_water_access) {
                        b->has_water_access = 2;
                    }
                }
            } else if (map_terrain_is(new_offset, TERRAIN_AQUEDUCT)) {
                if (!map_aqueduct_has_water_access_at(new_offset)) {
                    if (next_offset == -1) {
                        next_offset = new_offset;
                    } else {
                        queue.items[queue.tail++] = new_offset;
                        if (queue.tail >= MAX_QUEUE) {
                            queue.tail = 0;
                        }
                    }
                }
            }
        }
        if (next_offset == -1) {
            if (queue.head == queue.tail) {
                return;
            }
            next_offset = queue.items[queue.head++];
            if (queue.head >= MAX_QUEUE) {
                queue.head = 0;
            }
        }
        grid_offset = next_offset;
    } while (next_offset > -1);
}

void map_water_supply_update_reservoir_fountain(void)
{
    map_terrain_remove_all(TERRAIN_FOUNTAIN_RANGE | TERRAIN_RESERVOIR_RANGE);
    // reservoirs
    set_all_aqueducts_to_no_water();
    for (building *b = building_first_of_type(BUILDING_RESERVOIR); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (map_terrain_exists_tile_in_area_with_type(b->x - 1, b->y - 1, 5, TERRAIN_WATER)) {
            b->has_water_access = 2;
        } else {
            b->has_water_access = 0;
        }
    }
    // fill reservoirs from full ones
    int changed = 1;
    static const int CONNECTOR_OFFSETS[] = { OFFSET(1,-1), OFFSET(3,1), OFFSET(1,3), OFFSET(-1,1) };
    while (changed == 1) {
        changed = 0;
        for (building *b = building_first_of_type(BUILDING_RESERVOIR); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE) {
                continue;
            }
            if (b->has_water_access == 2) {
                b->has_water_access = 1;
                changed = 1;
                for (int d = 0; d < 4; d++) {
                    fill_aqueducts_from_offset(b->grid_offset + CONNECTOR_OFFSETS[d]);
                }
            }
        }
    }
    // mark reservoir ranges
    for (building *b = building_first_of_type(BUILDING_RESERVOIR); b; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE && b->has_water_access) {
            map_terrain_add_with_radius(b->x, b->y, 3, map_water_supply_reservoir_radius(), TERRAIN_RESERVOIR_RANGE);
        }
    }

    // Neptune GT module 2 bonus
    if (building_monument_gt_module_is_active(NEPTUNE_MODULE_2_CAPACITY_AND_WATER)) {
        building *b = building_get(building_monument_get_neptune_gt());
        map_terrain_add_with_radius(b->x, b->y, 7, map_water_supply_reservoir_radius(), TERRAIN_RESERVOIR_RANGE);
    }

    // fountains
    for (building *b = building_first_of_type(BUILDING_FOUNTAIN); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->desirability > 60) {
            b->upgrade_level = 3;
        } else if (b->desirability > 40) {
            b->upgrade_level = 2;
        } else if (b->desirability > 20) {
            b->upgrade_level = 1;
        } else {
            b->upgrade_level = 0;
        }
        map_building_tiles_add(b->id, b->x, b->y, 1, building_image_get(b), TERRAIN_BUILDING);
        if (map_terrain_is(b->grid_offset, TERRAIN_RESERVOIR_RANGE) && b->num_workers) {
            b->has_water_access = 1;
            map_terrain_add_with_radius(b->x, b->y, 1,
                map_water_supply_fountain_radius(), TERRAIN_FOUNTAIN_RANGE);
        } else {
            b->has_water_access = 0;
        }
    }
    // Ponds
    static const building_type ponds[] = { BUILDING_SMALL_POND, BUILDING_LARGE_POND };
    for (int i = 0; i < 2; i++) {
        for (building *b = building_first_of_type(ponds[i]); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE) {
                continue;
            }
            if (map_terrain_exists_tile_in_area_with_type(b->x, b->y, b->size, TERRAIN_RESERVOIR_RANGE)) {
                b->has_water_access = 1;
            } else {
                b->has_water_access = 0;
            }
            map_building_tiles_add(b->id, b->x, b->y, b->size, building_image_get(b), TERRAIN_BUILDING);
        }
    }

    // Large statues, to check if they should play the animation
    for (building *b = building_first_of_type(BUILDING_LARGE_STATUE); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (map_terrain_exists_tile_in_area_with_type(b->x, b->y, b->size, TERRAIN_RESERVOIR_RANGE)) {
            b->has_water_access = 1;
        } else {
            b->has_water_access = 0;
        }
    }

    for (building *b = building_first_of_type(BUILDING_COLOSSEUM); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (map_terrain_exists_tile_in_area_with_type(b->x, b->y, b->size, TERRAIN_RESERVOIR_RANGE)) {
            b->has_water_access = 1;
        } else {
            b->has_water_access = 0;
        }
    }
}

int map_water_supply_is_well_unnecessary(int well_id, int radius)
{
    building *well = building_get(well_id);
    int num_houses = 0;
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(well->x, well->y, 1, radius, &x_min, &y_min, &x_max, &y_max);

    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int grid_offset = map_grid_offset(xx, yy);
            int building_id = map_building_at(grid_offset);
            if (building_id && building_get(building_id)->house_size) {
                num_houses++;
                if (!map_terrain_is(grid_offset, TERRAIN_FOUNTAIN_RANGE)) {
                    return WELL_NECESSARY;
                }
            }
        }
    }
    return num_houses ? WELL_UNNECESSARY_FOUNTAIN : WELL_UNNECESSARY_NO_HOUSES;
}

int map_water_supply_fountain_radius(void)
{
    int radius = scenario_property_climate() == CLIMATE_DESERT ? FOUNTAIN_RADIUS - 1 : FOUNTAIN_RADIUS;
    if (building_monument_working(BUILDING_GRAND_TEMPLE_NEPTUNE)) {
        radius++;
    }

    return radius;
}

int map_water_supply_reservoir_radius(void)
{
    int radius = RESERVOIR_RADIUS;
    if (building_monument_working(BUILDING_GRAND_TEMPLE_NEPTUNE)) {
        radius += 2;
    }

    return radius;
}

int map_water_supply_well_radius(void)
{
    int radius = WELL_RADIUS;
    if (building_monument_working(BUILDING_GRAND_TEMPLE_NEPTUNE)) {
        radius++;
    }

    return radius;
}
