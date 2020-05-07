#include "new_career.h"

#include "core/image_group.h"
#include "core/lang.h"
#include "core/string.h"
#include "game/settings.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "input/keyboard.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "widget/input_box.h"
#include "window/mission_selection.h"

static void start_mission(int param1, int param2);
static void button_back(int param1, int param2);

static image_button image_buttons[] = {
    {0,   2, 31, 20, IB_NORMAL, GROUP_MESSAGE_ICON,     8, button_back,   button_none, 0, 0, 1},
    {305, 0, 27, 27, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 56, start_mission, button_none, 1, 0, 1}
};

static input_box player_name_input = { 160, 208, 20, 2 };

static uint8_t player_name[32];

static void init(void)
{
    setting_clear_personal_savings();
    scenario_settings_init();
    string_copy(lang_get_string(9, 5), player_name, 32);
    keyboard_start_capture(player_name, 32, 1, &player_name_input, FONT_NORMAL_WHITE);
}

static void draw_background(void)
{
    graphics_clear_screens();
    graphics_in_dialog();
    image_draw(image_group(GROUP_MAIN_MENU_BACKGROUND), 0, 0);
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    outer_panel_draw(128, 160, 24, 8);
    lang_text_draw_centered(31, 0, 128, 172, 384, FONT_LARGE_BLACK);
    lang_text_draw(13, 5, 352, 256, FONT_NORMAL_BLACK);
    lang_text_draw(12, 0, 200, 256, FONT_NORMAL_BLACK);
    input_box_draw(&player_name_input);
    text_capture_cursor(keyboard_cursor_position(), keyboard_offset_start(), keyboard_offset_end());
    text_draw(player_name, 176, 216, FONT_NORMAL_WHITE, 0);
    text_draw_cursor(176, 217, keyboard_is_insert());

    image_buttons_draw(159, 249, image_buttons, 2);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (input_box_handle_mouse(m_dialog, &player_name_input) ||
        image_buttons_handle_mouse(m_dialog, 159, 249, image_buttons, 2, 0)) {
        return;
    }
    if (keyboard_input_is_accepted()) {
        start_mission(0, 0);
        return;
    }
    if (input_go_back_requested(m, h)) {
        keyboard_stop_capture();
        window_go_back();
    }
}

static void button_back(int param1, int param2)
{
    keyboard_stop_capture();
    window_go_back();
}

static void start_mission(int param1, int param2)
{
    keyboard_stop_capture();
    setting_set_player_name(player_name);
    window_mission_selection_show();
}

void window_new_career_show(void)
{
    window_type window = {
        WINDOW_NEW_CAREER,
        draw_background,
        draw_foreground,
        handle_input
    };
    init();
    window_show(&window);
}
