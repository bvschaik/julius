#include "house.h"

#include "core/config.h"
#include "core/image.h"
#include "game/resource.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/random.h"
#include "map/terrain.h"

#define MAX_DIR 4

#define OFFSET(x,y) (x + GRID_SIZE * y)

static const int HOUSE_TILE_OFFSETS[] = {
    OFFSET(0,0), OFFSET(1,0), OFFSET(0,1), OFFSET(1,1), // 2x2
    OFFSET(2,0), OFFSET(2,1), OFFSET(2,2), OFFSET(1,2), OFFSET(0,2), // 3x3
    OFFSET(3,0), OFFSET(3,1), OFFSET(3,2), OFFSET(3,3), OFFSET(2,3), OFFSET(1,3), OFFSET(0,3) // 4x4
};

static const struct {
    int group;
    int offset;
    int num_types;
} HOUSE_IMAGE[20] = {
    {GROUP_BUILDING_HOUSE_TENT, 0, 2}, {GROUP_BUILDING_HOUSE_TENT, 2, 2},
    {GROUP_BUILDING_HOUSE_SHACK, 0, 2}, {GROUP_BUILDING_HOUSE_SHACK, 2, 2},
    {GROUP_BUILDING_HOUSE_HOVEL, 0, 2}, {GROUP_BUILDING_HOUSE_HOVEL, 2, 2},
    {GROUP_BUILDING_HOUSE_CASA, 0, 2}, {GROUP_BUILDING_HOUSE_CASA, 2, 2},
    {GROUP_BUILDING_HOUSE_INSULA_1, 0, 2}, {GROUP_BUILDING_HOUSE_INSULA_1, 2, 2},
    {GROUP_BUILDING_HOUSE_INSULA_2, 0, 2}, {GROUP_BUILDING_HOUSE_INSULA_2, 2, 2},
    {GROUP_BUILDING_HOUSE_VILLA_1, 0, 2}, {GROUP_BUILDING_HOUSE_VILLA_1, 2, 2},
    {GROUP_BUILDING_HOUSE_VILLA_2, 0, 1}, {GROUP_BUILDING_HOUSE_VILLA_2, 1, 1},
    {GROUP_BUILDING_HOUSE_PALACE_1, 0, 1}, {GROUP_BUILDING_HOUSE_PALACE_1, 1, 1},
    {GROUP_BUILDING_HOUSE_PALACE_2, 0, 1}, {GROUP_BUILDING_HOUSE_PALACE_2, 1, 1},
};

static const struct {
    int x;
    int y;
    int offset;
} EXPAND_DIRECTION_DELTA[MAX_DIR] = {{0, 0, 0}, {-1, -1, -GRID_SIZE - 1}, {-1, 0, -1}, {0, -1, -GRID_SIZE}};

static struct {
    int x;
    int y;
    int inventory[INVENTORY_MAX];
    int population;
} merge_data;

void building_house_change_to(building *house, building_type type)
{
    house->type = type;
    house->subtype.house_level = house->type - BUILDING_HOUSE_VACANT_LOT;
    int image_id = image_group(HOUSE_IMAGE[house->subtype.house_level].group);
    if (house->house_is_merged) {
        image_id += 4;
        if (HOUSE_IMAGE[house->subtype.house_level].offset) {
            image_id += 1;
        }
    } else {
        image_id += HOUSE_IMAGE[house->subtype.house_level].offset;
        image_id += map_random_get(house->grid_offset) & (HOUSE_IMAGE[house->subtype.house_level].num_types - 1);
    }
    map_building_tiles_add(house->id, house->x, house->y, house->size, image_id, TERRAIN_BUILDING);
}

static void create_vacant_lot(int x, int y, int image_id)
{
    building *b = building_create(BUILDING_HOUSE_VACANT_LOT, x, y);
    b->house_population = 0;
    b->distance_from_entry = 0;
    map_building_tiles_add(b->id, b->x, b->y, 1, image_id, TERRAIN_BUILDING);
}

void building_house_change_to_vacant_lot(building *house)
{
    house->type = BUILDING_HOUSE_VACANT_LOT;
    house->subtype.house_level = house->type - BUILDING_HOUSE_VACANT_LOT;
    int image_id = image_group(GROUP_BUILDING_HOUSE_VACANT_LOT);
    if (house->house_is_merged) {
        map_building_tiles_remove(house->id, house->x, house->y);
        house->house_is_merged = 0;
        house->size = house->house_size = 1;
        map_building_tiles_add(house->id, house->x, house->y, 1, image_id, TERRAIN_BUILDING);

        create_vacant_lot(house->x + 1, house->y, image_id);
        create_vacant_lot(house->x, house->y + 1, image_id);
        create_vacant_lot(house->x + 1, house->y + 1, image_id);
    } else {
        map_image_set(house->grid_offset, image_id);
    }
}

static void prepare_for_merge(int building_id, int num_tiles)
{
    for (int i = 0; i < INVENTORY_MAX; i++) {
        merge_data.inventory[i] = 0;
    }
    merge_data.population = 0;
    int grid_offset = map_grid_offset(merge_data.x, merge_data.y);
    for (int i = 0; i < num_tiles; i++) {
        int house_offset = grid_offset + HOUSE_TILE_OFFSETS[i];
        if (map_terrain_is(house_offset, TERRAIN_BUILDING)) {
            building *house = building_get(map_building_at(house_offset));
            if (house->id != building_id && house->house_size) {
                merge_data.population += house->house_population;
                for (int inv = 0; inv < INVENTORY_MAX; inv++) {
                    merge_data.inventory[inv] += house->data.house.inventory[inv];
                    house->house_population = 0;
                    house->state = BUILDING_STATE_DELETED_BY_GAME;
                }
            }
        }
    }
}

static void merge(building *b)
{
    prepare_for_merge(b->id, 4);

    b->size = b->house_size = 2;
    b->house_population += merge_data.population;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        b->data.house.inventory[i] += merge_data.inventory[i];
    }
    int image_id = image_group(HOUSE_IMAGE[b->subtype.house_level].group) + 4;
    if (HOUSE_IMAGE[b->subtype.house_level].offset) {
        image_id += 1;
    }

    map_building_tiles_remove(b->id, b->x, b->y);
    b->x = merge_data.x;
    b->y = merge_data.y;
    b->grid_offset = map_grid_offset(b->x, b->y);
    b->house_is_merged = 1;
    map_building_tiles_add(b->id, b->x, b->y, 2, image_id, TERRAIN_BUILDING);
}

void building_house_merge(building *house)
{
    if (house->house_is_merged) {
        return;
    }
    if (!config_get(CONFIG_GP_CH_ALL_HOUSES_MERGE)) {    
        if ((map_random_get(house->grid_offset) & 7) >= 5) {
            return;
        }
    }
    int num_house_tiles = 0;
    for (int i = 0; i < 4; i++) {
        int tile_offset = house->grid_offset + HOUSE_TILE_OFFSETS[i];
        if (map_terrain_is(tile_offset, TERRAIN_BUILDING)) {
            building *other_house = building_get(map_building_at(tile_offset));
            if (other_house->id == house->id) {
                num_house_tiles++;
            } else if (other_house->state == BUILDING_STATE_IN_USE && other_house->house_size &&
                    other_house->subtype.house_level == house->subtype.house_level &&
                    !other_house->house_is_merged) {
                num_house_tiles++;
            }
        }
    }
    if (num_house_tiles == 4) {
        merge_data.x = house->x + EXPAND_DIRECTION_DELTA[0].x;
        merge_data.y = house->y + EXPAND_DIRECTION_DELTA[0].y;
        merge(house);
    }
}

int building_house_can_expand(building *house, int num_tiles)
{
    // merge with other houses
    for (int dir = 0; dir < MAX_DIR; dir++) {
        int base_offset = EXPAND_DIRECTION_DELTA[dir].offset + house->grid_offset;
        int ok_tiles = 0;
        for (int i = 0; i < num_tiles; i++) {
            int tile_offset = base_offset + HOUSE_TILE_OFFSETS[i];
            if (map_terrain_is(tile_offset, TERRAIN_BUILDING)) {
                building *other_house = building_get(map_building_at(tile_offset));
                if (other_house->id == house->id) {
                    ok_tiles++;
                } else if (other_house->state == BUILDING_STATE_IN_USE && other_house->house_size) {
                    if (other_house->subtype.house_level <= house->subtype.house_level) {
                        ok_tiles++;
                    }
                }
            }
        }
        if (ok_tiles == num_tiles) {
            merge_data.x = house->x + EXPAND_DIRECTION_DELTA[dir].x;
            merge_data.y = house->y + EXPAND_DIRECTION_DELTA[dir].y;
            return 1;
        }
    }
    // merge with houses and empty terrain
    for (int dir = 0; dir < MAX_DIR; dir++) {
        int base_offset = EXPAND_DIRECTION_DELTA[dir].offset + house->grid_offset;
        int ok_tiles = 0;
        for (int i = 0; i < num_tiles; i++) {
            int tile_offset = base_offset + HOUSE_TILE_OFFSETS[i];
            if (!map_terrain_is(tile_offset, TERRAIN_NOT_CLEAR)) {
                ok_tiles++;
            } else if (map_terrain_is(tile_offset, TERRAIN_BUILDING)) {
                building *other_house = building_get(map_building_at(tile_offset));
                if (other_house->id == house->id) {
                    ok_tiles++;
                } else if (other_house->state == BUILDING_STATE_IN_USE && other_house->house_size) {
                    if (other_house->subtype.house_level <= house->subtype.house_level) {
                        ok_tiles++;
                    }
                }
            }
        }
        if (ok_tiles == num_tiles) {
            merge_data.x = house->x + EXPAND_DIRECTION_DELTA[dir].x;
            merge_data.y = house->y + EXPAND_DIRECTION_DELTA[dir].y;
            return 1;
        }
    }
    // merge with houses, empty terrain and gardens
    for (int dir = 0; dir < MAX_DIR; dir++) {
        int base_offset = EXPAND_DIRECTION_DELTA[dir].offset + house->grid_offset;
        int ok_tiles = 0;
        for (int i = 0; i < num_tiles; i++) {
            int tile_offset = base_offset + HOUSE_TILE_OFFSETS[i];
            if (!map_terrain_is(tile_offset, TERRAIN_NOT_CLEAR)) {
                ok_tiles++;
            } else if (map_terrain_is(tile_offset, TERRAIN_BUILDING)) {
                building *other_house = building_get(map_building_at(tile_offset));
                if (other_house->id == house->id) {
                    ok_tiles++;
                } else if (other_house->state == BUILDING_STATE_IN_USE && other_house->house_size) {
                    if (other_house->subtype.house_level <= house->subtype.house_level) {
                        ok_tiles++;
                    }
                }
            } else if (map_terrain_is(tile_offset, TERRAIN_GARDEN)) {
                ok_tiles++;
            }
        }
        if (ok_tiles == num_tiles) {
            merge_data.x = house->x + EXPAND_DIRECTION_DELTA[dir].x;
            merge_data.y = house->y + EXPAND_DIRECTION_DELTA[dir].y;
            return 1;
        }
    }
    house->data.house.no_space_to_expand = 1;
    return 0;
}

static int house_image_group(int level)
{
    return image_group(HOUSE_IMAGE[level].group) + HOUSE_IMAGE[level].offset;
}

static void create_house_tile(building_type type, int x, int y, int image_id, int population, const int *inventory)
{
    building *house = building_create(type, x, y);
    house->house_population = population;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] = inventory[i];
    }
    house->distance_from_entry = 0;
    map_building_tiles_add(house->id, house->x, house->y, 1,
                           image_id + (map_random_get(house->grid_offset) & 1), TERRAIN_BUILDING);
}

static void split_size2(building *house, building_type new_type)
{
    int inventory_per_tile[INVENTORY_MAX];
    int inventory_remainder[INVENTORY_MAX];
    for (int i = 0; i < INVENTORY_MAX; i++) {
        inventory_per_tile[i] = house->data.house.inventory[i] / 4;
        inventory_remainder[i] = house->data.house.inventory[i] % 4;
    }
    int population_per_tile = house->house_population / 4;
    int population_remainder = house->house_population % 4;

    map_building_tiles_remove(house->id, house->x, house->y);

    // main tile
    house->type = new_type;
    house->subtype.house_level = house->type - BUILDING_HOUSE_VACANT_LOT;
    house->size = house->house_size = 1;
    house->house_is_merged = 0;
    house->house_population = population_per_tile + population_remainder;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] = inventory_per_tile[i] + inventory_remainder[i];
    }
    house->distance_from_entry = 0;

    int image_id = house_image_group(house->subtype.house_level);
    map_building_tiles_add(house->id, house->x, house->y, house->size,
                           image_id + (map_random_get(house->grid_offset) & 1), TERRAIN_BUILDING);

    // the other tiles (new buildings)
    create_house_tile(house->type, house->x + 1, house->y, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(house->type, house->x, house->y + 1, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(house->type, house->x + 1, house->y + 1, image_id, population_per_tile, inventory_per_tile);
}

static void split_size3(building *house)
{
    int inventory_per_tile[INVENTORY_MAX];
    int inventory_remainder[INVENTORY_MAX];
    for (int i = 0; i < INVENTORY_MAX; i++) {
        inventory_per_tile[i] = house->data.house.inventory[i] / 9;
        inventory_remainder[i] = house->data.house.inventory[i] % 9;
    }
    int population_per_tile = house->house_population / 9;
    int population_remainder = house->house_population % 9;

    map_building_tiles_remove(house->id, house->x, house->y);

    // main tile
    house->type = BUILDING_HOUSE_MEDIUM_INSULA;
    house->subtype.house_level = house->type - BUILDING_HOUSE_VACANT_LOT;
    house->size = house->house_size = 1;
    house->house_is_merged = 0;
    house->house_population = population_per_tile + population_remainder;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] = inventory_per_tile[i] + inventory_remainder[i];
    }
    house->distance_from_entry = 0;

    int image_id = house_image_group(house->subtype.house_level);
    map_building_tiles_add(house->id, house->x, house->y, house->size,
                           image_id + (map_random_get(house->grid_offset) & 1), TERRAIN_BUILDING);

    // the other tiles (new buildings)
    create_house_tile(house->type, house->x, house->y + 1, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(house->type, house->x + 1, house->y + 1, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(house->type, house->x + 2, house->y + 1, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(house->type, house->x, house->y + 2, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(house->type, house->x + 1, house->y + 2, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(house->type, house->x + 2, house->y + 2, image_id, population_per_tile, inventory_per_tile);
}

static void split(building *house, int num_tiles)
{
    int grid_offset = map_grid_offset(merge_data.x, merge_data.y);
    for (int i = 0; i < num_tiles; i++) {
        int tile_offset = grid_offset + HOUSE_TILE_OFFSETS[i];
        if (map_terrain_is(tile_offset, TERRAIN_BUILDING)) {
            building *other_house = building_get(map_building_at(tile_offset));
            if (other_house->id != house->id && other_house->house_size) {
                if (other_house->house_is_merged == 1) {
                    split_size2(other_house, other_house->type);
                } else if (other_house->house_size == 2) {
                    split_size2(other_house, BUILDING_HOUSE_MEDIUM_INSULA);
                } else if (other_house->house_size == 3) {
                    split_size3(other_house);
                }
            }
        }
    }
}

/// OLD STUFF

void building_house_expand_to_large_insula(building *house)
{
    split(house, 4);
    prepare_for_merge(house->id, 4);

    house->type = BUILDING_HOUSE_LARGE_INSULA;
    house->subtype.house_level = HOUSE_LARGE_INSULA;
    house->size = house->house_size = 2;
    house->house_population += merge_data.population;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] += merge_data.inventory[i];
    }
    int image_id = house_image_group(house->subtype.house_level) + (map_random_get(house->grid_offset) & 1);
    map_building_tiles_remove(house->id, house->x, house->y);
    house->x = merge_data.x;
    house->y = merge_data.y;
    house->grid_offset = map_grid_offset(house->x, house->y);
    map_building_tiles_add(house->id, house->x, house->y, house->size, image_id, TERRAIN_BUILDING);
}

void building_house_expand_to_large_villa(building *house)
{
    split(house, 9);
    prepare_for_merge(house->id, 9);

    house->type = BUILDING_HOUSE_LARGE_VILLA;
    house->subtype.house_level = HOUSE_LARGE_VILLA;
    house->size = house->house_size = 3;
    house->house_population += merge_data.population;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] += merge_data.inventory[i];
    }
    int image_id = house_image_group(house->subtype.house_level);
    map_building_tiles_remove(house->id, house->x, house->y);
    house->x = merge_data.x;
    house->y = merge_data.y;
    house->grid_offset = map_grid_offset(house->x, house->y);
    map_building_tiles_add(house->id, house->x, house->y, house->size, image_id, TERRAIN_BUILDING);
}

void building_house_expand_to_large_palace(building *house)
{
    split(house, 16);
    prepare_for_merge(house->id, 16);

    house->type = BUILDING_HOUSE_LARGE_PALACE;
    house->subtype.house_level = HOUSE_LARGE_PALACE;
    house->size = house->house_size = 4;
    house->house_population += merge_data.population;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] += merge_data.inventory[i];
    }
    int image_id = house_image_group(house->subtype.house_level);
    map_building_tiles_remove(house->id, house->x, house->y);
    house->x = merge_data.x;
    house->y = merge_data.y;
    house->grid_offset = map_grid_offset(house->x, house->y);
    map_building_tiles_add(house->id, house->x, house->y, house->size, image_id, TERRAIN_BUILDING);
}

void building_house_devolve_from_large_insula(building *house)
{
    split_size2(house, BUILDING_HOUSE_MEDIUM_INSULA);
}

void building_house_devolve_from_large_villa(building *house)
{
    int inventory_per_tile[INVENTORY_MAX];
    int inventory_remainder[INVENTORY_MAX];
    for (int i = 0; i < INVENTORY_MAX; i++) {
        inventory_per_tile[i] = house->data.house.inventory[i] / 6;
        inventory_remainder[i] = house->data.house.inventory[i] % 6;
    }
    int population_per_tile = house->house_population / 6;
    int population_remainder = house->house_population % 6;

    map_building_tiles_remove(house->id, house->x, house->y);

    // main tile
    house->type = BUILDING_HOUSE_MEDIUM_VILLA;
    house->subtype.house_level = house->type - BUILDING_HOUSE_VACANT_LOT;
    house->size = house->house_size = 2;
    house->house_is_merged = 0;
    house->house_population = population_per_tile + population_remainder;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] = inventory_per_tile[i] + inventory_remainder[i];
    }
    house->distance_from_entry = 0;

    int image_id = house_image_group(house->subtype.house_level);
    map_building_tiles_add(house->id, house->x, house->y, house->size,
                           image_id + (map_random_get(house->grid_offset) & 1), TERRAIN_BUILDING);

    // the other tiles (new buildings)
    image_id = house_image_group(HOUSE_MEDIUM_INSULA);
    create_house_tile(BUILDING_HOUSE_MEDIUM_INSULA, house->x + 2, house->y, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(BUILDING_HOUSE_MEDIUM_INSULA, house->x + 2, house->y + 1, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(BUILDING_HOUSE_MEDIUM_INSULA, house->x, house->y + 2, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(BUILDING_HOUSE_MEDIUM_INSULA, house->x + 1, house->y + 2, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(BUILDING_HOUSE_MEDIUM_INSULA, house->x + 2, house->y + 2, image_id, population_per_tile, inventory_per_tile);
}

void building_house_devolve_from_large_palace(building *house)
{
    int inventory_per_tile[INVENTORY_MAX];
    int inventory_remainder[INVENTORY_MAX];
    for (int i = 0; i < INVENTORY_MAX; i++) {
        inventory_per_tile[i] = house->data.house.inventory[i] / 8;
        inventory_remainder[i] = house->data.house.inventory[i] % 8;
    }
    int population_per_tile = house->house_population / 8;
    int population_remainder = house->house_population % 8;

    map_building_tiles_remove(house->id, house->x, house->y);

    // main tile
    house->type = BUILDING_HOUSE_MEDIUM_PALACE;
    house->subtype.house_level = house->type - BUILDING_HOUSE_VACANT_LOT;
    house->size = house->house_size = 3;
    house->house_is_merged = 0;
    house->house_population = population_per_tile + population_remainder;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] = inventory_per_tile[i] + inventory_remainder[i];
    }
    house->distance_from_entry = 0;

    int image_id = house_image_group(house->subtype.house_level);
    map_building_tiles_add(house->id, house->x, house->y, house->size, image_id, TERRAIN_BUILDING);

    // the other tiles (new buildings)
    image_id = house_image_group(HOUSE_MEDIUM_INSULA);
    create_house_tile(BUILDING_HOUSE_MEDIUM_INSULA, house->x + 3, house->y, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(BUILDING_HOUSE_MEDIUM_INSULA, house->x + 3, house->y + 1, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(BUILDING_HOUSE_MEDIUM_INSULA, house->x + 3, house->y + 2, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(BUILDING_HOUSE_MEDIUM_INSULA, house->x, house->y + 3, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(BUILDING_HOUSE_MEDIUM_INSULA, house->x + 1, house->y + 3, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(BUILDING_HOUSE_MEDIUM_INSULA, house->x + 2, house->y + 3, image_id, population_per_tile, inventory_per_tile);
    create_house_tile(BUILDING_HOUSE_MEDIUM_INSULA, house->x + 3, house->y + 3, image_id, population_per_tile, inventory_per_tile);
}

void building_house_check_for_corruption(building *house)
{
    int calc_grid_offset = map_grid_offset(house->x, house->y);
    house->data.house.no_space_to_expand = 0;
    if (house->grid_offset != calc_grid_offset || map_building_at(house->grid_offset) != house->id) {
        int map_width, map_height;
        map_grid_size(&map_width, &map_height);
        for (int y = 0; y < map_height; y++) {
            for (int x = 0; x < map_width; x++) {
                int grid_offset = map_grid_offset(x, y);
                if (map_building_at(grid_offset) == house->id) {
                    house->grid_offset = grid_offset;
                    house->x = map_grid_offset_to_x(grid_offset);
                    house->y = map_grid_offset_to_y(grid_offset);
                    building_totals_add_corrupted_house(0);
                    return;
                }
            }
        }
        building_totals_add_corrupted_house(1);
        house->state = BUILDING_STATE_RUBBLE;
    }
}
