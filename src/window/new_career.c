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
#include "input/keyboard.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "window/mission_selection.h"

static void start_mission(int param1, int param2);
static void button_back(int param1, int param2);

static image_button image_button_back = {
    0, 0, 31, 20, IB_NORMAL, 90, 8, button_back, button_none, 0, 0, 1
};

static image_button image_button_start_mission = {
    0, 0, 27, 27, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 56, start_mission, button_none, 1, 0, 1
};

static uint8_t player_name[32];

static void init(void)
{
    setting_clear_personal_savings();
    scenario_settings_init();
    string_copy(lang_get_string(9, 5), player_name, 32);
    keyboard_start_capture(player_name, 32, 1, 280, FONT_NORMAL_WHITE);
}

static void draw_background(void)
{
    graphics_clear_screen();
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
    lang_text_draw(12, 0, 212, 256, FONT_NORMAL_BLACK);
    inner_panel_draw(160, 208, 20, 2);
    text_capture_cursor(keyboard_cursor_position(), keyboard_offset_start(), keyboard_offset_end());
    text_draw(player_name, 176, 216, FONT_NORMAL_WHITE, 0);
    text_draw_cursor(176, 217, keyboard_is_insert());

    image_buttons_draw(464, 249, &image_button_start_mission, 1);
    image_buttons_draw(150, 251, &image_button_back, 1);

    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    if (m->right.went_up) {
        keyboard_stop_capture();
        window_go_back();
    }

    if (image_buttons_handle_mouse(mouse_in_dialog(m), 150, 251, &image_button_back, 1, 0)) {
        return;
    }

    if (image_buttons_handle_mouse(mouse_in_dialog(m), 464, 249, &image_button_start_mission, 1, 0)) {
        return;
    }
    if (keyboard_input_is_accepted()) {
        start_mission(0, 0);
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
        handle_mouse,
        0
    };
    init();
    window_show(&window);
}
