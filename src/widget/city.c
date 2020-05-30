#include "city.h"

#include "building/construction.h"
#include "building/properties.h"
#include "city/finance.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/direction.h"
#include "core/string.h"
#include "figure/formation_legion.h"
#include "game/settings.h"
#include "game/state.h"
#include "graphics/graphics.h"
#include "graphics/menu.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/scroll.h"
#include "input/zoom.h"
#include "input/touch.h"
#include "map/building.h"
#include "map/grid.h"
#include "scenario/property.h"
#include "sound/city.h"
#include "sound/speech.h"
#include "sound/effect.h"
#include "widget/city_with_overlay.h"
#include "widget/city_without_overlay.h"
#include "widget/minimap.h"
#include "window/building_info.h"
#include "window/city.h"

static struct {
    map_tile current_tile;
    map_tile selected_tile;
    int new_start_grid_offset;
    int capture_input;
} data;

static void set_city_scaled_clip_rectangle(void)
{
    int x, y, width, height;
    city_view_get_scaled_viewport(&x, &y, &width, &height);
    graphics_set_clip_rectangle(x, y, width, height);
}

static void set_city_unscaled_clip_rectangle(void)
{
    int x, y, width, height;
    city_view_get_unscaled_viewport(&x, &y, &width, &height);
    graphics_set_clip_rectangle(x, y, width, height);
}

static void update_zoom_level(void)
{
    int zoom = city_view_get_scale();
    pixel_offset offset;
    city_view_get_camera_in_pixels(&offset.x, &offset.y);
    if (zoom_update_value(&zoom, &offset)) {
        city_view_set_scale(zoom);
        city_view_set_camera_from_pixel_position(offset.x, offset.y);
        sound_city_decay_views();
    }
}

void widget_city_draw(void)
{
    if (config_get(CONFIG_UI_ZOOM)) {
        update_zoom_level();
        graphics_set_active_canvas(CANVAS_CITY);
    }
    set_city_scaled_clip_rectangle();

    if (game_state_overlay()) {
        city_with_overlay_draw(&data.current_tile);
    } else {
        city_without_overlay_draw(0, 0, &data.current_tile);
    }

    graphics_set_active_canvas(CANVAS_UI);
}

void widget_city_draw_for_figure(int figure_id, pixel_coordinate *coord)
{
    set_city_scaled_clip_rectangle();

    city_without_overlay_draw(figure_id, coord, &data.current_tile);

    graphics_reset_clip_rectangle();
}

int widget_city_draw_construction_cost_and_size(void)
{
    if (!building_construction_in_progress()) {
        return 0;
    }
    if (scroll_in_progress()) {
        return 0;
    }
    int size_x, size_y;
    int cost = building_construction_cost();
    int has_size = building_construction_size(&size_x, &size_y);
    if (!cost && !has_size) {
        return 0;
    }
    set_city_unscaled_clip_rectangle();
    int x, y;
    city_view_get_selected_tile_pixels(&x, &y);
    int inverted_scale = calc_percentage(100, city_view_get_scale());
    x = calc_adjust_with_percentage(x, inverted_scale);
    y = calc_adjust_with_percentage(y, inverted_scale);

    if (cost) {
        color_t color;
        if (cost <= city_finance_treasury()) {
            // Color blind friendly
            color = scenario_property_climate() == CLIMATE_DESERT ? COLOR_FONT_ORANGE : COLOR_FONT_ORANGE_LIGHT;
        } else {
            color = COLOR_FONT_RED;
        }
        text_draw_number_colored(cost, '@', " ", x + 58 + 1, y + 1, FONT_NORMAL_PLAIN, COLOR_BLACK);
        text_draw_number_colored(cost, '@', " ", x + 58, y, FONT_NORMAL_PLAIN, color);
    }
    if (has_size) {
        int width = -text_get_width(string_from_ascii("  "), FONT_SMALL_PLAIN);
        width += text_draw_number_colored(size_x, '@', "x", x - 15 + 1, y + 25 + 1, FONT_SMALL_PLAIN, COLOR_BLACK);
        text_draw_number_colored(size_x, '@', "x", x - 15, y + 25, FONT_SMALL_PLAIN, COLOR_FONT_YELLOW);
        text_draw_number_colored(size_y, '@', " ", x - 15 + width + 1, y + 25 + 1, FONT_SMALL_PLAIN, COLOR_BLACK);
        text_draw_number_colored(size_y, '@', " ", x - 15 + width, y + 25, FONT_SMALL_PLAIN, COLOR_FONT_YELLOW);
    }
    graphics_reset_clip_rectangle();
    return 1;
}

// INPUT HANDLING

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

static int handle_right_click_allow_building_info(const map_tile *tile)
{
    int allow = 1;
    if (!window_is(WINDOW_CITY)) {
        allow = 0;
    }
    window_city_show();

    if (!tile->grid_offset) {
        allow = 0;
    }
    if (allow && city_has_warnings()) {
        city_warning_clear_all();
        allow = 0;
    }
    return allow;
}

static int handle_legion_click(const map_tile *tile)
{
    if (tile->grid_offset) {
        int formation_id = formation_legion_at_grid_offset(tile->grid_offset);
        if (formation_id > 0 && !formation_get(formation_id)->in_distant_battle) {
            window_city_military_show(formation_id);
            return 1;
        }
    }
    return 0;
}

static void build_start(const map_tile *tile)
{
    if (tile->grid_offset) { // Allow building on paused
        building_construction_start(tile->x, tile->y, tile->grid_offset);
    }
}

static void build_move(const map_tile *tile)
{
    if (!building_construction_in_progress()) {
        return;
    }
    building_construction_update(tile->x, tile->y, tile->grid_offset);
}

static void build_end(void)
{
    if (building_construction_in_progress()) {
        if (building_construction_type() != BUILDING_NONE) {
            sound_effect_play(SOUND_EFFECT_BUILD);
        }
        building_construction_place();
    }
}

static void scroll_map(const mouse *m)
{
    pixel_offset delta;
    if (scroll_get_delta(m, &delta, SCROLL_TYPE_CITY)) {
        city_view_scroll(delta.x, delta.y);
        sound_city_decay_views();
    }
}

static int adjust_offset_for_orientation(int grid_offset, int size)
{
    switch (city_view_orientation()) {
        case DIR_0_TOP:
            return map_grid_add_delta(grid_offset, -size + 1, -size + 1);
        case DIR_2_RIGHT:
            return map_grid_add_delta(grid_offset, 0, -size + 1);
        case DIR_6_LEFT:
            return map_grid_add_delta(grid_offset, -size + 1, 0);
        default:
            return grid_offset;
    }
}

static int has_confirmed_construction(int ghost_offset, int tile_offset, int range_size)
{
    tile_offset = adjust_offset_for_orientation(tile_offset, range_size);
    int x = map_grid_offset_to_x(tile_offset);
    int y = map_grid_offset_to_y(tile_offset);
    if (ghost_offset <= 0 || !map_grid_is_inside(x, y, range_size)) {
        return 0;
    }
    for (int dy = 0; dy < range_size; dy++) {
        for (int dx = 0; dx < range_size; dx++) {
            if (ghost_offset == tile_offset + map_grid_delta(dx, dy)) {
                return 1;
            }
        }
    }
    return 0;
}

static int input_coords_in_city(int x, int y)
{
    int x_offset, y_offset, width, height;
    city_view_get_unscaled_viewport(&x_offset, &y_offset, &width, &height);

    x -= x_offset;
    y -= y_offset;

    return (x >= 0 && x < width && y >= 0 && y < height);
}

static void handle_touch_scroll(const touch *t)
{
    if (building_construction_type()) {
        if (t->has_started) {
            int x_offset, y_offset, width, height;
            city_view_get_unscaled_viewport(&x_offset, &y_offset, &width, &height);
            scroll_set_custom_margins(x_offset, y_offset, width, height);
        }
        if (t->has_ended) {
            scroll_restore_margins();
        }
        return;
    }
    scroll_restore_margins();

    if (!data.capture_input) {
        return;
    }
    int was_click = touch_was_click(get_latest_touch());
    if (t->has_started || was_click) {
        scroll_drag_start(1);
        return;
    }
    if (!touch_not_click(t)) {
        return;
    }
    if (t->has_ended) {
        scroll_drag_end();
    }
}

static void handle_touch_zoom(const touch *first, const touch *last)
{
    if (touch_not_click(first)) {
        zoom_update_touch(first, last, city_view_get_scale());
    }
    if (first->has_ended || last->has_ended) {
        zoom_end_touch();
    }
}

static void handle_last_touch(void)
{
    const touch *last = get_latest_touch();
    if (!last->in_use) {
        return;
    }
    if (touch_was_click(last)) {
        building_construction_cancel();
        return;
    }
    if (touch_not_click(last)) {
        handle_touch_zoom(get_earliest_touch(), last);
    }
}

static int handle_cancel_construction_button(const touch *t)
{
    if (!building_construction_type()) {
        return 0;
    }
    int x, y, width, height;
    city_view_get_unscaled_viewport(&x, &y, &width, &height);
    int box_size = 5 * 16;
    width -= box_size;

    if (t->current_point.x < width || t->current_point.x >= width + box_size ||
        t->current_point.y < 24 || t->current_point.y >= 40 + box_size) {
        return 0;
    }
    building_construction_cancel();
    return 1;
}

static void handle_first_touch(map_tile *tile)
{
    const touch *first = get_earliest_touch();
    building_type type = building_construction_type();

    if (touch_was_click(first)) {
        if (handle_cancel_construction_button(first) || handle_legion_click(tile)) {
            return;
        }
        if (type == BUILDING_NONE && handle_right_click_allow_building_info(tile)) {
            scroll_drag_end();
            data.capture_input = 0;
            window_building_info_show(tile->grid_offset);
            return;
        }
    }

    handle_touch_scroll(first);

    if (!input_coords_in_city(first->current_point.x, first->current_point.y) || type == BUILDING_NONE) {
        return;
    }

    if (building_construction_is_updatable()) {
        if (!building_construction_in_progress()) {
            if (first->has_started) {
                build_start(tile);
                data.new_start_grid_offset = 0;
            }
        } else {
            if (first->has_started) {
                if (data.selected_tile.grid_offset != tile->grid_offset) {
                    data.new_start_grid_offset = tile->grid_offset;
                }
            }
            if (touch_not_click(first) && data.new_start_grid_offset) {
                data.new_start_grid_offset = 0;
                data.selected_tile.grid_offset = 0;
                building_construction_cancel();
                build_start(tile);
            }
            build_move(tile);
            if (data.selected_tile.grid_offset != tile->grid_offset) {
                data.selected_tile.grid_offset = 0;
            }
            if (first->has_ended) {
                if (data.selected_tile.grid_offset == tile->grid_offset) {
                    build_end();
                    widget_city_clear_current_tile();
                    data.new_start_grid_offset = 0;
                } else {
                    data.selected_tile.grid_offset = tile->grid_offset;
                }
            }
        }
        return;
    }

    int size = building_properties_for_type(type)->size;
    if (type == BUILDING_WAREHOUSE) {
        size = 3;
    }

    if (touch_was_click(first) && first->has_ended && data.capture_input &&
        has_confirmed_construction(data.selected_tile.grid_offset, tile->grid_offset, size)) {
        build_start(&data.selected_tile);
        build_move(&data.selected_tile);
        build_end();
        widget_city_clear_current_tile();
    } else if (first->has_ended) {
        data.selected_tile = *tile;
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
    if (!building_construction_in_progress() || input_coords_in_city(first->current_point.x, first->current_point.y)) {
        update_city_view_coords(first->current_point.x, first->current_point.y, tile);
    }

    if (first->has_started && input_coords_in_city(first->current_point.x, first->current_point.y)) {
        data.capture_input = 1;
        scroll_restore_margins();
    }

    handle_last_touch();
    handle_first_touch(tile);

    if (first->has_ended) {
        data.capture_input = 0;
    }

    building_construction_reset_draw_as_constructing();
}

int widget_city_has_input(void)
{
    return data.capture_input;
}

static void handle_mouse(const mouse *m)
{
    map_tile *tile = &data.current_tile;
    update_city_view_coords(m->x, m->y, tile);
    zoom_map(m);    
    building_construction_reset_draw_as_constructing();
    if (m->left.went_down) {
        if (handle_legion_click(tile)) {
            return;
        }
        if (!building_construction_in_progress()) {
            build_start(tile);
        }
        build_move(tile);
    } else if (m->left.is_down || building_construction_in_progress()) {
        build_move(tile);
    }
    if (m->left.went_up) {
        build_end();
    }
    if (m->right.went_down && input_coords_in_city(m->x, m->y) && !building_construction_type()) {
        scroll_drag_start(0);
    }
    if (m->right.went_up) {
        if (!building_construction_type()) {
            int has_scrolled = scroll_drag_end();
            if (!has_scrolled && handle_right_click_allow_building_info(tile)) {
                window_building_info_show(tile->grid_offset);
            }
        } else {
            building_construction_cancel();
        }
    }
}

void widget_city_handle_input(const mouse *m, const hotkeys *h)
{
    scroll_map(m);

    if (m->is_touch) {
        handle_touch();
    } else {
        handle_mouse(m);
    }

    if (h->escape_pressed) {
        if (building_construction_type()) {
            building_construction_cancel();
        } else {
            hotkey_handle_escape();
        }
    }
}

static void military_map_click(int legion_formation_id, const map_tile *tile)
{
    if (!tile->grid_offset) {
        window_city_show();
        return;
    }
    formation *m = formation_get(legion_formation_id);
    if (m->in_distant_battle || m->cursed_by_mars) {
        return;
    }
    int other_formation_id = formation_legion_at_building(tile->grid_offset);
    if (other_formation_id && other_formation_id == legion_formation_id) {
        formation_legion_return_home(m);
    } else {
        formation_legion_move_to(m, tile->x, tile->y);
        sound_speech_play_file("wavs/cohort5.wav");
    }
    window_city_show();
}

void widget_city_handle_input_military(const mouse *m, const hotkeys *h, int legion_formation_id)
{
    map_tile *tile = &data.current_tile;
    update_city_view_coords(m->x, m->y, tile);
    if (!city_view_is_sidebar_collapsed() && widget_minimap_handle_mouse(m)) {
        return;
    }
    if (m->is_touch) {
        const touch *t = get_earliest_touch();
        if (!t->in_use) {
            return;
        }
        if (t->has_started) {
            data.capture_input = 1;
        }
        handle_touch_scroll(t);
        if (t->has_ended) {
            data.capture_input = 0;
        }
    }
    scroll_map(m);
    if (m->right.went_up || h->escape_pressed) {
        data.capture_input = 0;
        city_warning_clear_all();
        window_city_show();
    } else {
        update_city_view_coords(m->x, m->y, tile);
        if ((!m->is_touch && m->left.went_down) || (m->is_touch && m->left.went_up && touch_was_click(get_earliest_touch()))) {
            military_map_click(legion_formation_id, tile);
        }
    }
}

void widget_city_get_tooltip(tooltip_context *c)
{
    if (setting_tooltips() == TOOLTIPS_NONE) {
        return;
    }
    if (!window_is(WINDOW_CITY)) {
        return;
    }
    if (data.current_tile.grid_offset == 0) {
        return;
    }
    int grid_offset = data.current_tile.grid_offset;
    int building_id = map_building_at(grid_offset);
    int overlay = game_state_overlay();
    // regular tooltips
    if (overlay == OVERLAY_NONE && building_id && building_get(building_id)->type == BUILDING_SENATE_UPGRADED) {
        c->type = TOOLTIP_SENATE;
        c->high_priority = 1;
        return;
    }
    // overlay tooltips
    if (overlay != OVERLAY_NONE) {
        c->text_group = 66;
        c->text_id = city_with_overlay_get_tooltip_text(c, grid_offset);
        if (c->text_id) {
            c->type = TOOLTIP_OVERLAY;
            c->high_priority = 1;
        }
    }
}

void widget_city_clear_current_tile(void)
{
    data.selected_tile.x = -1;
    data.selected_tile.y = -1;
    data.selected_tile.grid_offset = 0;
    data.current_tile.grid_offset = 0;
}
