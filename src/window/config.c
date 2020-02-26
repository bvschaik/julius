#include "config.h"

#include "core/config.h"
#include "core/image_group.h"
#include "core/string.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "window/main_menu.h"

#define NUM_CHECKBOXES 19
#define CONFIG_PAGES 2
#define NUM_BOTTOM_BUTTONS 5

#define FIRST_BUTTON_Y 72
#define BUTTON_SPACING 24
#define TEXT_Y_OFFSET 4

static int options_per_page[CONFIG_PAGES] = { 5,14 };
static void toggle_switch(int id, int param2);
static void button_reset_defaults(int param1, int param2);
static void button_close(int save, int param2);
static void button_page(int param1, int param2);


static generic_button checkbox_buttons[] = {
    { 20, 72, 20, 20, toggle_switch, button_none, CONFIG_UI_SHOW_INTRO_VIDEO },
    { 20, 96, 20, 20, toggle_switch, button_none, CONFIG_UI_SIDEBAR_INFO },
    { 20, 120, 20, 20, toggle_switch, button_none, CONFIG_UI_SMOOTH_SCROLLING },
    { 20, 144, 20, 20, toggle_switch, button_none, CONFIG_UI_WALKER_WAYPOINTS },
    { 20, 168, 20, 20, toggle_switch, button_none, CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE },
    { 20, 72, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_IMMIGRATION_BUG },
    { 20, 96, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_100_YEAR_GHOSTS },
    { 20, 120, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_GRANDFESTIVAL },
    { 20, 144, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_JEALOUS_GODS },
    { 20, 168, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_GLOBAL_LABOUR },
    { 20, 192, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_SCHOOL_WALKERS },
    { 20, 216, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_RETIRE_AT_60 },
    { 20, 240, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_FIXED_WORKERS },
    { 20, 264, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_EXTRA_FORTS },
    { 20, 288, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_WOLVES_BLOCK },
    { 20, 312, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_DYNAMIC_GRANARIES },
    { 20, 336, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_MORE_STOCKPILE },
    { 20, 360, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_NO_BUYER_DISTRIBUTION },
    { 20, 384, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_EDITOR_EVENTS },
};



static generic_button bottom_buttons[] = {
    { 200, 430, 150, 30, button_reset_defaults, button_none },
    { 410, 430, 100, 30, button_close, button_none, 0 },
    { 520, 430, 100, 30, button_close, button_none, 1 },
    { 20, 430, 30, 30, button_page, button_none, 0 },
    { 160, 430, 30, 30, button_page, button_none, 1 },

};

static const char *bottom_button_texts[] = {
    "Reset defaults",
    "Cancel",
    "OK",
    "-",
    "+"
};

static const char *pages_names[] = {
    "UI Improvements",
    "Gameplay Changes",
};

static const char *option_names[] = {
    "Play intro videos",
    "Extra information in the control panel",
    "Enable smooth scrolling",
    "Draw walker waypoints on overlay after right clicking on a building",
    "Improve visual clarity when clearing",
    "Fix immigration bug on very hard",
    "Fix 100-year-old ghosts",
    "Grand festivals allow extra blessing from a god",
    "Disable jealousness of gods",
    "Enable global labour pool",
    "Extend school walkers range",
    "Change citizens' retirement age from 50 to 60",
    "Fixed worker pool - 38% of population",
    "Allow building 3 extra forts",
    "Block building around wolves",
    "Dynamic granaries",
    "Houses stockpile more goods from market",
    "Buying market ladies don't distribute goods",
    "Fix Emperor change and survival time in custom missions"
};

static struct {
    int focus_button;
    int bottom_focus_button;
    int values[CONFIG_MAX_ENTRIES];
    int page;
    int starting_option;
} data;


static void init(void)
{
    data.page = 0;
    data.starting_option = 0;
    for (int i = 0; i < NUM_CHECKBOXES; i++) {
        config_key key = checkbox_buttons[i].parameter1;
        data.values[key] = config_get(key);
    }
}

static void draw_background(void)
{
    graphics_clear_screen();

    image_draw_fullscreen_background(image_group(GROUP_CONFIG));

    graphics_in_dialog();
    outer_panel_draw(0, 0, 40, 30);

    text_draw_centered(string_from_ascii(pages_names[data.page]), 16, 16, 608, FONT_LARGE_BLACK, 0);


    for(int i = 0; i < options_per_page[data.page]; i++) {
        text_draw(string_from_ascii(option_names[data.starting_option+i]), 44, FIRST_BUTTON_Y + BUTTON_SPACING * i + TEXT_Y_OFFSET, FONT_NORMAL_BLACK, 0);
    }
    for (int i = 0; i < options_per_page[data.page]; i++) {
	int value = i + data.starting_option;
        generic_button *btn = &checkbox_buttons[value];
        if (data.values[btn->parameter1]) {
            text_draw(string_from_ascii("x"), btn->x + 6, btn->y + 3, FONT_NORMAL_BLACK, 0);
        }
    }

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        text_draw_centered(string_from_ascii(bottom_button_texts[i]), bottom_buttons[i].x, bottom_buttons[i].y + 9, bottom_buttons[i].width, FONT_NORMAL_BLACK, 0);
    }
    text_draw_centered(string_from_ascii("Page"), 80, 440, 30, FONT_NORMAL_BLACK, 0);
    text_draw_number(data.page+1, '@', " ", 120 , 440, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    for (int i = 0; i < options_per_page[data.page]; i++) {
        int value = data.starting_option + i;
        generic_button *btn = &checkbox_buttons[value];
        button_border_draw(btn->x, btn->y, btn->width, btn->height, data.focus_button == value + 1);
    }
    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        button_border_draw(bottom_buttons[i].x, bottom_buttons[i].y, bottom_buttons[i].width, bottom_buttons[i].height, data.bottom_focus_button == i + 1);
    }
    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    int starting_option = 0;

    generic_buttons_min_handle_mouse(m_dialog, 0, 0, checkbox_buttons, data.starting_option+options_per_page[data.page], &data.focus_button,data.starting_option);
    generic_buttons_handle_mouse(m_dialog, 0, 0, bottom_buttons, NUM_BOTTOM_BUTTONS, &data.bottom_focus_button);
}

static void toggle_switch(int key, int param2)
{
    data.values[key] = 1 - data.values[key];
    window_invalidate();
}

static void button_reset_defaults(int param1, int param2)
{
    config_set_defaults();
    init();
    window_invalidate();
}

static void button_close(int save, int param2)
{
    if (save) {
        for (int i = 0; i < NUM_CHECKBOXES; i++) {
            config_key key = checkbox_buttons[i].parameter1;
            config_set(key, data.values[key]);
        }
    }
    window_main_menu_show(0);
}

static void button_page(int param1, int param2)
{
    if (param1) {
        data.page++;
        if (data.page >= CONFIG_PAGES) {
            data.page = 0;
        }
    } else {
        data.page--;
        if (data.page < 0) {
            data.page = CONFIG_PAGES-1;
        }
    }
    data.starting_option = 0;
    for (int i = 0; i < data.page; i++) {
        data.starting_option += options_per_page[i];
    }
    window_invalidate();
}

void window_config_show()
{
    window_type window = {
        WINDOW_CONFIG,
        draw_background,
        draw_foreground,
        handle_mouse
    };
    init();
    window_show(&window);
}
