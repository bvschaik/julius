#include "display_options.h"

#include "game/settings.h"
#include "game/system.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"

static void button_fullscreen(int param1, int param2);
static void button_set_resolution(int id, int param2);
static void button_cancel(int param1, int param2);

static generic_button buttons[] = {
    {128, 136, 224, 20, button_fullscreen, button_none, 1, 0},
    {128, 160, 224, 20, button_set_resolution, button_none, 1, 0},
    {128, 184, 224, 20, button_set_resolution, button_none, 2, 0},
    {128, 208, 224, 20, button_set_resolution, button_none, 3, 0},
    {128, 232, 224, 20, button_cancel, button_none, 1, 0},
};

static struct {
    int focus_button_id;
    void (*close_callback)(void);
} data;

static void init(void (*close_callback)(void))
{
    data.focus_button_id = 0;
    data.close_callback = close_callback;
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(96, 80, 18, 12);

    label_draw(128, 136, 14, data.focus_button_id == 1 ? 1 : 2);
    label_draw(128, 160, 14, data.focus_button_id == 2 ? 1 : 2);
    label_draw(128, 184, 14, data.focus_button_id == 3 ? 1 : 2);
    label_draw(128, 208, 14, data.focus_button_id == 4 ? 1 : 2);
    label_draw(128, 232, 14, data.focus_button_id == 5 ? 1 : 2);

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
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 5, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested()) {
        data.close_callback();
    }
}

static void button_fullscreen(int param1, int param2)
{
    system_set_fullscreen(!setting_fullscreen());
    data.close_callback();
}

static void button_set_resolution(int id, int param2)
{
    switch (id) {
        case 1: system_resize(640, 480); break;
        case 2: system_resize(800, 600); break;
        case 3: system_resize(1024, 768); break;
    }
    data.close_callback();
}

static void button_cancel(int param1, int param2)
{
    data.close_callback();
}

void window_display_options_show(void (*close_callback)(void))
{
    window_type window = {
        WINDOW_DISPLAY_OPTIONS,
        window_draw_underlying_window,
        draw_foreground,
        handle_mouse
    };
    init(close_callback);
    window_show(&window);
}
