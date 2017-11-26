#include "advisors_private.h"
#include "popupdialog.h"
#include "window.h"

#include "cityinfo.h"
#include "formation.h"
#include "resource.h"

#include "data/settings.hpp"

#include "empire/city.h"
#include "scenario/request.h"

#include <game>

static void buttonDonateToCity(int param1, int param2);
static void buttonSetSalary(int param1, int param2);
static void buttonGiftToCaesar(int param1, int param2);
static void buttonRequest(int param1, int param2);

static int getRequestStatus(int index);

static void confirmNothing(int accepted);
static void confirmSendTroops(int accepted);
static void confirmSendGoods(int accepted);

static CustomButton imperialButtons[] =
{
    {320, 367, 570, 387, CustomButton_Immediate, buttonDonateToCity, Widget::Button::doNothing, 0, 0},
    {70, 393, 570, 413, CustomButton_Immediate, buttonSetSalary, Widget::Button::doNothing, 0, 0},
    {320, 341, 570, 361, CustomButton_Immediate, buttonGiftToCaesar, Widget::Button::doNothing, 0, 0},
    {38, 96, 598, 136, CustomButton_Immediate, buttonRequest, Widget::Button::doNothing, 0, 0},
    {38, 138, 598, 178, CustomButton_Immediate, buttonRequest, Widget::Button::doNothing, 1, 0},
    {38, 180, 598, 220, CustomButton_Immediate, buttonRequest, Widget::Button::doNothing, 2, 0},
    {38, 222, 598, 262, CustomButton_Immediate, buttonRequest, Widget::Button::doNothing, 3, 0},
    {38, 264, 598, 304, CustomButton_Immediate, buttonRequest, Widget::Button::doNothing, 4, 0},
};

static int focusButtonId;
static int selectedRequestId;

void draw_request(int index, const scenario_request *request)
{
    if (index >= 5)
    {
        return;
    }
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    Widget::Panel::drawButtonBorder(baseOffsetX + 38, baseOffsetY + 96 + 42 * index, 560, 40, 0);
    Widget::Text::drawNumber(request->amount, '@', " ",
                             baseOffsetX + 40, baseOffsetY + 102 + 42 * index, FONT_NORMAL_WHITE);
    int resourceOffset = request->resource +
                         Resource_getGraphicIdOffset(request->resource, 3);
    Graphics_drawImage(image_group(GROUP_RESOURCE_ICONS) + resourceOffset,
                       baseOffsetX + 110, baseOffsetY + 100 + 42 * index);
    Widget_GameText_draw(23, request->resource,
                         baseOffsetX + 150, baseOffsetY + 102 + 42 * index, FONT_NORMAL_WHITE);

    int width = Widget_GameText_drawNumberWithDescription(8, 4, request->months_to_comply,
                baseOffsetX + 310, baseOffsetY + 102 + 42 * index, FONT_NORMAL_WHITE);
    Widget_GameText_draw(12, 2, baseOffsetX + 310 + width, baseOffsetY + 102 + 42 * index, FONT_NORMAL_WHITE);

    if (request->resource == RESOURCE_DENARII)
    {
        // request for money
        width = Widget::Text::drawNumber(Data_CityInfo.treasury, '@', " ",
                                         baseOffsetX + 40, baseOffsetY + 120 + 42 * index, FONT_NORMAL_WHITE);
        width += Widget_GameText_draw(52, 44,
                                      baseOffsetX + 40 + width, baseOffsetY + 120 + 42 * index, FONT_NORMAL_WHITE);
        if (Data_CityInfo.treasury < request->amount)
        {
            Widget_GameText_draw(52, 48,
                                 baseOffsetX + 80 + width, baseOffsetY + 120 + 42 * index, FONT_NORMAL_WHITE);
        }
        else
        {
            Widget_GameText_draw(52, 47,
                                 baseOffsetX + 80 + width, baseOffsetY + 120 + 42 * index, FONT_NORMAL_WHITE);
        }
    }
    else
    {
        // normal goods request
        int resourceId = request->resource;
        width = Widget::Text::drawNumber(Data_CityInfo.resourceStored[resourceId], '@', " ",
                                         baseOffsetX + 40, baseOffsetY + 120 + 42 * index, FONT_NORMAL_WHITE);
        width += Widget_GameText_draw(52, 43,
                                      baseOffsetX + 40 + width, baseOffsetY + 120 + 42 * index, FONT_NORMAL_WHITE);
        if (Data_CityInfo.resourceStored[resourceId] < request->amount)
        {
            Widget_GameText_draw(52, 48,
                                 baseOffsetX + 80 + width, baseOffsetY + 120 + 42 * index, FONT_NORMAL_WHITE);
        }
        else
        {
            Widget_GameText_draw(52, 47,
                                 baseOffsetX + 80 + width, baseOffsetY + 120 + 42 * index, FONT_NORMAL_WHITE);
        }
    }
}

void UI_Advisor_Imperial_drawBackground(int *advisorHeight)
{
    CityInfo_Imperial_calculateGiftCosts();

    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    *advisorHeight = 27;
    Widget::Panel::drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
    Graphics_drawImage(image_group(GROUP_ADVISOR_ICONS) + 2, baseOffsetX + 10, baseOffsetY + 10);

    Widget::Text::draw(Data_Settings.playerName, baseOffsetX + 60, baseOffsetY + 12, FONT_LARGE_BLACK, 0);

    int width = Widget_GameText_draw(52, 0, baseOffsetX + 60, baseOffsetY + 44, FONT_NORMAL_BLACK);
    Widget::Text::drawNumber(Data_CityInfo.ratingFavor, '@', " ", baseOffsetX + 60 + width, baseOffsetY + 44, FONT_NORMAL_BLACK);

    Widget_GameText_drawMultiline(52, Data_CityInfo.ratingFavor / 5 + 22,
                                  baseOffsetX + 60, baseOffsetY + 60, 544, FONT_NORMAL_BLACK);

    Widget::Panel::drawInnerPanel(baseOffsetX + 32, baseOffsetY + 90, 36, 14);

    int numRequests = 0;
    if (Data_CityInfo.distantBattleMonthsToBattle > 0 && Data_CityInfo.distantBattleRomanMonthsToTravel <= 0)
    {
        // can send to distant battle
        Widget::Panel::drawButtonBorder(baseOffsetX + 38, baseOffsetY + 96, 560, 40, 0);
        Graphics_drawImage(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WEAPONS,
                           baseOffsetX + 50, baseOffsetY + 106);
        width = Widget_GameText_draw(52, 72, baseOffsetX + 80, baseOffsetY + 102, FONT_NORMAL_WHITE);
        Widget_GameText_draw(21, empire_city_get(Data_CityInfo.distantBattleCityId)->name_id,
                             baseOffsetX + 50 + width, baseOffsetY + 102, FONT_NORMAL_WHITE);
        int strengthTextId;
        if (Data_CityInfo.distantBattleEnemyStrength < 46)
        {
            strengthTextId = 73;
        }
        else if (Data_CityInfo.distantBattleEnemyStrength < 89)
        {
            strengthTextId = 74;
        }
        else
        {
            strengthTextId = 75;
        }
        width = Widget_GameText_draw(52, strengthTextId, baseOffsetX + 80, baseOffsetY + 120, FONT_NORMAL_WHITE);
        Widget_GameText_drawNumberWithDescription(8, 4, Data_CityInfo.distantBattleMonthsToBattle,
                baseOffsetX + 80 + width, baseOffsetY + 120, FONT_NORMAL_WHITE);
        numRequests = 1;
    }
    numRequests = scenario_request_foreach_visible(numRequests, draw_request);
    if (!numRequests)
    {
        Widget_GameText_drawMultiline(52, 21, baseOffsetX + 64, baseOffsetY + 160, 512, FONT_NORMAL_WHITE);
    }
}

void UI_Advisor_Imperial_drawForeground()
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    Widget::Panel::drawInnerPanel(baseOffsetX + 64, baseOffsetY + 324, 32, 6);

    Widget_GameText_draw(32, Data_CityInfo.playerRank,
                         baseOffsetX + 72, baseOffsetY + 338, FONT_LARGE_BROWN);

    int width = Widget_GameText_draw(52, 1,
                                     baseOffsetX + 72, baseOffsetY + 372, FONT_NORMAL_WHITE);
    Widget::Text::drawMoney(Data_CityInfo.personalSavings,
                            baseOffsetX + 80 + width, baseOffsetY + 372, FONT_NORMAL_WHITE);

    Widget::Panel::drawButtonBorder(baseOffsetX + 320, baseOffsetY + 367,
                                    250, 20, focusButtonId == 1);
    Widget_GameText_drawCentered(52, 2, baseOffsetX + 320, baseOffsetY + 372, 250, FONT_NORMAL_WHITE);

    Widget::Panel::drawButtonBorder(baseOffsetX + 70, baseOffsetY + 393,
                                    500, 20, focusButtonId == 2);
    width = Widget_GameText_draw(52, Data_CityInfo.salaryRank + 4,
                                 baseOffsetX + 120, baseOffsetY + 398, FONT_NORMAL_WHITE);
    width += Widget::Text::drawNumber(Data_CityInfo.salaryAmount, '@', " ",
                                      baseOffsetX + 120 + width, baseOffsetY + 398, FONT_NORMAL_WHITE);
    Widget_GameText_draw(52, 3, baseOffsetX + 120 + width, baseOffsetY + 398, FONT_NORMAL_WHITE);

    Widget::Panel::drawButtonBorder(baseOffsetX + 320, baseOffsetY + 341,
                                    250, 20, focusButtonId == 3);
    Widget_GameText_drawCentered(52, 49, baseOffsetX + 320, baseOffsetY + 346, 250, FONT_NORMAL_WHITE);

    // Request buttons
    if (getRequestStatus(0))
    {
        Widget::Panel::drawButtonBorder(baseOffsetX + 38, baseOffsetY + 96,
                                        560, 40, focusButtonId == 4);
    }
    if (getRequestStatus(1))
    {
        Widget::Panel::drawButtonBorder(baseOffsetX + 38, baseOffsetY + 138,
                                        560, 40, focusButtonId == 5);
    }
    if (getRequestStatus(2))
    {
        Widget::Panel::drawButtonBorder(baseOffsetX + 38, baseOffsetY + 180,
                                        560, 40, focusButtonId == 6);
    }
    if (getRequestStatus(3))
    {
        Widget::Panel::drawButtonBorder(baseOffsetX + 38, baseOffsetY + 222,
                                        560, 40, focusButtonId == 7);
    }
    if (getRequestStatus(4))
    {
        Widget::Panel::drawButtonBorder(baseOffsetX + 38, baseOffsetY + 264,
                                        560, 40, focusButtonId == 8);
    }
}

static int getRequestStatus(int index)
{
    int numRequests = 0;
    if (Data_CityInfo.distantBattleMonthsToBattle > 0 && Data_CityInfo.distantBattleRomanMonthsToTravel <= 0)
    {
        numRequests = 1;
        if (index == 0)
        {
            if (Data_CityInfo.militaryTotalLegions <= 0)
            {
                return -4;
            }
            else if (Data_CityInfo.militaryTotalLegionsEmpireService <= 0)
            {
                return -3;
            }
            else
            {
                return -2;
            }
        }
    }
    const scenario_request *request = scenario_request_get_visible(index - numRequests);
    if (request)
    {
        if (request->resource == RESOURCE_DENARII)
        {
            if (Data_CityInfo.treasury <= request->amount)
            {
                return -1;
            }
        }
        else
        {
            if (Data_CityInfo.resourceStored[request->resource] < request->amount)
            {
                return -1;
            }
        }
        return request->id + 1;
    }
    return 0;
}

void UI_Advisor_Imperial_handleMouse()
{
    int offsetX = Data_Screen.offset640x480.x;
    int offsetY = Data_Screen.offset640x480.y;
    Widget::Button::handleCustomButtons(offsetX, offsetY,
                                        imperialButtons, 8, &focusButtonId);
}

static void buttonDonateToCity(int param1, int param2)
{
    UI_Window_goTo(Window_DonateToCityDialog);
}

static void buttonSetSalary(int param1, int param2)
{
    UI_Window_goTo(Window_SetSalaryDialog);
}

static void buttonGiftToCaesar(int param1, int param2)
{
    UI_Window_goTo(Window_SendGiftToCaesarDialog);
}

static void buttonRequest(int index, int param2)
{
    int status = getRequestStatus(index);
    if (status)
    {
        Data_CityInfo.militaryTotalLegionsEmpireService = 0;
        switch (status)
        {
        case -4:
            UI_PopupDialog_show(PopupDialog_NoLegionsAvailable, confirmNothing, 0);
            break;
        case -3:
            UI_PopupDialog_show(PopupDialog_NoLegionsSelected, confirmNothing, 0);
            break;
        case -2:
            UI_PopupDialog_show(PopupDialog_RequestSendTroops, confirmSendTroops, 2);
            break;
        case -1:
            UI_PopupDialog_show(PopupDialog_RequestNotEnoughGoods, confirmNothing, 0);
            break;
        default:
            selectedRequestId = status - 1;
            UI_PopupDialog_show(PopupDialog_RequestSendGoods, confirmSendGoods, 2);
            break;
        }
    }
}

static void confirmNothing(int accepted)
{
    // do nothing..
}

static void confirmSendTroops(int accepted)
{
    if (accepted)
    {
        Formation_dispatchLegionsToDistantBattle();
        UI_Window_goTo(Window_Empire);
    }
}

static void confirmSendGoods(int accepted)
{
    if (accepted)
    {
        scenario_request_dispatch(selectedRequestId);
    }
}

int UI_Advisor_Imperial_getTooltip()
{
    if (focusButtonId && focusButtonId <= 2)
    {
        return 93 + focusButtonId;
    }
    else if (focusButtonId == 3)
    {
        return 131;
    }
    else
    {
        return 0;
    }
}
