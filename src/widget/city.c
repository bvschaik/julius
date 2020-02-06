#include "city.h"

#include "building/construction.h"
#include "city/finance.h"
#include "city/view.h"
#include "city/warning.h"
#include "figure/formation_legion.h"
#include "game/settings.h"
#include "game/state.h"
#include "graphics/graphics.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/scroll.h"
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
    int capture_input;
} data;

static void set_city_clip_rectangle(void)
{
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    graphics_set_clip_rectangle(x, y, width, height);
}

void widget_city_draw(void)
{
    set_city_clip_rectangle();

    if (game_state_overlay()) {
        city_with_overlay_draw(&data.current_tile);
    } else {
        city_without_overlay_draw(0, 0, &data.current_tile);
    }

    graphics_reset_clip_rectangle();
}

void widget_city_draw_for_figure(int figure_id, pixel_coordinate *coord)
{
    set_city_clip_rectangle();

    city_without_overlay_draw(figure_id, coord, &data.current_tile);

    graphics_reset_clip_rectangle();
}

void widget_city_draw_construction_cost(void)
{
    if (!building_construction_in_progress()) {
        return;
    }
    if (scroll_in_progress()) {
        return;
    }
    int cost = building_construction_cost();
    if (!cost) {
        return;
    }
    set_city_clip_rectangle();
    color_t color;
    if (cost <= city_finance_treasury()) {
        // Color blind friendly
        color = (scenario_property_climate() == CLIMATE_DESERT) ? COLOR_ORANGE : COLOR_ORANGE_LIGHT;
    } else {
        color = COLOR_RED;
    }
    int x, y;
    city_view_get_selected_tile_pixels(&x, &y);
    text_draw_number_colored(cost, '@', " ", x + 58 + 1, y + 1, FONT_NORMAL_PLAIN, COLOR_BLACK);
    text_draw_number_colored(cost, '@', " ", x + 58, y, FONT_NORMAL_PLAIN, color);
    graphics_reset_clip_rectangle();
}

// INPUT HANDLING

static void update_city_view_coords(int x, int y, map_tile *tile)
{
    tile->grid_offset = city_view_pixels_to_grid_offset(x, y);
    if (tile->grid_offset) {
        tile->x = map_grid_offset_to_x(tile->grid_offset);
        tile->y = map_grid_offset_to_y(tile->grid_offset);
    } else {
        tile->x = tile->y = 0;
    }
}

static int handle_right_click_allow_building_info(const map_tile *tile)
{
    int allow = 1;
    if (!window_is(WINDOW_CITY)) {
        allow = 0;
    }
    if (building_construction_type()) {
        allow = 0;
    }
    building_construction_set_type(BUILDING_NONE);
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
    if (tile->grid_offset /*&& !game_state_is_paused()*/) { // TODO FIXME
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

static void scroll_map(int direction)
{
    if (city_view_scroll(direction)) {
        sound_city_decay_views();
    } else {
        view_tile position;
        if (scroll_decay(&position)) {
            city_view_set_camera(position.x, position.y);
            sound_city_decay_views();
        }
    }
}

static int touch_in_city(const touch *t)
{
    int x_offset, y_offset, width, height;
    city_view_get_viewport(&x_offset, &y_offset, &width, &height);

    touch_coords coords = t->current_point;
    coords.x -= x_offset;
    coords.y -= y_offset;

    return (coords.x >= 0 && coords.x < width && coords.y >= 0 && coords.y < height);
}

static void widget_city_handle_touch_scroll(const touch *t)
{
    if (building_construction_type()) {
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
    view_tile camera_position;
    
    if (!data.capture_input) {
        return;
    }
    int was_click = touch_was_click(get_latest_touch());
    if (t->has_started || was_click) {
        city_view_get_camera(&camera_position.x, &camera_position.y);
        scroll_start_touch_drag(&camera_position, (was_click) ? t->current_point : t->start_point);
        return;
    }

    if (!touch_not_click(t)) {
        return;
    }

    touch_coords position = scroll_get_original_touch_position();
    view_tile original, current;
    if (!city_view_pixels_to_view_tile(position.x, position.y, &original)) {
        return;
    }
    if (!city_view_pixels_to_view_tile(t->current_point.x, t->current_point.y, &current)) {
        return;
    }

    if (scroll_move_touch_drag(original.x, original.y, current.x, current.y, &camera_position)) {
        city_view_set_camera(camera_position.x, camera_position.y);
        sound_city_decay_views();
    }

    if (t->has_ended) {
        scroll_end_touch_drag();
    }
}

static void widget_city_handle_last_touch(map_tile *tile)
{
    const touch *last = get_latest_touch();
    if (!last->in_use) {
        return;
    }
    if (touch_was_click(last)) {
        if (building_construction_in_progress()) {
            building_construction_cancel();
            return;
        }
        if (handle_right_click_allow_building_info(tile)) {
            scroll_end_touch_drag();
            data.capture_input = 0;
            window_building_info_show(tile->grid_offset);
        }
    }
}

static void widget_city_handle_first_touch(map_tile *tile)
{
    const touch *first = get_earliest_touch();

    if (touch_was_click(first) && handle_legion_click(tile)) {
        return;
    }

    widget_city_handle_touch_scroll(first);

    if (building_construction_is_updatable()) {
        if (first->has_started) {
            build_start(tile);
        }
        if (building_construction_in_progress()) {
            build_move(tile);
            if (first->has_ended) {
                build_end();
                widget_city_clear_current_tile();
            }
        }
        return;
    }

    if (first->has_ended && data.capture_input) {
        build_start(tile);
        build_move(tile);
        build_end();
        widget_city_clear_current_tile();
    }
}

static void widget_city_handle_touch(void)
{
    const touch *first = get_earliest_touch();
    if (!first->in_use) {
        scroll_restore_margins();
        return;
    }

    map_tile *tile = &data.current_tile;
    update_city_view_coords(first->current_point.x, first->current_point.y, tile);

    if (first->has_started && touch_in_city(first)) {
        data.capture_input = 1;
        scroll_restore_margins();
    }

    widget_city_handle_last_touch(tile);
    widget_city_handle_first_touch(tile);

    if (first->has_ended) {
        data.capture_input = 0;
    }

    building_construction_reset_draw_as_constructing();
}

int widget_city_has_input(void)
{
    return data.capture_input;
}

void widget_city_handle_mouse(const mouse *m)
{
    scroll_map(scroll_get_direction(m));
    if (m->is_touch) {
        widget_city_handle_touch();
        return;
    }
    map_tile *tile = &data.current_tile;
    update_city_view_coords(m->x, m->y, tile);
    building_construction_reset_draw_as_constructing();
    if (m->left.went_down) {
        if (handle_legion_click(tile)) {
            return;
        }
        build_start(tile);
        build_move(tile);
    } else if (m->left.is_down) {
        build_move(tile);
    }
    if (m->left.went_up) {
        build_end();
    }
    if (m->right.went_up) {
        if (!building_construction_in_progress()) {
            if (handle_right_click_allow_building_info(tile)) {
                window_building_info_show(tile->grid_offset);
            }
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

void widget_city_handle_mouse_military(const mouse *m, int legion_formation_id)
{
    map_tile *tile = &data.current_tile;
    update_city_view_coords(m->x, m->y, tile);
    if (!city_view_is_sidebar_collapsed() && widget_minimap_handle_mouse(m)) {
        return;
    }
    if (m->is_touch) {
        const touch *t = get_earliest_touch();
        if (t->has_started) {
            data.capture_input = 1;
        }
        widget_city_handle_touch_scroll(t);
        if (t->has_ended) {
            data.capture_input = 0;
        }
    }
    scroll_map(scroll_get_direction(m));
    if (m->right.went_up) {
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
    data.current_tile.grid_offset = 0;
}
