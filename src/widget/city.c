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
#include "map/building.h"
#include "map/grid.h"
#include "sound/city.h"
#include "sound/speech.h"
#include "sound/effect.h"
#include "widget/city_with_overlay.h"
#include "widget/city_without_overlay.h"
#include "widget/minimap.h"
#include "window/building_info.h"
#include "window/city.h"

#include "Data/State.h"

static struct {
    map_tile current_tile;
} data;

static void set_city_clip_rectangle()
{
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    graphics_set_clip_rectangle(x, y, width, height);
}

void widget_city_draw()
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

void widget_city_draw_construction_cost()
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
        color = COLOR_ORANGE;
    } else {
        color = COLOR_RED;
    }
    int x, y;
    city_view_get_selected_tile_pixels(&x, &y);
    text_draw_number_colored(cost, '@', " ", x + 58 + 1, y + 1, FONT_NORMAL_PLAIN, COLOR_BLACK);
    text_draw_number_colored(cost, '@', " ", x + 58, y, FONT_NORMAL_PLAIN, color);
    graphics_reset_clip_rectangle();
}

// MOUSE HANDLING

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

static int is_legion_click(const map_tile *tile)
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
    if (tile->grid_offset /*&& !Data_State.gamePaused*/) { // TODO FIXME
        building_construction_start(tile->x, tile->y, tile->grid_offset);
    }
}

static void build_move(const map_tile *tile)
{
    if (!building_construction_in_progress() || !tile->grid_offset) {
        return;
    }
    Data_State.selectedBuilding.gridOffsetEnd = tile->grid_offset;
    building_construction_update(tile->x, tile->y);
}

static void build_end(map_tile *tile)
{
    if (building_construction_in_progress()) {
        if (!tile->grid_offset) {
            tile->grid_offset = Data_State.selectedBuilding.gridOffsetEnd;
        }
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
    }
}

void widget_city_handle_mouse(const mouse *m)
{
    map_tile *tile = &data.current_tile;
    scroll_map(scroll_get_direction(m));
    update_city_view_coords(m, tile);
    Data_State.selectedBuilding.drawAsConstructing = 0;
    if (m->left.went_down) {
        if (!is_legion_click(tile)) {
            build_start(tile);
            build_move(tile);
        }
    } else if (m->left.is_down) {
        build_move(tile);
    } else if (m->left.went_up) {
        build_end(tile);
    } else if (m->right.went_up) {
        if (handle_right_click_allow_building_info(tile)) {
            window_building_info_show(tile->grid_offset);
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
    update_city_view_coords(m, tile);
    if (!city_view_is_sidebar_collapsed() && widget_minimap_handle_mouse(m)) {
        return;
    }
    scroll_map(scroll_get_direction(m));
    if (m->right.went_up) {
        city_warning_clear_all();
        window_city_show();
    } else {
        update_city_view_coords(m, tile);
        if (m->left.went_down) {
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
    int gridOffset = data.current_tile.grid_offset;
    int buildingId = map_building_at(gridOffset);
    int overlay = game_state_overlay();
    // regular tooltips
    if (overlay == OVERLAY_NONE && buildingId && building_get(buildingId)->type == BUILDING_SENATE_UPGRADED) {
        c->type = TOOLTIP_SENATE;
        c->high_priority = 1;
        return;
    }
    // overlay tooltips
    if (overlay != OVERLAY_NONE) {
        c->text_group = 66;
        c->text_id = city_with_overlay_get_tooltip_text(c, gridOffset);
        if (c->text_id) {
            c->type = TOOLTIP_OVERLAY;
            c->high_priority = 1;
        }
    }
}

void widget_city_clear_current_tile()
{
    data.current_tile.grid_offset = 0;
}
