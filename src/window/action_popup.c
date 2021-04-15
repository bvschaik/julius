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

#define CONFIRM_BUTTON 4

static void button_select_option(int option, int param2);

static generic_button buttons[] = {
        {40, 85, 100, 100, button_select_option, button_none, TRADE_POLICY_1, 0},
        {40, 190, 100, 100, button_select_option, button_none, TRADE_POLICY_2, 0},
        {40, 295, 100, 100, button_select_option, button_none, TRADE_POLICY_3, 0},
        {40, 407, 180, 20, button_select_option, button_none, 0, 0},
        {260, 407, 180, 20, button_select_option, button_none, CONFIRM_BUTTON, 0}
};

static struct {
    int title;
    int prompt;
    option_menu_item option_1;
    option_menu_item option_2;
    option_menu_item option_3;
    int width_blocks;
    int height_blocks;
    void (*close_func)(int selection);
    int focus_button_id;
    int base_border_image_id;
    int selected_policy;
    int current_policy;
} data;

static int init(int title, int prompt, option_menu_item option_1, option_menu_item option_2, option_menu_item option_3,
                void (*close_func)(int selection), int current_option)
{
    if (window_is(WINDOW_POPUP_DIALOG)) {
        // don't show popup over popup
        return 0;
    }
    data.selected_policy = current_option;
    data.current_policy = current_option;
    data.close_func = close_func;
    data.title = title;
    data.prompt = prompt;
    data.option_1 = option_1;
    data.option_2 = option_2;
    data.option_3 = option_3;
    data.width_blocks = 30;
    data.height_blocks = 28;
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
    window_draw_underlying_window();
    graphics_in_dialog();
    outer_panel_draw(80, 40, data.width_blocks, data.height_blocks);

    text_draw_centered(translation_for(data.title), 80, 60, 480, FONT_LARGE_BLACK, 0);
    text_draw_multiline(translation_for(data.prompt), 120, 100, 405, FONT_NORMAL_BLACK, 0);

    if (data.option_1.image_id) {
        image_draw(data.option_1.image_id, 120, 140);
    }

    if (data.selected_policy == 1) {
        inner_panel_draw(250, 140, 18, 6);
    }

    text_draw_multiline(translation_for(data.option_1.header), 260, 150, 270, data.selected_policy == 1 ? FONT_NORMAL_WHITE : FONT_NORMAL_BLACK, 0);
    text_draw_multiline(translation_for(data.option_1.desc), 260, 170, 270, data.selected_policy == 1 ? FONT_NORMAL_WHITE : FONT_NORMAL_BLACK, 0);

    if (data.option_2.image_id) {
        image_draw(data.option_2.image_id, 120, 240);
    }
    if (data.selected_policy == 2) {
        inner_panel_draw(250, 240, 18, 6);
    }

    text_draw_multiline(translation_for(data.option_2.header), 260, 250, 270, data.selected_policy == 2 ? FONT_NORMAL_WHITE : FONT_NORMAL_BLACK, 0);
    text_draw_multiline(translation_for(data.option_2.desc), 260, 270, 270, data.selected_policy == 2 ? FONT_NORMAL_WHITE : FONT_NORMAL_BLACK, 0);

    if (data.option_3.image_id) {
        image_draw(data.option_3.image_id, 120, 340);
    }

    if (data.selected_policy == 3) {
        inner_panel_draw(250, 340, 18, 6);
    }

    text_draw_multiline(translation_for(data.option_3.header), 260, 350, 270, data.selected_policy == 3 ? FONT_NORMAL_WHITE : FONT_NORMAL_BLACK, 0);
    text_draw_multiline(translation_for(data.option_3.desc), 260, 370, 270, data.selected_policy == 3 ? FONT_NORMAL_WHITE : FONT_NORMAL_BLACK, 0);

    lang_text_draw_centered(13, 4, 120, 452, 180, FONT_NORMAL_BLACK);

    int enabled = data.selected_policy && data.selected_policy != data.current_policy;
    text_draw_centered(translation_for(TR_CONFIRM_SELECT_POLICY), 340, 452, 180, enabled ? FONT_NORMAL_BLACK : FONT_NORMAL_PLAIN, enabled ? 0 : COLOR_FONT_LIGHT_GRAY);

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

    button_border_draw(120, 447, 180, 20, data.focus_button_id == 4);

    int enabled = data.selected_policy && data.selected_policy != data.current_policy;
    button_border_draw(340, 447, 180, 20, data.focus_button_id == 5 && enabled);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (input_go_back_requested(m, h)) {
        data.close_func(0);
        window_go_back();
    }

    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 80, 40, buttons, 5, &data.focus_button_id)) {
        return;
    }
}

static void button_select_option(int option, int param2)
{
    if (option && option != CONFIRM_BUTTON) {
        data.selected_policy = option;
        window_request_refresh();
    } else {
        if (option == CONFIRM_BUTTON) {
            if (data.selected_policy && data.current_policy != data.selected_policy) {
                data.close_func(data.selected_policy);
                window_go_back();
            }
        } else {
            data.close_func(0);
            window_go_back();
        }
    }
}

void window_action_popup_show(int title, int subtitle, const option_menu_item *options,
                              void (*close_func)(int selection), int current_option)
{
    if (init(title, subtitle, options[0], options[1], options[2], close_func, current_option)) {
        window_type window = {
                WINDOW_POPUP_DIALOG,
                draw_background,
                draw_foreground,
                handle_input
        };
        window_show(&window);
    }
}
