#include "city.h"

#include "building/building_variant.h"
#include "building/clone.h"
#include "building/construction.h"
#include "building/data_transfer.h"
#include "building/menu.h"
#include "building/properties.h"
#include "building/rotation.h"
#include "building/type.h"
#include "city/message.h"
#include "city/victory.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/config.h"
#include "figure/formation.h"
#include "figure/formation_legion.h"
#include "game/orientation.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/time.h"
#include "game/undo.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
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
#include "widget/sidebar/extra.h"
#include "widget/sidebar/military.h"
#include "window/advisors.h"
#include "window/empire.h"
#include "window/file_dialog.h"
#include "window/message_list.h"

static void draw_background(void)
{
    widget_sidebar_city_draw_background();
    widget_top_menu_draw(1);
}

static void draw_background_military(void)
{
    if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR)) {
        widget_sidebar_military_draw_background();
    } else {
        widget_sidebar_city_draw_background();
    }
    widget_top_menu_draw(1);
}

static int center_in_city(int element_width_pixels)
{
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    int margin = (width - element_width_pixels) / 2;
    return x + margin;
}

static void draw_paused_banner(void)
{
    if (game_state_is_paused()) {
        int x_offset = center_in_city(448);
        outer_panel_draw(x_offset, 40, 28, 3);
        lang_text_draw_centered(13, 2, x_offset, 58, 448, FONT_NORMAL_BLACK);
    }
}

static void draw_time_left(void)
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
        text_draw_number(total_months, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK, 0);
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
        text_draw_number(total_months, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK, 0);
    }
}

static void draw_foreground(void)
{
    widget_top_menu_draw(0);
    window_city_draw();
    widget_sidebar_city_draw_foreground();
    if (window_is(WINDOW_CITY) || window_is(WINDOW_CITY_MILITARY)) {
        draw_time_left();
        widget_city_draw_construction_buttons();
        if (!mouse_get()->is_touch || sidebar_extra_is_information_displayed(SIDEBAR_EXTRA_DISPLAY_GAME_SPEED)) {
            draw_paused_banner();
        }
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
    if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR)) {
        widget_sidebar_military_draw_foreground();
    } else {
        widget_sidebar_city_draw_foreground();
    }
    draw_time_left();
    widget_city_draw_construction_buttons();
    if (!mouse_get()->is_touch || sidebar_extra_is_information_displayed(SIDEBAR_EXTRA_DISPLAY_GAME_SPEED)) {
        draw_paused_banner();
    }
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

// this is mix of get_clone_type_from_grid_offset & get_clone_type_from_building functions with reduced code for overlay purpose
static int get_building_type_from_grid_offset(int grid_offset)
{
    int terrain = map_terrain_get(grid_offset);

    if (terrain & TERRAIN_BUILDING) {
        int building_id = map_building_at(grid_offset);
        if (building_id) {
            building *building = building_main(building_get(building_id));
            return building->type;
        }
    } else if (terrain & TERRAIN_AQUEDUCT) {
        return BUILDING_AQUEDUCT;
    } else if (terrain & TERRAIN_GARDEN) {
        return BUILDING_GARDENS;
    } else if (terrain & TERRAIN_ROAD) {
        if (map_property_is_plaza_or_earthquake(grid_offset)) {
            return BUILDING_PLAZA;
        }
        return BUILDING_ROAD;
    }

    return BUILDING_NONE;
}

static void show_overlay_from_grid_offset(int grid_offset)
{
    int overlay = OVERLAY_NONE;
    int clone_type = get_building_type_from_grid_offset(grid_offset);
    switch (clone_type) {
        case BUILDING_PLAZA:
        case BUILDING_ROAD:
        case BUILDING_ROADBLOCK:
        case BUILDING_GARDEN_WALL_GATE:
            overlay = OVERLAY_ROADS;
            break;
        case BUILDING_AQUEDUCT:
        case BUILDING_RESERVOIR:
        case BUILDING_FOUNTAIN:
        case BUILDING_WELL:
            overlay = OVERLAY_WATER;
            break;
        case BUILDING_ORACLE:
        case BUILDING_SMALL_TEMPLE_CERES:
        case BUILDING_SMALL_TEMPLE_NEPTUNE:
        case BUILDING_SMALL_TEMPLE_MERCURY:
        case BUILDING_SMALL_TEMPLE_MARS:
        case BUILDING_SMALL_TEMPLE_VENUS:
        case BUILDING_LARGE_TEMPLE_CERES:
        case BUILDING_LARGE_TEMPLE_NEPTUNE:
        case BUILDING_LARGE_TEMPLE_MERCURY:
        case BUILDING_LARGE_TEMPLE_MARS:
        case BUILDING_LARGE_TEMPLE_VENUS:
        case BUILDING_GRAND_TEMPLE_CERES:
        case BUILDING_GRAND_TEMPLE_NEPTUNE:
        case BUILDING_GRAND_TEMPLE_MERCURY:
        case BUILDING_GRAND_TEMPLE_MARS:
        case BUILDING_GRAND_TEMPLE_VENUS:
        case BUILDING_PANTHEON:
        case BUILDING_LARARIUM:
        case BUILDING_NYMPHAEUM:
        case BUILDING_SMALL_MAUSOLEUM:
        case BUILDING_LARGE_MAUSOLEUM:
            overlay = OVERLAY_RELIGION;
            break;
        case BUILDING_PREFECTURE:
        case BUILDING_BURNING_RUIN:
            overlay = OVERLAY_FIRE;
            break;
        case BUILDING_ENGINEERS_POST:
        case BUILDING_ARCHITECT_GUILD:
            overlay = OVERLAY_DAMAGE;
            break;
        case BUILDING_THEATER:
        case BUILDING_ACTOR_COLONY:
            overlay = OVERLAY_THEATER;
            break;
        case BUILDING_AMPHITHEATER:
        case BUILDING_GLADIATOR_SCHOOL:
            overlay = OVERLAY_AMPHITHEATER;
            break;
        case BUILDING_TAVERN:
            overlay = OVERLAY_TAVERN;
            break;
        case BUILDING_ARENA:
            overlay = OVERLAY_ARENA;
            break;
        case BUILDING_COLOSSEUM:
        case BUILDING_LION_HOUSE:
            overlay = OVERLAY_COLOSSEUM;
            break;
        case BUILDING_HIPPODROME:
        case BUILDING_CHARIOT_MAKER:
            overlay = OVERLAY_HIPPODROME;
            break;
        case BUILDING_SCHOOL:
            overlay = OVERLAY_SCHOOL;
            break;
        case BUILDING_LIBRARY:
            overlay = OVERLAY_LIBRARY;
            break;
        case BUILDING_ACADEMY:
            overlay = OVERLAY_ACADEMY;
            break;
        case BUILDING_BARBER:
            overlay = OVERLAY_BARBER;
            break;
        case BUILDING_BATHHOUSE:
            overlay = OVERLAY_BATHHOUSE;
            break;
        case BUILDING_DOCTOR:
            overlay = OVERLAY_CLINIC;
            break;
        case BUILDING_HOSPITAL:
            overlay = OVERLAY_HOSPITAL;
            break;
        case BUILDING_FORUM:
        case BUILDING_FORUM_UPGRADED:
        case BUILDING_SENATE:
        case BUILDING_SENATE_UPGRADED:
            overlay = OVERLAY_TAX_INCOME;
            break;
        case BUILDING_MARKET:
        case BUILDING_GRANARY:
        case BUILDING_FRUIT_FARM:
        case BUILDING_OLIVE_FARM:
        case BUILDING_PIG_FARM:
        case BUILDING_VEGETABLE_FARM:
        case BUILDING_VINES_FARM:
        case BUILDING_WHEAT_FARM:
        case BUILDING_OIL_WORKSHOP:
        case BUILDING_WINE_WORKSHOP:
        case BUILDING_WHARF:
            overlay = OVERLAY_FOOD_STOCKS;
            break;
        case BUILDING_GARDENS:
        case BUILDING_GOVERNORS_HOUSE:
        case BUILDING_GOVERNORS_VILLA:
        case BUILDING_GOVERNORS_PALACE:
        case BUILDING_HOUSE_SMALL_TENT:
        case BUILDING_HOUSE_LARGE_TENT:
        case BUILDING_HOUSE_SMALL_SHACK:
        case BUILDING_HOUSE_LARGE_SHACK:
        case BUILDING_HOUSE_SMALL_HOVEL:
        case BUILDING_HOUSE_LARGE_HOVEL:
        case BUILDING_HOUSE_SMALL_CASA:
        case BUILDING_HOUSE_LARGE_CASA:
        case BUILDING_HOUSE_SMALL_INSULA:
        case BUILDING_HOUSE_MEDIUM_INSULA:
        case BUILDING_HOUSE_LARGE_INSULA:
        case BUILDING_HOUSE_GRAND_INSULA:
        case BUILDING_HOUSE_SMALL_VILLA:
        case BUILDING_HOUSE_MEDIUM_VILLA:
        case BUILDING_HOUSE_LARGE_VILLA:
        case BUILDING_HOUSE_GRAND_VILLA:
        case BUILDING_HOUSE_SMALL_PALACE:
        case BUILDING_HOUSE_MEDIUM_PALACE:
        case BUILDING_HOUSE_LARGE_PALACE:
        case BUILDING_HOUSE_LUXURY_PALACE:
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
        case BUILDING_TRIUMPHAL_ARCH:
        case BUILDING_SMALL_POND:
        case BUILDING_LARGE_POND:
        case BUILDING_PINE_TREE:
        case BUILDING_FIR_TREE:
        case BUILDING_OAK_TREE:
        case BUILDING_ELM_TREE:
        case BUILDING_FIG_TREE:
        case BUILDING_PLUM_TREE:
        case BUILDING_PALM_TREE:
        case BUILDING_DATE_TREE:
        case BUILDING_PINE_PATH:
        case BUILDING_FIR_PATH:
        case BUILDING_OAK_PATH:
        case BUILDING_ELM_PATH:
        case BUILDING_FIG_PATH:
        case BUILDING_PLUM_PATH:
        case BUILDING_PALM_PATH:
        case BUILDING_DATE_PATH:
        case BUILDING_GARDEN_PATH:
        case BUILDING_PAVILION_BLUE:
        case BUILDING_PAVILION_RED:
        case BUILDING_PAVILION_ORANGE:
        case BUILDING_PAVILION_YELLOW:
        case BUILDING_PAVILION_GREEN:
        case BUILDING_SMALL_STATUE_ALT:
        case BUILDING_SMALL_STATUE_ALT_B:
        case BUILDING_OBELISK:
        case BUILDING_HORSE_STATUE:
        case BUILDING_LEGION_STATUE:
        case BUILDING_GLADIATOR_STATUE:
            overlay = OVERLAY_DESIRABILITY;
            break;
        case BUILDING_MISSION_POST:
        case BUILDING_NATIVE_HUT:
        case BUILDING_NATIVE_MEETING:
            overlay = OVERLAY_NATIVE;
            break;
        case BUILDING_WAREHOUSE:
        case BUILDING_WAREHOUSE_SPACE:
            overlay = OVERLAY_WAREHOUSE;
            break;
        case BUILDING_DOCK:
            overlay = OVERLAY_SICKNESS;
            break;
        case BUILDING_NONE:
            if (map_terrain_get(grid_offset) & TERRAIN_RUBBLE) {
                overlay = OVERLAY_DAMAGE;
            }
            break;
        default:
            break;
    }
    if (!(game_state_overlay() == OVERLAY_NONE && overlay == OVERLAY_NONE)) {
        show_overlay(overlay);
    }
}

static void cycle_legion(void)
{
    static int current_legion_id = 1;
    if (window_is(WINDOW_CITY) || window_is(WINDOW_CITY_MILITARY)) {
        int legion_id = current_legion_id;
        current_legion_id = 0;
        for (int i = 1; i < formation_count(); i++) {
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
            if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR) && window_is(WINDOW_CITY_MILITARY)) {
                window_city_military_show(current_legion_id);
            } else {
                window_invalidate();
            }
        }
    }
}

static void toggle_pause(void)
{
    game_state_toggle_paused();
    city_warning_clear_all();
}

static void set_construction_building_type(building_type type)
{
    if (scenario_building_allowed(type) && building_menu_is_enabled(type)) {
        building_construction_cancel();
        building_construction_set_type(type);
        window_request_refresh();
    }
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
    if (h->show_overlay_relative) {
        show_overlay_from_grid_offset(widget_city_current_grid_offset());
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
        if (!building_construction_in_progress()) {
            game_orientation_rotate_left();
            window_invalidate();
        }
    }
    if (h->rotate_map_right) {
        if (!building_construction_in_progress()) {
            game_orientation_rotate_right();
            window_invalidate();
        }
    }
    if (h->rotate_map_north) {
        if (!building_construction_in_progress()) {
            game_orientation_rotate_north();
            window_invalidate();
        }
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
    if (h->rotate_building) {
        building_rotation_rotate_forward();
    }
    if (h->rotate_building_back) {
        building_rotation_rotate_backward();
    }
    if (h->building) {
        set_construction_building_type(h->building);
    }
    if (h->undo) {
        game_undo_perform();
        window_invalidate();
    }
    if (h->mothball_toggle) {
        int building_id = map_building_at(widget_city_current_grid_offset());
        if (building_id) {
            building *b = building_main(building_get(building_id));
            building_mothball_toggle(b);
            if (b->state == BUILDING_STATE_IN_USE) {
                city_warning_clear_all();
                city_warning_show(WARNING_DATA_MOTHBALL_OFF, NEW_WARNING_SLOT);
            } else if (b->state == BUILDING_STATE_MOTHBALLED) {
                city_warning_clear_all();
                city_warning_show(WARNING_DATA_MOTHBALL_ON, NEW_WARNING_SLOT);
            }
        }
    }
    if (h->clone_building) {
        building_type type = building_clone_type_from_grid_offset(widget_city_current_grid_offset());
        if (type) {
            set_construction_building_type(type);
        }
    }
    if (h->copy_building_settings) {
        int building_id = map_building_at(widget_city_current_grid_offset());
        if (building_id) {
            building *b = building_main(building_get(building_id));
            building_data_transfer_copy(b);
        }
    }
    if (h->paste_building_settings) {
        int building_id = map_building_at(widget_city_current_grid_offset());
        if (building_id) {
            building *b = building_main(building_get(building_id));
            building_data_transfer_paste(b);
        }
    }
    if (h->show_empire_map) {
        if (!window_is(WINDOW_EMPIRE)) {
            window_empire_show_checked();
        }
    }
    if (h->show_messages) {
        window_message_list_show();
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
    if (widget_top_menu_handle_input(m, h)) {
        return;
    }
    if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR) && widget_sidebar_military_handle_input(m)) {
        return;
    }
    widget_city_handle_input_military(m, h, formation_get_selected());
}

static void get_tooltip(tooltip_context *c)
{
    int text_id = widget_top_menu_get_tooltip_text(c);
    if (!text_id) {
        if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR) && formation_get_selected()) {
            text_id = widget_sidebar_military_get_tooltip_text(c);
        } else {
            text_id = widget_sidebar_city_get_tooltip_text(c);
        }
    }
    if (text_id || c->translation_key) {
        c->type = TOOLTIP_BUTTON;
        c->text_id = text_id;
        return;
    }
    widget_city_get_tooltip(c);
}

int window_city_military_is_cursor_in_menu(void)
{
    if (!config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR) || !window_is(WINDOW_CITY_MILITARY)) {
        return 0;
    }
    const mouse *m = mouse_get();
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    y += 24;
    height += 24;
    return m->x < x || m->x >= width || m->y < y || m->y >= height;
}

void window_city_draw_all(void)
{
    if (formation_get_selected() && config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR)) {
        draw_background_military();
        draw_foreground_military();
    } else {
        draw_background();
        draw_foreground();
    }
}

void window_city_draw_panels(void)
{
    if (formation_get_selected() && config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR)) {
        draw_background_military();
    } else {
        draw_background();
    }
}

void window_city_draw(void)
{
    widget_city_draw();
}

void window_city_show(void)
{
    if (formation_get_selected()) {
        formation_set_selected(0);
        if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR) && widget_sidebar_military_exit()) {
            return;
        }
    }
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
    if (building_construction_type()) {
        building_construction_cancel();
        building_construction_clear_type();
    }
    formation_set_selected(legion_formation_id);
    if (config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR) && widget_sidebar_military_enter(legion_formation_id)) {
        return;
    }
    window_type window = {
        WINDOW_CITY_MILITARY,
        draw_background_military,
        draw_foreground_military,
        handle_input_military,
        get_tooltip
    };
    window_show(&window);
}

void window_city_return(void)
{
    int formation_id = formation_get_selected();
    if (formation_id) {
        window_city_military_show(formation_id);
    } else {
        window_city_show();
    }
}
