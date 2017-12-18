#include "water_supply.h"

#include "building/building.h"
#include "building/list.h"
#include "graphics/image.h"
#include "map/aqueduct.h"
#include "map/desirability.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/terrain.h"
#include "scenario/property.h"

#include "../Terrain.h"

#include "Data/State.h"

#include <string.h>

#define MAX_QUEUE 1000

static const int ADJACENT_OFFSETS[] = {-GRID_SIZE, 1, GRID_SIZE, -1};

static struct {
    int items[MAX_QUEUE];
    int head;
    int tail;
} queue;

void map_water_supply_update_houses()
{
    building_list_small_clear();
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b)) {
            continue;
        }
        if (b->type == BUILDING_WELL) {
            building_list_small_add(i);
        } else if (b->houseSize) {
            b->hasWaterAccess = 0;
            b->hasWellAccess = 0;
            if (Terrain_existsTileWithinAreaWithType(
                b->x, b->y, b->size, TERRAIN_FOUNTAIN_RANGE)) {
                b->hasWaterAccess = 1;
            }
        }
    }
    int total_wells = building_list_small_size();
    const int *wells = building_list_small_items();
    for (int i = 0; i < total_wells; i++) {
        Terrain_markBuildingsWithinWellRadius(wells[i], 2);
    }
}

static void set_all_aqueducts_to_no_water()
{
    int image_without_water = image_group(GROUP_BUILDING_AQUEDUCT) + 15;
    for (int y = 0; y < Data_State.map.height; y++) {
        for (int x = 0; x < Data_State.map.width; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
                map_aqueduct_set(grid_offset, 0);
                int image_id = map_image_at(grid_offset);
                if (image_id < image_without_water) {
                    map_image_set(grid_offset, image_id + 15);
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
    int image_without_water = image_group(GROUP_BUILDING_AQUEDUCT) + 15;
    do {
        if (++guard >= GRID_SIZE * GRID_SIZE) {
            break;
        }
        map_aqueduct_set(grid_offset, 1);
        int image_id = map_image_at(grid_offset);
        if (image_id >= image_without_water) {
            map_image_set(grid_offset, image_id - 15);
        }
        next_offset = -1;
        for (int i = 0; i < 4; i++) {
            int new_offset = grid_offset + ADJACENT_OFFSETS[i];
            building *b = building_get(map_building_at(new_offset));
            if (b->id && b->type == BUILDING_RESERVOIR) {
                // check if aqueduct connects to reservoir --> doesn't connect to corner
                int xy = map_property_multi_tile_xy(new_offset);
                if (xy != Edge_X0Y0 && xy != Edge_X2Y0 && xy != Edge_X0Y2 && xy != Edge_X2Y2) {
                    if (!b->hasWaterAccess) {
                        b->hasWaterAccess = 2;
                    }
                }
            } else if (map_terrain_is(new_offset, TERRAIN_AQUEDUCT)) {
                if (!map_aqueduct_at(new_offset)) {
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

void map_water_supply_update_reservoir_fountain()
{
    map_terrain_remove_all(TERRAIN_FOUNTAIN_RANGE | TERRAIN_RESERVOIR_RANGE);
    // reservoirs
    set_all_aqueducts_to_no_water();
    building_list_large_clear(1);
    // mark reservoirs next to water
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (BuildingIsInUse(b) && b->type == BUILDING_RESERVOIR) {
            building_list_large_add(i);
            if (Terrain_existsTileWithinAreaWithType(b->x - 1, b->y - 1, 5, TERRAIN_WATER)) {
                b->hasWaterAccess = 2;
            } else {
                b->hasWaterAccess = 0;
            }
        }
    }
    int total_reservoirs = building_list_large_size();
    const int *reservoirs = building_list_large_items();
    // fill reservoirs from full ones
    int changed = 1;
    static const int CONNECTOR_OFFSETS[] = {-161, 165, 487, 161};
    while (changed == 1) {
        changed = 0;
        for (int i = 0; i < total_reservoirs; i++) {
            building *b = building_get(reservoirs[i]);
            if (b->hasWaterAccess == 2) {
                b->hasWaterAccess = 1;
                changed = 1;
                for (int d = 0; d < 4; d++) {
                    fill_aqueducts_from_offset(b->gridOffset + CONNECTOR_OFFSETS[d]);
                }
            }
        }
    }
    // mark reservoir ranges
    for (int i = 0; i < total_reservoirs; i++) {
        building *b = building_get(reservoirs[i]);
        if (b->hasWaterAccess) {
            Terrain_setWithRadius(b->x, b->y, 3, 10, TERRAIN_RESERVOIR_RANGE);
        }
    }
    // fountains
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || b->type != BUILDING_FOUNTAIN) {
            continue;
        }
        int des = map_desirability_get(b->gridOffset);
        int image_id;
        if (des > 60) {
            image_id = image_group(GROUP_BUILDING_FOUNTAIN_4);
        } else if (des > 40) {
            image_id = image_group(GROUP_BUILDING_FOUNTAIN_3);
        } else if (des > 20) {
            image_id = image_group(GROUP_BUILDING_FOUNTAIN_2);
        } else {
            image_id = image_group(GROUP_BUILDING_FOUNTAIN_1);
        }
        Terrain_addBuildingToGrids(i, b->x, b->y, 1, image_id, TERRAIN_BUILDING);
        if (map_terrain_is(b->gridOffset, TERRAIN_RESERVOIR_RANGE) && b->numWorkers) {
            b->hasWaterAccess = 1;
            Terrain_setWithRadius(b->x, b->y, 1,
                scenario_property_climate() == CLIMATE_DESERT ? 3 : 4,
                TERRAIN_FOUNTAIN_RANGE);
        } else {
            b->hasWaterAccess = 0;
        }
    }
}
