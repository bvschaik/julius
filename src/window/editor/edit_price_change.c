#include "edit_price_change.h"

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
#include "scenario/price_change.h"
#include "scenario/property.h"
#include "window/editor/map.h"
#include "window/numeric_input.h"
#include "window/plain_message_dialog.h"
#include "window/select_list.h"

static void button_year(const generic_button *button);
static void button_resource(const generic_button *button);
static void button_toggle_rise(const generic_button *button);
static void button_amount(const generic_button *button);
static void button_delete(const generic_button *button);
static void button_cancel(const generic_button *button);
static void button_save(const generic_button *button);

#define NUM_BUTTONS (sizeof(buttons) / sizeof(generic_button))

static generic_button buttons[] = {
    {30, 152, 60, 25, button_year},
    {240, 152, 120, 25, button_resource},
    {100, 192, 200, 25, button_toggle_rise},
    {350, 192, 100, 25, button_amount},
    {16, 238, 250, 25, button_delete},
    {379, 238, 100, 25, button_cancel},
    {492, 238, 100, 25, button_save}
};

#define MAX_POSSIBLE_ERRORS 3

static struct {
    price_change_t price_change;
    unsigned int focus_button_id;
    int is_new_price_change;
    const uint8_t *errors[MAX_POSSIBLE_ERRORS];
    resource_type available_resources[RESOURCE_MAX];
} data;

static void init(int id)
{
    const price_change_t *price_change = scenario_price_change_get(id);
    data.is_new_price_change = price_change->resource == RESOURCE_NONE;
    data.price_change = *price_change;
}

static void draw_background(void)
{
    window_editor_map_draw_all();

    graphics_in_dialog();

    outer_panel_draw(0, 100, 38, 11);
    lang_text_draw(44, 95, 20, 114, FONT_LARGE_BLACK);

    text_draw_number_centered_prefix(data.price_change.year, '+', 30, 158, 60, FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + data.price_change.year, 100, 158, FONT_NORMAL_BLACK);

    text_draw_centered(resource_get_data(data.price_change.resource)->text, 240, 158, 120, FONT_NORMAL_BLACK,
        COLOR_MASK_NONE);

    lang_text_draw_centered(44, data.price_change.is_rise ? 104 : 103, 100, 198, 200, FONT_NORMAL_BLACK);

    text_draw_number_centered(data.price_change.amount, 350, 198, 100, FONT_NORMAL_BLACK);

    lang_text_draw_centered_colored(44, 105, 16, 244, 250, FONT_NORMAL_PLAIN,
        data.is_new_price_change ? COLOR_FONT_LIGHT_GRAY : COLOR_RED);

    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_BUTTON_CANCEL, 379, 244, 100, FONT_NORMAL_BLACK);

    lang_text_draw_centered(18, 3, 492, 244, 100, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    for (size_t i = 0; i < NUM_BUTTONS; i++) {
        int focus = data.focus_button_id == i + 1;
        if (i == 4 && data.is_new_price_change) {
            focus = 0;
        }
        button_border_draw(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, focus);
    }

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, NUM_BUTTONS, &data.focus_button_id)) {
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
    data.price_change.year = value;
}

static void button_year(const generic_button *button)
{
    window_numeric_input_show(0, 0, button, 3, 999, set_year);
}

static void set_resource(int value)
{
    data.price_change.resource = data.available_resources[value];
}

static void button_resource(const generic_button *button)
{
    static const uint8_t *resource_texts[RESOURCE_MAX];
    static int total_resources = 0;
    if (!total_resources) {
        for (resource_type resource = RESOURCE_NONE; resource < RESOURCE_MAX; resource++) {
            if (!resource_is_storable(resource)) {
                continue;
            }
            resource_texts[total_resources] = resource_get_data(resource)->text;
            data.available_resources[total_resources] = resource;
            total_resources++;
        }
    }
    window_select_list_show_text(screen_dialog_offset_x(), screen_dialog_offset_y(), button,
        resource_texts, total_resources, set_resource);
}

static void button_toggle_rise(const generic_button *button)
{
    data.price_change.is_rise = !data.price_change.is_rise;
    window_request_refresh();
}

static void set_amount(int value)
{
    data.price_change.amount = value;
}

static void button_amount(const generic_button *button)
{
    window_numeric_input_show(0, 0, button, 2, 99, set_amount);
}

static void button_delete(const generic_button *button)
{
    if (data.is_new_price_change) {
        return;
    }
    scenario_price_change_delete(data.price_change.id);
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

    if (data.price_change.resource == RESOURCE_NONE) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_REQUEST_NO_RESOURCE);
    }
    if (data.price_change.amount <= 0) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_REQUEST_NO_AMOUNT);
    }
    if (data.price_change.year == 0) {
        data.errors[num_errors++] = translation_for(TR_EDITOR_EDIT_DEMAND_CHANGE_NO_YEAR);
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
    scenario_price_change_update(&data.price_change);
    scenario_editor_set_as_unsaved();
    window_go_back();
}

void window_editor_edit_price_change_show(int id)
{
    window_type window = {
        WINDOW_EDITOR_EDIT_PRICE_CHANGE,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(id);
    window_show(&window);
}
