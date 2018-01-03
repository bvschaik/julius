#include "house.h"

#include "game/resource.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/random.h"
#include "map/terrain.h"

#include "Data/State.h"

#define MAX_DIR 4

static const int HOUSE_TILE_OFFSETS[] = {
    0, 1, 162, 163, // 2x2
    2, 164, 326, 325, 324, // 3x3
    3, 165, 327, 489, 488, 487, 486 // 4x4
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
    {GROUP_BUILDING_HOUSE_INSULA_2, 0, 2}, {GROUP_BUILDING_HOUSE_INSULA_1, 2, 2},
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
    house->subtype.houseLevel = house->type - BUILDING_HOUSE_VACANT_LOT;
    int image_id = image_group(HOUSE_IMAGE[house->subtype.houseLevel].group);
    if (house->houseIsMerged) {
        image_id += 4;
        if (HOUSE_IMAGE[house->subtype.houseLevel].offset) {
            image_id += 1;
        }
    } else {
        image_id += HOUSE_IMAGE[house->subtype.houseLevel].offset;
        image_id += map_random_get(house->gridOffset) & (HOUSE_IMAGE[house->subtype.houseLevel].num_types - 1);
    }
    map_building_tiles_add(house->id, house->x, house->y, house->size, image_id, TERRAIN_BUILDING);
}

static void create_vacant_lot(int x, int y, int image_id)
{
    building *b = building_create(BUILDING_HOUSE_VACANT_LOT, x, y);
    b->housePopulation = 0;
    b->distanceFromEntry = 0;
    map_building_tiles_add(b->id, b->x + 1, b->y, 1, image_id, TERRAIN_BUILDING);
}

void building_house_change_to_vacant_lot(building *house)
{
    house->type = BUILDING_HOUSE_VACANT_LOT;
    house->subtype.houseLevel = house->type - 10;
    int image_id = image_group(GROUP_BUILDING_HOUSE_VACANT_LOT);
    if (house->houseIsMerged) {
        map_building_tiles_remove(house->id, house->x, house->y);
        house->houseIsMerged = 0;
        house->size = house->houseSize = 1;
        map_building_tiles_add(house->id, house->x, house->y, 1, image_id, TERRAIN_BUILDING);

        create_vacant_lot(house->x + 1, house->y, image_id);
        create_vacant_lot(house->x, house->y + 1, image_id);
        create_vacant_lot(house->x + 1, house->y + 1, image_id);
    } else {
        map_image_set(house->gridOffset, image_id);
    }
}

static void prepare_for_merge(int buildingId, int num_tiles)
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
            if (house->id != buildingId && house->houseSize) {
                merge_data.population += house->housePopulation;
                for (int i = 0; i < INVENTORY_MAX; i++) {
                    merge_data.inventory[i] += house->data.house.inventory[i];
                    house->housePopulation = 0;
                    house->state = BUILDING_STATE_DELETED_BY_GAME;
                }
            }
        }
    }
}

static void merge(building *b)
{
    prepare_for_merge(b->id, 4);

    b->size = b->houseSize = 2;
    b->housePopulation += merge_data.population;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        b->data.house.inventory[i] += merge_data.inventory[i];
    }
    int image_id = image_group(HOUSE_IMAGE[b->subtype.houseLevel].group) + 4;
    if (HOUSE_IMAGE[b->subtype.houseLevel].offset) {
        image_id += 1;
    }
    
    map_building_tiles_remove(b->id, b->x, b->y);
    b->x = merge_data.x;
    b->y = merge_data.y;
    b->gridOffset = map_grid_offset(b->x, b->y);
    b->houseIsMerged = 1;
    map_building_tiles_add(b->id, b->x, b->y, 2, image_id, TERRAIN_BUILDING);
}

void building_house_merge(building *house)
{
    if (house->houseIsMerged) {
        return;
    }
    if ((map_random_get(house->gridOffset) & 7) >= 5) {
        return;
    }
    int num_house_tiles = 0;
    for (int i = 0; i < 4; i++) {
        int tile_offset = house->gridOffset + HOUSE_TILE_OFFSETS[i];
        if (map_terrain_is(tile_offset, TERRAIN_BUILDING)) {
            building *other_house = building_get(map_building_at(tile_offset));
            if (other_house->id == house->id) {
                num_house_tiles++;
            } else if (other_house->state == BUILDING_STATE_IN_USE && other_house->houseSize &&
                    other_house->subtype.houseLevel == house->subtype.houseLevel &&
                    !other_house->houseIsMerged) {
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
        int base_offset = EXPAND_DIRECTION_DELTA[dir].offset + house->gridOffset;
        int ok_tiles = 0;
        for (int i = 0; i < num_tiles; i++) {
            int tile_offset = base_offset + HOUSE_TILE_OFFSETS[i];
            if (map_terrain_is(tile_offset, TERRAIN_BUILDING)) {
                building *other_house = building_get(map_building_at(tile_offset));
                if (other_house->id == house->id) {
                    ok_tiles++;
                } else if (other_house->state == BUILDING_STATE_IN_USE && other_house->houseSize) {
                    if (other_house->subtype.houseLevel <= house->subtype.houseLevel) {
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
        int base_offset = EXPAND_DIRECTION_DELTA[dir].offset + house->gridOffset;
        int ok_tiles = 0;
        for (int i = 0; i < num_tiles; i++) {
            int tile_offset = base_offset + HOUSE_TILE_OFFSETS[i];
            if (!map_terrain_is(tile_offset, TERRAIN_NOT_CLEAR)) {
                ok_tiles++;
            } else if (map_terrain_is(tile_offset, TERRAIN_BUILDING)) {
                building *other_house = building_get(map_building_at(tile_offset));
                if (other_house->id == house->id) {
                    ok_tiles++;
                } else if (other_house->state == BUILDING_STATE_IN_USE && other_house->houseSize) {
                    if (other_house->subtype.houseLevel <= house->subtype.houseLevel) {
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
        int base_offset = EXPAND_DIRECTION_DELTA[dir].offset + house->gridOffset;
        int ok_tiles = 0;
        for (int i = 0; i < num_tiles; i++) {
            int tile_offset = base_offset + HOUSE_TILE_OFFSETS[i];
            if (!map_terrain_is(tile_offset, TERRAIN_NOT_CLEAR)) {
                ok_tiles++;
            } else if (map_terrain_is(tile_offset, TERRAIN_BUILDING)) {
                building *other_house = building_get(map_building_at(tile_offset));
                if (other_house->id == house->id) {
                    ok_tiles++;
                } else if (other_house->state == BUILDING_STATE_IN_USE && other_house->houseSize) {
                    if (other_house->subtype.houseLevel <= house->subtype.houseLevel) {
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
    house->data.house.noSpaceToExpand = 1;
    return 0;
}

static int house_image_group(int level)
{
    return image_group(HOUSE_IMAGE[level].group) + HOUSE_IMAGE[level].offset;
}

static void create_house_tile(building_type type, int x, int y, int image_id, int population, const int *inventory)
{
    building *house = building_create(type, x, y);
    house->housePopulation = population;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] = inventory[i];
    }
    house->distanceFromEntry = 0;
    map_building_tiles_add(house->id, house->x, house->y, 1,
                           image_id + (map_random_get(house->gridOffset) & 1), TERRAIN_BUILDING);
}

static void split_size2(building *house, building_type new_type)
{
    int inventory_per_tile[INVENTORY_MAX];
    int inventory_remainder[INVENTORY_MAX];
    for (int i = 0; i < INVENTORY_MAX; i++) {
        inventory_per_tile[i] = house->data.house.inventory[i] / 4;
        inventory_remainder[i] = house->data.house.inventory[i] % 4;
    }
    int population_per_tile = house->housePopulation / 4;
    int population_remainder = house->housePopulation % 4;

    map_building_tiles_remove(house->id, house->x, house->y);

    // main tile
    house->type = new_type;
    house->subtype.houseLevel = house->type - BUILDING_HOUSE_VACANT_LOT;
    house->size = house->houseSize = 1;
    house->houseIsMerged = 0;
    house->housePopulation = population_per_tile + population_remainder;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] = inventory_per_tile[i] + inventory_remainder[i];
    }
    house->distanceFromEntry = 0;

    int image_id = house_image_group(house->subtype.houseLevel);
    map_building_tiles_add(house->id, house->x, house->y, house->size,
                           image_id + (map_random_get(house->gridOffset) & 1), TERRAIN_BUILDING);

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
    int population_per_tile = house->housePopulation / 9;
    int population_remainder = house->housePopulation % 9;

    map_building_tiles_remove(house->id, house->x, house->y);

    // main tile
    house->type = BUILDING_HOUSE_MEDIUM_INSULA;
    house->subtype.houseLevel = house->type - BUILDING_HOUSE_VACANT_LOT;
    house->size = house->houseSize = 1;
    house->houseIsMerged = 0;
    house->housePopulation = population_per_tile + population_remainder;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] = inventory_per_tile[i] + inventory_remainder[i];
    }
    house->distanceFromEntry = 0;

    int image_id = house_image_group(house->subtype.houseLevel);
    map_building_tiles_add(house->id, house->x, house->y, house->size,
                           image_id + (map_random_get(house->gridOffset) & 1), TERRAIN_BUILDING);

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
            if (other_house->id != house->id && other_house->houseSize) {
                if (other_house->houseIsMerged == 1) {
                    split_size2(other_house, other_house->type);
                } else if (other_house->houseSize == 2) {
                    split_size2(house, BUILDING_HOUSE_MEDIUM_INSULA);
                } else if (other_house->houseSize == 3) {
                    split_size3(house);
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
    house->subtype.houseLevel = HOUSE_LARGE_INSULA;
    house->size = house->houseSize = 2;
    house->housePopulation += merge_data.population;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] += merge_data.inventory[i];
    }
    int image_id = house_image_group(house->subtype.houseLevel) + (map_random_get(house->gridOffset) & 1);
    map_building_tiles_remove(house->id, house->x, house->y);
    house->x = merge_data.x;
    house->y = merge_data.y;
    house->gridOffset = map_grid_offset(house->x, house->y);
    map_building_tiles_add(house->id, house->x, house->y, house->size, image_id, TERRAIN_BUILDING);
}

void building_house_expand_to_large_villa(building *house)
{
    split(house, 9);
    prepare_for_merge(house->id, 9);

    house->type = BUILDING_HOUSE_LARGE_VILLA;
    house->subtype.houseLevel = HOUSE_LARGE_VILLA;
    house->size = house->houseSize = 3;
    house->housePopulation += merge_data.population;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] += merge_data.inventory[i];
    }
    int image_id = house_image_group(house->subtype.houseLevel);
    map_building_tiles_remove(house->id, house->x, house->y);
    house->x = merge_data.x;
    house->y = merge_data.y;
    house->gridOffset = map_grid_offset(house->x, house->y);
    map_building_tiles_add(house->id, house->x, house->y, house->size, image_id, TERRAIN_BUILDING);
}

void building_house_expand_to_large_palace(building *house)
{
    split(house, 16);
    prepare_for_merge(house->id, 16);

    house->type = BUILDING_HOUSE_LARGE_PALACE;
    house->subtype.houseLevel = HOUSE_LARGE_PALACE;
    house->size = house->houseSize = 4;
    house->housePopulation += merge_data.population;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] += merge_data.inventory[i];
    }
    int image_id = house_image_group(house->subtype.houseLevel);
    map_building_tiles_remove(house->id, house->x, house->y);
    house->x = merge_data.x;
    house->y = merge_data.y;
    house->gridOffset = map_grid_offset(house->x, house->y);
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
    int population_per_tile = house->housePopulation / 6;
    int population_remainder = house->housePopulation % 6;

    map_building_tiles_remove(house->id, house->x, house->y);

    // main tile
    house->type = BUILDING_HOUSE_MEDIUM_VILLA;
    house->subtype.houseLevel = house->type - 10;
    house->size = house->houseSize = 2;
    house->houseIsMerged = 0;
    house->housePopulation = population_per_tile + population_remainder;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] = inventory_per_tile[i] + inventory_remainder[i];
    }
    house->distanceFromEntry = 0;

    int image_id = house_image_group(house->subtype.houseLevel);
    map_building_tiles_add(house->id, house->x, house->y, house->size,
                           image_id + (map_random_get(house->gridOffset) & 1), TERRAIN_BUILDING);

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
    int population_per_tile = house->housePopulation / 8;
    int population_remainder = house->housePopulation % 8;

    map_building_tiles_remove(house->id, house->x, house->y);

    // main tile
    house->type = BUILDING_HOUSE_MEDIUM_PALACE;
    house->subtype.houseLevel = house->type - 10;
    house->size = house->houseSize = 3;
    house->houseIsMerged = 0;
    house->housePopulation = population_per_tile + population_remainder;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        house->data.house.inventory[i] = inventory_per_tile[i] + inventory_remainder[i];
    }
    house->distanceFromEntry = 0;

    int image_id = house_image_group(house->subtype.houseLevel);
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
    house->data.house.noSpaceToExpand = 0;
    if (house->gridOffset != calc_grid_offset || map_building_at(house->gridOffset) != house->id) {
        ++Data_Buildings_Extra.incorrectHousePositions;
        for (int y = 0; y < Data_State.map.height; y++) {
            for (int x = 0; x < Data_State.map.width; x++) {
                int gridOffset = map_grid_offset(x, y);
                if (map_building_at(gridOffset) == house->id) {
                    house->gridOffset = gridOffset;
                    house->x = map_grid_offset_to_x(gridOffset);
                    house->y = map_grid_offset_to_y(gridOffset);
                    return;
                }
            }
        }
        ++Data_Buildings_Extra.unfixableHousePositions;
        house->state = BUILDING_STATE_RUBBLE;
    }
}
