#include "minimap.h"

#include "city/view.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"
#include "scenario/property.h"

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
        .water = {{0xff394a7b, 0xff31427b}, {0xff394a7b, 0xff314273}, {0xff313973, 0xff314273}, {0xff31427b, 0xff394a7b}},
        .tree = {{0xff6b8431, 0xff102108}, {0xff103908, 0xff737b29}, {0xff103108, 0xff526b21}, {0xff737b31, 0xff084a10}},
        .rock = {{0xff948484, 0xff635a4a}, {0xffa59c94, 0xffb5ada5}, {0xffb5ada5, 0xff8c8484}, {0xff635a4a, 0xffa59c94}},
        .meadow = {{0xffd6bd63, 0xff9c8c39}, {0xff948c39, 0xffd6bd63}, {0xffd6bd63, 0xff9c9439}, {0xff848431, 0xffada54a}},
        .grass = {
            {0xff6b8c31, 0xff6b7b29}, {0xff738431, 0xff6b7b29}, {0xff6b7329, 0xff7b8c39}, {0xff527b29, 0xff6b7321},
            {0xff6b8431, 0xff737b31}, {0xff6b7b31, 0xff737b29}, {0xff636b18, 0xff526b21}, {0xff737b31, 0xff737b29}
        },
        .road = {0xff736b63, 0xff4a3121}
    },
    // northern
    {
        .water = {{0xff394a7b, 0xff31427b}, {0xff394a7b, 0xff314273}, {0xff313973, 0xff314273}, {0xff31427b, 0xff394a7b}},
        .tree = {{0xff527b31, 0xff082108}, {0xff083908, 0xff5a7329}, {0xff082908, 0xff316b21}, {0xff527b29, 0xff084a21}},
        .rock = {{0xff8c8484, 0xff5a5252}, {0xff9c9c94, 0xffa5a5a5}, {0xffa5a5a5, 0xff848484}, {0xff5a5252, 0xff9c9c94}},
        .meadow = {{0xff427318, 0xff8c9442}, {0xffb5ad4a, 0xff738c39}, {0xff8c8c39, 0xff6b7b29}, {0xff527331, 0xff5a8442}},
        .grass = {
            {0xff4a8431, 0xff4a7329}, {0xff527b29, 0xff4a7329}, {0xff526b29, 0xff5a8439}, {0xff397321, 0xff4a6b21},
            {0xff527b31, 0xff5a7331}, {0xff4a7329, 0xff5a7329}, {0xff4a6b18, 0xff316b21}, {0xff527b29, 0xff527329}
        },
        .road = {0xff736b63, 0xff4a3121}
    },
    // desert
    {
        .water = {{0xff4a84c6, 0xff4a7bc6}, {0xff4a84c6, 0xff4a7bc6}, {0xff4a84c6, 0xff5284c6}, {0xff4a7bbd, 0xff4a7bc6}},
        .tree = {{0xffa59c7b, 0xff6b7b18}, {0xff214210, 0xffada573}, {0xff526b21, 0xffcec6a5}, {0xffa59c7b, 0xff316321}},
        .rock = {{0xffa59494, 0xff736352}, {0xffa59c94, 0xffb5ada5}, {0xffb5ada5, 0xff8c847b}, {0xff736352, 0xffbdada5}},
        .meadow = {{0xff739c31, 0xff9cbd52}, {0xff7bb529, 0xff63ad21}, {0xff9cbd52, 0xff8c944a}, {0xff7ba539, 0xff739c31}},
        .grass = {
            {0xffbdbd9c, 0xffb5b594}, {0xffc6bda5, 0xffbdbda5}, {0xffbdbd9c, 0xffc6c6ad}, {0xffd6cead, 0xffc6bd9c},
            {0xffa59c7b, 0xffbdb594}, {0xffcecead, 0xffb5ad94}, {0xffc6c6a5, 0xffdedebd}, {0xffcecead, 0xffd6d6b5}
        },
        .road = {0xff6b5a52, 0xff4a4239}
    }
};

static struct {
    int absolute_x;
    int absolute_y;
    int width_tiles;
    int height_tiles;
    int x_offset;
    int y_offset;
} data;

static void foreach_map_tile(map_callback *callback)
{
    city_view_foreach_minimap_tile(
        data.x_offset, data.y_offset, data.absolute_x, data.absolute_y,
        data.width_tiles, data.height_tiles, callback);
}

static void set_bounds(int x_offset, int y_offset, int width, int height)
{
    data.width_tiles = width / 2;
    data.height_tiles = height;
    data.x_offset = x_offset;
    data.y_offset = y_offset;
    data.absolute_x = (VIEW_X_MAX - data.width_tiles) / 2;
    data.absolute_y = (VIEW_Y_MAX - data.height_tiles) / 2;

    // ensure even height
    data.absolute_y &= ~1;
}

static void draw_minimap_tile(int x_view, int y_view, int grid_offset)
{
    if (grid_offset < 0) {
        return;
    }

    int terrain = map_terrain_get(grid_offset);

    if (terrain & TERRAIN_BUILDING) {
        // Native huts/fields
        if (map_property_is_draw_tile(grid_offset)) {
            int image_id = image_group(GROUP_MINIMAP_BUILDING);
            switch (map_property_multi_tile_size(grid_offset)) {
                case 1: image_draw(image_id, x_view, y_view); break;
                case 2: image_draw(image_id + 1, x_view, y_view - 1); break;
            }
        }
    } else {
        int rand = map_random_get(grid_offset);
        const tile_color *color;
        const tile_color_set *set = &MINIMAP_COLOR_SETS[scenario_property_climate()];
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

void widget_scenario_minimap_draw(int x_offset, int y_offset, int width, int height)
{
    set_bounds(x_offset, y_offset, width + 2, height);
    graphics_set_clip_rectangle(x_offset, y_offset, width, height);
    foreach_map_tile(draw_minimap_tile);
    graphics_reset_clip_rectangle();
}
