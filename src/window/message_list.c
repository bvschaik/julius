#include "message_list.h"

#include "city/message.h"
#include "core/calc.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "game/campaign.h"
#include "game/time.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/custom_messages.h"
#include "scenario/property.h"
#include "translation/translation.h"
#include "window/city.h"
#include "window/message_dialog.h"
#include "window/mission_briefing.h"

#define MAX_MESSAGES 10

typedef enum {
    ALL_MESSAGES_SHOWN,
    COMMON_MESSAGES_ONLY,
    CUSTOM_MESSAGES_ONLY
} message_filter;


static void button_help(int param1, int param2);
static void button_close(int param1, int param2);
static void button_message(const generic_button *button);
static void button_delete(const generic_button *button);
static void button_delete_all_read(const generic_button *button);
static void button_delete_all_common(const generic_button *button);
static void button_mission_briefing(int param1, int param2);
static void button_cycle_message_type(const generic_button *button);
static void on_scroll(void);

static image_button image_button_help = {
    0, 0, 27, 27, IB_NORMAL, GROUP_CONTEXT_ICONS, 0, button_help, button_none, 0, 0, 1
};
static image_button image_button_close = {
    0, 0, 24, 24, IB_NORMAL, GROUP_CONTEXT_ICONS, 4, button_close, button_none, 0, 0, 1
};
static image_button show_briefing_button = {
    0, 0, 33, 22, IB_NORMAL, GROUP_SIDEBAR_BRIEFING_ROTATE_BUTTONS, 0, button_mission_briefing, button_none, 0, 0, 1
};
static generic_button generic_buttons_messages[] = {
    {0, 0, 412, 18, button_message, button_delete},
    {0, 20, 412, 18, button_message, button_delete, 1},
    {0, 40, 412, 18, button_message, button_delete, 2},
    {0, 60, 412, 18, button_message, button_delete, 3},
    {0, 80, 412, 18, button_message, button_delete, 4},
    {0, 100, 412, 18, button_message, button_delete, 5},
    {0, 120, 412, 18, button_message, button_delete, 6},
    {0, 140, 412, 18, button_message, button_delete, 7},
    {0, 160, 412, 18, button_message, button_delete, 8},
    {0, 180, 412, 18, button_message, button_delete, 9},
};

static generic_button generic_button_messages_type[] = {
    { 75, 77, 300, 20, button_cycle_message_type, button_cycle_message_type }
};

static generic_button generic_button_delete_read[] = {
    { 0, 0, 20, 20, button_delete_all_read }
};

static generic_button generic_button_delete_common[] = {
    { 0, 0, 20, 20, button_delete_all_common }
};


static scrollbar_type scrollbar = {432, 112, 208, 416, MAX_MESSAGES, on_scroll, 1};

static void draw_delete_read_button(int x, int y, int focused)
{
    uint8_t delete_read_text[] = { 'x', 0 };
    button_border_draw(x, y, 20, 20, focused ? 1 : 0);
    text_draw_centered(delete_read_text, x + 1, y + 4, 20, FONT_NORMAL_BLACK, 0);
}

static void draw_delete_common_button(int x, int y, int focused)
{
    uint8_t delete_common_text[] = { 'x', 0 };
    button_border_draw(x, y, 20, 20, focused ? 1 : 0);
    text_draw_centered(delete_common_text, x + 1, y + 4, 20, FONT_NORMAL_BLACK, 0);
}

static struct {
    int width_blocks;
    int height_blocks;
    int x_text;
    int y_text;
    int text_width_blocks;
    int text_height_blocks;
    unsigned int focus_button_id;
    message_filter type_displayed;
    int message_count;
    int *filtered_message_list;
} data;

static void draw_message_type_button(int x, int y, int focused)
{
    button_border_draw(x, y, generic_button_messages_type->width, generic_button_messages_type->height, focused ? 1 : 0);
    text_draw_centered(translation_for(TR_WINDOW_MESSAGE_LIST_SELECTED_ALL + data.type_displayed), x + 4, y + 4, 
        generic_button_messages_type->width, FONT_NORMAL_BLACK, 0);
}

static int review_briefing_button_should_be_active(void)
{
    if (game_campaign_is_original()) {
        return 1;
    }
    if (!scenario_intro_message()) {
        return 0;
    }
    custom_message_t *custom_message = custom_messages_get(scenario_intro_message());
    if (!custom_message) {
        return 0;
    }
    return custom_messages_get_text(custom_message) ||
        custom_messages_get_title(custom_message) || custom_messages_get_subtitle(custom_message);
}

static void clear_filtered_message_list(void)
{
    free(data.filtered_message_list);
    data.filtered_message_list = 0;
}

static void select_messages(void)
{
    if (data.filtered_message_list) {
        clear_filtered_message_list();
    }

    data.message_count = 0;
    data.filtered_message_list = (int *) malloc(sizeof(int) * city_message_count());
    for (int i = 0; i < city_message_count(); i++) {
        const city_message *msg = city_message_get(i);
        if (data.type_displayed == COMMON_MESSAGES_ONLY && (msg->message_type == MESSAGE_CUSTOM_MESSAGE)) {
            continue;
        }

        if (data.type_displayed == CUSTOM_MESSAGES_ONLY && (msg->message_type != MESSAGE_CUSTOM_MESSAGE)) {
            continue;
        }

        data.filtered_message_list[data.message_count] = i;
        data.message_count++;

    }
    scrollbar_update_total_elements(&scrollbar, data.message_count);

}

static void init(void)
{
    city_message_sort_and_compact();
    select_messages();
    scrollbar_init(&scrollbar, city_message_scroll_position(), data.message_count);
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
    lang_text_draw_centered(63, 0, 0, 48, BLOCK_SIZE * data.width_blocks, FONT_LARGE_BLACK);
    inner_panel_draw(data.x_text, data.y_text, data.text_width_blocks, data.text_height_blocks);

    if (review_briefing_button_should_be_active()) {
        button_border_draw(data.x_text + data.text_width_blocks * BLOCK_SIZE - 39, data.y_text - 30, 39, 28, 0);
    }

    if (city_message_count() > 0) {
        lang_text_draw(63, 2, data.x_text + 42, data.y_text - 12, FONT_SMALL_PLAIN);
        lang_text_draw(63, 3, data.x_text + 180, data.y_text - 12, FONT_SMALL_PLAIN);
        lang_text_draw_multiline(63, 4,
            data.x_text + 55, data.y_text + 12 + BLOCK_SIZE * data.text_height_blocks,
            BLOCK_SIZE * data.text_width_blocks - 64, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_multiline(63, 1,
            data.x_text + 16, data.y_text + 80,
            BLOCK_SIZE * data.text_width_blocks - 48, FONT_NORMAL_GREEN);
    }
    graphics_reset_dialog();
}

static void draw_messages(unsigned int total_messages)
{
    unsigned int max = total_messages < MAX_MESSAGES ? total_messages : MAX_MESSAGES;
    unsigned int index = scrollbar.scroll_position;
    for (unsigned int i = 0; i < max; i++, index++) {
        const city_message *msg = city_message_get(data.filtered_message_list[index]);
        int image_offset = 0;
        const lang_message *lang_msg = 0;
        if (msg->message_type != MESSAGE_CUSTOM_MESSAGE) {
            lang_msg = lang_get_message(city_message_get_text_id(msg->message_type));
            if (lang_msg->message_type == MESSAGE_TYPE_DISASTER) {
                image_offset = 2;
            }
        }
        if (msg->is_read) {
            image_draw(image_group(GROUP_MESSAGE_ICON) + 15 + image_offset,
                data.x_text + 12, data.y_text + 6 + 20 * i, COLOR_MASK_NONE, SCALE_NONE);
        } else {
            image_draw(image_group(GROUP_MESSAGE_ICON) + 14 + image_offset,
                data.x_text + 12, data.y_text + 6 + 20 * i, COLOR_MASK_NONE, SCALE_NONE);
        }
        font_t font = FONT_NORMAL_WHITE;
        if (data.focus_button_id == i + 1) {
            font = FONT_NORMAL_RED;
        }
        int width = lang_text_draw(25, msg->month, data.x_text + 42, data.y_text + 8 + 20 * i, font);
        lang_text_draw_year(msg->year,
            data.x_text + 42 + width, data.y_text + 8 + 20 * i, font);
        if (msg->message_type != MESSAGE_CUSTOM_MESSAGE && lang_msg) {
            text_draw(
                lang_msg->title.text,
                data.x_text + 180, data.y_text + 8 + 20 * i, font, 0);
        } else {
            custom_message_t *custom_msg = custom_messages_get(msg->param1);
            if (custom_msg->title) {
                text_draw(custom_msg->title->text, data.x_text + 180, data.y_text + 8 + 20 * i, font, 0);
            } else {
                text_draw(translation_for(TR_ACTION_TYPE_A_MESSAGE), data.x_text + 180, data.y_text + 8 + 20 * i, font, 0);
            }
        }
    }
    scrollbar_draw(&scrollbar);
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    image_buttons_draw(16, 32 + BLOCK_SIZE * data.height_blocks - 42, &image_button_help, 1);
    image_buttons_draw(BLOCK_SIZE * data.width_blocks - 38, 32 + BLOCK_SIZE * data.height_blocks - 42,
        &image_button_close, 1);
    if (review_briefing_button_should_be_active()) {
        image_buttons_draw(data.x_text + data.text_width_blocks * BLOCK_SIZE - 36, data.y_text - 27, &show_briefing_button, 1);
    }
    draw_delete_read_button(BLOCK_SIZE * data.width_blocks - 61, 30 + BLOCK_SIZE * data.height_blocks - 40,
        data.focus_button_id == 14);
    draw_delete_common_button(43, 30 + BLOCK_SIZE * data.height_blocks - 40,
        data.focus_button_id == 16);
    draw_message_type_button(generic_button_messages_type->x, generic_button_messages_type->y, data.focus_button_id == 17);

    int total_messages = data.message_count;
    if (total_messages > 0) {
        draw_messages(total_messages);
    }

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    unsigned int old_button_id = data.focus_button_id;
    data.focus_button_id = 0;

    if (scrollbar_handle_mouse(&scrollbar, m_dialog, 1)) {
        data.focus_button_id = 13;
        return;
    }

    unsigned int button_id;
    int handled = image_buttons_handle_mouse(m_dialog, 16, 32 + BLOCK_SIZE * data.height_blocks - 42,
        &image_button_help, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 11;
    }
    handled |= image_buttons_handle_mouse(m_dialog, BLOCK_SIZE * data.width_blocks - 38,
        32 + BLOCK_SIZE * data.height_blocks - 42, &image_button_close, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 12;
    }
    handled |= generic_buttons_handle_mouse(m_dialog, BLOCK_SIZE * data.width_blocks - 61,
        30 + BLOCK_SIZE * data.height_blocks - 40, generic_button_delete_read, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 14;
    }

    if (review_briefing_button_should_be_active()) {
        handled |= image_buttons_handle_mouse(m_dialog, data.x_text + data.text_width_blocks * BLOCK_SIZE - 36,
            data.y_text - 27, &show_briefing_button, 1, &button_id);
        if (button_id) {
            data.focus_button_id = 15;
        }
    }
    handled |= generic_buttons_handle_mouse(m_dialog, 43,
        30 + BLOCK_SIZE * data.height_blocks - 40, generic_button_delete_common, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 16;
    }
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0,
    generic_button_messages_type, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 17;
    }

    handled |= generic_buttons_handle_mouse(m_dialog, data.x_text, data.y_text + 4,
        generic_buttons_messages, MAX_MESSAGES, &button_id);
    if (!data.focus_button_id) {
        data.focus_button_id = button_id;
    }
    if (button_id && old_button_id != button_id) {
        window_invalidate();
    }
    if (!handled && input_go_back_requested(m, h)) {
        button_close(0, 0);
    }
}

static void on_scroll(void)
{
    city_message_set_scroll_position(scrollbar.scroll_position);
    window_invalidate();
}

static void button_help(int param1, int param2)
{
    window_message_dialog_show(MESSAGE_DIALOG_MESSAGES, window_city_draw_all);
}

static void button_close(int param1, int param2)
{
    if (data.filtered_message_list) {
        clear_filtered_message_list();
    }
    window_city_show();
}

static void button_message(const generic_button *button)
{
    int index = button->parameter1;
    int id = data.filtered_message_list[city_message_set_current(scrollbar.scroll_position + index)];
    if (id < city_message_count()) {
        const city_message *msg = city_message_get(id);
        city_message_mark_read(id);
        if (msg->message_type != MESSAGE_CUSTOM_MESSAGE) {
            window_message_dialog_show_city_message(
                city_message_get_text_id(msg->message_type),
                msg->year, msg->month, msg->param1, msg->param2,
                city_message_get_advisor(msg->message_type),
                0);
        } else {
            window_message_dialog_show_custom_message(msg->param1, msg->year, msg->month);
        }
    }
}

static void button_delete(const generic_button *button)
{
    int id_to_delete = button->parameter1;
    int id = city_message_set_current(scrollbar.scroll_position + id_to_delete);
    if (id < data.message_count) {
        city_message_delete(data.filtered_message_list[id]);
        select_messages();
        window_invalidate();
    }
}

static void button_delete_all_common(const generic_button *button)
{
    for (int id = 0; id < city_message_count();) {
        const city_message *msg = city_message_get(id);
        if (msg->message_type != MESSAGE_CUSTOM_MESSAGE) {
            city_message_delete(id);
        } else {
            id++;
        }
    }
    select_messages();
    window_invalidate();
}

static void button_delete_all_read(const generic_button *button)
{
    for (int id = 0; id < city_message_count();) {
            const city_message* msg = city_message_get(id);
            if (msg->is_read) {
                city_message_delete(id);
            } else {
                id++;
            }
    }
    select_messages();
    window_invalidate();
}

static void button_mission_briefing(int param1, int param2)
{
    if (game_campaign_is_original() || scenario_intro_message()) {
        window_mission_briefing_show_review();
    }
}

static void get_tooltip(tooltip_context *c)
{
    if (data.focus_button_id == 11) {
        c->text_id = 1;
    } else if (data.focus_button_id == 12) {
        c->text_id = 2;
    } else if (data.focus_button_id == 14) {
        c->translation_key = TR_TOOLTIP_BUTTON_DELETE_READ_MESSAGES;
    } else if (data.focus_button_id == 15) {
        c->text_group = 68;
        c->text_id = 42;
    } else if (data.focus_button_id == 16) {
        c->translation_key = TR_TOOLTIP_BUTTON_DELETE_COMMON_MESSAGES;
    } else {
        return;
    }
    c->type = TOOLTIP_BUTTON;
}

static void button_cycle_message_type(const generic_button *button)
{
    data.type_displayed++;
    if (data.type_displayed > CUSTOM_MESSAGES_ONLY) {
        data.type_displayed = ALL_MESSAGES_SHOWN;
    }
    select_messages();
    window_invalidate();
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
