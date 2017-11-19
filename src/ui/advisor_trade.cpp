#include "advisors_private.h"
#include "tooltip.h"
#include "window.h"
#include "messagedialog.h"
#include "cityinfo.h"
#include "resource.h"

#include <data>
#include <scenario>

#include "building/count.h"
#include "core/calc.h"
#include "empire/city.h"
#include "empire/trade_prices.h"

static void buttonPrices(int param1, int param2);
static void buttonEmpire(int param1, int param2);
static void buttonResource(int param1, int param2);

static void resourceSettingsHelp(int param1, int param2);
static void resourceSettingsOk(int param1, int param2);
static void resourceSettingsExportUpDown(int param1, int param2);

static void resourceSettingsToggleIndustry(int param1, int param2);
static void resourceSettingsToggleTrade(int param1, int param2);
static void resourceSettingsToggleStockpile(int param1, int param2);

static CustomButton resourceButtons[] =
{
    {400, 398, 600, 421, CustomButton_Immediate, buttonPrices, Widget_Button_doNothing, 1, 0},
    {100, 398, 300, 421, CustomButton_Immediate, buttonEmpire, Widget_Button_doNothing, 1, 0},
    {80, 56, 560, 76, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 0, 0},
    {80, 78, 560, 98, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 1, 0},
    {80, 100, 560, 120, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 2, 0},
    {80, 122, 560, 142, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 3, 0},
    {80, 144, 560, 164, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 4, 0},
    {80, 166, 560, 186, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 5, 0},
    {80, 188, 560, 208, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 6, 0},
    {80, 210, 560, 230, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 7, 0},
    {80, 232, 560, 252, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 8, 0},
    {80, 254, 560, 274, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 9, 0},
    {80, 276, 560, 296, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 10, 0},
    {80, 298, 560, 318, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 11, 0},
    {80, 320, 560, 340, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 12, 0},
    {80, 342, 560, 362, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 13, 0},
    {80, 364, 560, 384, CustomButton_Immediate, buttonResource, Widget_Button_doNothing, 14, 0}
};

static ImageButton resourceImageButtons[] =
{
    {58, 332, 27, 27, ImageButton_Normal, 134, 0, resourceSettingsHelp, Widget_Button_doNothing, 0, 0, 1},
    {558, 335, 24, 24, ImageButton_Normal, 134, 4, resourceSettingsOk, Widget_Button_doNothing, 0, 0, 1}
};

static ArrowButton resourceArrowButtons[] =
{
    {314, 215, 17, 24, resourceSettingsExportUpDown, 1, 0},
    {338, 215, 15, 24, resourceSettingsExportUpDown, 0, 0}
};

static CustomButton resourceCustomButtons[] =
{
    {98, 250, 530, 280, CustomButton_Immediate, resourceSettingsToggleIndustry, Widget_Button_doNothing, 0, 0},
    {98, 212, 530, 242, CustomButton_Immediate, resourceSettingsToggleTrade, Widget_Button_doNothing, 0, 0},
    {98, 288, 530, 338, CustomButton_Immediate, resourceSettingsToggleStockpile, Widget_Button_doNothing, 0, 0},
};

static int focusButtonId;

static int selectedResourceId;
static int resourceFocusButtonId;

void UI_Advisor_Trade_drawBackground(int *advisorHeight)
{
    CityInfo_Resource_calculateAvailableResources();

    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    *advisorHeight = 27;
    Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
    Graphics_drawImage(image_group(ID_Graphic_AdvisorIcons) + 4,
                       baseOffsetX + 10, baseOffsetY + 10);

    Widget_GameText_draw(54, 0, baseOffsetX + 60, baseOffsetY + 12, FONT_LARGE_BLACK);

    Widget_GameText_draw(54, 1, baseOffsetX + 400, baseOffsetY + 38, FONT_NORMAL_BLACK);
}

void UI_Advisor_Trade_drawForeground()
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 52, 36, 21);
    for (int i = 0; i < Data_CityInfo_Resource.numAvailableResources; i++)
    {
        int offsetY = baseOffsetY + 22 * i;
        int resource = Data_CityInfo_Resource.availableResources[i];
        int graphicOffset = resource + Resource_getGraphicIdOffset(resource, 3);
        Graphics_drawImage(image_group(ID_Graphic_ResourceIcons) + graphicOffset,
                           baseOffsetX + 48, offsetY + 54);
        Graphics_drawImage(image_group(ID_Graphic_ResourceIcons) + graphicOffset,
                           baseOffsetX + 568, offsetY + 54);

        if (focusButtonId - 3 == i)
        {
            Widget_Panel_drawButtonBorder(baseOffsetX + 80, offsetY + 54, 480, 24, 1);
        }
        Widget_GameText_draw(23, resource, baseOffsetX + 88, offsetY + 61, FONT_NORMAL_WHITE);
        Widget::Text::drawNumberCentered(Data_CityInfo.resourceStored[resource],
                                         baseOffsetX + 180, offsetY + 61, 100, FONT_NORMAL_WHITE);
        if (Data_CityInfo.resourceIndustryMothballed[resource])
        {
            Widget_GameText_draw(18, 5, baseOffsetX + 300, offsetY + 61, FONT_NORMAL_WHITE);
        }
        if (Data_CityInfo.resourceStockpiled[resource])
        {
            Widget_GameText_draw(54, 3, baseOffsetX + 380, offsetY + 61, FONT_NORMAL_WHITE);
        }
        else if (Data_CityInfo.resourceTradeStatus[resource] == TradeStatus_Import)
        {
            Widget_GameText_draw(54, 5, baseOffsetX + 380, offsetY + 61, FONT_NORMAL_WHITE);
        }
        else if (Data_CityInfo.resourceTradeStatus[resource] == TradeStatus_Export)
        {
            Widget_GameText_draw(54, 6, baseOffsetX + 380, offsetY + 61, FONT_NORMAL_WHITE);
            Widget::Text::drawNumber(Data_CityInfo.resourceTradeExportOver[resource], '@', " ",
                                     baseOffsetX + 500, offsetY + 61, FONT_NORMAL_WHITE);
        }
    }

    Widget_Panel_drawButtonBorder(baseOffsetX + 398, baseOffsetY + 396, 200, 24, focusButtonId == 1);
    Widget_GameText_drawCentered(54, 2, baseOffsetX + 400, baseOffsetY + 402, 200, FONT_NORMAL_BLACK);

    Widget_Panel_drawButtonBorder(baseOffsetX + 98, baseOffsetY + 396, 200, 24, focusButtonId == 2);
    Widget_GameText_drawCentered(54, 30, baseOffsetX + 100, baseOffsetY + 402, 200, FONT_NORMAL_BLACK);
}

void UI_Advisor_Trade_handleMouse()
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    Widget_Button_handleCustomButtons(baseOffsetX, baseOffsetY,
                                      resourceButtons, Data_CityInfo_Resource.numAvailableResources + 2, &focusButtonId);
}

static void buttonPrices(int param1, int param2)
{
    UI_Window_goTo(Window_TradePricesDialog);
}

static void buttonEmpire(int param1, int param2)
{
    UI_Window_goTo(Window_Empire);
}

static void buttonResource(int resourceIndex, int param2)
{
    selectedResourceId = Data_CityInfo_Resource.availableResources[resourceIndex];
    UI_Window_goTo(Window_ResourceSettingsDialog);
}

int UI_Advisor_Trade_getTooltip()
{
    if (focusButtonId == 1)
    {
        return 106;
    }
    else if (focusButtonId == 2)
    {
        return 41; // FIXED used to say 'select this resource'
    }
    else if (focusButtonId)
    {
        return 107;
    }
    else
    {
        return 0;
    }
}


void UI_TradePricesDialog_drawBackground()
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    Graphics_shadeRect(baseOffsetX + 33, baseOffsetY + 53, 574, 334, 0);
    Widget_Panel_drawOuterPanel(baseOffsetX + 16, baseOffsetY + 144, 38, 11);
    Widget_GameText_draw(54, 21, baseOffsetX + 26, baseOffsetY + 153, FONT_LARGE_BLACK);
    Widget_GameText_draw(54, 22, baseOffsetX + 26, baseOffsetY + 228, FONT_NORMAL_BLACK);
    Widget_GameText_draw(54, 23, baseOffsetX + 26, baseOffsetY + 253, FONT_NORMAL_BLACK);
    for (int i = 1; i < 16; i++)
    {
        int graphicOffset = i + Resource_getGraphicIdOffset(i, 3);
        Graphics_drawImage(image_group(ID_Graphic_ResourceIcons) + graphicOffset,
                           baseOffsetX + 126 + 30 * i, baseOffsetY + 194);
        Widget::Text::drawNumberCentered(trade_price_buy(i),
                                         baseOffsetX + 120 + 30 * i, baseOffsetY + 229, 30, FONT_SMALL_PLAIN);
        Widget::Text::drawNumberCentered(trade_price_sell(i),
                                         baseOffsetX + 120 + 30 * i, baseOffsetY + 254, 30, FONT_SMALL_PLAIN);
    }
    Widget_GameText_drawCentered(13, 1, baseOffsetX + 16, baseOffsetY + 296, 608, FONT_NORMAL_BLACK);
}

void UI_TradePricesDialog_handleMouse(const mouse *m)
{
    if (m->right.went_up)
    {
        UI_Window_goTo(Window_Advisors);
    }
}

static int getTradePriceTooltipResource(struct TooltipContext *c)
{
    int xBase = Data_Screen.offset640x480.x + 124;
    int y = Data_Screen.offset640x480.y + 192;
    int xMouse = c->mouse_x;
    int yMouse = c->mouse_y;

    for (int i = 1; i < 16; i++)
    {
        int x = xBase + 30 * i;
        if (x <= xMouse && x + 24 > xMouse && y <= yMouse && y + 24 > yMouse)
        {
            return i;
        }
    }
    return 0;
}

void UI_TradePricesDialog_getTooltip(struct TooltipContext *c)
{
    int resource = getTradePriceTooltipResource(c);
    if (!resource)
    {
        return;
    }
    c->type = TooltipType_Button;
    c->textId = 131 + resource;
}

void UI_ResourceSettingsDialog_drawBackground()
{
    int dummy;
    UI_Advisor_Trade_drawBackground(&dummy);
    UI_Advisor_Trade_drawForeground();
}

void UI_ResourceSettingsDialog_drawForeground()
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    Widget_Panel_drawOuterPanel(baseOffsetX + 48, baseOffsetY + 128, 34, 15);
    int graphicOffset = selectedResourceId + Resource_getGraphicIdOffset(selectedResourceId, 3);
    Graphics_drawImage(image_group(ID_Graphic_ResourceIcons) + graphicOffset,
                       baseOffsetX + 58, baseOffsetY + 136);

    Widget_GameText_draw(23, selectedResourceId, baseOffsetX + 92, baseOffsetY + 137, FONT_LARGE_BLACK);

    if (empire_can_produce_resource(selectedResourceId))
    {
        int totalBuildings = building_count_industry_total(selectedResourceId);
        int activeBuildings = building_count_industry_active((resource_type)selectedResourceId);
        if (building_count_industry_total(selectedResourceId) <= 0)
        {
            Widget_GameText_draw(54, 7, baseOffsetX + 98, baseOffsetY + 172, FONT_NORMAL_BLACK);
        }
        else if (Data_CityInfo.resourceIndustryMothballed[selectedResourceId] == 1)
        {
            int width = Widget::Text::drawNumber(
                            totalBuildings, '@', " ",
                            baseOffsetX + 98, baseOffsetY + 172, FONT_NORMAL_BLACK);
            if (totalBuildings == 1)
            {
                Widget_GameText_draw(54, 10, baseOffsetX + 98 + width, baseOffsetY + 172, FONT_NORMAL_BLACK);
            }
            else
            {
                Widget_GameText_draw(54, 11, baseOffsetX + 98 + width, baseOffsetY + 172, FONT_NORMAL_BLACK);
            }
        }
        else if (totalBuildings == activeBuildings)
        {
            // not mothballed, all working
            int width = Widget::Text::drawNumber(
                            totalBuildings, '@', " ",
                            baseOffsetX + 98, baseOffsetY + 172, FONT_NORMAL_BLACK);
            if (totalBuildings == 1)
            {
                Widget_GameText_draw(54, 8, baseOffsetX + 98 + width, baseOffsetY + 172, FONT_NORMAL_BLACK);
            }
            else
            {
                Widget_GameText_draw(54, 9, baseOffsetX + 98 + width, baseOffsetY + 172, FONT_NORMAL_BLACK);
            }
        }
        else
        {
            // not mothballed, some working
            int width = Widget::Text::drawNumber(
                            activeBuildings, '@', " ",
                            baseOffsetX + 98, baseOffsetY + 172, FONT_NORMAL_BLACK);
            width += Widget_GameText_draw(54, 12, baseOffsetX + 98 + width, baseOffsetY + 172, FONT_NORMAL_BLACK);
            width += Widget::Text::drawNumber(
                         totalBuildings -
                         activeBuildings, '@', " ",
                         baseOffsetX + 98 + width, baseOffsetY + 172, FONT_NORMAL_BLACK);
            if (activeBuildings == 1)
            {
                Widget_GameText_draw(54, 13, baseOffsetX + 98 + width, baseOffsetY + 172, FONT_NORMAL_BLACK);
            }
            else
            {
                Widget_GameText_draw(54, 14, baseOffsetX + 98 + width, baseOffsetY + 172, FONT_NORMAL_BLACK);
            }
        }
    }
    else if (selectedResourceId != RESOURCE_MEAT || !scenario_building_allowed(BUILDING_WHARF))
    {
        // we cannot produce this good
        Widget_GameText_draw(54, 25, baseOffsetX + 98, baseOffsetY + 172, FONT_NORMAL_BLACK);
    }

    int width = Widget_GameText_drawNumberWithDescription(8, 10,
                Data_CityInfo.resourceStored[selectedResourceId],
                baseOffsetX + 98, baseOffsetY + 192, FONT_NORMAL_BLACK);
    Widget_GameText_draw(54, 15, baseOffsetX + 98 + width, baseOffsetY + 192, FONT_NORMAL_BLACK);

    int tradeFlags = TradeStatus_None;
    if (empire_can_import_resource(selectedResourceId))
    {
        tradeFlags |= TradeStatus_Import;
    }
    if (empire_city_can_export_resource(selectedResourceId))
    {
        tradeFlags |= TradeStatus_Export;
    }
    if (!tradeFlags)
    {
        Widget_GameText_draw(54, 24, baseOffsetX + 98, baseOffsetY + 212, FONT_NORMAL_BLACK);
    }
    else
    {
        Widget_Panel_drawButtonBorder(baseOffsetX + 98, baseOffsetY + 212, 432, 30,
                                      resourceFocusButtonId == 2);
        switch (Data_CityInfo.resourceTradeStatus[selectedResourceId])
        {
        case TradeStatus_None:
            Widget_GameText_drawCentered(54, 18,
                                         baseOffsetX + 114, baseOffsetY + 221, 400, FONT_NORMAL_BLACK);
            break;
        case TradeStatus_Import:
            Widget_GameText_drawCentered(54, 19,
                                         baseOffsetX + 114, baseOffsetY + 221, 400, FONT_NORMAL_BLACK);
            break;
        case TradeStatus_Export:
            Widget_GameText_drawCentered(54, 20,
                                         baseOffsetX + 114, baseOffsetY + 221, 200, FONT_NORMAL_BLACK);
            break;
        }
    }

    if (Data_CityInfo.resourceTradeStatus[selectedResourceId] == TradeStatus_Export)
    {
        Widget_GameText_drawNumberWithDescription(8, 10,
                Data_CityInfo.resourceTradeExportOver[selectedResourceId],
                baseOffsetX + 386, baseOffsetY + 221, FONT_NORMAL_BLACK);
    }

    if (building_count_industry_total(selectedResourceId) > 0)
    {
        Widget_Panel_drawButtonBorder(baseOffsetX + 98, baseOffsetY + 250, 432, 30,
                                      resourceFocusButtonId == 1);
        if (Data_CityInfo.resourceIndustryMothballed[selectedResourceId])
        {
            Widget_GameText_drawCentered(54, 17, baseOffsetX + 114, baseOffsetY + 259, 400, FONT_NORMAL_BLACK);
        }
        else
        {
            Widget_GameText_drawCentered(54, 16, baseOffsetX + 114, baseOffsetY + 259, 400, FONT_NORMAL_BLACK);
        }
    }

    Widget_Panel_drawButtonBorder(baseOffsetX + 98, baseOffsetY + 288, 432, 50,
                                  resourceFocusButtonId == 3);
    if (Data_CityInfo.resourceStockpiled[selectedResourceId])
    {
        Widget_GameText_drawCentered(54, 26, baseOffsetX + 114, baseOffsetY + 296, 400, FONT_NORMAL_BLACK);
        Widget_GameText_drawCentered(54, 27, baseOffsetX + 114, baseOffsetY + 316, 400, FONT_NORMAL_BLACK);
    }
    else
    {
        Widget_GameText_drawCentered(54, 28, baseOffsetX + 114, baseOffsetY + 296, 400, FONT_NORMAL_BLACK);
        Widget_GameText_drawCentered(54, 29, baseOffsetX + 114, baseOffsetY + 316, 400, FONT_NORMAL_BLACK);
    }

    Widget_Button_drawImageButtons(baseOffsetX, baseOffsetY, resourceImageButtons, 2);
    if (Data_CityInfo.resourceTradeStatus[selectedResourceId] == TradeStatus_Export)
    {
        Widget_Button_drawArrowButtons(baseOffsetX, baseOffsetY, resourceArrowButtons, 2);
    }
}

void UI_ResourceSettingsDialog_handleMouse(const mouse *m)
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    if (m->right.went_up)
    {
        UI_Window_goTo(Window_Advisors);
    }
    else if (Widget_Button_handleImageButtons(baseOffsetX, baseOffsetY, resourceImageButtons, 2, 0))
    {
        return;
    }
    else if (Data_CityInfo.resourceTradeStatus[selectedResourceId] == TradeStatus_Export &&
             Widget_Button_handleArrowButtons(baseOffsetX, baseOffsetY, resourceArrowButtons, 2))
    {
        return;
    }
    else
    {
        Widget_Button_handleCustomButtons(baseOffsetX, baseOffsetY,
                                          resourceCustomButtons, 3, &resourceFocusButtonId);
    }
}

static void resourceSettingsHelp(int param1, int param2)
{
    UI_MessageDialog_show(MessageDialog_Industry, 1);
}

static void resourceSettingsOk(int param1, int param2)
{
    UI_Window_goTo(Window_Advisors);
}

static void resourceSettingsExportUpDown(int isDown, int param2)
{
    if (isDown)
    {
        --Data_CityInfo.resourceTradeExportOver[selectedResourceId];
    }
    else
    {
        ++Data_CityInfo.resourceTradeExportOver[selectedResourceId];
    }
    Data_CityInfo.resourceTradeExportOver[selectedResourceId] =
        calc_bound(Data_CityInfo.resourceTradeExportOver[selectedResourceId], 0, 100);
}

static void resourceSettingsToggleIndustry(int param1, int param2)
{
    if (building_count_industry_total(selectedResourceId) > 0)
    {
        if (Data_CityInfo.resourceIndustryMothballed[selectedResourceId])
        {
            Data_CityInfo.resourceIndustryMothballed[selectedResourceId] = 0;
        }
        else
        {
            Data_CityInfo.resourceIndustryMothballed[selectedResourceId] = 1;
        }
    }
}

static void resourceSettingsToggleTrade(int param1, int param2)
{
    ++Data_CityInfo.resourceTradeStatus[selectedResourceId];
    if (Data_CityInfo.resourceTradeStatus[selectedResourceId] > TradeStatus_Export)
    {
        Data_CityInfo.resourceTradeStatus[selectedResourceId] = TradeStatus_None;
    }

    if (Data_CityInfo.resourceTradeStatus[selectedResourceId] == TradeStatus_Import &&
            !empire_can_import_resource(selectedResourceId))
    {
        Data_CityInfo.resourceTradeStatus[selectedResourceId] = TradeStatus_Export;
    }
    if (Data_CityInfo.resourceTradeStatus[selectedResourceId] == TradeStatus_Export &&
            !empire_city_can_export_resource(selectedResourceId))
    {
        Data_CityInfo.resourceTradeStatus[selectedResourceId] = TradeStatus_None;
    }
}

static void resourceSettingsToggleStockpile(int param1, int param2)
{
    if (Data_CityInfo.resourceStockpiled[selectedResourceId])
    {
        Data_CityInfo.resourceStockpiled[selectedResourceId] = 0;
    }
    else
    {
        Data_CityInfo.resourceStockpiled[selectedResourceId] = 1;
        if (Data_CityInfo.resourceTradeStatus[selectedResourceId] == TradeStatus_Export)
        {
            Data_CityInfo.resourceTradeStatus[selectedResourceId] = TradeStatus_None;
        }
    }
}
