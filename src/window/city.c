#include "city.h"

#include "building/building.h"
#include "building/construction.h"
#include "city/buildings.h"
#include "city/message.h"
#include "city/victory.h"
#include "city/view.h"
#include "city/warning.h"
#include "figure/formation.h"
#include "game/orientation.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/time.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "map/aqueduct.h"
#include "map/bookmark.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/terrain.h"
#include "scenario/building.h"
#include "scenario/criteria.h"
#include "widget/city.h"
#include "widget/city_with_overlay.h"
#include "widget/top_menu.h"
#include "widget/sidebar/city.h"
#include "window/advisors.h"
#include "window/file_dialog.h"

static int selected_legion_formation_id;

static void draw_background(void)
{
    widget_sidebar_city_draw_background();
    widget_top_menu_draw(1);
}

static int center_in_city(int element_width_pixels)
{
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    int margin = (width - element_width_pixels) / 2;
    return x + margin;
}

static void draw_paused_and_time_left(void)
{
    if (scenario_criteria_time_limit_enabled() && !city_victory_has_won()) {
        int years;
        if (scenario_criteria_max_year() <= game_time_year() + 1) {
            years = 0;
        } else {
            years = scenario_criteria_max_year() - game_time_year() - 1;
        }
        int total_months = 12 - game_time_month() + 12 * years;
        label_draw(1, 25, 15, 1);
        int width = lang_text_draw(6, 2, 6, 29, FONT_NORMAL_BLACK);
        text_draw_number(total_months, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK);
    } else if (scenario_criteria_survival_enabled() && !city_victory_has_won()) {
        int years;
        if (scenario_criteria_max_year() <= game_time_year() + 1) {
            years = 0;
        } else {
            years = scenario_criteria_max_year() - game_time_year() - 1;
        }
        int total_months = 12 - game_time_month() + 12 * years;
        label_draw(1, 25, 15, 1);
        int width = lang_text_draw(6, 3, 6, 29, FONT_NORMAL_BLACK);
        text_draw_number(total_months, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK);
    }
    if (game_state_is_paused()) {
        int x_offset = center_in_city(448);
        outer_panel_draw(x_offset, 40, 28, 3);
        lang_text_draw_centered(13, 2, x_offset, 58, 448, FONT_NORMAL_BLACK);
    }
}

static void draw_cancel_construction(void)
{
    if (!mouse_get()->is_touch || !building_construction_type()) {
        return;
    }
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    width -= 4 * 16;
    inner_panel_draw(width - 4, 40, 3, 2);
    image_draw(image_group(GROUP_OK_CANCEL_SCROLL_BUTTONS) + 4, width, 44);
}

static void draw_foreground(void)
{
    widget_top_menu_draw(0);
    window_city_draw();
    widget_sidebar_city_draw_foreground();
    if (window_is(WINDOW_CITY) || window_is(WINDOW_CITY_MILITARY)) {
        draw_paused_and_time_left();
        draw_cancel_construction();
    }
    widget_city_draw_construction_cost_and_size();
    if (window_is(WINDOW_CITY)) {
        city_message_process_queue();
    }
}

static void draw_foreground_military(void)
{
    widget_top_menu_draw(0);
    window_city_draw();
    widget_sidebar_city_draw_foreground_military();
    draw_paused_and_time_left();
}

static void exit_military_command(void)
{
    if (window_is(WINDOW_CITY_MILITARY)) {
        window_city_show();
    }
}

static void show_overlay(int overlay)
{
    exit_military_command();
    if (game_state_overlay() == overlay) {
        game_state_set_overlay(OVERLAY_NONE);
    } else {
        game_state_set_overlay(overlay);
    }
    city_with_overlay_update();
    window_invalidate();
}

static void cycle_legion(void)
{
    static int current_legion_id = 1;
    if (window_is(WINDOW_CITY)) {
        int legion_id = current_legion_id;
        current_legion_id = 0;
        for (int i = 1; i < MAX_FORMATIONS; i++) {
            legion_id++;
            if (legion_id > MAX_LEGIONS) {
                legion_id = 1;
            }
            const formation *m = formation_get(legion_id);
            if (m->in_use == 1 && !m->is_herd && m->is_legion) {
                if (current_legion_id == 0) {
                    current_legion_id = legion_id;
                    break;
                }
            }
        }
        if (current_legion_id > 0) {
            const formation *m = formation_get(current_legion_id);
            city_view_go_to_grid_offset(map_grid_offset(m->x_home, m->y_home));
            window_invalidate();
        }
    }
}

/** 
    Takes a building and retrieve its proper type for cloning.
    For example, given a fort, return the enumaration value corresponding to
    the specific type of fort rather than the general value

    @param building Building to examine
    @return the building_type value to clone, or BUILDING_NONE if not cloneable
*/
static short get_clone_type_from_building(building* building)
{
    short clone_type = building->type;

    if (building_is_house(clone_type)) {
        return BUILDING_HOUSE_VACANT_LOT;
    }

    switch (clone_type) {
        case BUILDING_FORT:
            switch (building->subtype.fort_figure_type) {
                case FIGURE_FORT_LEGIONARY: return BUILDING_FORT_LEGIONARIES;
                case FIGURE_FORT_JAVELIN: return BUILDING_FORT_JAVELIN;
                case FIGURE_FORT_MOUNTED: return BUILDING_FORT_MOUNTED;
            }
            return BUILDING_NONE;
        case BUILDING_TRIUMPHAL_ARCH:
            // Triumphal arches don't seem to have any protection around making
            // more than you've earned, so check that here
            if (city_buildings_triumphal_arch_available()) {
                break;
            }
            // fallthrough
        case BUILDING_NATIVE_CROPS:
        case BUILDING_NATIVE_HUT:
        case BUILDING_NATIVE_MEETING:
        case BUILDING_BURNING_RUIN:
            return BUILDING_NONE;
    }

    return clone_type;
}

/**
    Helper function for retrieving which construction mode to enter.
    First checks for clear terrain for an early exit opportunity,
    then checks for buildings, aqueducts, road/plaza and walls.

    @param grid_offset the grid_offset of the tile to examine
    @return type to use in building_construction_set_type (0 if none)
*/
static int get_clone_type_from_grid_offset(int grid_offset)
{
    // we can short circuit a bit if this is an empty tile in general
    if (!map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
        return 0;
    }

    int building_id = map_building_at(grid_offset);
    if (building_id) {
        return get_clone_type_from_building(building_main(building_get(building_id)));
    }

    if (map_aqueduct_at(grid_offset)) {
        return BUILDING_AQUEDUCT;
    }

    if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
        if (map_property_is_plaza_or_earthquake(grid_offset)) {
            return BUILDING_PLAZA;
        }
        return BUILDING_ROAD;
    }

    if (map_terrain_is(grid_offset, TERRAIN_WALL)) {
        return BUILDING_WALL;
    }

    return 0;
}

/**
    Enter construction mode with the same building as cursor is currently over
*/
static void clone_building_at_current_grid_offset(void)
{
    int grid_offset = widget_city_current_grid_offset();
    short clone_type = get_clone_type_from_grid_offset(grid_offset);

    if (clone_type) {
        building_construction_cancel();
        building_construction_set_type(clone_type);
    }
}

static void toggle_pause(void)
{
    game_state_toggle_paused();
    city_warning_clear_all();
}

static void handle_hotkeys(const hotkeys *h)
{
    if (h->toggle_pause) {
        toggle_pause();
    }
    if (h->decrease_game_speed) {
        setting_decrease_game_speed();
    }
    if (h->increase_game_speed) {
        setting_increase_game_speed();
    }
    if (h->show_overlay) {
        show_overlay(h->show_overlay);
    }
    if (h->toggle_overlay) {
        exit_military_command();
        game_state_toggle_overlay();
        city_with_overlay_update();
        window_invalidate();
    }
    if (h->show_advisor) {
        window_advisors_show_advisor(h->show_advisor);
    }
    if (h->cycle_legion) {
        cycle_legion();
    }
    if (h->rotate_map_left) {
        game_orientation_rotate_left();
        window_invalidate();
    }
    if (h->rotate_map_right) {
        game_orientation_rotate_right();
        window_invalidate();
    }
    if (h->go_to_bookmark) {
        if (map_bookmark_go_to(h->go_to_bookmark - 1)) {
            window_invalidate();
        }
    }
    if (h->set_bookmark) {
        map_bookmark_save(h->set_bookmark - 1);
    }
    if (h->load_file) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_LOAD);
    }
    if (h->save_file) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_SAVE);
    }
    if (h->building) {
        if (scenario_building_allowed(h->building)) {
            building_construction_cancel();
            building_construction_set_type(h->building);
        }
    }
    if (h->clone_building) {
        clone_building_at_current_grid_offset();
    }
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    handle_hotkeys(h);
    if (!building_construction_in_progress()) {
        if (widget_top_menu_handle_input(m, h)) {
            return;
        }
        if (widget_sidebar_city_handle_mouse(m)) {
            return;
        }
    }
    widget_city_handle_input(m, h);
}

static void handle_input_military(const mouse *m, const hotkeys *h)
{
    handle_hotkeys(h);
    widget_city_handle_input_military(m, h, selected_legion_formation_id);
}

static void get_tooltip(tooltip_context *c)
{
    int text_id = widget_top_menu_get_tooltip_text(c);
    if (!text_id) {
        text_id = widget_sidebar_city_get_tooltip_text();
    }
    if (text_id) {
        c->type = TOOLTIP_BUTTON;
        c->text_id = text_id;
        return;
    }
    widget_city_get_tooltip(c);
}

void window_city_draw_all(void)
{
    draw_background();
    draw_foreground();
}

void window_city_draw_panels(void)
{
    draw_background();
}

void window_city_draw(void)
{
    widget_city_draw();
}

void window_city_show(void)
{
    window_type window = {
        WINDOW_CITY,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    window_show(&window);
}

void window_city_military_show(int legion_formation_id)
{
    selected_legion_formation_id = legion_formation_id;
    window_type window = {
        WINDOW_CITY_MILITARY,
        draw_background,
        draw_foreground_military,
        handle_input_military,
        get_tooltip
    };
    window_show(&window);
}
