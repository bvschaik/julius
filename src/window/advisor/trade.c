#include "trade.h"

#include "city/resource.h"
#include "game/resource.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "window/empire.h"
#include "window/resource_settings.h"
#include "window/trade_prices.h"

#include "Data/CityInfo.h"

#define ADVISOR_HEIGHT 27

static void button_prices(int param1, int param2);
static void button_empire(int param1, int param2);
static void button_resource(int resource_index, int param2);

static generic_button resource_buttons[] = {
    {400, 398, 600, 421, GB_IMMEDIATE, button_prices, button_none, 1, 0},
    {100, 398, 300, 421, GB_IMMEDIATE, button_empire, button_none, 1, 0},
    {80, 56, 560, 76, GB_IMMEDIATE, button_resource, button_none, 0, 0},
    {80, 78, 560, 98, GB_IMMEDIATE, button_resource, button_none, 1, 0},
    {80, 100, 560, 120, GB_IMMEDIATE, button_resource, button_none, 2, 0},
    {80, 122, 560, 142, GB_IMMEDIATE, button_resource, button_none, 3, 0},
    {80, 144, 560, 164, GB_IMMEDIATE, button_resource, button_none, 4, 0},
    {80, 166, 560, 186, GB_IMMEDIATE, button_resource, button_none, 5, 0},
    {80, 188, 560, 208, GB_IMMEDIATE, button_resource, button_none, 6, 0},
    {80, 210, 560, 230, GB_IMMEDIATE, button_resource, button_none, 7, 0},
    {80, 232, 560, 252, GB_IMMEDIATE, button_resource, button_none, 8, 0},
    {80, 254, 560, 274, GB_IMMEDIATE, button_resource, button_none, 9, 0},
    {80, 276, 560, 296, GB_IMMEDIATE, button_resource, button_none, 10, 0},
    {80, 298, 560, 318, GB_IMMEDIATE, button_resource, button_none, 11, 0},
    {80, 320, 560, 340, GB_IMMEDIATE, button_resource, button_none, 12, 0},
    {80, 342, 560, 362, GB_IMMEDIATE, button_resource, button_none, 13, 0},
    {80, 364, 560, 384, GB_IMMEDIATE, button_resource, button_none, 14, 0}
};

static int focus_button_id;

static int draw_background()
{
    city_resource_determine_available();

    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 4, 10, 10);

    lang_text_draw(54, 0, 60, 12, FONT_LARGE_BLACK);
    lang_text_draw(54, 1, 400, 38, FONT_NORMAL_BLACK);

    return ADVISOR_HEIGHT;
}

static void draw_foreground()
{
    inner_panel_draw(32, 52, 36, 21);
    for (int i = 0; i < Data_CityInfo_Resource.numAvailableResources; i++) {
        int offsetY = 22 * i;
        int resource = Data_CityInfo_Resource.availableResources[i];
        int graphicOffset = resource + resource_image_offset(resource, RESOURCE_IMAGE_ICON);
        image_draw(image_group(GROUP_RESOURCE_ICONS) + graphicOffset, 48, offsetY + 54);
        image_draw(image_group(GROUP_RESOURCE_ICONS) + graphicOffset, 568, offsetY + 54);
        
        if (focus_button_id - 3 == i) {
            button_border_draw(80, offsetY + 54, 480, 24, 1);
        }
        lang_text_draw(23, resource, 88, offsetY + 61, FONT_NORMAL_WHITE);
        text_draw_number_centered(Data_CityInfo.resourceStored[resource],
            180, offsetY + 61, 100, FONT_NORMAL_WHITE);
        if (Data_CityInfo.resourceIndustryMothballed[resource]) {
            lang_text_draw(18, 5, 300, offsetY + 61, FONT_NORMAL_WHITE);
        }
        if (Data_CityInfo.resourceStockpiled[resource]) {
            lang_text_draw(54, 3, 380, offsetY + 61, FONT_NORMAL_WHITE);
        } else if (Data_CityInfo.resourceTradeStatus[resource] == TRADE_STATUS_IMPORT) {
            lang_text_draw(54, 5, 380, offsetY + 61, FONT_NORMAL_WHITE);
        } else if (Data_CityInfo.resourceTradeStatus[resource] == TRADE_STATUS_EXPORT) {
            lang_text_draw(54, 6, 380, offsetY + 61, FONT_NORMAL_WHITE);
            text_draw_number(Data_CityInfo.resourceTradeExportOver[resource], '@', " ",
                500, offsetY + 61, FONT_NORMAL_WHITE);
        }
    }

    button_border_draw(398, 396, 200, 24, focus_button_id == 1);
    lang_text_draw_centered(54, 2, 400, 402, 200, FONT_NORMAL_BLACK);

    button_border_draw(98, 396, 200, 24, focus_button_id == 2);
    lang_text_draw_centered(54, 30, 100, 402, 200, FONT_NORMAL_BLACK);
}

static void handle_mouse(const mouse *m)
{
    generic_buttons_handle_mouse(m, 0, 0,
                                 resource_buttons, Data_CityInfo_Resource.numAvailableResources + 2, &focus_button_id);
}

static void button_prices(int param1, int param2)
{
    window_trade_prices_show();
}

static void button_empire(int param1, int param2)
{
    window_empire_show();
}

static void button_resource(int resource_index, int param2)
{
    window_resource_settings_show(Data_CityInfo_Resource.availableResources[resource_index]);
}

static int get_tooltip_text()
{
    if (focus_button_id == 1) {
        return 106;
    } else if (focus_button_id == 2) {
        return 41;
    } else if (focus_button_id) {
        return 107;
    } else {
        return 0;
    }
}

const advisor_window_type *window_advisor_trade()
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    return &window;
}

void window_advisor_trade_draw_dialog_background()
{
    draw_background();
    draw_foreground();
}
