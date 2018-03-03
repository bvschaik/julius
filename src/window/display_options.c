#include "display_options.h"

#include "game/settings.h"
#include "game/system.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "window/city.h"

static void button_fullscreen(int param1, int param2);
static void button_set_resolution(int id, int param2);
static void button_cancel(int param1, int param2);

static generic_button buttons[] = {
    {144, 136, 336, 156, GB_IMMEDIATE, button_fullscreen, button_none, 1, 0},
    {144, 160, 336, 180, GB_IMMEDIATE, button_set_resolution, button_none, 1, 0},
    {144, 184, 336, 204, GB_IMMEDIATE, button_set_resolution, button_none, 2, 0},
    {144, 208, 336, 228, GB_IMMEDIATE, button_set_resolution, button_none, 3, 0},
    {144, 232, 336, 252, GB_IMMEDIATE, button_cancel, button_none, 1, 0},
};

static int focus_button_id;

static void draw_foreground()
{
    graphics_in_dialog();

    outer_panel_draw(96, 80, 18, 12);
    
    label_draw(128, 136, 14, focus_button_id == 1 ? 1 : 2);
    label_draw(128, 160, 14, focus_button_id == 2 ? 1 : 2);
    label_draw(128, 184, 14, focus_button_id == 3 ? 1 : 2);
    label_draw(128, 208, 14, focus_button_id == 4 ? 1 : 2);
    label_draw(128, 232, 14, focus_button_id == 5 ? 1 : 2);

    lang_text_draw_centered(42, 0, 128, 94, 224, FONT_LARGE_BLACK);

    lang_text_draw_centered(42, setting_fullscreen() ? 2 : 1, 128, 140, 224, FONT_NORMAL_GREEN);

    lang_text_draw_centered(42, 3, 128, 164, 224, FONT_NORMAL_GREEN);
    lang_text_draw_centered(42, 4, 128, 188, 224, FONT_NORMAL_GREEN);
    lang_text_draw_centered(42, 5, 128, 212, 224, FONT_NORMAL_GREEN);
    lang_text_draw_centered(42, 6, 128, 236, 224, FONT_NORMAL_GREEN);

    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    if (m->right.went_up) {
        // cancel dialog
        window_city_show();
    } else {
        generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 5, &focus_button_id);
    }
}

static void button_fullscreen(int param1, int param2)
{
    system_toggle_fullscreen();
    window_city_show();
}

static void button_set_resolution(int id, int param2)
{
    switch (id) {
        case 1: system_resize(640, 480); break;
        case 2: system_resize(800, 600); break;
        case 3: system_resize(1024, 768); break;
    }
    window_city_show();
}

static void button_cancel(int param1, int param2)
{
    window_city_show();
}

void window_display_options_show()
{
    window_type window = {
        WINDOW_DISPLAY_OPTIONS,
        0,
        draw_foreground,
        handle_mouse
    };
    window_show(&window);
}
