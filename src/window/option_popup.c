#include "option_popup.h"

#include "assets/assets.h"
#include "core/image_group.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "translation/translation.h"

#define CANCEL_BUTTON 0
#define CONFIRM_BUTTON 1
#define MAX_OPTIONS 3
#define START_Y_OFFSET 88

static const int Y_OFFSET_PER_OPTION[2] = { 112, 144 };
static int border_image_ids[2];

static void on_scroll(void);

static void button_select_option(int option, int param2);

static generic_button buttons[] = {
    {40, 0, 180, 20, button_select_option, button_none, CANCEL_BUTTON, 0},
    {260, 0, 180, 20, button_select_option, button_none, CONFIRM_BUTTON, 0},
    {20, 0, 0, 0, button_select_option, button_none, 2, 0},
    {20, 0, 0, 0, button_select_option, button_none, 3, 0},
    {20, 0, 0, 0, button_select_option, button_none, 4, 0}
};

static scrollbar_type scrollbar = { 420, START_Y_OFFSET + 40, 0, 400, 0, on_scroll, 0, 4 };

static struct {
    int title;
    int subtitle;
    option_menu_item *options;
    int num_options;
    int width_blocks;
    int height_blocks;
    void (*close_func)(int selection);
    int focus_button_id;
    int original_option;
    int selected_option;
    int price;
    int visible_options;
    int scroll_position;
    int height;
    option_menu_row_size row_size;
} data;

static int init(int title, int subtitle, option_menu_item *options, int num_options,
    void (*close_func)(int selection), int original_option, int price, option_menu_row_size row_size)
{
    if (window_is(WINDOW_POPUP_DIALOG)) {
        // don't show popup over popup
        return 0;
    }
    data.num_options = num_options <= MAX_OPTIONS ? num_options : MAX_OPTIONS;
    data.close_func = close_func;
    data.title = title;
    data.subtitle = subtitle;
    data.width_blocks = 30;
    data.original_option = original_option;
    data.selected_option = data.original_option;
    data.price = price;
    data.options = options;
    data.height = 0;
    data.row_size = row_size;

    for (int i = 0; i < MAX_OPTIONS; i++) {
        buttons[i + 2].y = 40 + START_Y_OFFSET + i * Y_OFFSET_PER_OPTION[data.row_size];
        buttons[i + 2].height = Y_OFFSET_PER_OPTION[data.row_size] - 16;
    }

    if (!border_image_ids[0]) {
        border_image_ids[0] = assets_get_image_id("UI", "Image Border Medium");
        border_image_ids[1] = assets_get_image_id("UI", "Image Border Large");
    }
    return 1;
}

static void calculate_visible_options(void)
{
    if (data.height == screen_height()) {
        return;
    }
    data.height = screen_height();
    data.visible_options = (data.height - 160) / Y_OFFSET_PER_OPTION[data.row_size];
    if (data.visible_options > data.num_options) {
        data.visible_options = data.num_options;
    }
    data.height_blocks = 10 + data.visible_options * (data.row_size == OPTION_MENU_SMALL_ROW ? 7 : 9);
    buttons[0].y = buttons[1].y = START_Y_OFFSET + 42 + Y_OFFSET_PER_OPTION[data.row_size] * data.visible_options;

    buttons[2].width = buttons[3].width = buttons[4].width = data.num_options == data.visible_options ? 430 : 400;

    scrollbar.height = Y_OFFSET_PER_OPTION[data.row_size] * data.visible_options;
    scrollbar.elements_in_view =  data.visible_options;
    scrollbar_init(&scrollbar, 0, data.num_options);
    if (data.selected_option > data.visible_options) {
        scrollbar.scroll_position = data.selected_option - data.visible_options;
    }
}

static void draw_apply_button(int x, int y, int box_width)
{
    font_t font = data.selected_option != data.original_option ? FONT_NORMAL_BLACK : FONT_NORMAL_PLAIN;
    color_t color = data.selected_option != data.original_option ? 0 : COLOR_FONT_LIGHT_GRAY;
    if (!data.price) {
        text_draw_centered(translation_for(TR_OPTION_MENU_APPLY), x, y, box_width, font, color);
    } else {
        text_draw_with_money(translation_for(TR_OPTION_MENU_APPLY), data.price, " (", ")",
            x, y, box_width, font, color);
    }
}

static void draw_background(void)
{
    window_draw_underlying_window();

    calculate_visible_options();
    graphics_in_dialog_with_size(16 * data.width_blocks, 16 * data.height_blocks);
    outer_panel_draw(0, 0, data.width_blocks, data.height_blocks);

    text_draw_centered(translation_for(data.title), 0, 20, 480, FONT_LARGE_BLACK, 0);
    text_draw_multiline(translation_for(data.subtitle), 20, 60, 440, FONT_NORMAL_BLACK, 0);
    if (data.price) {
        text_draw_with_money(translation_for(TR_OPTION_MENU_COST), data.price, " ", ".",
            20, 110, 0, FONT_NORMAL_BLACK, 0);
    }

    int y_offset = START_Y_OFFSET;

    for (int i = 0; i < data.visible_options; i++) {
        int text_width = data.num_options == data.visible_options ? 448 : 400;
        int text_x = 20;

        if (data.options[i + scrollbar.scroll_position].image_id) {
            image_draw(data.options[i + scrollbar.scroll_position].image_id, text_x, y_offset + 42,
                COLOR_MASK_NONE, SCALE_NONE);
            int offset = data.row_size == OPTION_MENU_SMALL_ROW ? 128 : 160;
            text_x += offset;
            text_width -= offset;
        }
        if (data.selected_option == i + scrollbar.scroll_position + 1) {
            inner_panel_draw(text_x - 6, y_offset + 44,
                text_width / 16,
                Y_OFFSET_PER_OPTION[data.row_size] / 16 - 1);
        }
        text_draw_multiline(translation_for(data.options[i + scrollbar.scroll_position].header),
            text_x, y_offset + 49, text_width - 8,
            data.selected_option == i + scrollbar.scroll_position + 1 ? FONT_NORMAL_WHITE : FONT_NORMAL_BLACK, 0);
        text_draw_multiline(translation_for(data.options[i + scrollbar.scroll_position].desc),
            text_x, y_offset + 69, text_width - 8,
            data.selected_option == i + scrollbar.scroll_position + 1 ? FONT_NORMAL_WHITE : FONT_NORMAL_BLACK, 0);


        y_offset += Y_OFFSET_PER_OPTION[data.row_size];
    }

    lang_text_draw_centered(13, 4, 40, buttons[0].y + 4,
        180, FONT_NORMAL_BLACK);
    draw_apply_button(260, buttons[1].y + 4, 180);

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog_with_size(16 * data.width_blocks, 16 * data.height_blocks);
    for (int i = 0; i < data.visible_options; i++) {
        if (data.options[i + scrollbar.scroll_position].image_id) {
            color_t color = data.focus_button_id == i + 3 ? COLOR_BORDER_RED : COLOR_BORDER_GREEN;
            image_draw_border(border_image_ids[data.row_size], 20, buttons[i + 2].y + 2, color);
        }
    }

    button_border_draw(40, buttons[0].y, 180, 20, data.focus_button_id == 1);

    button_border_draw(260, buttons[1].y, 180, 20,
        data.focus_button_id == 2 && data.selected_option != data.original_option);

    if (data.num_options > data.visible_options) {
        inner_panel_draw(scrollbar.x + 4, scrollbar.y + 32, 2, scrollbar.height / 16 - 4);
        scrollbar_draw(&scrollbar);
    }

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (input_go_back_requested(m, h)) {
        data.close_func(0);
        window_go_back();
        return;
    }
    const mouse *m_dialog = mouse_in_dialog_with_size(m, data.width_blocks * 16, data.height_blocks * 16);
    if (scrollbar_handle_mouse(&scrollbar, m_dialog, 1)) {
        data.focus_button_id = 0;
        return;
    }
    generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, data.visible_options + 2, &data.focus_button_id);
}

static void on_scroll(void)
{
    window_invalidate();
}

static void button_select_option(int option, int param2)
{
    switch (option) {
        case CANCEL_BUTTON:
            data.close_func(0);
            window_go_back();
            break;
        case CONFIRM_BUTTON:
            if (data.selected_option != data.original_option) {
                data.close_func(data.selected_option);
                window_go_back();
            }
            break;
        default:
            data.selected_option = option + scrollbar.scroll_position - CONFIRM_BUTTON;
            window_request_refresh();
            break;
    }
}

void window_option_popup_show(int title, int subtitle, option_menu_item *options, int num_options,
    void (*close_func)(int selection), int current_option, int price, option_menu_row_size row_size)
{
    if (init(title, subtitle, options, num_options, close_func, current_option, price, row_size)) {
        window_type window = {
            WINDOW_POPUP_DIALOG,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}
