#include "image_context.h"

#include "assets/assets.h"
#include "building/building.h"
#include "building/rotation.h"
#include "city/view.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/terrain.h"

#define MAX_TILES 8

struct building_image_context {
    const unsigned char tiles[MAX_TILES];
    const unsigned char offset_for_orientation[4];
    const unsigned char max_item_offset;
    const int rotation;
    unsigned char current_item_offset;
};

static char connecting_grid[GRID_SIZE * GRID_SIZE] = { 0 };
static int connecting_grid_building = 0;

// 0 = no match
// 1 = match
// 2 = don't care

// For rotation 
// -1 any, otherwise shown value

static struct building_image_context building_images_hedges[18] = {
    { { 1, 2, 1, 2, 0, 2, 0, 2 }, {  4,  5,  2,  3 }, 0, -1, 0 },
    { { 0, 2, 1, 2, 1, 2, 0, 2 }, {  3,  4,  5,  2 }, 0, -1, 0 },
    { { 0, 2, 0, 2, 1, 2, 1, 2 }, {  2,  3,  4,  5 }, 0, -1, 0 },
    { { 1, 2, 0, 2, 0, 2, 1, 2 }, {  5,  2,  3,  4 }, 0, -1, 0 },
    { { 1, 2, 0, 2, 1, 2, 0, 2 }, {  1,  0,  1,  0 }, 0, -1, 0 },
    { { 0, 2, 1, 2, 0, 2, 1, 2 }, {  0,  1,  0,  1 }, 0, -1, 0 },
    { { 1, 2, 0, 2, 0, 2, 0, 2 }, {  1,  0,  1,  0 }, 0, -1, 0 },
    { { 0, 2, 1, 2, 0, 2, 0, 2 }, {  0,  1,  0,  1 }, 0, -1, 0 },
    { { 0, 2, 0, 2, 1, 2, 0, 2 }, {  1,  0,  1,  0 }, 0, -1, 0 },
    { { 0, 2, 0, 2, 0, 2, 1, 2 }, {  0,  1,  0,  1 }, 0, -1, 0 },
    { { 1, 2, 1, 2, 1, 2, 0, 2 }, {  9,  7,  6,  8 }, 0, -1, 0 },
    { { 0, 2, 1, 2, 1, 2, 1, 2 }, {  8,  9,  7,  6 }, 0, -1, 0 },
    { { 1, 2, 0, 2, 1, 2, 1, 2 }, {  6,  8,  9,  7 }, 0, -1, 0 },
    { { 1, 2, 1, 2, 0, 2, 1, 2 }, {  7,  6,  8,  9 }, 0, -1, 0 },
    { { 1, 2, 1, 2, 1, 2, 1, 2 }, { 10, 10, 10, 10 }, 0, -1, 0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, {  1,  0,  1,  0 }, 0,  0, 0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, {  0,  1,  0,  1 }, 0,  1, 0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 10, 10, 10, 10 }, 0, -1, 0 },
};

static struct building_image_context building_images_path_intersection[9] = {
    { { 1, 2, 1, 2, 0, 2, 0, 2 }, { 2, 3, 0, 1 }, 0, -1, 0 },
    { { 0, 2, 1, 2, 1, 2, 0, 2 }, { 1, 2, 3, 0 }, 0, -1, 0 },
    { { 0, 2, 0, 2, 1, 2, 1, 2 }, { 0, 1, 2, 3 }, 0, -1, 0 },
    { { 1, 2, 0, 2, 0, 2, 1, 2 }, { 3, 0, 1, 2 }, 0, -1, 0 },
    { { 1, 2, 1, 2, 1, 2, 0, 2 }, { 5, 6, 7, 4 }, 0, -1, 0 },
    { { 0, 2, 1, 2, 1, 2, 1, 2 }, { 4, 5, 6, 7 }, 0, -1, 0 },
    { { 1, 2, 0, 2, 1, 2, 1, 2 }, { 7, 4, 5, 6 }, 0, -1, 0 },
    { { 1, 2, 1, 2, 0, 2, 1, 2 }, { 6, 7, 4, 5 }, 0, -1, 0 },
    { { 1, 2, 1, 2, 1, 2, 1, 2 }, { 8, 8, 8, 8 }, 0, -1, 0 },
};

static struct building_image_context building_images_tree_path[8] = {
    { { 1, 2, 0, 2, 1, 2, 0, 2 }, {  0, 56,  0, 56 }, 0, -1, 0 },
    { { 0, 2, 1, 2, 0, 2, 1, 2 }, { 56,  0, 56,  0 }, 0, -1, 0 },
    { { 1, 2, 0, 2, 0, 2, 0, 2 }, {  0, 56,  0, 56 }, 0, -1, 0 },
    { { 0, 2, 1, 2, 0, 2, 0, 2 }, { 56,  0, 56,  0 }, 0, -1, 0 },
    { { 0, 2, 0, 2, 1, 2, 0, 2 }, {  0, 56,  0, 56 }, 0, -1, 0 },
    { { 0, 2, 0, 2, 0, 2, 1, 2 }, { 56,  0, 56,  0 }, 0, -1, 0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, {  0, 56,  0, 56 }, 0,  0, 0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 56,  0, 56,  0 }, 0, -1, 0 },
};

static struct building_image_context building_images_treeless_path[8] = {
    { { 1, 2, 0, 2, 1, 2, 0, 2 }, { 54, 0,  54,  0 }, 0, -1, 0 },
    { { 0, 2, 1, 2, 0, 2, 1, 2 }, {  0, 54,  0, 54 }, 0, -1, 0 },
    { { 1, 2, 0, 2, 0, 2, 0, 2 }, { 54, 0,  54,  0 }, 0, -1, 0 },
    { { 0, 2, 1, 2, 0, 2, 0, 2 }, {  0, 54,  0, 54 }, 0, -1, 0 },
    { { 0, 2, 0, 2, 1, 2, 0, 2 }, { 54, 0,  54,  0 }, 0, -1, 0 },
    { { 0, 2, 0, 2, 0, 2, 1, 2 }, {  0, 54,  0, 54 }, 0, -1, 0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 54, 0,  54,  0 }, 0,  0, 0 },
    { { 2, 2, 2, 2, 2, 2, 2, 2 }, {  0, 54,  0, 54 }, 0, -1, 0 },
};

enum {
    CONTEXT_HEDGES,
    CONTEXT_COLONNADE,
    CONTEXT_GARDEN_TREE_PATH,
    CONTEXT_GARDEN_PATH_INTERSECTION,
    CONTEXT_GARDEN_TREELESS_PATH,
    CONTEXT_GARDEN_WALLS,
    CONTEXT_MAX_ITEMS
};

static struct {
    struct building_image_context *context;
    int size;
} context_pointers[] = {
    { building_images_hedges, 18 },
    { building_images_hedges, 18 },
    { building_images_tree_path, 8 },
    { building_images_path_intersection, 9 },
    { building_images_treeless_path, 8 },
    { building_images_hedges, 18 },
};

void building_image_context_clear_connection_grid()
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        connecting_grid[i] = 0;
    }
}

void building_image_context_set_connecting_type(int building_type)
{
    connecting_grid_building = building_type;
}

void building_image_context_mark_connection_grid(int map_grid)
{
    connecting_grid[map_grid] = 1;
}

static void clear_current_offset(struct building_image_context *items, int num_items)
{
    for (int i = 0; i < num_items; i++) {
        items[i].current_item_offset = 0;
    }
}

static int context_matches_tiles(const struct building_image_context *context,
    const int tiles[MAX_TILES], int rotation)
{
    for (int i = 0; i < MAX_TILES; i++) {
        if (context->tiles[i] != 2 && tiles[i] != context->tiles[i]) {
            return 0;
        }
    }
    return context->rotation == -1 || context->rotation == rotation;
}

static const building_image *get_image(int group, int tiles[MAX_TILES], int rotation)
{
    static building_image result;
    result.is_valid = 0;
    struct building_image_context *context = context_pointers[group].context;
    int size = context_pointers[group].size;
    for (int i = 0; i < size; i++) {
        if (context_matches_tiles(&context[i], tiles, rotation)) {
            context[i].current_item_offset++;
            if (context[i].current_item_offset >= context[i].max_item_offset) {
                context[i].current_item_offset = 0;
            }
            result.is_valid = 1;
            result.group_offset = context[i].offset_for_orientation[city_view_orientation() / 2];
            result.item_offset = context[i].current_item_offset;
            break;
        }
    }
    return &result;
}

const building_image *building_image_context_get_hedges(int grid_offset)
{
    int tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);
        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !connecting_grid[offset]) {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (b->type == BUILDING_HEDGE_DARK || b->type == BUILDING_HEDGE_LIGHT ||
            (connecting_grid[offset] &&
            (connecting_grid_building == BUILDING_HEDGE_DARK || connecting_grid_building == BUILDING_HEDGE_LIGHT))) {
            tiles[i] = 1;
        }
    }
    int building_id = map_building_at(grid_offset);
    int rotation;
    if (building_id) {
        rotation = building_get(building_id)->subtype.orientation;
    } else {
        rotation = building_rotation_get_rotation_with_limit(CONNECTING_BUILDINGS_ROTATION_LIMIT_HEDGES);
    }
    return get_image(CONTEXT_HEDGES, tiles, rotation);
}

const building_image *building_image_context_get_colonnade(int grid_offset)
{
    int tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);
        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !connecting_grid[offset]) {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (b->type == BUILDING_COLONNADE ||
            (connecting_grid[offset] && connecting_grid_building == BUILDING_COLONNADE)) {
            tiles[i] = 1;
        }
    }
    int building_id = map_building_at(grid_offset);
    int rotation;
    if (building_id) {
        rotation = building_get(building_id)->subtype.orientation;
    } else {
        rotation = building_rotation_get_rotation_with_limit(CONNECTING_BUILDINGS_ROTATION_LIMIT_HEDGES);
    }
    return get_image(CONTEXT_COLONNADE, tiles, rotation);
}

const building_image *building_image_context_get_garden_wall(int grid_offset)
{
    int tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);
        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !connecting_grid[offset]) {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (b->type == BUILDING_GARDEN_WALL ||
            (connecting_grid[offset] && connecting_grid_building == BUILDING_GARDEN_WALL)) {
            tiles[i] = 1;
        }
    }
    int building_id = map_building_at(grid_offset);
    int rotation;
    if (building_id) {
        rotation = building_get(building_id)->subtype.orientation;
    } else {
        rotation = building_rotation_get_rotation_with_limit(CONNECTING_BUILDINGS_ROTATION_LIMIT_HEDGES);
    }
    return get_image(CONTEXT_GARDEN_WALLS, tiles, rotation);
}

static int is_garden_path(int type)
{
    switch (type) {
        case BUILDING_DATE_PATH:
        case BUILDING_ELM_PATH:
        case BUILDING_FIG_PATH:
        case BUILDING_FIR_PATH:
        case BUILDING_OAK_PATH:
        case BUILDING_PALM_PATH:
        case BUILDING_PINE_PATH:
        case BUILDING_PLUM_PATH:
        case BUILDING_GARDEN_PATH:
            return 1;
        default:
            return 0;
    }
}

static int path_image_for_garden_connections(void)
{
    return assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "Garden Path 01");
}

static int path_image_for_type(int type)
{
    switch (type) {
        case BUILDING_DATE_PATH:
            return assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "path orn date");
        case BUILDING_ELM_PATH:
            return assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "path orn elm");
        case BUILDING_FIG_PATH:
            return assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "path orn fig");
        case BUILDING_FIR_PATH:
            return assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "path orn fir");
        case BUILDING_OAK_PATH:
            return assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "path orn oak");
        case BUILDING_PALM_PATH:
            return assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "path orn palm");
        case BUILDING_PINE_PATH:
            return assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "path orn pine");
        case BUILDING_PLUM_PATH:
            return assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "path orn plum");
        case BUILDING_GARDEN_PATH:
            return assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "garden path r");
        default:
            return 0;
    }
}

const building_image *building_image_context_get_garden_path(int grid_offset, int context)
{
    int tiles[MAX_TILES] = { 0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);
        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !connecting_grid[offset]) {
            continue;
        }
        building *b = building_get(map_building_at(offset));
        if (is_garden_path(b->type) || (connecting_grid[offset] && is_garden_path(connecting_grid_building))) {
            tiles[i] = 1;
        }
    }
    int building_id = map_building_at(grid_offset);
    int rotation;
    if (building_id) {
        rotation = building_get(building_id)->subtype.orientation;
    } else {
        rotation = building_rotation_get_rotation_with_limit(CONNECTING_BUILDINGS_ROTATION_LIMIT_PATHS);
    }
    return get_image(context, tiles, rotation);
}

void building_image_context_init(void)
{
    for (int i = 0; i < CONTEXT_MAX_ITEMS; i++) {
        clear_current_offset(context_pointers[i].context, context_pointers[i].size);
    }
}

void building_image_context_set_hedge_image(int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_BUILDING) && !connecting_grid[grid_offset]) {
        return;
    }
    building *b = building_get(map_building_at(grid_offset));
    if (b->type != BUILDING_HEDGE_DARK && b->type != BUILDING_HEDGE_LIGHT && b->type != BUILDING_COLONNADE &&
        !is_garden_path(b->type) && b->type != BUILDING_GARDEN_WALL && !connecting_grid[grid_offset]) {
        return;
    }

    if (connecting_grid[grid_offset]) {
        int image_id = building_image_context_get_connecting_image_for_tile(grid_offset, connecting_grid_building);
        map_image_set(grid_offset, image_id);
    } else {
        int image_id = building_image_context_get_connecting_image_for_tile(grid_offset, b->type);
        map_building_tiles_add(b->id, b->x, b->y, b->size, image_id, TERRAIN_BUILDING);
    }
    map_property_set_multi_tile_size(grid_offset, 1);
    map_property_mark_draw_tile(grid_offset);
}

int building_image_context_get_connecting_image_for_tile(int grid_offset, int building_type)
{
    const building_image *img;
    int image_group = 0;

    if (!building_type && connecting_grid[grid_offset]) {
        building_type = connecting_grid_building;
    }

    if (building_type == BUILDING_HEDGE_DARK) {
        img = building_image_context_get_hedges(grid_offset);
        image_group = assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "D Hedge 01");
    } else if (building_type == BUILDING_HEDGE_LIGHT) {
        img = building_image_context_get_hedges(grid_offset);
        image_group = assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "L Hedge 01");
    } else if (building_type == BUILDING_COLONNADE) {
        img = building_image_context_get_colonnade(grid_offset);
        image_group = assets_get_image_id(assets_get_group_id("Lizzaran", "Aesthetics_L"), "G Colonnade 01");
    } else if (building_type == BUILDING_GARDEN_WALL) {
        img = building_image_context_get_garden_wall(grid_offset);
        image_group = assets_get_image_id(assets_get_group_id("Areldir", "Aesthetics"), "C Garden Wall 01");
    } else if (is_garden_path(building_type)) {
        img = building_image_context_get_garden_path(grid_offset, CONTEXT_GARDEN_PATH_INTERSECTION);
        image_group = path_image_for_garden_connections();
        // If path isn't an intersection, it's a straight path instead
        if (!img->is_valid) {
            if (building_type == BUILDING_GARDEN_PATH) {
                img = building_image_context_get_garden_path(grid_offset, CONTEXT_GARDEN_TREELESS_PATH);
            } else {
                img = building_image_context_get_garden_path(grid_offset, CONTEXT_GARDEN_TREE_PATH);
            }
            image_group = path_image_for_type(building_type);
        }
    } else {
        return 0;
    }
    int image_id = image_group + img->group_offset;
    return image_id;
}
