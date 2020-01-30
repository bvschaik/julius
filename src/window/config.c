#include "config.h"

#include "core/config.h"
#include "core/image_group.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "window/main_menu.h"

#define NUM_CHECKBOXES 2
#define NUM_BOTTOM_BUTTONS 3

static void toggle_switch(int id, int param2);
static void button_reset_defaults(int param1, int param2);
static void button_close(int save, int param2);

static generic_button checkbox_buttons[] = {
    { 20, 72, 20, 20, toggle_switch, button_none, CONFIG_UI_SIDEBAR_INFO },
    { 20, 144, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_IMMIGRATION_BUG },
};

static generic_button bottom_buttons[] = {
    { 20, 430, 150, 30, button_reset_defaults, button_none },
    { 410, 430, 100, 30, button_close, button_none, 0 },
    { 520, 430, 100, 30, button_close, button_none, 1 },
};

static const char *bottom_button_texts[] = {
    "Reset defaults",
    "Cancel",
    "OK"
};

static struct {
    int focus_button;
    int bottom_focus_button;
    int options[CONFIG_MAX_ENTRIES];
} data;

static const uint8_t* ascii(const char *str)
{
    return (const uint8_t *) str;
}

static void init(void)
{
    for (int i = 0; i < NUM_CHECKBOXES; i++) {
        config_key key = checkbox_buttons[i].parameter1;
        data.options[key] = config_get(key);
    }
}

static void draw_background(void)
{
    graphics_clear_screen();

    image_draw_fullscreen_background(image_group(GROUP_CONFIG));

    graphics_in_dialog();
    outer_panel_draw(0, 0, 40, 30);

    text_draw_centered(ascii("Julius configuration options"), 16, 16, 608, FONT_LARGE_BLACK, 0);

    text_draw(ascii("User interface changes"), 20, 53, FONT_NORMAL_BLACK, 0);
    text_draw(ascii("Extra information in the sidebar"), 50, 77, FONT_NORMAL_BLACK, 0);
    text_draw(ascii("Gameplay changes"), 20, 125, FONT_NORMAL_BLACK, 0);
    text_draw(ascii("Fix immigration bug on very hard"), 50, 149, FONT_NORMAL_BLACK, 0);

    for (int i = 0; i < NUM_CHECKBOXES; i++) {
        generic_button *btn = &checkbox_buttons[i];
        if (data.options[btn->parameter1]) {
            text_draw(ascii("x"), btn->x + 6, btn->y + 3, FONT_NORMAL_BLACK, 0);
        }
    }

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        text_draw_centered(ascii(bottom_button_texts[i]), bottom_buttons[i].x, bottom_buttons[i].y + 9, bottom_buttons[i].width, FONT_NORMAL_BLACK, 0);
    }

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    for (int i = 0; i < NUM_CHECKBOXES; i++) {
        generic_button *btn = &checkbox_buttons[i];
        button_border_draw(btn->x, btn->y, btn->width, btn->height, data.focus_button == i + 1);
    }
    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        button_border_draw(bottom_buttons[i].x, bottom_buttons[i].y, bottom_buttons[i].width, bottom_buttons[i].height, data.bottom_focus_button == i + 1);
    }
    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    generic_buttons_handle_mouse(m_dialog, 0, 0, checkbox_buttons, NUM_CHECKBOXES, &data.focus_button);
    generic_buttons_handle_mouse(m_dialog, 0, 0, bottom_buttons, NUM_BOTTOM_BUTTONS, &data.bottom_focus_button);
}

static void toggle_switch(int key, int param2)
{
    data.options[key] = 1 - data.options[key];
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
            config_set(key, data.options[key]);
        }
    }
    window_main_menu_show(0);
}

void window_configuration_show()
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
