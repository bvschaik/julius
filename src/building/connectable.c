#include "connectable.h"

#include "building/building.h"
#include "building/construction.h"
#include "building/image.h"
#include "building/rotation.h"
#include "city/view.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/terrain.h"

#define MAX_TILES 8

typedef struct  {
    const unsigned char tiles[MAX_TILES];
    const unsigned char offset_for_orientation[4];
    const int rotation;
} building_image_context;

static const building_type connectable_buildings[] = {
    BUILDING_HEDGE_DARK,
    BUILDING_HEDGE_LIGHT,
    BUILDING_COLONNADE,
    BUILDING_GARDEN_PATH,
    BUILDING_DATE_PATH,
    BUILDING_ELM_PATH,
    BUILDING_FIG_PATH,
    BUILDING_FIR_PATH,
    BUILDING_OAK_PATH,
    BUILDING_PALM_PATH,
    BUILDING_PINE_PATH,
    BUILDING_PLUM_PATH,
    BUILDING_GARDEN_WALL,
    BUILDING_ROOFED_GARDEN_WALL,
};

static const int MAX_CONNECTABLE_BUILDINGS = sizeof(connectable_buildings) / sizeof(building_type);

// 0 = no match
// 1 = match
// 2 = don't care

// For rotation 
// -1 any, otherwise shown value

static const  building_image_context building_images_hedges[18] = {
    { { 1, 2, 1, 2, 0, 2, 0, 2 }, {  4,  5,  2,  3 }, -1 },
    { { 0, 2, 1, 2, 1, 2, 0, 2 }, {  3,  4,  5,  2 }, -1 },
    { { 0, 2, 0, 2, 1, 2, 1, 2 }, {  2,  3,  4,  5 }, -1 },
    { { 1, 2, 0, 2, 0, 2, 1, 2 }, {  5,  2,  3,  4 }, -1 },
    { { 1, 2, 0, 2, 1, 2, 0, 2 }, {  1,  0,  1,  0 }, -1 },
    { { 0, 2, 1, 2, 0, 2, 1, 2 }, {  0,  1,  0,  1 }, -1 },
    { { 1, 2, 0, 2, 0, 2, 0, 2 }, {  1,  0,  1,  0 }, -1 },
    { { 0, 2, 1, 2, 0, 2, 0, 2 }, {  0,  1,  0,  1 }, -1 },
    { { 0, 2, 0, 2, 1, 2, 0, 2 }, {  1,  0,  1,  0 }, -1 },
    { { 0, 2, 0, 2, 0, 2, 1, 2 }, {  0,  1,  0,  1 }, -1 },
    { { 1, 2, 1, 2, 1, 2, 0, 2 }, {  9,  7,  6,  8 }, -1 },
    { { 0, 2, 1, 2, 1, 2, 1, 2 }, {  8,  9,  7,  6 }, -1 },
    { { 1, 2, 0, 2, 1, 2, 1, 2 }, {  6,  8,  9,  7 }, -1 },
    { { 1, 2, 1, 2, 0, 2, 1, 2 }, {  7,  6,  8,  9 }, -1 },
    { { 1, 2, 1, 2, 1, 2, 1, 2 }, { 10, 10, 10, 10 }, -1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, {  1,  0,  1,  0 },  0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, {  0,  1,  0,  1 },  1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 10, 10, 10, 10 }, -1 },
};

static const building_image_context building_images_path_intersection[9] = {
    { { 1, 2, 1, 2, 0, 2, 0, 2 }, { 2, 3, 0, 1 }, -1 },
    { { 0, 2, 1, 2, 1, 2, 0, 2 }, { 1, 2, 3, 0 }, -1 },
    { { 0, 2, 0, 2, 1, 2, 1, 2 }, { 0, 1, 2, 3 }, -1 },
    { { 1, 2, 0, 2, 0, 2, 1, 2 }, { 3, 0, 1, 2 }, -1 },
    { { 1, 2, 1, 2, 1, 2, 0, 2 }, { 5, 6, 7, 4 }, -1 },
    { { 0, 2, 1, 2, 1, 2, 1, 2 }, { 4, 5, 6, 7 }, -1 },
    { { 1, 2, 0, 2, 1, 2, 1, 2 }, { 7, 4, 5, 6 }, -1 },
    { { 1, 2, 1, 2, 0, 2, 1, 2 }, { 6, 7, 4, 5 }, -1 },
    { { 1, 2, 1, 2, 1, 2, 1, 2 }, { 8, 8, 8, 8 }, -1 },
};

static const building_image_context building_images_tree_path[8] = {
    { { 1, 2, 0, 2, 1, 2, 0, 2 }, {  0, 67,  0, 67 }, -1 },
    { { 0, 2, 1, 2, 0, 2, 1, 2 }, { 67,  0, 67,  0 }, -1 },
    { { 1, 2, 0, 2, 0, 2, 0, 2 }, {  0, 67,  0, 67 }, -1 },
    { { 0, 2, 1, 2, 0, 2, 0, 2 }, { 67,  0, 67,  0 }, -1 },
    { { 0, 2, 0, 2, 1, 2, 0, 2 }, {  0, 67,  0, 67 }, -1 },
    { { 0, 2, 0, 2, 0, 2, 1, 2 }, { 67,  0, 67,  0 }, -1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, {  0, 67,  0, 67 },  0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 67,  0, 67,  0 }, -1 },
};

static const building_image_context building_images_treeless_path[8] = {
    { { 1, 2, 0, 2, 1, 2, 0, 2 }, { 54, 0,  54,  0 }, -1 },
    { { 0, 2, 1, 2, 0, 2, 1, 2 }, {  0, 54,  0, 54 }, -1 },
    { { 1, 2, 0, 2, 0, 2, 0, 2 }, { 54, 0,  54,  0 }, -1 },
    { { 0, 2, 1, 2, 0, 2, 0, 2 }, {  0, 54,  0, 54 }, -1 },
    { { 0, 2, 0, 2, 1, 2, 0, 2 }, { 54, 0,  54,  0 }, -1 },
    { { 0, 2, 0, 2, 0, 2, 1, 2 }, {  0, 54,  0, 54 }, -1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 54, 0,  54,  0 },  0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, {  0, 54,  0, 54 }, -1 },
};

static struct {
    const building_image_context *context;
    int size;
} context_pointers[] = {
    { building_images_hedges, 18 },
    { building_images_hedges, 18 },
    { building_images_tree_path, 8 },
    { building_images_path_intersection, 9 },
    { building_images_treeless_path, 8 },
    { building_images_hedges, 18 },
};

static int context_matches_tiles(const building_image_context *context,
    const int tiles[MAX_TILES], int rotation)
{
    for (int i = 0; i < MAX_TILES; i++) {
        if (context->tiles[i] != 2 && tiles[i] != context->tiles[i]) {
            return 0;
        }
    }
    return context->rotation == -1 || context->rotation == rotation;
}

static int get_image_offset(int group, int tiles[MAX_TILES], int rotation)
{
    const building_image_context *context = context_pointers[group].context;
    int size = context_pointers[group].size;
    for (int i = 0; i < size; i++) {
        if (context_matches_tiles(&context[i], tiles, rotation)) {
            return context[i].offset_for_orientation[city_view_orientation() / 2];
        }
    }
    return -1;
}

int building_connectable_get_hedge_offset(int grid_offset)
{
    int tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);
        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !map_property_is_constructing(offset))
        {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (b->type == BUILDING_HEDGE_DARK || b->type == BUILDING_HEDGE_LIGHT ||
            (map_property_is_constructing(offset) &&
            (building_construction_type() == BUILDING_HEDGE_DARK || building_construction_type() == BUILDING_HEDGE_LIGHT))) {
            tiles[i] = 1;
        }
    }
    int building_id = map_building_at(grid_offset);
    int rotation;
    if (building_id) {
        rotation = building_get(building_id)->subtype.orientation;
    } else {
        rotation = building_rotation_get_rotation_with_limit(BUILDING_CONNECTABLE_ROTATION_LIMIT_HEDGES);
    }
    return get_image_offset(CONTEXT_HEDGES, tiles, rotation);
}

int building_connectable_get_colonnade_offset(int grid_offset)
{
    int tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);
        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !map_property_is_constructing(offset)) {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (b->type == BUILDING_COLONNADE ||
            (map_property_is_constructing(offset) && building_construction_type() == BUILDING_COLONNADE)) {
            tiles[i] = 1;
        }
    }
    int building_id = map_building_at(grid_offset);
    int rotation;
    if (building_id) {
        rotation = building_get(building_id)->subtype.orientation;
    } else {
        rotation = building_rotation_get_rotation_with_limit(BUILDING_CONNECTABLE_ROTATION_LIMIT_HEDGES);
    }
    return get_image_offset(CONTEXT_COLONNADE, tiles, rotation);
}

int building_connectable_get_garden_wall_offset(int grid_offset)
{
    int tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);
        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !map_property_is_constructing(offset)) {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (b->type == BUILDING_GARDEN_WALL || b->type == BUILDING_ROOFED_GARDEN_WALL ||
            (map_property_is_constructing(offset) && (building_construction_type() == BUILDING_GARDEN_WALL 
                || building_construction_type() == BUILDING_ROOFED_GARDEN_WALL))) {
            tiles[i] = 1;
        }
    }
    int building_id = map_building_at(grid_offset);
    int rotation;
    if (building_id) {
        rotation = building_get(building_id)->subtype.orientation;
    } else {
        rotation = building_rotation_get_rotation_with_limit(BUILDING_CONNECTABLE_ROTATION_LIMIT_HEDGES);
    }
    return get_image_offset(CONTEXT_GARDEN_WALLS, tiles, rotation);
}

static int is_garden_path(building_type type)
{
    return type == BUILDING_DATE_PATH || type == BUILDING_ELM_PATH || type == BUILDING_FIG_PATH ||
        type == BUILDING_FIR_PATH || type == BUILDING_OAK_PATH || type == BUILDING_PALM_PATH ||
        type == BUILDING_PINE_PATH || type == BUILDING_PLUM_PATH || type == BUILDING_GARDEN_PATH;
}

int building_connectable_get_garden_path_offset(int grid_offset, int context)
{
    int tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);
        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !map_property_is_constructing(offset)) {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (is_garden_path(b->type) || (map_property_is_constructing(offset) && is_garden_path(building_construction_type()))) {
            tiles[i] = 1;
        }
    }
    int building_id = map_building_at(grid_offset);
    int rotation;
    if (building_id) {
        rotation = building_get(building_id)->subtype.orientation;
    } else {
        rotation = building_rotation_get_rotation_with_limit(BUILDING_CONNECTABLE_ROTATION_LIMIT_PATHS);
    }
    return get_image_offset(context, tiles, rotation);
}

int building_is_connectable(building_type type)
{
    for (int i = 0; i < MAX_CONNECTABLE_BUILDINGS; i++) {
        if (type == connectable_buildings[i]) {
            return 1;
        }
    }
    return 0;
}

void building_connectable_update_connections_for_type(building_type type)
{
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        map_image_set(b->grid_offset, building_image_get(b));
    }
}

void building_connectable_update_connections(void)
{
    for (int i = 0; i < MAX_CONNECTABLE_BUILDINGS; i++) {
        building_connectable_update_connections_for_type(connectable_buildings[i]);
    }
}
