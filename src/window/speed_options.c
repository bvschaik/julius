#include "speed_options.h"


#include "game/settings.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "window/city.h"

static void button_ok(int param1, int param2);
static void button_cancel(int param1, int param2);

static void arrow_button_game(int is_down, int param2);
static void arrow_button_scroll(int is_down, int param2);

static generic_button buttons[] = {
    {144, 232, 336, 252, GB_IMMEDIATE, button_ok, button_none, 1, 0},
    {144, 262, 336, 282, GB_IMMEDIATE, button_cancel, button_none, 1, 0},
};

static arrow_button arrow_buttons[] = {
    {112, 100, 17, 24, arrow_button_game, 1, 0},
    {136, 100, 15, 24, arrow_button_game, 0, 0},
    {112, 136, 17, 24, arrow_button_scroll, 1, 0},
    {136, 136, 15, 24, arrow_button_scroll, 0, 0},
};

static int original_game_speed;
static int original_scroll_speed;

static int focus_button_id;

static void init()
{
    original_game_speed = setting_game_speed();
    original_scroll_speed = setting_scroll_speed();
    focus_button_id = 0;
}

static void draw_foreground()
{
    graphics_in_dialog();

    outer_panel_draw(96, 80, 18, 14);
    // ok/cancel labels
    label_draw(144, 232, 12, focus_button_id == 1 ? 1 : 2);
    label_draw(144, 262, 12, focus_button_id == 2 ? 1 : 2);
    
    // title
    lang_text_draw_centered(45, 0, 128, 92, 224, FONT_LARGE_BLACK);
    // ok/cancel label texts
    lang_text_draw_centered(45, 4, 128, 236, 224, FONT_NORMAL_GREEN);
    lang_text_draw_centered(45, 1, 128, 266, 224, FONT_NORMAL_GREEN);
    // game speed
    lang_text_draw(45, 2, 144, 146, FONT_NORMAL_PLAIN);
    text_draw_percentage(setting_game_speed(), 296, 146, FONT_NORMAL_PLAIN);
    // scroll speed
    lang_text_draw(45, 3, 144, 182, FONT_NORMAL_PLAIN);
    text_draw_percentage(setting_scroll_speed(), 296, 182,FONT_NORMAL_PLAIN);

    arrow_buttons_draw(128, 40, arrow_buttons, 4);
    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    if (m->right.went_up) {
        // cancel dialog
        window_city_show();
    } else {
        const mouse *m_dialog = mouse_in_dialog(m);
        if (!generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, 2, &focus_button_id)) {
            arrow_buttons_handle_mouse(m_dialog, 128, 40, arrow_buttons, 4);
        }
    }
}

static void button_ok(int param1, int param2)
{
    window_city_show();
}

static void button_cancel(int param1, int param2)
{
    setting_reset_speeds(original_game_speed, original_scroll_speed);
    window_city_show();
}

static void arrow_button_game(int is_down, int param2)
{
    if (is_down) {
        setting_decrease_game_speed();
    } else {
        setting_increase_game_speed();
    }
}

static void arrow_button_scroll(int is_down, int param2)
{
    if (is_down) {
        setting_decrease_scroll_speed();
    } else {
        setting_increase_scroll_speed();
    }
}

void window_speed_options_show()
{
    window_type window = {
        WINDOW_SPEED_OPTIONS,
        0,
        draw_foreground,
        handle_mouse
    };
    init();
    window_show(&window);
}
