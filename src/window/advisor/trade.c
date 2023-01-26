#include "trade.h"

#include "assets/assets.h"
#include "building/caravanserai.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/resource.h"
#include "city/trade_policy.h"
#include "core/lang.h"
#include "core/string.h"
#include "empire/city.h"
#include "game/resource.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h" 
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "sound/speech.h"
#include "translation/translation.h"
#include "window/empire.h"
#include "window/option_popup.h"
#include "window/resource_settings.h"
#include "window/trade_prices.h"

#define ADVISOR_HEIGHT 27
#define NUMBER_WIDTH 20
#define RESOURCE_Y_OFFSET 54
#define RESOURCE_ROW_HEIGHT 41
#define MAX_VISIBLE_ROWS 8

static void on_scroll(void);

static void button_prices(int param1, int param2);
static void button_empire(int param1, int param2);
static void button_policy(int param1, int param2);
static void button_resource(int resource_index, int param2);

static scrollbar_type scrollbar = {
    580, RESOURCE_Y_OFFSET, RESOURCE_ROW_HEIGHT * MAX_VISIBLE_ROWS, 560, MAX_VISIBLE_ROWS, on_scroll, 0, 4
};

static generic_button resource_buttons[] = {
    {375, 392, 200, 24, button_prices, button_none, 1, 0},
    {160, 392, 200, 24, button_empire, button_none, 1, 0},
    {45, 390, 40, 30, button_policy, button_none, LAND_TRADE_POLICY, 0},
    {95, 390, 40, 30, button_policy, button_none, SEA_TRADE_POLICY, 0},
    {64, 56, 480, RESOURCE_ROW_HEIGHT - 2, button_resource, button_none, 0, 0},
    {64, 97, 480, RESOURCE_ROW_HEIGHT - 2, button_resource, button_none, 1, 0},
    {64, 138, 480, RESOURCE_ROW_HEIGHT - 2, button_resource, button_none, 2, 0},
    {64, 179, 480, RESOURCE_ROW_HEIGHT - 2, button_resource, button_none, 3, 0},
    {64, 220, 480, RESOURCE_ROW_HEIGHT - 2, button_resource, button_none, 4, 0},
    {64, 261, 480, RESOURCE_ROW_HEIGHT - 2, button_resource, button_none, 5, 0},
    {64, 302, 480, RESOURCE_ROW_HEIGHT - 2, button_resource, button_none, 6, 0},
    {64, 343, 480, RESOURCE_ROW_HEIGHT - 2, button_resource, button_none, 7, 0}
};

static struct {
    int title;
    int subtitle;
    const char *base_image_name;
    option_menu_item items[3];
    const char *wav_file;
} policy_options[] = {
    {
        TR_BUILDING_CARAVANSERAI_POLICY_TITLE,
        TR_BUILDING_CARAVANSERAI_POLICY_TEXT,
        "Trade Policy",
        {
            { TR_BUILDING_CARAVANSERAI_POLICY_1_TITLE, TR_BUILDING_CARAVANSERAI_POLICY_1 },
            { TR_BUILDING_CARAVANSERAI_POLICY_2_TITLE, TR_BUILDING_CARAVANSERAI_POLICY_2 },
            { TR_BUILDING_CARAVANSERAI_POLICY_3_TITLE, TR_BUILDING_CARAVANSERAI_POLICY_3 }
        },
        "wavs/market4.wav"
    },
    {
        TR_BUILDING_LIGHTHOUSE_POLICY_TITLE,
        TR_BUILDING_LIGHTHOUSE_POLICY_TEXT,
        "Sea Trade Policy",
        {
            { TR_BUILDING_LIGHTHOUSE_POLICY_1_TITLE, TR_BUILDING_LIGHTHOUSE_POLICY_1 },
            { TR_BUILDING_LIGHTHOUSE_POLICY_2_TITLE, TR_BUILDING_LIGHTHOUSE_POLICY_2 },
            { TR_BUILDING_LIGHTHOUSE_POLICY_3_TITLE, TR_BUILDING_LIGHTHOUSE_POLICY_3 }
        },
        "wavs/dock1.wav"
    }
};

static struct {
    int focus_button_id;
    const resource_list *list;
    int margin_right;
    trade_policy_type policy_type;
} data;

static void init(void)
{
    city_resource_determine_available();
    data.list = city_resource_get_potential();
    scrollbar_init(&scrollbar, 0, data.list->size);
    if (data.list->size > MAX_VISIBLE_ROWS) {
        data.margin_right = 48;
    }
}

static void draw_resource_status_text(int resource, int x, int y, int box_width)
{
    int trade_flags_potential = TRADE_STATUS_NONE;
    if (empire_can_import_resource_potentially(resource)) {
        trade_flags_potential |= TRADE_STATUS_IMPORT;
    }
    if (empire_can_export_resource_potentially(resource)) {
        trade_flags_potential |= TRADE_STATUS_EXPORT;
    }

    if (trade_flags_potential == TRADE_STATUS_NONE) {
        if (city_resource_is_stockpiled(resource)) {
            lang_text_draw_centered(54, 3, x, y + 10, box_width, FONT_NORMAL_RED);
        }
        return;
    }

    int trade_flags = TRADE_STATUS_NONE;
    if (empire_can_import_resource(resource)) {
        trade_flags |= TRADE_STATUS_IMPORT;
    }
    if (empire_can_export_resource(resource)) {
        trade_flags |= TRADE_STATUS_EXPORT;
    }
    resource_trade_status trade_status = city_resource_trade_status(resource);

    int two_lines = trade_flags_potential == TRADE_STATUS_IMPORT_EXPORT ||
        (trade_flags_potential & TRADE_STATUS_IMPORT && city_resource_is_stockpiled(resource));

    if (!two_lines) {
        y += 10;
    }

    if (trade_status & TRADE_STATUS_IMPORT) {
        int width = (box_width - 20 - lang_text_get_width(54, 5, FONT_NORMAL_WHITE)) / 2;
        width += lang_text_draw(54, 5, x + width, y, FONT_NORMAL_WHITE);
        int import_limit = city_resource_import_over(resource);
        if (import_limit > 0) {
            text_draw_number(import_limit, 0, " ", x + width, y, FONT_NORMAL_WHITE, 0);
        } else {
            text_draw(translation_for(TR_ADVISOR_TRADE_MAX), x + width, y, FONT_NORMAL_WHITE, 0);
        }
    } else if (trade_flags & TRADE_STATUS_IMPORT) {
        text_draw_centered(translation_for(TR_ADVISOR_TRADE_IMPORTABLE), x, y, box_width, FONT_NORMAL_GREEN, 0);
    } else if (trade_flags_potential & TRADE_STATUS_IMPORT) {
        text_draw_centered(translation_for(TR_ADVISOR_OPEN_TO_IMPORT), x, y, box_width, FONT_NORMAL_GREEN, 0);
    }

    if (two_lines) {
        y += 20;
    }

    if (city_resource_is_stockpiled(resource)) {
        lang_text_draw_centered(54, 3, x, y, box_width, FONT_NORMAL_RED);
    } else if (trade_status & TRADE_STATUS_EXPORT) {
        int width = (box_width - 15 - lang_text_get_width(54, 6, FONT_NORMAL_WHITE)) / 2;
        width += lang_text_draw(54, 6, x + width, y, FONT_NORMAL_WHITE);
        text_draw_number(city_resource_export_over(resource), 0, " ", x + width, y, FONT_NORMAL_WHITE, 0);
    } else if (trade_flags & TRADE_STATUS_EXPORT) {
        text_draw_centered(translation_for(TR_ADVISOR_TRADE_EXPORTABLE), x, y, box_width, FONT_NORMAL_GREEN, 0);
    } else if (trade_flags_potential & TRADE_STATUS_EXPORT) {
        text_draw_centered(translation_for(TR_ADVISOR_OPEN_TO_EXPORT), x, y, box_width, FONT_NORMAL_GREEN, 0);
    }
}

static int draw_background(void)
{
    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 4, 10, 10, COLOR_MASK_NONE, SCALE_NONE);

    lang_text_draw(54, 0, 60, 12, FONT_LARGE_BLACK);
    int width = lang_text_get_width(54, 1, FONT_NORMAL_BLACK);
    lang_text_draw(54, 1, 600 - width, 38, FONT_NORMAL_BLACK);

    button_border_draw(375, 392, 200, 24, data.focus_button_id == 1);
    lang_text_draw_centered(54, 2, 375, 398, 200, FONT_NORMAL_BLACK);

    button_border_draw(160, 392, 200, 24, data.focus_button_id == 2);
    lang_text_draw_centered(54, 30, 160, 398, 200, FONT_NORMAL_BLACK);

    if (data.list->size > MAX_VISIBLE_ROWS) {
        inner_panel_draw(scrollbar.x + 4, scrollbar.y + 28, 2, scrollbar.height / BLOCK_SIZE - 3);
    }

    int land_policy_available = building_monument_working(BUILDING_CARAVANSERAI);
    int sea_policy_available = building_monument_working(BUILDING_LIGHTHOUSE);

    button_border_draw(45, 390, 40, 30, land_policy_available && data.focus_button_id == 3);
    int image_id;
    
    if (land_policy_available) {
        image_id = image_group(GROUP_EMPIRE_TRADE_ROUTE_TYPE) + 1;
    } else {
        image_id = assets_get_image_id("UI", "Land Trade Policy Off Button");
    }
    image_draw(image_id, 51, 394, COLOR_MASK_NONE, SCALE_NONE);

    button_border_draw(95, 390, 40, 30, sea_policy_available && data.focus_button_id == 4);

    if (sea_policy_available) {
        image_id = image_group(GROUP_EMPIRE_TRADE_ROUTE_TYPE);
    } else {
        image_id = assets_get_image_id("UI", "Sea Trade Policy Off Button");
    }
    image_draw(image_id, 99, 394, COLOR_MASK_NONE, SCALE_NONE);

    return ADVISOR_HEIGHT;
}

static void draw_foreground(void)
{
    inner_panel_draw(16, RESOURCE_Y_OFFSET - 2, 38 - data.margin_right / BLOCK_SIZE, 21);

    int y_offset = RESOURCE_Y_OFFSET;
    for (int i = 0; i < data.list->size && i < MAX_VISIBLE_ROWS; i++) {
        int resource = data.list->items[i + scrollbar.scroll_position];
        int image_id = resource_get_data(resource)->image.icon;
        const image *img = image_get(image_id);
        int base_y = (RESOURCE_ROW_HEIGHT - img->height) / 2;
        image_draw(image_id, 32, y_offset + base_y, COLOR_MASK_NONE, SCALE_NONE);
        image_draw(image_id, 584 - data.margin_right, y_offset + base_y, COLOR_MASK_NONE, SCALE_NONE);

        if (data.focus_button_id - 5 == i) {
            button_border_draw(64, y_offset, 512 - data.margin_right, RESOURCE_ROW_HEIGHT, 1);
        }
        text_draw(resource_get_data(resource)->text, 72, y_offset + 17, FONT_NORMAL_WHITE, COLOR_MASK_NONE);
        int amount = city_resource_count(resource);
        if (resource_is_food(resource)) {
            amount += city_resource_count_food_on_granaries(resource) / 100;
        }
        text_draw_number_centered(amount, 164, y_offset + 17, 60, FONT_NORMAL_WHITE);
        if (city_resource_is_mothballed(resource)) {
            lang_text_draw_centered(18, 5, 204, y_offset + 17, 100, FONT_NORMAL_WHITE);
        }
        draw_resource_status_text(resource, 240, y_offset + 7, 352 - data.margin_right);

        y_offset += RESOURCE_ROW_HEIGHT;

        if (i < MAX_VISIBLE_ROWS - 1) {
            graphics_draw_inset_rect(24, y_offset, 592 - data.margin_right, 2);
        }
    }

    button_border_draw(375, 392, 200, 24, data.focus_button_id == 1);
    button_border_draw(160, 392, 200, 24, data.focus_button_id == 2);

    int land_policy_available = building_monument_working(BUILDING_CARAVANSERAI);
    int sea_policy_available = building_monument_working(BUILDING_LIGHTHOUSE);
    button_border_draw(45, 390, 40, 30, land_policy_available && data.focus_button_id == 3);
    button_border_draw(95, 390, 40, 30, sea_policy_available && data.focus_button_id == 4);

    if (data.list->size > MAX_VISIBLE_ROWS) {
        scrollbar_draw(&scrollbar);
    }
}

static void on_scroll(void)
{
    window_request_refresh();
}

static int handle_mouse(const mouse *m)
{
    data.focus_button_id = 0;
    return scrollbar_handle_mouse(&scrollbar, m, 1) ||
        generic_buttons_handle_mouse(m, 0, 0, resource_buttons, MAX_VISIBLE_ROWS + 4, &data.focus_button_id);
}

static void apply_policy(int selected_policy)
{
    if (selected_policy == NO_POLICY) {
        return;
    }
    city_trade_policy_set(data.policy_type, selected_policy);
    sound_speech_play_file(policy_options[data.policy_type].wav_file);
    city_finance_process_sundry(TRADE_POLICY_COST);
}

static void show_policy(trade_policy_type policy_type)
{
    data.policy_type = policy_type;
    if (!policy_options[policy_type].items[0].image_id) {
        int base_policy_image = assets_get_image_id("UI",
            policy_options[policy_type].base_image_name);
        policy_options[policy_type].items[0].image_id = base_policy_image + 1;
        policy_options[policy_type].items[1].image_id = base_policy_image + 2;
        policy_options[policy_type].items[2].image_id = base_policy_image + 3;
    }
    window_option_popup_show(policy_options[policy_type].title, policy_options[policy_type].subtitle,
        policy_options[policy_type].items, 3, apply_policy, city_trade_policy_get(policy_type),
        TRADE_POLICY_COST, OPTION_MENU_SMALL_ROW);
}

static void button_prices(int param1, int param2)
{
    window_trade_prices_show(17, 53, 622, 334);
}

static void button_empire(int param1, int param2)
{
    window_empire_show();
}

static void button_policy(int policy_type, int param2)
{
    if ((policy_type == LAND_TRADE_POLICY && !building_monument_working(BUILDING_CARAVANSERAI)) ||
        (policy_type == SEA_TRADE_POLICY && !building_monument_working(BUILDING_LIGHTHOUSE))) {
        return;
    }
    show_policy(policy_type);
}

static void button_resource(int resource_index, int param2)
{
    window_resource_settings_show(city_resource_get_potential()->items[resource_index + scrollbar.scroll_position]);
}

static void write_resource_storage_tooltip(advisor_tooltip_result *r, int resource)
{
    static uint8_t tooltip_resource_info[200];
    int amount_warehouse = city_resource_count(resource);
    int amount_granary = city_resource_count_food_on_granaries(resource) / RESOURCE_ONE_LOAD;
    uint8_t *text = tooltip_resource_info;
    text += string_from_int(text, amount_warehouse, 0);
    *text = ' ';
    text++;
    text = string_copy(lang_get_string(52, 43), text, 200 - (int) (text - tooltip_resource_info));
    *text = '\n';
    text++;
    text += string_from_int(text, amount_granary, 0);
    *text = ' ';
    text++;
    text = string_copy(translation_for(TR_ADVISOR_FROM_GRANARIES), text, 200 - (int) (text - tooltip_resource_info));
    r->precomposed_text = tooltip_resource_info;
}

static void get_tooltip_text(advisor_tooltip_result *r)
{
    if (data.focus_button_id == 1) {
        r->text_id = 106;
    } else if (data.focus_button_id == 2) {
        r->text_id = 41;
    } else if (data.focus_button_id == 3) {
        if (building_monument_working(BUILDING_CARAVANSERAI)) {
            r->translation_key = TR_TOOLTIP_ADVISOR_TRADE_LAND_POLICY;
        } else {
            r->translation_key = TR_TOOLTIP_ADVISOR_TRADE_LAND_POLICY_REQUIRED;
        }
    } else if (data.focus_button_id == 4) {
        if (building_monument_working(BUILDING_LIGHTHOUSE)) {
            r->translation_key = TR_TOOLTIP_ADVISOR_TRADE_SEA_POLICY;
        } else {
            r->translation_key = TR_TOOLTIP_ADVISOR_TRADE_SEA_POLICY_REQUIRED;
        }
    } else if (data.focus_button_id > 4) {
        const mouse *m = mouse_in_dialog(mouse_get());
        int resource = city_resource_get_potential()->items[data.focus_button_id - 5 + scrollbar.scroll_position];
        if (resource_is_food(resource) && m->x > 180 && m->x < 220) {
            write_resource_storage_tooltip(r, resource);
            return;
        }
        r->text_id = 107;
    }
}

const advisor_window_type *window_advisor_trade(void)
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    init();
    return &window;
}
