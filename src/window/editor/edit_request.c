#include "edit_request.h"

#include "core/string.h"
#include "game/resource.h"
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
#include "scenario/property.h"
#include "scenario/request.h"
#include "window/editor/map.h"
#include "window/numeric_input.h"
#include "window/plain_message_dialog.h"
#include "window/select_list.h"

enum {
    AMOUNT_MIN = 0,
    AMOUNT_MAX = 1
};

enum {
    REQUEST_REPEAT_NEVER = 0,
    REQUEST_REPEAT_FOREVER = 1,
    REQUEST_REPEAT_TIMES = 2
};

enum {
    DO_NOT_DISABLE = 0,
    DISABLE_ON_NEW_ITEM = 1,
    DISABLE_ON_NO_REPEAT = 2
};

static void button_year(const generic_button *button);
static void button_amount(const generic_button *button);
static void button_resource(const generic_button *button);
static void button_deadline_years(const generic_button *button);
static void button_favor(const generic_button *button);
static void button_extension_months(const generic_button *button);
static void button_extension_disfavor(const generic_button *button);
static void button_ignored_disfavor(const generic_button *button);
static void button_repeat_type(const generic_button *button);
static void button_repeat_times(const generic_button *button);
static void button_repeat_between(const generic_button *button);
static void button_delete(const generic_button *button);
static void button_cancel(const generic_button *button);
static void button_save(const generic_button *button);

#define BASE_Y_OFFSET 0
#define SECTION_CONTENT_LEFT_OFFSET 60
#define MAX_POSSIBLE_ERRORS 3
#define NUMBER_OF_REQUEST_BUTTONS (sizeof(request_buttons) / sizeof(generic_button))
#define NUMBER_OF_BOTTOM_BUTTONS (sizeof(bottom_buttons) / sizeof(generic_button))

static struct {
    scenario_request request;
    unsigned int focus_button_id;
    unsigned int bottom_button_focus_id;
    int is_new_request;
    int repeat_type;
    resource_type avaialble_resources[RESOURCE_MAX];
    int section_title_width;
    const uint8_t *errors[MAX_POSSIBLE_ERRORS];
} data;

static generic_button request_buttons[] = {
    {0, 48, 60, 25, button_year},
    {0, 82, 100, 25, button_resource},
    {80, 114, 50, 25, button_amount, 0, AMOUNT_MIN},
    {170, 114, 50, 25, button_amount, 0, AMOUNT_MAX},
    {0, 150, 140, 25, button_deadline_years},
    {0, 184, 80, 25, button_favor},
    {0, 218, 80, 25, button_extension_months},
    {0, 252, 80, 25, button_extension_disfavor},
    {0, 286, 80, 25, button_ignored_disfavor},
    {0, 320, 20, 20, button_repeat_type, 0, REQUEST_REPEAT_NEVER},
    {0, 350, 20, 20, button_repeat_type, 0, REQUEST_REPEAT_FOREVER},
    {0, 380, 20, 20, button_repeat_type, 0, REQUEST_REPEAT_TIMES},
    {30, 378, 190, 25, button_repeat_times},
    {80, 408, 50, 25, button_repeat_between, 0, AMOUNT_MIN, DISABLE_ON_NO_REPEAT},
    {170, 408, 50, 25, button_repeat_between, 0, AMOUNT_MAX, DISABLE_ON_NO_REPEAT},
};

static generic_button bottom_buttons[] = {
    {12, 444, 250, 25, button_delete, 0, 0, DISABLE_ON_NEW_ITEM},
    {413, 444, 100, 25, button_cancel},
    {528, 444, 100, 25, button_save},
};

#define NUMBER_OF_SECTIONS (sizeof(sections) / sizeof(sections[0]))

static const struct {
    translation_key title;
    int height;
} sections[] = {
    {TR_EDITOR_REQUEST_DATE, 34 },
    {TR_EDITOR_REQUEST_RESOURCE, 34 },
    {TR_EDITOR_REQUEST_AMOUNT, 34 },
    {TR_EDITOR_REQUEST_DEADLINE, 34 },
    {TR_EDITOR_REQUEST_FAVOR_GAINED, 34 },
    {TR_EDITOR_FAVOUR_EXTENSION_MONTHS, 34 },
    {TR_EDITOR_FAVOUR_DISFAVOR, 34 },
    {TR_EDITOR_FAVOUR_IGNORED, 34 },
    {TR_EDITOR_REPEAT, 90 },
    {TR_EDITOR_REPEAT_FREQUENCY, 34 }
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

static void bound_request_values(void)
{
    if (data.request.repeat.times <= 0) {
        data.repeat_type = data.request.repeat.times == 0 ? REQUEST_REPEAT_NEVER : REQUEST_REPEAT_FOREVER;
        data.request.repeat.times = 1;
    } else {
        data.repeat_type = REQUEST_REPEAT_TIMES;
    }
    if (data.request.repeat.interval.min < 1) {
        data.request.repeat.interval.min = 1;
    }
    if (data.request.repeat.interval.max > 50) {
        data.request.repeat.interval.max = 50;
    }
    if (data.request.repeat.interval.max < data.request.repeat.interval.min) {
        data.request.repeat.interval.max = data.request.repeat.interval.min;
    }
    if (data.request.repeat.interval.min > data.request.repeat.interval.max) {
        data.request.repeat.interval.min = data.request.repeat.interval.max;
    }
}

static void init(int id)
{
    const scenario_request *request = scenario_request_get(id);
    data.is_new_request = request->resource == RESOURCE_NONE;
    data.request = *request;
    bound_request_values();

    data.section_title_width = get_largest_section_title_width();
}

static void draw_background(void)
{
    window_editor_map_draw_all();

    graphics_in_dialog();

    outer_panel_draw(0, BASE_Y_OFFSET, 40, 30);
    lang_text_draw_centered(44, 21, 14, BASE_Y_OFFSET + 14, 580, FONT_LARGE_BLACK);

    int y_offset = BASE_Y_OFFSET + 52;

    // Section names
    for (size_t i = 0; i < NUMBER_OF_SECTIONS; i++) {
        lang_text_draw_right_aligned(CUSTOM_TRANSLATION, sections[i].title, 26, y_offset,
            data.section_title_width, FONT_NORMAL_BLACK);
        y_offset += sections[i].height;
    }

    int x_offset = data.section_title_width + SECTION_CONTENT_LEFT_OFFSET;
    const generic_button *btn;

    // Year text
    btn = &request_buttons[0];
    text_draw_number_centered_prefix(data.request.year, '+', x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width,
        FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + data.request.year, x_offset + btn->x + btn->width + 20,
        BASE_Y_OFFSET + btn->y + 6, FONT_NORMAL_BLACK);

    // Resource type
    btn = &request_buttons[1];
    text_draw_centered(resource_get_data(data.request.resource)->text, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width,
        FONT_NORMAL_BLACK, 0);

    // Amount
    btn = &request_buttons[2];
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_BETWEEN, x_offset, BASE_Y_OFFSET + btn->y + 6,
        FONT_NORMAL_BLACK);
    text_draw_number_centered(data.request.amount.min, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width,
        FONT_NORMAL_BLACK);
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_AND, x_offset + btn->x + btn->width,
        BASE_Y_OFFSET + btn->y + 6, btn[1].x - (btn->x + btn->width), FONT_NORMAL_BLACK);
    btn = &request_buttons[3];
    text_draw_number_centered(data.request.amount.max, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width,
        FONT_NORMAL_BLACK);
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_UNITS, x_offset + btn->x + btn->width + 10,
        BASE_Y_OFFSET + btn->y + 6, FONT_NORMAL_BLACK);

    // Deadline
    btn = &request_buttons[4];
    lang_text_draw_amount_centered(8, 8, data.request.deadline_years, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6,
        btn->width, FONT_NORMAL_BLACK);

    // Favor gained
    btn = &request_buttons[5];
    text_draw_number_centered_prefix(data.request.favor, '+', x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6,
        btn->width, FONT_NORMAL_BLACK);

    // Extension months
    btn = &request_buttons[6];
    text_draw_number_centered_prefix(data.request.extension_months_to_comply, '+', x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6,
        btn->width, FONT_NORMAL_BLACK);

    // Favor lost if extenstion needed
    btn = &request_buttons[7];
    text_draw_number_centered_prefix(data.request.extension_disfavor, '-', x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6,
        btn->width, FONT_NORMAL_BLACK);

    // Favor lost if request ignored
    btn = &request_buttons[8];
    text_draw_number_centered_prefix(data.request.ignored_disfavor, '-', x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6,
        btn->width, FONT_NORMAL_BLACK);

    // Repeat type selected checkbox
    btn = &request_buttons[9 + data.repeat_type];
    text_draw(string_from_ascii("x"), x_offset + btn->x + 6, BASE_Y_OFFSET + btn->y + 3, FONT_NORMAL_BLACK, 0);

    // Never repeat
    btn = &request_buttons[9];
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_DO_NOT_REPEAT, x_offset + btn->x + 30,
        BASE_Y_OFFSET + btn->y + 3, FONT_NORMAL_BLACK);

    // Repeat forever
    btn = &request_buttons[10];
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_FOREVER, x_offset + btn->x + 30,
        BASE_Y_OFFSET + btn->y + 3, FONT_NORMAL_BLACK);

    // Repeat N times
    btn = &request_buttons[12];
    if (data.request.repeat.times == 0 || data.request.repeat.times > 2) {
        text_draw_label_and_number_centered(lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_TEXT),
            data.request.repeat.times,
            (const char *) lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_TIMES),
            x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width, FONT_NORMAL_BLACK, 0);
    } else {
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_ONCE + data.request.repeat.times - 1,
            x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6, btn->width, FONT_NORMAL_BLACK);
    }

    // Invasion interval text
    font_t enabled_font = data.repeat_type == REQUEST_REPEAT_NEVER ? FONT_NORMAL_PLAIN : FONT_NORMAL_BLACK;
    color_t enabled_color = data.repeat_type == REQUEST_REPEAT_NEVER ? COLOR_FONT_LIGHT_GRAY : COLOR_MASK_NONE;

    btn = &request_buttons[13];
    lang_text_draw_colored(CUSTOM_TRANSLATION, TR_EDITOR_BETWEEN, x_offset, BASE_Y_OFFSET + btn->y + 6,
        enabled_font, enabled_color);
    text_draw_number_centered_colored(data.request.repeat.interval.min, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6,
        btn->width, enabled_font, enabled_color);
    lang_text_draw_centered_colored(CUSTOM_TRANSLATION, TR_EDITOR_AND, x_offset + btn->x + btn->width,
        BASE_Y_OFFSET + btn->y + 6, btn[1].x - (btn->x + btn->width), enabled_font, enabled_color);
    btn = &request_buttons[14];
    text_draw_number_centered_colored(data.request.repeat.interval.max, x_offset + btn->x, BASE_Y_OFFSET + btn->y + 6,
        btn->width, enabled_font, enabled_color);
    lang_text_draw_colored(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_FREQUENCY_YEARS, x_offset + btn->x + btn->width + 10,
        BASE_Y_OFFSET + btn->y + 6, enabled_font, enabled_color);


    // Bottom button labels
    lang_text_draw_centered_colored(44, 25, bottom_buttons[0].x, BASE_Y_OFFSET + bottom_buttons[0].y + 7,
        bottom_buttons[0].width, FONT_NORMAL_PLAIN, data.is_new_request ? COLOR_FONT_LIGHT_GRAY : COLOR_RED);
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_BUTTON_CANCEL, bottom_buttons[1].x,
        BASE_Y_OFFSET + bottom_buttons[1].y + 7, bottom_buttons[1].width, FONT_NORMAL_BLACK);
    lang_text_draw_centered(18, 3, bottom_buttons[2].x, BASE_Y_OFFSET + bottom_buttons[2].y + 7,
        bottom_buttons[2].width, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    int x_offset = data.section_title_width + SECTION_CONTENT_LEFT_OFFSET;

    for (size_t i = 0; i < NUMBER_OF_REQUEST_BUTTONS; i++) {
        int focus = data.focus_button_id == i + 1;
        if (request_buttons[i].parameter2 == DISABLE_ON_NO_REPEAT && data.repeat_type == REQUEST_REPEAT_NEVER) {
            focus = 0;
        }
        button_border_draw(x_offset + request_buttons[i].x, request_buttons[i].y + BASE_Y_OFFSET,
            request_buttons[i].width, request_buttons[i].height, focus);
    }

    for (size_t i = 0; i < NUMBER_OF_BOTTOM_BUTTONS; i++) {
        int focus = data.bottom_button_focus_id == i + 1;
        if (data.is_new_request && bottom_buttons[i].parameter2 == DISABLE_ON_NEW_ITEM) {
            focus = 0;
        }
        button_border_draw(bottom_buttons[i].x, bottom_buttons[i].y + BASE_Y_OFFSET,
            bottom_buttons[i].width, bottom_buttons[i].height, focus);
    }

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (generic_buttons_handle_mouse(m_dialog, data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET,
        request_buttons, NUMBER_OF_REQUEST_BUTTONS, &data.focus_button_id) ||
        generic_buttons_handle_mouse(m_dialog, 0, BASE_Y_OFFSET,
            bottom_buttons, NUMBER_OF_BOTTOM_BUTTONS, &data.bottom_button_focus_id)) {
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
    data.request.year = value;
}

static void button_year(const generic_button *button)
{
    window_numeric_input_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button,
        3, 999, set_year);
}

static void set_amount_min(int value)
{
    data.request.amount.min = value;
    if (data.request.amount.max < value) {
        data.request.amount.max = value;
    }
}

static void set_amount_max(int value)
{
    data.request.amount.max = value;
    if (data.request.amount.min > value) {
        data.request.amount.min = value;
    }
}

static void button_amount(const generic_button *button)
{
    int amount_type = button->parameter1;

    int max_amount = 999;
    int max_digits = 3;
    if (data.request.resource == RESOURCE_DENARII) {
        max_amount = 30000;
        max_digits = 5;
    }
    window_numeric_input_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button,
        max_digits, max_amount, amount_type == AMOUNT_MIN ? set_amount_min : set_amount_max);
}

static void set_resource(int value)
{
    data.request.resource = data.avaialble_resources[value];
    if (data.request.resource != RESOURCE_DENARII) {
        if (data.request.amount.min > 999) {
            data.request.amount.min = 999;
        }
        if (data.request.amount.max > 999) {
            data.request.amount.max = 999;
        }
    }
}

static void button_resource(const generic_button *button)
{
    static const uint8_t *resource_texts[RESOURCE_MAX + RESOURCE_TOTAL_SPECIAL];
    static int total_resources = 0;
    if (!total_resources) {
        for (resource_type resource = RESOURCE_NONE; resource < RESOURCE_MAX + RESOURCE_TOTAL_SPECIAL; resource++) {
            if ((!resource_is_storable(resource) && resource < RESOURCE_MAX) || resource == RESOURCE_TROOPS) {
                continue;
            }
            resource_texts[total_resources] = resource_get_data(resource)->text;
            data.avaialble_resources[total_resources] = resource;
            total_resources++;
        }
    }
    window_select_list_show_text(screen_dialog_offset_x() + data.section_title_width + SECTION_CONTENT_LEFT_OFFSET,
        screen_dialog_offset_y() + BASE_Y_OFFSET, button, resource_texts, total_resources, set_resource);
}

static void set_deadline_years(int value)
{
    data.request.deadline_years = value;
}

static void button_deadline_years(const generic_button *button)
{
    window_numeric_input_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button,
        3, 999, set_deadline_years);
}

static void set_favor(int value)
{
    data.request.favor = value;
}

static void button_favor(const generic_button *button)
{
    window_numeric_input_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button,
        3, 100, set_favor);
}

static void set_extension_months(int value)
{
    data.request.extension_months_to_comply = value;
}

static void button_extension_months(const generic_button *button)
{
    window_numeric_input_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button,
        3, 120, set_extension_months);
}

static void set_extension_disfavor(int value)
{
    data.request.extension_disfavor = value;
}

static void button_extension_disfavor(const generic_button *button)
{
    window_numeric_input_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button,
        3, 100, set_extension_disfavor);
}

static void set_ignored_disfavor(int value)
{
    data.request.ignored_disfavor = value;
}

static void button_ignored_disfavor(const generic_button *button)
{
    window_numeric_input_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button,
        3, 100, set_ignored_disfavor);
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
        data.repeat_type = REQUEST_REPEAT_NEVER;
    } else {
        data.repeat_type = REQUEST_REPEAT_TIMES;
        data.request.repeat.times = value;
    }
}

static void button_repeat_times(const generic_button *button)
{
    window_numeric_input_bound_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button,
        3, 1, 999, set_repeat_times);
}

static void set_repeat_interval_min(int value)
{
    data.request.repeat.interval.min = value;
    if (data.request.repeat.interval.max < value) {
        data.request.repeat.interval.max = value;
    }
}

static void set_repeat_interval_max(int value)
{
    data.request.repeat.interval.max = value;
    if (data.request.repeat.interval.min > value) {
        data.request.repeat.interval.min = value;
    }
}

static void button_repeat_between(const generic_button *button)
{
    int amount_type = button->parameter1;
    if (data.repeat_type == REQUEST_REPEAT_NEVER) {
        return;
    }

    window_numeric_input_bound_show(data.section_title_width + SECTION_CONTENT_LEFT_OFFSET, BASE_Y_OFFSET, button,
        2, 1, 50, amount_type == AMOUNT_MIN ? set_repeat_interval_min : set_repeat_interval_max);
}

static void button_delete(const generic_button *button)
{
    if (data.is_new_request) {
        return;
    }
    scenario_request_delete(data.request.id);
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

    if (data.request.resource == RESOURCE_NONE) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_REQUEST_NO_RESOURCE);
    }
    if (data.request.amount.min <= 0) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_REQUEST_NO_AMOUNT);
    }
    if (data.request.deadline_years == 0) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_REQUEST_SET_DEADLINE);
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
    if (data.repeat_type == REQUEST_REPEAT_NEVER) {
        data.request.repeat.times = 0;
    } else if (data.repeat_type == REQUEST_REPEAT_FOREVER) {
        data.request.repeat.times = REQUESTS_REPEAT_INFINITE;
    }
    scenario_request_update(&data.request);
    scenario_editor_set_as_unsaved();
    window_go_back();
}

void window_editor_edit_request_show(int id)
{
    window_type window = {
        WINDOW_EDITOR_EDIT_REQUEST,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(id);
    window_show(&window);
}
