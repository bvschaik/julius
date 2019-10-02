#include "map_editor.h"

#include "city/view.h"
#include "editor/tool.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "input/scroll.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/point.h"
#include "map/property.h"
#include "sound/city.h"
#include "widget/city_figure.h"
#include "widget/map_editor_tool.h"

static struct {
    map_tile current_tile;
} data;

static struct {
    time_millis last_water_animation_time;
    int advance_water_animation;

    int image_id_water_first;
    int image_id_water_last;
} draw_context;

static void init_draw_context(void)
{
    draw_context.advance_water_animation = 0;
    time_millis now = time_get_millis();
    if (now - draw_context.last_water_animation_time > 60) {
        draw_context.last_water_animation_time = now;
        draw_context.advance_water_animation = 1;
    }
    draw_context.image_id_water_first = image_group(GROUP_TERRAIN_WATER);
    draw_context.image_id_water_last = 5 + draw_context.image_id_water_first;
}

static void draw_footprint(int x, int y, int grid_offset)
{
    if (grid_offset < 0) {
        // Outside map: draw black tile
        image_draw_isometric_footprint_from_draw_tile(image_group(GROUP_TERRAIN_BLACK), x, y, 0);
    } else if (map_property_is_draw_tile(grid_offset)) {
        // Valid grid_offset and leftmost tile -> draw
        color_t color_mask = 0;
        int image_id = map_image_at(grid_offset);
        if (draw_context.advance_water_animation &&
            image_id >= draw_context.image_id_water_first &&
            image_id <= draw_context.image_id_water_last) {
            image_id++;
            if (image_id > draw_context.image_id_water_last) {
                image_id = draw_context.image_id_water_first;
            }
            map_image_set(grid_offset, image_id);
        }
        image_draw_isometric_footprint_from_draw_tile(image_id, x, y, color_mask);
    }
}

static void draw_top(int x, int y, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }
    int image_id = map_image_at(grid_offset);
    color_t color_mask = 0;
    image_draw_isometric_top_from_draw_tile(image_id, x, y, color_mask);
}

static void draw_flags(int x, int y, int grid_offset)
{
    int figure_id = map_figure_at(grid_offset);
    while (figure_id) {
        figure *f = figure_get(figure_id);
        if (!f->is_ghost) {
            city_draw_figure(f, x, y);
        }
        figure_id = f->next_figure_id_on_same_tile;
    }
}

static void set_city_clip_rectangle(void)
{
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    graphics_set_clip_rectangle(x, y, width, height);
}

void widget_map_editor_draw(void)
{
    set_city_clip_rectangle();

    init_draw_context();
    city_view_foreach_map_tile(draw_footprint);
    city_view_foreach_valid_map_tile(draw_flags, draw_top, 0);
    map_editor_tool_draw(&data.current_tile);

    graphics_reset_clip_rectangle();
}

static void update_city_view_coords(const mouse *m, map_tile *tile)
{
    tile->grid_offset = city_view_pixels_to_grid_offset(m->x, m->y);
    if (tile->grid_offset) {
        tile->x = map_grid_offset_to_x(tile->grid_offset);
        tile->y = map_grid_offset_to_y(tile->grid_offset);
    } else {
        tile->x = tile->y = 0;
    }
}

static void scroll_map(int direction)
{
    if (city_view_scroll(direction)) {
        sound_city_decay_views();
    }
}

void widget_map_editor_handle_mouse(const mouse *m)
{
    scroll_map(scroll_get_direction(m));
    map_tile *tile = &data.current_tile;
    update_city_view_coords(m, tile);

    if (m->left.went_down) {
        editor_tool_start_use(tile);
        editor_tool_update_use(tile);
    } else if (m->left.is_down) {
        editor_tool_update_use(tile);
    }
    if (m->left.went_up) {
        editor_tool_end_use(tile);
    }
    if (m->right.went_down) {
        editor_tool_deactivate();
    }
}
