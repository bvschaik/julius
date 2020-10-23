#include "option_popup.h"

#include "core/image_group.h"
#include "core/mods.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "translation/translation.h"

static void button_select_option_1(int param1, int param2);
static void button_select_option_2(int param1, int param2);

static image_button buttons[] = {
    {20, 120, 150, 130, 0, 0, 0, button_select_option_1, button_none, 0, 0, 1},
    {20, 260, 150, 130, 0, 0, 0, button_select_option_2, button_none, 0, 0, 1},
};

static struct {
    int title;
    int prompt;
    option_menu_item option_1;
    option_menu_item option_2;
    int width_blocks;
    int height_blocks;
    void (*close_func)(int selection);
} data;

static int init(int title, int prompt, option_menu_item option_1, option_menu_item option_2,
        void (*close_func)(int selection))
{
    if (window_is(WINDOW_POPUP_DIALOG)) {
        // don't show popup over popup
        return 0;
    }
    data.close_func = close_func;
    data.title = title;
    data.prompt = prompt;
    data.option_1 = option_1;
    data.option_2 = option_2;
    data.width_blocks = 30;
    data.height_blocks = 26;
    return 1;
}

static void draw_background(void)
{
    int text_width = 440;
    int text_x = 100;
    
    window_draw_underlying_window();
    graphics_in_dialog();
    outer_panel_draw(80, 80, data.width_blocks, data.height_blocks);

    text_draw_centered(translation_for(data.title), 80, 100, 480, FONT_LARGE_BLACK, 0);
    text_draw_multiline(translation_for(data.prompt), text_x, 140, text_width, FONT_NORMAL_BLACK, 0);

    if (data.option_1.image_id) {
        image_draw(data.option_1.image_id, text_x, 200);
        text_x += 160;
        text_width = 270;
    } else if (*data.option_1.mod_author) {
        image_draw(mods_get_image_id(mods_get_group_id((char *) data.option_1.mod_author, (char *) data.option_1.mod_name), (char *)data.option_1.mod_image_id), text_x, 200);
        text_x += 160;
        text_width = 270;
    }

    text_draw_multiline(translation_for(data.option_1.header), text_x, 205, text_width, FONT_NORMAL_BLACK, 0);
    text_draw_multiline(translation_for(data.option_1.desc), text_x, 225, text_width, FONT_NORMAL_BLACK, 0);    
    
    text_width = 420;
    text_x = 100;

    if (data.option_2.image_id) {
        image_draw(data.option_2.image_id, text_x, 340);
        text_x += 160;
        text_width = 270;
    } else if (*data.option_2.mod_author) {
        image_draw(mods_get_image_id(mods_get_group_id((char *) data.option_2.mod_author, (char *) data.option_2.mod_name), (char *) data.option_2.mod_image_id), text_x, 340);
        text_x += 160;
        text_width = 270;
    }

    text_draw_multiline(translation_for(data.option_2.header), text_x, 345, text_width, FONT_NORMAL_BLACK, 0);
    text_draw_multiline(translation_for(data.option_2.desc), text_x, 365, text_width, FONT_NORMAL_BLACK, 0);

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    image_buttons_draw(80, 80, buttons, 2);
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (image_buttons_handle_mouse(mouse_in_dialog(m), 80, 80, buttons, 2, 0)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        data.close_func(0);
        window_go_back();
    }
}

static void button_select_option_1(int param1, int param2)
{
    data.close_func(1);
    window_go_back();
}

static void button_select_option_2(int param1, int param2)
{
    data.close_func(2);
    window_go_back();
}

void window_show_option_popup(int title, int prompt, option_menu_item *options,
    void (*close_func)(int selection))
{
    if (init(title, prompt, options[0], options[1], close_func)) {
        window_type window = {
            WINDOW_POPUP_DIALOG,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}

