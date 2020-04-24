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
#include "sound/effect.h"
#include "widget/city_figure.h"
#include "widget/map_editor_tool.h"

static struct {
    map_tile current_tile;
    int selected_grid_offset;
    int new_start_grid_offset;
    int capture_input;
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

static void update_city_view_coords(int x, int y, map_tile *tile)
{
    view_tile view;
    if (city_view_pixels_to_view_tile(x, y, &view)) {
        tile->grid_offset = city_view_tile_to_grid_offset(&view);
        city_view_set_selected_view_tile(&view);
        tile->x = map_grid_offset_to_x(tile->grid_offset);
        tile->y = map_grid_offset_to_y(tile->grid_offset);
    } else {
        tile->grid_offset = tile->x = tile->y = 0;
    }
}

static void scroll_map(const mouse *m)
{
    pixel_offset delta;
    scroll_get_delta(m, &delta, SCROLL_TYPE_CITY);
    if (city_view_scroll(delta.x, delta.y)) {
        sound_city_decay_views();
    } else {
        pixel_offset position;
        if (scroll_decay(&position)) {
            city_view_set_camera_from_pixel_position(position.x, position.y);
            sound_city_decay_views();
        }
    }
}

static int touch_in_map(const touch *t)
{
    int x_offset, y_offset, width, height;
    city_view_get_viewport(&x_offset, &y_offset, &width, &height);

    touch_coords coords = t->current_point;
    coords.x -= x_offset;
    coords.y -= y_offset;

    return (coords.x >= 0 && coords.x < width && coords.y >= 0 && coords.y < height);
}

static void handle_touch_scroll(const touch *t)
{
    if (editor_tool_is_active()) {
        if (t->has_started) {
            int x_offset, y_offset, width, height;
            city_view_get_viewport(&x_offset, &y_offset, &width, &height);
            scroll_set_custom_margins(x_offset, y_offset, width, height);
        }
        if (t->has_ended) {
            scroll_restore_margins();
        }
        return;
    }
    scroll_restore_margins();
    pixel_offset camera_pixel_position;

    if (!data.capture_input) {
        return;
    }
    int was_click = touch_was_click(get_latest_touch());
    if (t->has_started || was_click) {
        city_view_get_camera_in_pixels(&camera_pixel_position.x, &camera_pixel_position.y);
        scroll_start_touch_drag(&camera_pixel_position, (was_click) ? t->current_point : t->start_point);
        return;
    }

    if (!touch_not_click(t)) {
        return;
    }

    touch_coords original = scroll_get_original_touch_position();
    if (scroll_move_touch_drag(original.x, original.y, t->current_point.x, t->current_point.y, &camera_pixel_position)) {
        city_view_set_camera_from_pixel_position(camera_pixel_position.x, camera_pixel_position.y);
        sound_city_decay_views();
    }

    if (t->has_ended) {
        scroll_end_touch_drag(1);
    }
}

static void handle_last_touch(void)
{
    const touch *last = get_latest_touch();
    if (last->in_use && touch_was_click(last)) {
        editor_tool_deactivate();
    }
}

static int handle_cancel_construction_button(const touch *t)
{
    if (!editor_tool_is_active()) {
        return 0;
    }
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    int box_size = 5 * 16;
    width -= box_size;

    if (t->current_point.x < width || t->current_point.x >= width + box_size ||
        t->current_point.y < 24 || t->current_point.y >= 40 + box_size) {
        return 0;
    }
    editor_tool_deactivate();
    return 1;
}

static void handle_first_touch(map_tile *tile)
{
    const touch *first = get_earliest_touch();

    if (touch_was_click(first)) {
        if (handle_cancel_construction_button(first)) {
            return;
        }
    }

    handle_touch_scroll(first);

    if (!touch_in_map(first)) {
        return;
    }

    if (editor_tool_is_updatable()) {
        if (!editor_tool_is_in_use()) {
            if (first->has_started) {
                editor_tool_start_use(tile);
                data.new_start_grid_offset = 0;
            }
        } else {
            if (first->has_started) {
                if (data.selected_grid_offset != tile->grid_offset) {
                    data.new_start_grid_offset = tile->grid_offset;
                }
            }
            if (touch_not_click(first) && data.new_start_grid_offset) {
                data.new_start_grid_offset = 0;
                data.selected_grid_offset = 0;
                editor_tool_deactivate();
                editor_tool_start_use(tile);
            }
            editor_tool_update_use(tile);
            if (data.selected_grid_offset != tile->grid_offset) {
                data.selected_grid_offset = 0;
            }
            if (first->has_ended) {
                if (data.selected_grid_offset == tile->grid_offset) {
                    editor_tool_end_use(tile);
                    widget_map_editor_clear_current_tile();
                    data.new_start_grid_offset = 0;
                } else {
                    data.selected_grid_offset = tile->grid_offset;
                }
            }
        }
        return;
    }

    if (editor_tool_is_brush()) {
        if (first->has_started) {
            editor_tool_start_use(tile);
        }
        editor_tool_update_use(tile);
        if (first->has_ended) {
            editor_tool_end_use(tile);
        }
        return;
    }

    if (touch_was_click(first) && first->has_ended && data.capture_input &&
        data.selected_grid_offset == tile->grid_offset) {
        editor_tool_start_use(tile);
        editor_tool_update_use(tile);
        editor_tool_end_use(tile);
        widget_map_editor_clear_current_tile();
    } else if (first->has_ended) {
        data.selected_grid_offset = tile->grid_offset;
    }
}

static void handle_touch(void)
{
    const touch *first = get_earliest_touch();
    if (!first->in_use) {
        scroll_restore_margins();
        return;
    }

    map_tile *tile = &data.current_tile;
    if (!editor_tool_is_in_use() || touch_in_map(first)) {
        update_city_view_coords(first->current_point.x, first->current_point.y, tile);
    }

    if (first->has_started && touch_in_map(first)) {
        data.capture_input = 1;
        scroll_restore_margins();
    }

    handle_last_touch();
    handle_first_touch(tile);

    if (first->has_ended) {
        data.capture_input = 0;
    }
}

void widget_map_editor_handle_input(const mouse *m, const hotkeys *h)
{
    if (m->is_touch) {
        scroll_map(m);
        handle_touch();
        return;
    }

    if (m->right.is_down && !m->right.went_down) {
        pixel_offset camera_pixel_position;

        if (scroll_move_mouse_drag(m, &camera_pixel_position)) {
            city_view_set_camera_from_pixel_position(camera_pixel_position.x, camera_pixel_position.y);
        }
    } else if (!m->right.went_up) {
        scroll_map(m);
    }

    if (m->right.went_down && !editor_tool_is_active()) {
        pixel_offset camera_pixel_position;
        city_view_get_camera_in_pixels(&camera_pixel_position.x, &camera_pixel_position.y);
        scroll_start_mouse_drag(m, &camera_pixel_position);
    }
    if (m->right.went_up) {
        if (!editor_tool_is_active()) {
            int has_scrolled = scroll_end_mouse_drag();
            if (!has_scrolled) {
                editor_tool_deactivate();
            }
        } else {
            editor_tool_deactivate();
        }
    }

    map_tile *tile = &data.current_tile;
    update_city_view_coords(m->x, m->y, tile);

    if (!tile->grid_offset) {
        return;
    }

    if (m->left.went_down) {
        if (!editor_tool_is_in_use()) {
            editor_tool_start_use(tile);
        }
        editor_tool_update_use(tile);
    } else if (m->left.is_down || editor_tool_is_in_use()) {
        editor_tool_update_use(tile);
    }
    if (m->left.went_up) {
        editor_tool_end_use(tile);
        sound_effect_play(SOUND_EFFECT_BUILD);
    }
    if (h->escape_pressed) {
        if (editor_tool_is_active()) {
            editor_tool_deactivate();
        } else {
            hotkey_handle_escape();
        }
    }
}

void widget_map_editor_clear_current_tile(void)
{
    data.selected_grid_offset = 0;
    data.current_tile.grid_offset = 0;
}
