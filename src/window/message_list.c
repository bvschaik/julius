#include "message_list.h"

#include "city/message.h"
#include "core/calc.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "window/city.h"
#include "window/message_dialog.h"

#define MAX_MESSAGES 10

static void button_help(int param1, int param2);
static void button_close(int param1, int param2);
static void button_scroll(int is_down, int num_lines);
static void button_message(int param1, int param2);
static void button_delete(int param1, int param2);

static image_button image_button_help = {
    0, 0, 27, 27, IB_NORMAL, GROUP_CONTEXT_ICONS, 0, button_help, button_none, 0, 0, 1
};
static image_button image_button_close = {
    0, 0, 24, 24, IB_NORMAL, GROUP_CONTEXT_ICONS, 4, button_close, button_none, 0, 0, 1
};
static image_button image_button_scroll_up = {
    0, 0, 39, 26, IB_SCROLL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 8, button_scroll, button_none, 0, 1, 1
};
static image_button image_button_scroll_down = {
    0, 0, 39, 26, IB_SCROLL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 12, button_scroll, button_none, 1, 1, 1
};
static generic_button generic_buttons_messages[] = {
    {0, 0, 412, 18, button_message, button_delete, 0, 0},
    {0, 20, 412, 18, button_message, button_delete, 1, 0},
    {0, 40, 412, 18, button_message, button_delete, 2, 0},
    {0, 60, 412, 18, button_message, button_delete, 3, 0},
    {0, 80, 412, 18, button_message, button_delete, 4, 0},
    {0, 100, 412, 18, button_message, button_delete, 5, 0},
    {0, 120, 412, 18, button_message, button_delete, 6, 0},
    {0, 140, 412, 18, button_message, button_delete, 7, 0},
    {0, 160, 412, 18, button_message, button_delete, 8, 0},
    {0, 180, 412, 18, button_message, button_delete, 9, 0},
};

static struct {
    int width_blocks;
    int height_blocks;
    int x_text;
    int y_text;
    int text_width_blocks;
    int text_height_blocks;

    int is_dragging_scrollbar;
    int scroll_position_drag;

    int focus_button_id;
} data;

static void init(void)
{
    city_message_sort_and_compact();
    city_message_update_scroll(MAX_MESSAGES);
}

static void draw_background(void)
{
    window_city_draw_all();

    graphics_in_dialog();
    data.width_blocks = 30;
    data.height_blocks = 22;
    data.x_text = 16;
    data.y_text = 112;
    data.text_width_blocks = data.width_blocks - 4;
    data.text_height_blocks = data.height_blocks - 9;

    outer_panel_draw(0, 32, data.width_blocks, data.height_blocks);
    lang_text_draw_centered(63, 0, 0, 48, 16 * data.width_blocks, FONT_LARGE_BLACK);
    inner_panel_draw(data.x_text, data.y_text, data.text_width_blocks, data.text_height_blocks);

    if (city_message_count() > 0) {
        lang_text_draw(63, 2, data.x_text + 42, data.y_text - 12, FONT_SMALL_PLAIN);
        lang_text_draw(63, 3, data.x_text + 180, data.y_text - 12, FONT_SMALL_PLAIN);
        lang_text_draw_multiline(63, 4,
            data.x_text + 50, data.y_text + 12 + 16 * data.text_height_blocks,
            16 * data.text_width_blocks - 100, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_multiline(63, 1,
            data.x_text + 16, data.y_text + 80,
            16 * data.text_width_blocks - 48, FONT_NORMAL_GREEN);
    }
    graphics_reset_dialog();
}

static void draw_messages(int total_messages)
{
    int max = total_messages < MAX_MESSAGES ? total_messages : MAX_MESSAGES;
    int index = city_message_scroll_position();
    for (int i = 0; i < max; i++, index++) {
        const city_message *msg = city_message_get(index);
        const lang_message *lang_msg = lang_get_message(city_message_get_text_id(msg->message_type));
        int image_offset = 0;
        if (lang_msg->message_type == MESSAGE_TYPE_DISASTER) {
            image_offset = 2;
        }
        if (msg->is_read) {
            image_draw(image_group(GROUP_MESSAGE_ICON) + 15 + image_offset,
                data.x_text + 12, data.y_text + 6 + 20 * i);
        } else {
            image_draw(image_group(GROUP_MESSAGE_ICON) + 14 + image_offset,
                data.x_text + 12, data.y_text + 6 + 20 * i);
        }
        font_t font = FONT_NORMAL_WHITE;
        if (data.focus_button_id == i + 1) {
            font = FONT_NORMAL_RED;
        }
        int width = lang_text_draw(25, msg->month, data.x_text + 42, data.y_text + 8 + 20 * i, font);
        lang_text_draw_year(msg->year,
            data.x_text + 42 + width, data.y_text + 8 + 20 * i, font);
        text_draw(
            lang_msg->title.text,
            data.x_text + 180, data.y_text + 8 + 20 * i, font, 0);
    }
    if (city_message_can_scroll()) {
        image_buttons_draw(
            data.x_text + 16 * data.text_width_blocks, data.y_text,
            &image_button_scroll_up, 1);
        image_buttons_draw(
            data.x_text + 16 * data.text_width_blocks, data.y_text + 16 * data.text_height_blocks - 26,
            &image_button_scroll_down, 1);
        int pct_scrolled = city_message_scroll_percentage();
        int dot_offset = calc_adjust_with_percentage(16 * data.text_height_blocks - 77, pct_scrolled);
        if (data.is_dragging_scrollbar) {
            dot_offset = data.scroll_position_drag;
        }
        image_draw(image_group(GROUP_PANEL_BUTTON) + 39,
            data.x_text + 9 + 16 * data.text_width_blocks, data.y_text + 26 + dot_offset);
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    image_buttons_draw(16, 32 + 16 * data.height_blocks - 42, &image_button_help, 1);
    image_buttons_draw(16 * data.width_blocks - 38, 32 + 16 * data.height_blocks - 36, &image_button_close, 1);

    int total_messages = city_message_count();
    if (total_messages > 0) {
        draw_messages(total_messages);
    }

    graphics_reset_dialog();
}

static int handle_mouse_scrollbar(const mouse *m)
{
    if (!city_message_can_scroll() || !m->left.is_down) {
        return 0;
    }
    int scrollbar_x = data.x_text + 16 * data.text_width_blocks + 1;
    int scrollbar_y = data.y_text + 26;
    int scrollbar_height = 16 * data.text_height_blocks - 52;
    if (m->x >= scrollbar_x && m->x <= scrollbar_x + 40 &&
        m->y >= scrollbar_y && m->y <= scrollbar_y + scrollbar_height) {
        int dot_offset = m->y - data.y_text - 11;
        if (dot_offset > scrollbar_height) {
            dot_offset = scrollbar_height;
        }
        int pct_scrolled = calc_percentage(dot_offset, scrollbar_height);
        city_message_set_scroll_percentage(pct_scrolled);
        data.is_dragging_scrollbar = 1;
        data.scroll_position_drag = dot_offset - 25;
        if (data.scroll_position_drag < 0) {
            data.scroll_position_drag = 0;
        }
        window_invalidate();
        return 1;
    }
    return 0;
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (m_dialog->scrolled == SCROLL_DOWN) {
        button_scroll(1, 3);
    } else if (m_dialog->scrolled == SCROLL_UP) {
        button_scroll(0, 3);
    }
    int button_id;
    image_buttons_handle_mouse(m_dialog, 16, 32 + 16 * data.height_blocks - 42, &image_button_help, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 11;
        return;
    }
    image_buttons_handle_mouse(m_dialog, 16 * data.width_blocks - 38,
        32 + 16 * data.height_blocks - 36, &image_button_close, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 12;
        return;
    }
    image_buttons_handle_mouse(m_dialog, data.x_text + 16 * data.text_width_blocks, data.y_text, &image_button_scroll_up, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 13;
        return;
    }
    image_buttons_handle_mouse(m_dialog, data.x_text + 16 * data.text_width_blocks,
        data.y_text + 16 * data.text_height_blocks - 26, &image_button_scroll_down, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 13;
        return;
    }
    int old_focus_button_id = data.focus_button_id;
    if (generic_buttons_handle_mouse(m_dialog, data.x_text, data.y_text + 4,
        generic_buttons_messages, MAX_MESSAGES, &data.focus_button_id)) {
        if (old_focus_button_id != data.focus_button_id) {
            window_invalidate();
        }
        return;
    }
    if (handle_mouse_scrollbar(m_dialog)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        button_close(0, 0);
    }
}

static void button_scroll(int is_down, int num_lines)
{
    city_message_scroll(is_down, num_lines);
    data.is_dragging_scrollbar = 0;
    window_invalidate();
}

static void button_help(int param1, int param2)
{
    window_message_dialog_show(MESSAGE_DIALOG_MESSAGES, window_city_draw_all);
}

static void button_close(int param1, int param2)
{
    window_city_show();
}

static void button_message(int param1, int param2)
{
    int id = city_message_set_current(city_message_scroll_position() + param1);
    if (id < city_message_count()) {
        const city_message *msg = city_message_get(id);
        city_message_mark_read(id);
        window_message_dialog_show_city_message(
            city_message_get_text_id(msg->message_type),
            msg->year, msg->month, msg->param1, msg->param2,
            city_message_get_advisor(msg->message_type),
            0);
    }
}

static void button_delete(int param1, int param2)
{
    int id = city_message_set_current(city_message_scroll_position() + param1);
    if (id < city_message_count()) {
        city_message_delete(id);
        city_message_update_scroll(MAX_MESSAGES);
        window_invalidate();
    }
}

static void get_tooltip(tooltip_context *c)
{
    if (data.focus_button_id == 11) {
        c->text_id = 1;
    } else if (data.focus_button_id == 12) {
        c->text_id = 2;
    } else {
        return;
    }
    c->type = TOOLTIP_BUTTON;
}

void window_message_list_show(void)
{
    window_type window = {
        WINDOW_MESSAGE_LIST,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    init();
    window_show(&window);
}
