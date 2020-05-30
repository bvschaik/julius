#include "minimap.h"

#include "building/building.h"
#include "city/view.h"
#include "figure/figure.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "input/scroll.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"
#include "scenario/property.h"

#include <stdlib.h>

enum {
    FIGURE_COLOR_NONE = 0,
    FIGURE_COLOR_SOLDIER = 1,
    FIGURE_COLOR_ENEMY = 2,
    FIGURE_COLOR_WOLF = 3
};

static const color_t ENEMY_COLOR_BY_CLIMATE[] = {
    COLOR_MINIMAP_ENEMY_CENTRAL,
    COLOR_MINIMAP_ENEMY_NORTHERN,
    COLOR_MINIMAP_ENEMY_DESERT
};

static struct {
    int absolute_x;
    int absolute_y;
    int width_tiles;
    int height_tiles;
    int x_offset;
    int y_offset;
    int width;
    int height;
    color_t enemy_color;
    color_t *cache;
    struct {
        int x;
        int y;
        int grid_offset;
    } mouse;
    int refresh_requested;
} data;

void widget_minimap_invalidate(void)
{
    data.refresh_requested = 1;
}

static void foreach_map_tile(map_callback *callback)
{
    city_view_foreach_minimap_tile(data.x_offset, data.y_offset,
                                   data.absolute_x, data.absolute_y,
                                   data.width_tiles, data.height_tiles,
                                   callback);
}

static void set_bounds(int x_offset, int y_offset, int width_tiles, int height_tiles)
{
    data.width_tiles = width_tiles;
    data.height_tiles = height_tiles;
    data.x_offset = x_offset;
    data.y_offset = y_offset;
    data.width = 2 * width_tiles;
    data.height = height_tiles;
    data.absolute_x = (VIEW_X_MAX - width_tiles) / 2;
    data.absolute_y = (VIEW_Y_MAX - height_tiles) / 2;

    int camera_x, camera_y;
    city_view_get_camera(&camera_x, &camera_y);
    int view_width_tiles, view_height_tiles;
    city_view_get_viewport_size_tiles(&view_width_tiles, &view_height_tiles);

    if ((map_grid_width() - width_tiles) / 2 > 0) {
        if (camera_x < data.absolute_x) {
            data.absolute_x = camera_x;
        } else if (camera_x > width_tiles + data.absolute_x - view_width_tiles) {
            data.absolute_x = view_width_tiles + camera_x - width_tiles;
        }
    }
    if ((2 * map_grid_height() - height_tiles) / 2 > 0) {
        if (camera_y < data.absolute_y) {
            data.absolute_y = camera_y;
        } else if (camera_y > height_tiles + data.absolute_y - view_height_tiles) {
            data.absolute_y = view_height_tiles + camera_y - height_tiles;
        }
    }
    // ensure even height
    data.absolute_y &= ~1;
}

static int has_figure_color(figure *f)
{
    int type = f->type;
    if (figure_is_legion(f)) {
        return FIGURE_COLOR_SOLDIER;
    }
    if (figure_is_enemy(f)) {
        return FIGURE_COLOR_ENEMY;
    }
    if (f->type == FIGURE_INDIGENOUS_NATIVE &&
        f->action_state == FIGURE_ACTION_159_NATIVE_ATTACKING) {
        return FIGURE_COLOR_ENEMY;
    }
    if (type == FIGURE_WOLF) {
        return FIGURE_COLOR_WOLF;
    }
    return FIGURE_COLOR_NONE;
}

static int draw_figure(int x_view, int y_view, int grid_offset)
{
    int color_type = map_figure_foreach_until(grid_offset, has_figure_color);
    if (color_type == FIGURE_COLOR_NONE) {
        return 0;
    }
    color_t color = COLOR_MINIMAP_WOLF;
    if (color_type == FIGURE_COLOR_SOLDIER) {
        color = COLOR_MINIMAP_SOLDIER;
    } else if (color_type == FIGURE_COLOR_ENEMY) {
        color = data.enemy_color;
    }
    graphics_draw_horizontal_line(x_view, x_view +1, y_view, color);
    return 1;
}

static void draw_minimap_tile(int x_view, int y_view, int grid_offset)
{
    if (grid_offset < 0) {
        image_draw(image_group(GROUP_MINIMAP_BLACK), x_view, y_view);
        return;
    }

    if (draw_figure(x_view, y_view, grid_offset)) {
        return;
    }

    int terrain = map_terrain_get(grid_offset);
    // exception for fort ground: display as empty land
    if (terrain & TERRAIN_BUILDING) {
        if (building_get(map_building_at(grid_offset))->type == BUILDING_FORT_GROUND) {
            terrain = 0;
        }
    }

    if (terrain & TERRAIN_BUILDING) {
        if (map_property_is_draw_tile(grid_offset)) {
            int image_id;
            building *b = building_get(map_building_at(grid_offset));
            if (b->house_size) {
                image_id = image_group(GROUP_MINIMAP_HOUSE);
            } else if (b->type == BUILDING_RESERVOIR) {
                image_id = image_group(GROUP_MINIMAP_AQUEDUCT) - 1;
            } else {
                image_id = image_group(GROUP_MINIMAP_BUILDING);
            }
            switch (map_property_multi_tile_size(grid_offset)) {
                case 1: image_draw(image_id, x_view, y_view); break;
                case 2: image_draw(image_id + 1, x_view, y_view - 1); break;
                case 3: image_draw(image_id + 2, x_view, y_view - 2); break;
                case 4: image_draw(image_id + 3, x_view, y_view - 3); break;
                case 5: image_draw(image_id + 4, x_view, y_view - 4); break;
            }
        }
    } else {
        int rand = map_random_get(grid_offset);
        int image_id;
        if (terrain & TERRAIN_WATER) {
            image_id = image_group(GROUP_MINIMAP_WATER) + (rand & 3);
        } else if (terrain & TERRAIN_SHRUB) {
            image_id = image_group(GROUP_MINIMAP_TREE) + (rand & 3);
        } else if (terrain & TERRAIN_TREE) {
            image_id = image_group(GROUP_MINIMAP_TREE) + (rand & 3);
        } else if (terrain & TERRAIN_ROCK) {
            image_id = image_group(GROUP_MINIMAP_ROCK) + (rand & 3);
        } else if (terrain & TERRAIN_ELEVATION) {
            image_id = image_group(GROUP_MINIMAP_ROCK) + (rand & 3);
        } else if (terrain & TERRAIN_ROAD) {
            image_id = image_group(GROUP_MINIMAP_ROAD);
        } else if (terrain & TERRAIN_AQUEDUCT) {
            image_id = image_group(GROUP_MINIMAP_AQUEDUCT);
        } else if (terrain & TERRAIN_WALL) {
            image_id = image_group(GROUP_MINIMAP_WALL);
        } else if (terrain & TERRAIN_MEADOW) {
            image_id = image_group(GROUP_MINIMAP_MEADOW) + (rand & 3);
        } else {
            image_id = image_group(GROUP_MINIMAP_EMPTY_LAND) + (rand & 7);
        }
        image_draw(image_id, x_view, y_view);
    }
}

static void draw_viewport_rectangle(void)
{
    int camera_x, camera_y;
    int camera_pixels_x, camera_pixels_y;
    city_view_get_camera(&camera_x, &camera_y);
    city_view_get_pixel_offset(&camera_pixels_x, &camera_pixels_y);
    int view_width_tiles, view_height_tiles;
    city_view_get_viewport_size_tiles(&view_width_tiles, &view_height_tiles);

    int x_offset = data.x_offset + 2 * (camera_x - data.absolute_x) - 2 + camera_pixels_x / 30;
    if (x_offset < data.x_offset) {
        x_offset = data.x_offset;
    }
    if (x_offset + 2 * view_width_tiles + 4 > data.x_offset + data.width_tiles) {
        x_offset -= 2;
    }
    int y_offset = data.y_offset + camera_y - data.absolute_y + 2;
    graphics_draw_rect(x_offset, y_offset,
        view_width_tiles * 2 + 4,
        view_height_tiles - 4,
        COLOR_MINIMAP_VIEWPORT);
}

static void prepare_minimap_cache(int width, int height)
{
    if (width != data.width || height != data.height) {
        free(data.cache);
        data.cache = (color_t *)malloc(sizeof(color_t) * width * height);
    }
}

static void cache_minimap(void)
{
    graphics_save_to_buffer(data.x_offset, data.y_offset, data.width, data.height, data.cache);
}

static void draw_minimap(void)
{
    graphics_set_clip_rectangle(data.x_offset, data.y_offset, data.width, data.height);
    foreach_map_tile(draw_minimap_tile);
    cache_minimap();
    draw_viewport_rectangle();
    graphics_reset_clip_rectangle();
}

static void draw_uncached(int x_offset, int y_offset, int width_tiles, int height_tiles)
{
    data.enemy_color = ENEMY_COLOR_BY_CLIMATE[scenario_property_climate()];
    prepare_minimap_cache(2 * width_tiles, height_tiles);
    set_bounds(x_offset, y_offset, width_tiles, height_tiles);
    draw_minimap();
}

void draw_using_cache(int x_offset, int y_offset, int width_tiles, int height_tiles, int is_scrolling)
{
    if (width_tiles * 2 != data.width || height_tiles != data.height || x_offset != data.x_offset) {
        draw_uncached(x_offset, y_offset, width_tiles, height_tiles);
        return;
    }

    if (is_scrolling) {
        int old_absolute_x = data.absolute_x;
        int old_absolute_y = data.absolute_y;
        set_bounds(x_offset, y_offset, width_tiles, height_tiles);
        if (data.absolute_x != old_absolute_x || data.absolute_y != old_absolute_y) {
            draw_minimap();
            return;
        }
    }

    graphics_set_clip_rectangle(x_offset, y_offset, 2 * width_tiles, height_tiles);
    graphics_draw_from_buffer(x_offset, y_offset, data.width, data.height, data.cache);
    draw_viewport_rectangle();
    graphics_reset_clip_rectangle();
}

void widget_minimap_draw(int x_offset, int y_offset, int width_tiles, int height_tiles, int force)
{
    if (data.refresh_requested || scroll_in_progress() || force) {
        if (data.refresh_requested) {
            draw_uncached(x_offset, y_offset, width_tiles, height_tiles);
            data.refresh_requested = 0;
        } else {
            draw_using_cache(x_offset, y_offset, width_tiles, height_tiles, scroll_in_progress());
        }
        graphics_draw_horizontal_line(x_offset - 1, x_offset - 1 + width_tiles * 2, y_offset - 1, COLOR_MINIMAP_DARK);
        graphics_draw_vertical_line(x_offset - 1, y_offset, y_offset + height_tiles, COLOR_MINIMAP_DARK);
        graphics_draw_vertical_line(x_offset - 1 + width_tiles * 2, y_offset, y_offset + height_tiles, COLOR_MINIMAP_LIGHT);
    }
}

static void update_mouse_grid_offset(int x_view, int y_view, int grid_offset)
{
    if (data.mouse.y == y_view && (data.mouse.x == x_view || data.mouse.x == x_view + 1)) {
        data.mouse.grid_offset = grid_offset < 0 ? 0 : grid_offset;
    }
}

static int get_mouse_grid_offset(const mouse *m)
{
    data.mouse.x = m->x;
    data.mouse.y = m->y;
    data.mouse.grid_offset = 0;
    foreach_map_tile(update_mouse_grid_offset);
    return data.mouse.grid_offset;
}

static int is_in_minimap(const mouse *m)
{
    if (m->x >= data.x_offset && m->x < data.x_offset + data.width &&
        m->y >= data.y_offset && m->y < data.y_offset + data.height) {
        return 1;
    }
    return 0;
}

int widget_minimap_handle_mouse(const mouse *m)
{
    if ((m->left.went_down || m->right.went_down) && is_in_minimap(m)) {
        int grid_offset = get_mouse_grid_offset(m);
        if (grid_offset > 0) {
            city_view_go_to_grid_offset(grid_offset);
            widget_minimap_invalidate();
            return 1;
        }
    }
    return 0;
}
