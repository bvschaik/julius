#include "option_popup.h"

#include "assets/assets.h"
#include "core/image_group.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "translation/translation.h"

#define CONFIRM_BUTTON 3

static void button_select_option(int option, int param2);

static generic_button buttons[] = {
    {20, 120, 150, 130, button_select_option, button_none, 1, 0},
    {20, 260, 150, 130, button_select_option, button_none, 2, 0},
    {40, 407, 180, 20, button_select_option, button_none, 0, 0},
    {260, 407, 180, 20, button_select_option, button_none, CONFIRM_BUTTON, 0}
};

static struct {
    int title;
    int prompt;
    option_menu_item option_1;
    option_menu_item option_2;
    int width_blocks;
    int height_blocks;
    int show_cancel_button;
    int show_confirm_button;
    void (*close_func)(int selection);
    int focus_button_id;
    int base_border_image_id;
    int selected_option;
} data;

static int init(int title, int prompt, option_menu_item option_1, option_menu_item option_2,
        void (*close_func)(int selection), int show_cancel_button, int show_confirm_button)
{
    if (window_is(WINDOW_POPUP_DIALOG)) {
        // don't show popup over popup
        return 0;
    }
    data.close_func = close_func;
    data.selected_option = 0;
    data.title = title;
    data.prompt = prompt;
    data.option_1 = option_1;
    data.option_2 = option_2;
    data.show_cancel_button = show_cancel_button;
    data.show_confirm_button = show_confirm_button;
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
    if (!data.base_border_image_id) {
        data.base_border_image_id = assets_get_image_id(assets_get_group_id("Areldir", "UI_Elements"), 
            "Monument Mod Selection Borders");
    }
    return 1;
}

static void draw_background(void)
{
    int text_width = 440;
    int text_x = 100;
    
    window_draw_underlying_window();
    graphics_in_dialog();
    outer_panel_draw(80, 40, data.width_blocks, data.height_blocks);

    text_draw_centered(translation_for(data.title), 80, 60, 480, FONT_LARGE_BLACK, 0);
    text_draw_multiline(translation_for(data.prompt), text_x, 100, text_width, FONT_NORMAL_BLACK, 0);

    if (data.option_1.show) {
        if (data.option_1.image_id) {
            image_draw(data.option_1.image_id, text_x, 170);
            text_x += 162;
            text_width = 270;
        }

        if (data.selected_option == 1) {
            inner_panel_draw(text_x - 6, 172, 18, 8);
        }

        text_draw_multiline(translation_for(data.option_1.header), text_x, 177, text_width, data.selected_option == 1 ? FONT_NORMAL_WHITE : FONT_NORMAL_BLACK, 0);
        text_draw_multiline(translation_for(data.option_1.desc), text_x, 197, text_width, data.selected_option == 1 ? FONT_NORMAL_WHITE : FONT_NORMAL_BLACK, 0);

        text_width = 420;
        text_x = 100;
    }

    if (data.option_2.show) {
        if (data.option_2.image_id) {
            image_draw(data.option_2.image_id, text_x, 310);
            text_x += 162;
            text_width = 270;
        }

        if (data.selected_option == 2) {
            inner_panel_draw(text_x - 6, 312, 18, 8);
        }

        text_draw_multiline(translation_for(data.option_2.header), text_x, 317, text_width, data.selected_option == 2 ? FONT_NORMAL_WHITE : FONT_NORMAL_BLACK, 0);
        text_draw_multiline(translation_for(data.option_2.desc), text_x, 337, text_width, data.selected_option == 2 ? FONT_NORMAL_WHITE : FONT_NORMAL_BLACK, 0);
    }

    if (data.show_cancel_button) {
        lang_text_draw_centered(13, 4, 120, 452, 180, FONT_NORMAL_BLACK);
    }

    if (data.show_confirm_button) {
        text_draw_centered(translation_for(TR_CONFIRM_SELECT_EPITHET), 340, 452, 180,
                           data.selected_option ? FONT_NORMAL_BLACK : FONT_NORMAL_PLAIN, data.selected_option ? 0 : COLOR_FONT_LIGHT_GRAY);

    }

    graphics_reset_dialog();
}

static void draw_option_image_border(int x, int y, int focused)
{
    image_draw(data.base_border_image_id + focused, x, y);
    image_draw(data.base_border_image_id + 2 + focused, x + 146, y + 5);
    image_draw(data.base_border_image_id + 4 + focused, x, y + 126);
    image_draw(data.base_border_image_id + 6 + focused, x, y + 5);
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    if (data.option_1.image_id && data.option_1.show) {
        draw_option_image_border(100, 170, data.focus_button_id == 1);
    }
    if (data.option_2.image_id && data.option_2.show) {
        draw_option_image_border(100, 310, data.focus_button_id == 2);
    }
    if (data.show_cancel_button) {
        button_border_draw(120, 447, 180, 20, data.focus_button_id == 3);
    }
    if (data.show_confirm_button) {
        button_border_draw(340, 447, 180, 20, data.focus_button_id == 4 && data.selected_option);
    }
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (input_go_back_requested(m, h)) {
        data.close_func(0);
        window_go_back();
    }

    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 80, 40, buttons,
                                     2 + data.show_cancel_button + data.show_confirm_button, &data.focus_button_id)) {
        return;
    }

}

static void button_select_option(int option, int param2)
{
    if (option && option != CONFIRM_BUTTON) {
        data.selected_option = option;
        window_request_refresh();
    } else {
        if (option == CONFIRM_BUTTON) {
            if (data.selected_option) {
                data.close_func(data.selected_option);
                window_go_back();
            }
        } else {
            data.close_func(0);
            window_go_back();
        }
    }
}

void window_option_popup_show(int title, int prompt, option_menu_item *options,
    void (*close_func)(int selection), int show_cancel_button, int show_confirm_button)
{
    if (init(title, prompt, options[0], options[1], close_func, show_cancel_button, show_confirm_button)) {
        window_type window = {
            WINDOW_POPUP_DIALOG,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}

