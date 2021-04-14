#include "action_popup.h"

#include "assets/assets.h"
#include "building/caravanserai.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "translation/translation.h"

static void button_select_option(int option, int param2);

static generic_button buttons[] = {
        {40, 85, 405, 100, button_select_option, button_none, TRADE_POLICY_1, 0},
        {40, 190, 405, 100, button_select_option, button_none, TRADE_POLICY_2, 0},
        {40, 295, 405, 100, button_select_option, button_none, TRADE_POLICY_3, 0},
        {120, 397, 240, 20, button_select_option, button_none, 0, 0}
};

static struct {
    int title;
    int prompt;
    option_menu_item option_1;
    option_menu_item option_2;
    option_menu_item option_3;
    int width_blocks;
    int height_blocks;
    int show_cancel_button;
    void (*close_func)(int selection);
    int focus_button_id;
    int base_border_image_id;
} data;

static int init(int title, int prompt, option_menu_item option_1, option_menu_item option_2, option_menu_item option_3,
                void (*close_func)(int selection), int show_cancel_button)
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
    data.option_3 = option_3;
    data.show_cancel_button = show_cancel_button;
    data.width_blocks = 30;
    data.height_blocks = data.show_cancel_button ? 28 : 27;
    if (!data.option_1.image_id && *data.option_1.asset_author) {
        data.option_1.image_id = assets_get_image_id(assets_get_group_id((char *) data.option_1.asset_author, (char *) data.option_1.asset_name),
                                                     (char *) data.option_1.asset_image_id);
    }
    if (!data.option_2.image_id && *data.option_2.asset_author) {
        data.option_2.image_id = assets_get_image_id(assets_get_group_id((char *) data.option_2.asset_author, (char *) data.option_2.asset_name),
                                                     (char *) data.option_2.asset_image_id);
    }
    if (!data.option_3.image_id && *data.option_3.asset_author) {
        data.option_3.image_id = assets_get_image_id(assets_get_group_id((char *) data.option_3.asset_author, (char *) data.option_3.asset_name),
                                                     (char *) data.option_3.asset_image_id);
    }

    data.base_border_image_id = assets_get_image_id(assets_get_group_id("Areldir", "Econ_Logistics"),
                                                        "Policy Selection Borders");

    return 1;
}

static void draw_background(void)
{
    int text_width = 405;
    int text_x = 120;

    window_draw_underlying_window();
    graphics_in_dialog();
    outer_panel_draw(80, 40, data.width_blocks, data.height_blocks);

    text_draw_centered(translation_for(data.title), 80, 60, 480, FONT_LARGE_BLACK, 0);
    text_draw_multiline(translation_for(data.prompt), text_x, 100, text_width, FONT_NORMAL_BLACK, 0);

    if (data.option_1.image_id) {
        image_draw(data.option_1.image_id, text_x, 140);
        text_x += 140;
        text_width = 270;
    }

    text_draw_multiline(translation_for(data.option_1.header), text_x, 150, text_width, FONT_NORMAL_BLACK, 0);
    text_draw_multiline(translation_for(data.option_1.desc), text_x, 170, text_width, FONT_NORMAL_BLACK, 0);

    text_width = 420;
    text_x = 120;

    if (data.option_2.image_id) {
        image_draw(data.option_2.image_id, text_x, 240);
        text_x += 140;
        text_width = 270;
    }

    text_draw_multiline(translation_for(data.option_2.header), text_x, 250, text_width, FONT_NORMAL_BLACK, 0);
    text_draw_multiline(translation_for(data.option_2.desc), text_x, 270, text_width, FONT_NORMAL_BLACK, 0);

    text_width = 420;
    text_x = 120;

    if (data.option_3.image_id) {
        image_draw(data.option_3.image_id, text_x, 340);
        text_x += 140;
        text_width = 270;
    }

    text_draw_multiline(translation_for(data.option_3.header), text_x, 350, text_width, FONT_NORMAL_BLACK, 0);
    text_draw_multiline(translation_for(data.option_3.desc), text_x, 370, text_width, FONT_NORMAL_BLACK, 0);


    if (data.show_cancel_button) {
        lang_text_draw_centered(13, 4, 200, 452, 240, FONT_NORMAL_BLACK);
    }

    graphics_reset_dialog();
}

static void draw_option_image_border(int x, int y, int focused)
{
    image_draw(data.base_border_image_id + focused, x, y);
    image_draw(data.base_border_image_id + 2 + focused, x + 105, y + 5);
    image_draw(data.base_border_image_id + 4 + focused, x, y + 90);
    image_draw(data.base_border_image_id + 6 + focused, x, y + 5);
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    if (data.option_1.image_id) {
        draw_option_image_border(120, 140, data.focus_button_id == 1);
    }
    if (data.option_2.image_id) {
        draw_option_image_border(120, 240, data.focus_button_id == 2);
    }
    if (data.option_3.image_id) {
        draw_option_image_border(120, 340, data.focus_button_id == 3);
    }
    if (data.show_cancel_button) {
        button_border_draw(200, 447, 240, 20, data.focus_button_id == 4);
    }
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (input_go_back_requested(m, h)) {
        data.close_func(0);
        window_go_back();
    }

    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 80, 40, buttons, data.show_cancel_button ? 4 : 3, &data.focus_button_id)) {
        return;
    }

}

static void button_select_option(int option, int param2)
{
    if (option == 1 || option == 2 || option == 3) {
        data.close_func(option);
        window_go_back();
    } else if (!option) {
        data.close_func(option);
        window_go_back();
    }

}

void window_action_popup_show(int title, int subtitle, const option_menu_item *options,
                              void (*close_func)(int selection), int show_cancel_button)
{
    if (init(title, subtitle, options[0], options[1], options[2], close_func, show_cancel_button)) {
        window_type window = {
                WINDOW_POPUP_DIALOG,
                draw_background,
                draw_foreground,
                handle_input
        };
        window_show(&window);
    }
}
