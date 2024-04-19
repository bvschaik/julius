#include "file_minimap.h"

#include "building/building.h"
#include "city/view.h"
#include "figure/figure.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"
#include "scenario/property.h"

#include <string.h>

typedef struct {
    color_t left;
    color_t right;
} tile_color;

typedef struct {
    tile_color water[4];
    tile_color tree[4];
    tile_color rock[4];
    tile_color meadow[4];
    tile_color grass[8];
    tile_color road;
} tile_color_set;

// Since the minimap tiles are only 25 color sets per climate, we just hardcode them.
// This "hack" is necessary to avoid reloading the climate graphics when selecting
// a scenario with another climate in the CCK selection screen, which is expensive.
const tile_color_set MINIMAP_COLOR_SETS[3] = {
    // central
    {
        .water = {{0x394a7b, 0x31427b}, {0x394a7b, 0x314273}, {0x313973, 0x314273}, {0x31427b, 0x394a7b}},
        .tree = {{0x6b8431, 0x102108}, {0x103908, 0x737b29}, {0x103108, 0x526b21}, {0x737b31, 0x084a10}},
        .rock = {{0x948484, 0x635a4a}, {0xa59c94, 0xb5ada5}, {0xb5ada5, 0x8c8484}, {0x635a4a, 0xa59c94}},
        .meadow = {{0xd6bd63, 0x9c8c39}, {0x948c39, 0xd6bd63}, {0xd6bd63, 0x9c9439}, {0x848431, 0xada54a}},
        .grass = {
            {0x6b8c31, 0x6b7b29}, {0x738431, 0x6b7b29}, {0x6b7329, 0x7b8c39}, {0x527b29, 0x6b7321},
            {0x6b8431, 0x737b31}, {0x6b7b31, 0x737b29}, {0x636b18, 0x526b21}, {0x737b31, 0x737b29}
        },
        .road = {0x736b63, 0x4a3121}
    },
    // northern
    {
        .water = {{0x394a7b, 0x31427b}, {0x394a7b, 0x314273}, {0x313973, 0x314273}, {0x31427b, 0x394a7b}},
        .tree = {{0x527b31, 0x082108}, {0x083908, 0x5a7329}, {0x082908, 0x316b21}, {0x527b29, 0x084a21}},
        .rock = {{0x8c8484, 0x5a5252}, {0x9c9c94, 0xa5a5a5}, {0xa5a5a5, 0x848484}, {0x5a5252, 0x9c9c94}},
        .meadow = {{0x427318, 0x8c9442}, {0xb5ad4a, 0x738c39}, {0x8c8c39, 0x6b7b29}, {0x527331, 0x5a8442}},
        .grass = {
            {0x4a8431, 0x4a7329}, {0x527b29, 0x4a7329}, {0x526b29, 0x5a8439}, {0x397321, 0x4a6b21},
            {0x527b31, 0x5a7331}, {0x4a7329, 0x5a7329}, {0x4a6b18, 0x316b21}, {0x527b29, 0x527329}
        },
        .road = {0x736b63, 0x4a3121}
    },
    // desert
    {
        .water = {{0x4a84c6, 0x4a7bc6}, {0x4a84c6, 0x4a7bc6}, {0x4a84c6, 0x5284c6}, {0x4a7bbd, 0x4a7bc6}},
        .tree = {{0xa59c7b, 0x6b7b18}, {0x214210, 0xada573}, {0x526b21, 0xcec6a5}, {0xa59c7b, 0x316321}},
        .rock = {{0xa59494, 0x736352}, {0xa59c94, 0xb5ada5}, {0xb5ada5, 0x8c847b}, {0x736352, 0xbdada5}},
        .meadow = {{0x739c31, 0x9cbd52}, {0x7bb529, 0x63ad21}, {0x9cbd52, 0x8c944a}, {0x7ba539, 0x739c31}},
        .grass = {
            {0xbdbd9c, 0xb5b594}, {0xc6bda5, 0xbdbda5}, {0xbdbd9c, 0xc6c6ad}, {0xd6cead, 0xc6bd9c},
            {0xa59c7b, 0xbdb594}, {0xcecead, 0xb5ad94}, {0xc6c6a5, 0xdedebd}, {0xcecead, 0xd6d6b5}
        },
        .road = {0x6b5a52, 0x4a4239}
    }
};

#define MINIMAP_WIDTH_TILES (FILE_MINIMAP_IMAGE_WIDTH / 2)
#define MINIMAP_HEIGHT_TILES FILE_MINIMAP_IMAGE_HEIGHT
#define MINIMAP_X_OFFSET ((VIEW_X_MAX - MINIMAP_WIDTH_TILES) / 2)
#define MINIMAP_Y_OFFSET (((VIEW_Y_MAX - MINIMAP_HEIGHT_TILES) / 2) & ~1) // ensure even height

static struct {
    color_t screen[FILE_MINIMAP_IMAGE_WIDTH * FILE_MINIMAP_IMAGE_HEIGHT];
    color_t dst[FILE_MINIMAP_IMAGE_WIDTH * FILE_MINIMAP_IMAGE_HEIGHT];
    const file_buffers *buffers;
} data;

static void draw_minimap_tile(int x_view, int y_view, int grid_offset)
{
    if (grid_offset < 0) {
        return;
    }

    int terrain = map_terrain_get_from_buffer(data.buffers->map_terrain, grid_offset);
    building b;
    // exception for fort ground: display as empty land
    if (terrain & TERRAIN_BUILDING && data.buffers->buildings) {
        building_get_from_buffer(data.buffers->buildings,
            map_building_from_buffer(data.buffers->map_building, grid_offset), &b);
        if (b.type == BUILDING_FORT_GROUND) {
            terrain = 0;
        }
    }

    if (terrain & TERRAIN_BUILDING) {
        // For scenarios these can be Native huts/fields
        if (map_property_is_draw_tile_from_buffer(data.buffers->map_edge, grid_offset)) {
            int image_id;
            if (data.buffers->buildings && b.house_size) {
                image_id = image_group(GROUP_MINIMAP_HOUSE);
            } else if (data.buffers->buildings && b.type == BUILDING_RESERVOIR) {
                image_id = image_group(GROUP_MINIMAP_AQUEDUCT) - 1;
            } else {
                image_id = image_group(GROUP_MINIMAP_BUILDING);
            }
            switch (map_property_multi_tile_size_from_buffer(data.buffers->map_bitfields, grid_offset)) {
                case 1: image_draw(image_id, x_view, y_view); break;
                case 2: image_draw(image_id + 1, x_view, y_view - 1); break;
                case 3: image_draw(image_id + 2, x_view, y_view - 2); break;
                case 4: image_draw(image_id + 3, x_view, y_view - 3); break;
                case 5: image_draw(image_id + 4, x_view, y_view - 4); break;
            }
        }
    } else if (terrain & TERRAIN_AQUEDUCT) {
        image_draw(image_group(GROUP_MINIMAP_AQUEDUCT), x_view, y_view);
    } else if (terrain & TERRAIN_WALL) {
        image_draw(image_group(GROUP_MINIMAP_WALL), x_view, y_view);
    } else {
        int rand = map_random_get_from_buffer(data.buffers->map_random, grid_offset);
        const tile_color *color;
        const tile_color_set *set = &MINIMAP_COLOR_SETS[data.buffers->climate];
        if (terrain & TERRAIN_WATER) {
            color = &set->water[rand & 3];
        } else if (terrain & (TERRAIN_SHRUB | TERRAIN_TREE)) {
            color = &set->tree[rand & 3];
        } else if (terrain & (TERRAIN_ROCK | TERRAIN_ELEVATION)) {
            color = &set->rock[rand & 3];
        } else if (terrain & TERRAIN_ROAD) {
            color = &set->road;
        } else if (terrain & TERRAIN_MEADOW) {
            color = &set->meadow[rand & 3];
        } else {
            color = &set->grass[rand & 7];
        }
        graphics_draw_vertical_line(x_view, y_view, y_view, color->left);
        graphics_draw_vertical_line(x_view + 1, y_view, y_view, color->right);
    }
}

const color_t *game_file_minimap_create(const file_buffers *buffers)
{
    data.buffers = buffers;

    graphics_save_to_buffer(0, 0, FILE_MINIMAP_IMAGE_WIDTH, FILE_MINIMAP_IMAGE_HEIGHT, data.screen);
    graphics_fill_rect(0, 0, FILE_MINIMAP_IMAGE_WIDTH, FILE_MINIMAP_IMAGE_HEIGHT, 0);
    city_view_set_custom_lookup(buffers->grid_start, buffers->grid_width,
        buffers->grid_height, buffers->grid_border_size);
    city_view_foreach_minimap_tile(0, 0, MINIMAP_X_OFFSET, MINIMAP_Y_OFFSET,
        MINIMAP_WIDTH_TILES, MINIMAP_HEIGHT_TILES, draw_minimap_tile);
    graphics_set_clip_rectangle(0, 0, FILE_MINIMAP_IMAGE_WIDTH, FILE_MINIMAP_IMAGE_HEIGHT);
    graphics_reset_clip_rectangle();
    city_view_restore_lookup();
    graphics_save_to_buffer(0, 0, FILE_MINIMAP_IMAGE_WIDTH, FILE_MINIMAP_IMAGE_HEIGHT, data.dst);
    graphics_draw_from_buffer(0, 0, FILE_MINIMAP_IMAGE_WIDTH, FILE_MINIMAP_IMAGE_HEIGHT, data.screen);

    return data.dst;
}
