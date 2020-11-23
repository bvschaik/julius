#include "image_context.h"

#include "building/building.h"
#include "city/view.h"
#include "mods/mods.h"
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
    unsigned char current_item_offset;
};


static char connecting_grid[GRID_SIZE * GRID_SIZE] = {0};
static int connecting_grid_building = 0;

// 0 = no match
// 1 = match
// 2 = don't care

static struct building_image_context building_images_hedges[16] = {
    {{1, 2, 1, 2, 0, 2, 0, 2}, {4, 5, 2, 3}, 1},
    {{0, 2, 1, 2, 1, 2, 0, 2}, {3, 4, 5, 2}, 1},
    {{0, 2, 0, 2, 1, 2, 1, 2}, {2, 3, 4, 5}, 1},
    {{1, 2, 0, 2, 0, 2, 1, 2}, {5, 2, 3, 4}, 1},
    {{1, 2, 0, 2, 1, 2, 0, 2}, {1, 0, 1, 0}, 1},
    {{0, 2, 1, 2, 0, 2, 1, 2}, {0, 1, 0, 1}, 1},
    {{1, 2, 0, 2, 0, 2, 0, 2}, {1, 0, 1, 0}, 1},
    {{0, 2, 1, 2, 0, 2, 0, 2}, {0, 1, 0, 1}, 1},
    {{0, 2, 0, 2, 1, 2, 0, 2}, {1, 0, 1, 0}, 1},
    {{0, 2, 0, 2, 0, 2, 1, 2}, {0, 1, 0, 1}, 1},
    {{1, 2, 1, 2, 1, 2, 0, 2}, {9, 7, 6, 8}, 1},
    {{0, 2, 1, 2, 1, 2, 1, 2}, {8, 9, 7, 6}, 1},
    {{1, 2, 0, 2, 1, 2, 1, 2}, {6, 8, 9, 7}, 1},
    {{1, 2, 1, 2, 0, 2, 1, 2}, {7, 6, 8, 9}, 1},
    {{1, 2, 1, 2, 1, 2, 1, 2}, {10, 10, 10, 10}, 1},
    {{2, 2, 2, 2, 2, 2, 2, 2}, {10, 10, 10, 10}, 1},
};

enum {
    CONTEXT_HEDGES,
    CONTEXT_MAX_ITEMS
};

static struct {
    struct building_image_context* context;
    int size;
} context_pointers[] = {
    {building_images_hedges, 16}
};

void building_image_context_clear_connection_grid() 
{
    for (int i = 0; i < GRID_SIZE*GRID_SIZE; i++)
    {
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

static void clear_current_offset(struct building_image_context* items, int num_items)
{
    for (int i = 0; i < num_items; i++) {
        items[i].current_item_offset = 0;
    }
}

static int context_matches_tiles(const struct building_image_context* context, const int tiles[MAX_TILES])
{
    for (int i = 0; i < MAX_TILES; i++) {
        if (context->tiles[i] != 2 && tiles[i] != context->tiles[i]) {
            return 0;
        }
    }
    return 1;
}

static const building_image* get_image(int group, int tiles[MAX_TILES])
{
    static building_image result;

    result.is_valid = 0;
    struct building_image_context* context = context_pointers[group].context;
    int size = context_pointers[group].size;
    for (int i = 0; i < size; i++) {
        if (context_matches_tiles(&context[i], tiles)) {
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

const building_image* building_image_context_get_hedges(int grid_offset, int include_construction)
{

    int tiles[MAX_TILES] = { 0,0,0,0,0,0,0,0 };
    for (int i = 0; i < MAX_TILES; i += 2) {
        int offset = grid_offset + map_grid_direction_delta(i);
        if (!map_terrain_is(offset, TERRAIN_BUILDING) && !connecting_grid[offset]) {
            continue;
        }
        building* b = building_get(map_building_at(offset));
        if (b->type == BUILDING_HEDGE_DARK || b->type == BUILDING_HEDGE_LIGHT || connecting_grid[offset]) {
                tiles[i] = 1;
        }
    }
    return get_image(CONTEXT_HEDGES, tiles);
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
    building* b = building_get(map_building_at(grid_offset));
    if (b->type != BUILDING_HEDGE_DARK && b->type != BUILDING_HEDGE_LIGHT && !connecting_grid[grid_offset]) {
        return;
    }
    const building_image* img = building_image_context_get_hedges(grid_offset, 1);
    int offset = img->group_offset;
    int image_group;
    if (b->type == BUILDING_HEDGE_DARK || (connecting_grid_building == BUILDING_HEDGE_DARK && connecting_grid[grid_offset])) {
        image_group = mods_get_image_id(mods_get_group_id("Areldir", "Aesthetics"), "D Hedge 01");
    } else {
        image_group = mods_get_image_id(mods_get_group_id("Areldir", "Aesthetics"), "L Hedge 01");
    }
    if (connecting_grid[grid_offset]) {
        map_image_set(grid_offset, image_group + offset);
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
    }
    else {
        map_building_tiles_add(b->id, b->x, b->y, b->size, image_group + offset, TERRAIN_BUILDING);
        map_property_set_multi_tile_size(grid_offset, 1);
        map_property_mark_draw_tile(grid_offset);
    }

}

