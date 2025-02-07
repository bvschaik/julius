#include "edit_invasion.h"

#include "core/lang.h"
#include "core/string.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/editor.h"
#include "scenario/invasion.h"
#include "scenario/property.h"
#include "scenario/types.h"
#include "window/editor/map.h"
#include "window/numeric_input.h"
#include "window/plain_message_dialog.h"
#include "window/select_list.h"

enum {
    AMOUNT_MIN = 0,
    AMOUNT_MAX = 1
};

enum {
    INVASION_REPEAT_NEVER = 0,
    INVASION_REPEAT_FOREVER = 1,
    INVASION_REPEAT_TIMES = 2
};

enum {
    DO_NOT_DISABLE = 0,
    DISABLE_ON_DISTANT_BATTLE = 1,
    DISABLE_ON_NO_REPEAT = 2
};

#define BASE_Y_OFFSET 20
#define SECTION_CONTENT_LEFT_OFFSET 96
#define MAX_POSSIBLE_ERRORS 3

static void button_year(const generic_button *button);
static void button_amount(const generic_button *button);
static void button_type(const generic_button *button);
static void button_from(const generic_button *button);
static void button_attack(const generic_button *button);
static void button_repeat_type(const generic_button *button);
static void button_repeat_times(const generic_button *button);
static void button_repeat_between(const generic_button *button);
static void button_delete(const generic_button *button);
static void button_cancel(const generic_button *button);
static void button_save(const generic_button *button);

#define NUMBER_OF_EDIT_BUTTONS (sizeof(edit_buttons) / sizeof(generic_button))

static generic_button edit_buttons[] = {
    {0, 52, 60, 25, button_year},
    {80, 90, 50, 25, button_amount, 0, AMOUNT_MIN},
    {170, 90, 50, 25, button_amount, 0, AMOUNT_MAX},
    {0, 128, 220, 25, button_type},
    {0, 166, 220, 25, button_from, 0, 0, DISABLE_ON_DISTANT_BATTLE},
    {0, 204, 220, 25, button_attack, 0, 0, DISABLE_ON_DISTANT_BATTLE},
    {0, 244, 220, 20, button_repeat_type, 0, INVASION_REPEAT_NEVER},
    {0, 274, 220, 20, button_repeat_type, 0, INVASION_REPEAT_FOREVER},
    {0, 304, 20, 20, button_repeat_type, 0, INVASION_REPEAT_TIMES},
    {30, 302, 190, 25, button_repeat_times},
    {80, 340, 50, 25, button_repeat_between, 0, AMOUNT_MIN, DISABLE_ON_NO_REPEAT},
    {170, 340, 50, 25, button_repeat_between, 0, AMOUNT_MAX, DISABLE_ON_NO_REPEAT},
};

#define NUMBER_OF_BOTTOM_BUTTONS (sizeof(bottom_buttons) / sizeof(generic_button))

static generic_button bottom_buttons[] = {
    {32, 378, 250, 25, button_delete},
    {393, 378, 100, 25, button_cancel},
    {508, 378, 100, 25, button_save},
};

static struct {
    invasion_t invasion;
    unsigned int focus_button_id;
    unsigned int bottom_button_focus_id;
    int section_title_width;
    int is_new_invasion;
    int repeat_type;
    const uint8_t *errors[MAX_POSSIBLE_ERRORS];
} data;

#define NUMBER_OF_SECTIONS (sizeof(sections) / sizeof(sections[0]))

static const struct {
    translation_key title;
    int height;
} sections[] = {
    { TR_EDITOR_INVASION_DATE, 38 },
    { TR_EDITOR_INVASION_SIZE, 38 },
    { TR_EDITOR_INVASION_ENEMY_TYPE, 38 },
    { TR_EDITOR_INVASION_FROM, 38 },
    { TR_EDITOR_INVASION_ATTACK_PRIORITY, 38 },
    { TR_EDITOR_REPEAT, 98 },
    { TR_EDITOR_REPEAT_FREQUENCY, 38 }
};

static int get_largest_section_title_width(void)
{
    int largest_width = 0;
    for (size_t i = 0; i < NUMBER_OF_SECTIONS; i++) {
        int width = lang_text_get_width(CUSTOM_TRANSLATION, sections[i].title, FONT_NORMAL_BLACK);
        if (width > largest_width) {
            largest_width = width;
        }
    }
    return largest_width;
}

static void bound_invasion_values(void)
{
    if (data.invasion.repeat.times <= 0) {
        data.repeat_type = data.invasion.repeat.times == 0 ? INVASION_REPEAT_NEVER : INVASION_REPEAT_FOREVER;
        data.invasion.repeat.times = 1;
    } else {
        data.repeat_type = INVASION_REPEAT_TIMES;
    }
    if (data.invasion.repeat.interval.min < 1) {
        data.invasion.repeat.interval.min = 1;
    }
    if (data.invasion.repeat.interval.max > 50) {
        data.invasion.repeat.interval.max = 50;
    }
    if (data.invasion.repeat.interval.max < data.invasion.repeat.interval.min) {
        data.invasion.repeat.interval.max = data.invasion.repeat.interval.min;
    }
    if (data.invasion.repeat.interval.min > data.invasion.repeat.interval.max) {
        data.invasion.repeat.interval.min = data.invasion.repeat.interval.max;
    }
}

static void init(int id)
{
    const invasion_t *invasion = scenario_invasion_get(id);
    data.is_new_invasion = invasion->type == INVASION_TYPE_NONE;
    data.invasion = *invasion;
    bound_invasion_values();

    data.section_title_width = get_largest_section_title_width();
}

static void draw_background(void)
{
    window_editor_map_draw_all();

    graphics_in_dialog();

    outer_panel_draw(16, BASE_Y_OFFSET, 38, 26);
    lang_text_draw_centered(44, 22, 16, BASE_Y_OFFSET + 14, 608, FONT_LARGE_BLACK);

    int y_offset = BASE_Y_OFFSET + 58;

    // Section names
    for (size_t i = 0; i < NUMBER_OF_SECTIONS; i++) {
        lang_text_draw_right_aligned(CUSTOM_TRANSLATION, sections[i].title, 56, y_offset,
            data.section_title_width, FONT_NORMAL_BLACK);
        y_offset += sections[i].height;
    }

    int x_offset = data.section_title_width + SECTION_CONTENT_LEFT_OFFSET;
    const generic_button *btn;

    // Year text
    btn = &edit_buttons[0];
    text_draw_number_centered_prefix(data.invasion.year, '+', x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width,
        FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + data.invasion.year, x_offset + btn->x + btn->width + 20,
        BASE_Y_OFFSET + btn->y + 6, FONT_NORMAL_BLACK);

    // Invasion size text
    btn = &edit_buttons[1];
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_BETWEEN, x_offset, BASE_Y_OFFSET + btn->y + 6,
        FONT_NORMAL_BLACK);
    text_draw_number_centered(data.invasion.amount.min, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width,
        FONT_NORMAL_BLACK);
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_AND, x_offset + btn->x + btn->width,
        BASE_Y_OFFSET + btn->y + 6, btn[1].x - (btn->x + btn->width), FONT_NORMAL_BLACK);
    btn = &edit_buttons[2];
    text_draw_number_centered(data.invasion.amount.max, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width,
        FONT_NORMAL_BLACK);
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_UNITS, x_offset + btn->x + btn->width + 10,
        BASE_Y_OFFSET + btn->y + 6, FONT_NORMAL_BLACK);

    // Invasion type text
    btn = &edit_buttons[3];
    lang_text_draw_centered(34, data.invasion.type, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width,
        FONT_NORMAL_BLACK);

    font_t enabled_font = data.invasion.type == INVASION_TYPE_DISTANT_BATTLE ? FONT_NORMAL_PLAIN : FONT_NORMAL_BLACK;
    color_t enabled_color = data.invasion.type == INVASION_TYPE_DISTANT_BATTLE ? COLOR_FONT_LIGHT_GRAY : COLOR_MASK_NONE;

    // Invasion from text
    btn = &edit_buttons[4];
    lang_text_draw_centered_colored(35, data.invasion.from, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width,
        enabled_font, enabled_color);

    // Invasion priority text
    btn = &edit_buttons[5];
    lang_text_draw_centered_colored(36, data.invasion.attack_type, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6,
        btn->width, enabled_font, enabled_color);

    // Repeat type selected checkbox
    btn = &edit_buttons[6 + data.repeat_type];
    text_draw(string_from_ascii("x"), x_offset + btn->x + 6, BASE_Y_OFFSET + btn->y + 3, FONT_NORMAL_BLACK, 0);

    // Never repeat
    btn = &edit_buttons[6];
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_DO_NOT_REPEAT, x_offset + btn->x + 30,
        BASE_Y_OFFSET + btn->y + 3, FONT_NORMAL_BLACK);

    // Repeat forever
    btn = &edit_buttons[7];
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_FOREVER, x_offset + btn->x + 30,
        BASE_Y_OFFSET + btn->y + 3, FONT_NORMAL_BLACK);

    // Repeat N times
    btn = &edit_buttons[9];
    if (data.invasion.repeat.times == 0 || data.invasion.repeat.times > 2) {
        text_draw_label_and_number_centered(lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_TEXT),
            data.invasion.repeat.times,
            (const char *) lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_TIMES),
            x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width, FONT_NORMAL_BLACK, 0);
    } else {
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_ONCE + data.invasion.repeat.times - 1,
            x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width, FONT_NORMAL_BLACK);
    }

    // Invasion interval text
    enabled_font = data.repeat_type == INVASION_REPEAT_NEVER ? FONT_NORMAL_PLAIN : FONT_NORMAL_BLACK;
    enabled_color = data.repeat_type == INVASION_REPEAT_NEVER ? COLOR_FONT_LIGHT_GRAY : COLOR_MASK_NONE;

    btn = &edit_buttons[10];
    lang_text_draw_colored(CUSTOM_TRANSLATION, TR_EDITOR_BETWEEN, x_offset, BASE_Y_OFFSET + btn->y + 6,
        enabled_font, enabled_color);
    text_draw_number_centered_colored(data.invasion.repeat.interval.min, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6,
        btn->width, enabled_font, enabled_color);
    lang_text_draw_centered_colored(CUSTOM_TRANSLATION, TR_EDITOR_AND, x_offset + btn->x + btn->width,
        BASE_Y_OFFSET + btn->y + 6, btn[1].x - (btn->x + btn->width), enabled_font, enabled_color);
    btn = &edit_buttons[11];
    text_draw_number_centered_colored(data.invasion.repeat.interval.max, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6,
        btn->width, enabled_font, enabled_color);
    lang_text_draw_colored(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_FREQUENCY_YEARS, x_offset + btn->x + btn->width + 10,
        BASE_Y_OFFSET + btn->y + 6, enabled_font, enabled_color);

    // Bottom button labels
    lang_text_draw_centered_colored(44, 26, bottom_buttons[0].x, BASE_Y_OFFSET + bottom_buttons[0].y + 6,
        bottom_buttons[0].width, FONT_NORMAL_PLAIN, data.is_new_invasion ? COLOR_FONT_LIGHT_GRAY : COLOR_RED);
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_BUTTON_CANCEL, bottom_buttons[1].x, BASE_Y_OFFSET + bottom_buttons[1].y + 6, bottom_buttons[1].width,
        FONT_NORMAL_BLACK);
    lang_text_draw_centered(18, 3, bottom_buttons[2].x, BASE_Y_OFFSET + bottom_buttons[2].y + 6, bottom_buttons[2].width,
        FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    int x_offset = data.section_title_width + SECTION_CONTENT_LEFT_OFFSET;

    for (size_t i = 0; i < NUMBER_OF_EDIT_BUTTONS; i++) {
        int focus = data.focus_button_id == i + 1;
        if ((edit_buttons[i].parameter2 == DISABLE_ON_DISTANT_BATTLE &&
                data.invasion.type == INVASION_TYPE_DISTANT_BATTLE) ||
            (edit_buttons[i].parameter2 == DISABLE_ON_NO_REPEAT && data.repeat_type == INVASION_REPEAT_NEVER)) {
            focus = 0;
        }
        int width = i == 6 || i == 7 ? 20 : edit_buttons[i].width;
        button_border_draw(x_offset + edit_buttons[i].x, BASE_Y_OFFSET + edit_buttons[i].y, width,
            edit_buttons[i].height, focus);
    }

    for (size_t i = 0; i < NUMBER_OF_BOTTOM_BUTTONS; i++) {
        int focus = data.bottom_button_focus_id == i + 1;
        if (i == 0 && data.is_new_invasion) {
            focus = 0;
        }
        button_border_draw(bottom_buttons[i].x, BASE_Y_OFFSET + bottom_buttons[i].y, bottom_buttons[i].width,
            bottom_buttons[i].height, focus);
    }

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (generic_buttons_handle_mouse(m_dialog, data.section_title_width + SECTION_CONTENT_LEFT_OFFSET,
            BASE_Y_OFFSET, edit_buttons, NUMBER_OF_EDIT_BUTTONS, &data.focus_button_id) ||
        generic_buttons_handle_mouse(m_dialog, 0, BASE_Y_OFFSET, bottom_buttons, NUMBER_OF_BOTTOM_BUTTONS,
            &data.bottom_button_focus_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        button_cancel(0);
        return;
    }
    if (h->enter_pressed) {
        button_save(0);
    }
}

static void set_year(int value)
{
    data.invasion.year = value;
}

static void button_year(const generic_button *button)
{
    window_numeric_input_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button,
        3, 999, set_year);
}

static void set_amount_min(int value)
{
    data.invasion.amount.min = value;
    if (data.invasion.amount.max < value) {
        data.invasion.amount.max = value;
    }
}

static void set_amount_max(int value)
{
    data.invasion.amount.max = value;
    if (data.invasion.amount.min > value) {
        data.invasion.amount.min = value;
    }
}

static void button_amount(const generic_button *button)
{
    int amount_type = button->parameter1;

    window_numeric_input_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button, 3, 200,
        amount_type == AMOUNT_MIN ? set_amount_min : set_amount_max);
}

static void set_type(int value)
{
    data.invasion.type = value == 3 ? 4 : value;
}

static void button_type(const generic_button *button)
{
    int x_offset = screen_dialog_offset_x() + data.section_title_width + SECTION_CONTENT_LEFT_OFFSET;
    int y_offset = screen_dialog_offset_y() + BASE_Y_OFFSET;

    window_select_list_show(x_offset, y_offset, button, 34, 4, set_type);
}

static void set_from(int value)
{
    data.invasion.from = value;
}

static void button_from(const generic_button *button)
{
    if (data.invasion.type == INVASION_TYPE_DISTANT_BATTLE) {
        return;
    }
    int x_offset = screen_dialog_offset_x() + data.section_title_width + SECTION_CONTENT_LEFT_OFFSET;
    int y_offset = screen_dialog_offset_y() + BASE_Y_OFFSET;

    window_select_list_show(x_offset, y_offset, button, 35, 9, set_from);
}

static void set_attack(int value)
{
    data.invasion.attack_type = value;
}

static void button_attack(const generic_button *button)
{
    if (data.invasion.type == INVASION_TYPE_DISTANT_BATTLE) {
        return;
    }
    int x_offset = screen_dialog_offset_x() + data.section_title_width + SECTION_CONTENT_LEFT_OFFSET;
    int y_offset = screen_dialog_offset_y() + BASE_Y_OFFSET;

    window_select_list_show(x_offset, y_offset, button, 36, 5, set_attack);
}

static void button_repeat_type(const generic_button *button)
{
    int repeat_type = button->parameter1;
    if (data.repeat_type == repeat_type) {
        return;
    }
    data.repeat_type = repeat_type;
    window_request_refresh();
}

static void set_repeat_times(int value)
{
    if (value == 0) {
        data.repeat_type = INVASION_REPEAT_NEVER;
    } else {
        data.repeat_type = INVASION_REPEAT_TIMES;
        data.invasion.repeat.times = value;
    }
}

static void button_repeat_times(const generic_button *button)
{
    window_numeric_input_bound_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button,
        3, 1, 999, set_repeat_times);
}

static void set_repeat_interval_min(int value)
{
    data.invasion.repeat.interval.min = value;
    if (data.invasion.repeat.interval.max < value) {
        data.invasion.repeat.interval.max = value;
    }
}

static void set_repeat_interval_max(int value)
{
    data.invasion.repeat.interval.max = value;
    if (data.invasion.repeat.interval.min > value) {
        data.invasion.repeat.interval.min = value;
    }
}

static void button_repeat_between(const generic_button *button)
{
    int amount_type = button->parameter1;
    if (data.repeat_type == INVASION_REPEAT_NEVER) {
        return;
    }

    window_numeric_input_bound_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button,
        2, 1, 50, amount_type == AMOUNT_MIN ? set_repeat_interval_min : set_repeat_interval_max);
}

static void button_delete(const generic_button *button)
{
    if (data.is_new_invasion) {
        return;
    }
    scenario_invasion_delete(data.invasion.id);
    scenario_editor_set_as_unsaved();
    window_go_back();
}

static void button_cancel(const generic_button *button)
{
    window_go_back();
}

static unsigned int validate(void)
{
    unsigned int num_errors = 0;

    for (int i = 0; i < MAX_POSSIBLE_ERRORS; i++) {
        data.errors[i] = 0;
    }

    if (data.invasion.type == INVASION_TYPE_NONE) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_INVASION_NO_TYPE);
    }
    if (data.invasion.amount.min <= 0) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_INVASION_NO_SOLDIERS);
    }
    if (data.invasion.year == 0) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_INVASION_SET_YEAR);
    }

    return num_errors;
}

static void button_save(const generic_button *button)
{
    unsigned int num_errors = validate();
    if (num_errors) {
        window_plain_message_dialog_show_text_list(TR_EDITOR_FORM_ERRORS_FOUND, TR_EDITOR_FORM_HAS_FOLLOWING_ERRORS,
            data.errors, num_errors);
        return;
    }

    if (data.repeat_type == INVASION_REPEAT_NEVER) {
        data.invasion.repeat.times = 0;
    } else if (data.repeat_type == INVASION_REPEAT_FOREVER) {
        data.invasion.repeat.times = INVASIONS_REPEAT_INFINITE;
    }
    scenario_invasion_update(&data.invasion);
    scenario_editor_set_as_unsaved();
    window_go_back();
}

void window_editor_edit_invasion_show(int id)
{
    window_type window = {
        WINDOW_EDITOR_EDIT_INVASION,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(id);
    window_show(&window);
}
