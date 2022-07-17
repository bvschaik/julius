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
#include "map/random.h"
#include "map/terrain.h"

#define MAX_TILES 8

typedef struct  {
    const unsigned char tiles[MAX_TILES];
    const unsigned char offset_for_orientation[4];
    const int rotation;
    const unsigned char terrain_tiles[MAX_TILES];
    const int use_terrain;
    const int max_random;
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
    BUILDING_GARDEN_WALL_GATE,
    BUILDING_PALISADE,
    BUILDING_HEDGE_GATE_DARK,
    BUILDING_HEDGE_GATE_LIGHT,
    BUILDING_PALISADE_GATE,
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
    { { 1, 2, 0, 2, 1, 2, 0, 2 }, {  0, 68,  0, 68 }, -1 },
    { { 0, 2, 1, 2, 0, 2, 1, 2 }, { 68,  0, 68,  0 }, -1 },
    { { 1, 2, 0, 2, 0, 2, 0, 2 }, {  0, 68,  0, 68 }, -1 },
    { { 0, 2, 1, 2, 0, 2, 0, 2 }, { 68,  0, 68,  0 }, -1 },
    { { 0, 2, 0, 2, 1, 2, 0, 2 }, {  0, 68,  0, 68 }, -1 },
    { { 0, 2, 0, 2, 0, 2, 1, 2 }, { 68,  0, 68,  0 }, -1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, {  0, 68,  0, 68 },  0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 68,  0, 68,  0 }, -1 },
};

static const building_image_context building_images_treeless_path[8] = {
    { { 1, 2, 0, 2, 1, 2, 0, 2 }, { 140, 0,  140,  0 }, -1 },
    { { 0, 2, 1, 2, 0, 2, 1, 2 }, {  0, 140,  0, 140 }, -1 },
    { { 1, 2, 0, 2, 0, 2, 0, 2 }, { 140, 0,  140,  0 }, -1 },
    { { 0, 2, 1, 2, 0, 2, 0, 2 }, {  0, 140,  0, 140 }, -1 },
    { { 0, 2, 0, 2, 1, 2, 0, 2 }, { 140, 0,  140,  0 }, -1 },
    { { 0, 2, 0, 2, 0, 2, 1, 2 }, {  0, 140,  0, 140 }, -1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 140, 0,  140,  0 },  0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, {  0, 140,  0, 140 }, -1 },
};

static const building_image_context building_images_garden_gate[14] = {
    { { 1, 2, 0, 2, 1, 2, 0, 2 }, { 2, 0, 2, 0 }, -1,},
    { { 0, 2, 1, 2, 0, 2, 1, 2 }, { 0, 2, 0, 2 }, -1,},
    { { 1, 2, 0, 2, 0, 2, 0, 2 }, { 2, 0, 2, 0 }, -1 },
    { { 0, 2, 1, 2, 0, 2, 0, 2 }, { 0, 2, 0, 2 }, -1 },
    { { 0, 2, 0, 2, 1, 2, 0, 2 }, { 2, 0, 2, 0 }, -1 },
    { { 0, 2, 0, 2, 0, 2, 1, 2 }, { 0, 2, 0, 2 }, -1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 0, 2, 0, 2 }, -1, { 1, 2, 0, 2, 1, 2, 0, 2 }, 1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 0, 2, 0 }, -1, { 0, 2, 1, 2, 0, 2, 1, 2 }, 1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 0, 2, 0, 2 }, -1, { 1, 2, 0, 2, 0, 2, 0, 2 }, 1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 0, 2, 0 }, -1, { 0, 2, 1, 2, 0, 2, 0, 2 }, 1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 0, 2, 0, 2 }, -1, { 0, 2, 0, 2, 1, 2, 0, 2 }, 1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 0, 2, 0 }, -1, { 0, 2, 0, 2, 0, 2, 1, 2 }, 1 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 0, 2, 0 },  0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 0, 2, 0, 2 }, -1 },
};

static const  building_image_context building_images_palisades[18] = {
    { { 1, 2, 1, 2, 0, 2, 0, 2 }, {  15,  14,  13,  12 }, -1, { 0 }, 0, 0 },
    { { 0, 2, 1, 2, 1, 2, 0, 2 }, {  12,  15,  14,  13 }, -1, { 0 }, 0, 0 },
    { { 0, 2, 0, 2, 1, 2, 1, 2 }, {  13,  12,  15,  14 }, -1, { 0 }, 0, 0 },
    { { 1, 2, 0, 2, 0, 2, 1, 2 }, {  14,  13,  12,  15 }, -1, { 0 }, 0, 0 },
    { { 1, 2, 0, 2, 1, 2, 0, 2 }, {  6,  0,  6,  0 }, -1, { 0 }, 0, 6 },
    { { 0, 2, 1, 2, 0, 2, 1, 2 }, {  0,  6,  0,  6 }, -1, { 0 }, 0, 6 },
    { { 1, 2, 0, 2, 0, 2, 0, 2 }, {  6,  0,  6,  0 }, -1, { 0 }, 0, 6 },
    { { 0, 2, 1, 2, 0, 2, 0, 2 }, {  0,  6,  0,  6 }, -1, { 0 }, 0, 6 },
    { { 0, 2, 0, 2, 1, 2, 0, 2 }, {  6,  0,  6,  0 }, -1, { 0 }, 0, 6 },
    { { 0, 2, 0, 2, 0, 2, 1, 2 }, {  0,  6,  0,  6 }, -1, { 0 }, 0, 6 },
    { { 1, 2, 1, 2, 1, 2, 0, 2 }, {  19,  17,  16,  18 }, -1, { 0 }, 0, 0 },
    { { 0, 2, 1, 2, 1, 2, 1, 2 }, {  18,  19,  17,  16 }, -1, { 0 }, 0, 0 },
    { { 1, 2, 0, 2, 1, 2, 1, 2 }, {  16,  18,  19,  17 }, -1, { 0 }, 0, 0 },
    { { 1, 2, 1, 2, 0, 2, 1, 2 }, {  17,  16,  18,  19 }, -1, { 0 }, 0, 0 },
    { { 1, 2, 1, 2, 1, 2, 1, 2 }, { 20, 20, 20, 20 }, -1, { 0 }, 0, 0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, {  6,  0,  6,  0 },  0, { 0 }, 0, 6 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, {  0,  6,  0,  6 },  1, { 0 }, 0, 6 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 20, 20, 20, 20 }, -1, { 0 }, 0, 0 },
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
    { building_images_garden_gate, 14},
    { building_images_palisades, 18 },
};

int building_connectable_gate_type(building_type type)
{
    switch (type) {
        case BUILDING_GARDEN_WALL:
        case BUILDING_ROOFED_GARDEN_WALL:
            return BUILDING_GARDEN_WALL_GATE;
        case BUILDING_HEDGE_DARK:
            return BUILDING_HEDGE_GATE_DARK;
        case BUILDING_HEDGE_LIGHT:
            return BUILDING_HEDGE_GATE_LIGHT;
        case BUILDING_PALISADE:
            return BUILDING_PALISADE_GATE;
        default:
            return 0;
    }
}

static int context_matches_tiles(const building_image_context *context,
    const int tiles[MAX_TILES], int rotation, int terrain_tiles[MAX_TILES])
{
    for (int i = 0; i < MAX_TILES; i++) {
        if (context->use_terrain) {
            if (context->terrain_tiles[i] != 2 && terrain_tiles[i] != context->terrain_tiles[i]) {
                return 0;
            }
        }
        if (context->tiles[i] != 2 && tiles[i] != context->tiles[i]) {
            return 0;
        }
    }

    return context->rotation == -1 || context->rotation == rotation;
}

static int get_image_offset(int group, int tiles[MAX_TILES], int rotation, int terrain_tiles[MAX_TILES], int grid_offset)
{
    const building_image_context *context = context_pointers[group].context;
    int size = context_pointers[group].size;
    for (int i = 0; i < size; i++) {
        if (context_matches_tiles(&context[i], tiles, rotation, terrain_tiles)) {
            int offset = context[i].offset_for_orientation[city_view_orientation() / 2];
            if (context[i].max_random) {
                offset += map_random_get(grid_offset) % context[i].max_random;
            }
            return offset;
        }
    }
    return -1;
}

static int is_hedge_wall_or_gate(building_type type)
{
    return type == BUILDING_HEDGE_DARK || type == BUILDING_HEDGE_GATE_DARK || type == BUILDING_HEDGE_LIGHT || type == BUILDING_HEDGE_GATE_LIGHT;
}

static int is_hedge_wall(building_type type)
{
    return type == BUILDING_HEDGE_DARK || type == BUILDING_HEDGE_LIGHT;
}

int building_connectable_get_hedge_offset(int grid_offset)
{
    int tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);
        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !map_property_is_constructing(offset)) {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (is_hedge_wall_or_gate(b->type) ||
            (map_property_is_constructing(offset) && (is_hedge_wall(building_construction_type())))) {
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
    return get_image_offset(CONTEXT_HEDGES, tiles, rotation, 0, grid_offset);
}

int building_connectable_get_hedge_gate_offset(int grid_offset)
{
    int tiles[MAX_TILES] = { 0 };
    int terrain_tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);

        if (map_terrain_is(offset, TERRAIN_ROAD)) {
            terrain_tiles[i] = 1;
        }

        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !map_property_is_constructing(offset)) {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (is_hedge_wall(b->type) || (map_property_is_constructing(offset) && !map_terrain_is(offset, TERRAIN_ROAD)
            && is_hedge_wall(building_construction_type()))) {
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
    return get_image_offset(CONTEXT_GARDEN_GATE, tiles, rotation, terrain_tiles, grid_offset);
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
    return get_image_offset(CONTEXT_COLONNADE, tiles, rotation, 0, grid_offset);
}

static int is_garden_path(building_type type)
{
    return type == BUILDING_DATE_PATH || type == BUILDING_ELM_PATH || type == BUILDING_FIG_PATH ||
        type == BUILDING_FIR_PATH || type == BUILDING_OAK_PATH || type == BUILDING_PALM_PATH ||
        type == BUILDING_PINE_PATH || type == BUILDING_PLUM_PATH || type == BUILDING_GARDEN_PATH;
}

static int is_garden_wall_or_gate(building_type type)
{
    return type == BUILDING_GARDEN_WALL || type == BUILDING_ROOFED_GARDEN_WALL || type == BUILDING_GARDEN_WALL_GATE;
}

static int is_garden_wall(building_type type)
{
    return type == BUILDING_GARDEN_WALL || type == BUILDING_ROOFED_GARDEN_WALL;
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
        if (is_garden_wall_or_gate(b->type) || (map_property_is_constructing(offset) && is_garden_wall_or_gate(building_construction_type()))) {
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
    return get_image_offset(CONTEXT_GARDEN_WALLS, tiles, rotation, 0, grid_offset);
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
    return get_image_offset(context, tiles, rotation, 0, grid_offset);
}

int building_connectable_get_garden_gate_offset(int grid_offset)
{
    int tiles[MAX_TILES] = { 0 };
    int terrain_tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);

        if (map_terrain_is(offset, TERRAIN_ROAD)) {
            terrain_tiles[i] = 1;
        }

        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !map_property_is_constructing(offset)) {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (is_garden_wall(b->type) || 
            (map_property_is_constructing(offset) && !map_terrain_is(offset, TERRAIN_ROAD) && is_garden_wall(building_construction_type()))) {
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
    return get_image_offset(CONTEXT_GARDEN_GATE, tiles, rotation, terrain_tiles, grid_offset);
}

static int is_palisade_wall_or_gate(building_type type)
{
    return type == BUILDING_PALISADE || type == BUILDING_PALISADE_GATE;
}

static int is_palisade_wall(building_type type)
{
    return type == BUILDING_PALISADE;
}

int building_connectable_get_palisade_offset(int grid_offset)
{
    int tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);
        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !map_property_is_constructing(offset)) {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (is_palisade_wall_or_gate(b->type) ||
            (map_property_is_constructing(offset) && is_palisade_wall_or_gate(building_construction_type()))) {
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
    return get_image_offset(CONTEXT_PALISADES, tiles, rotation, 0, grid_offset);
}

int building_connectable_get_palisade_gate_offset(int grid_offset)
{
    int tiles[MAX_TILES] = { 0 };
    int terrain_tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);

        if (map_terrain_is(offset, TERRAIN_ROAD)) {
            terrain_tiles[i] = 1;
        }

        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !map_property_is_constructing(offset)) {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (is_palisade_wall(b->type) || (map_property_is_constructing(offset) && !map_terrain_is(offset, TERRAIN_ROAD)
            && is_palisade_wall(building_construction_type()))) {
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
    return get_image_offset(CONTEXT_GARDEN_GATE, tiles, rotation, terrain_tiles, grid_offset);
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

int building_connectable_num_variants(building_type type)
{
    if (!building_is_connectable(type)) {
        return 0;
    }
    switch (type) {
        case BUILDING_HEDGE_DARK:
        case BUILDING_HEDGE_LIGHT:
        case BUILDING_COLONNADE:
        case BUILDING_GARDEN_WALL:
        case BUILDING_ROOFED_GARDEN_WALL:
        case BUILDING_PALISADE:
            return BUILDING_CONNECTABLE_ROTATION_LIMIT_HEDGES;
        default:
            return BUILDING_CONNECTABLE_ROTATION_LIMIT_PATHS;
    }
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
