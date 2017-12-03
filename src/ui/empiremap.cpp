#include "animation.h"
#include "core/calc.h"
#include "cityinfo.h"
#include "graphics.h"
#include "resource.h"
#include "scroll.h"
#include "sidebarmenu.h"

#include <ui>
#include <data>
#include <empire>
#include <scenario>
#include <game>

#include "graphics/image.h"

#define MAX_WIDTH 2032
#define MAX_HEIGHT 1136

static void drawPaneling();
static void drawPanelInfo();
static void drawPanelInfoCity(const empire_object *object);
static void drawPanelInfoRomanArmy(const empire_object *object);
static void drawPanelInfoEnemyArmy(const empire_object *object);
static void drawPanelInfoCityName(const empire_city *city);
static void drawPanelButtons(const empire_city *city);
static void drawEmpireMap();
static void determineSelectedObject(const mouse *m);

static void buttonHelp(int param1, int param2);
static void buttonReturnToCity(int param1, int param2);
static void buttonAdvisor(int param1, int param2);
static void buttonOpenTrade(int param1, int param2);
static void buttonEmpireMap(int param1, int param2);
static void confirmOpenTrade(int accepted);

static ImageButton imageButtonHelp[] =
{
    {0, 0, 27, 27, ImageButton_Normal, 134, 0, buttonHelp, Widget::Button::doNothing, 0, 0, 1}
};
static ImageButton imageButtonReturnToCity[] =
{
    {0, 0, 24, 24, ImageButton_Normal, 134, 4, buttonReturnToCity, Widget::Button::doNothing, 0, 0, 1}
};
static ImageButton imageButtonAdvisor[] =
{
    {-4, 0, 24, 24, ImageButton_Normal, 199, 12, buttonAdvisor, Widget::Button::doNothing, 5, 0, 1}
};
static CustomButton customButtonOpenTrade[] =
{
    {50, 68, 450, 91, CustomButton_Immediate, buttonOpenTrade, Widget::Button::doNothing, 0, 0}
};

static ImageButton imageButtonsTradeOpened[] =
{
    {92, 248, 28, 28, ImageButton_Normal, 199, 12, buttonAdvisor, Widget::Button::doNothing, 5, 0, 1},
    {522, 252, 24, 24, ImageButton_Normal, 134, 4, buttonEmpireMap, Widget::Button::doNothing, 0, 0, 1},
};

static struct
{
    int selectedButton;
    int selectedCity;
    int xMin, xMax, yMin, yMax;
    int xDrawOffset, yDrawOffset;
    int focusButtonId;
} data = {0, 1};

void UI_Empire_init()
{
    data.selectedButton = 0;
    int selectedObject = empire_selected_object();
    if (selectedObject)
    {
        data.selectedCity = empire_city_get_for_object(selectedObject - 1);
    }
    else
    {
        data.selectedCity = 0;
    }
    data.focusButtonId = 0;
}

static void draw_invasion_warning(int x, int y, int image_id)
{
    Graphics_drawImage(image_id, data.xDrawOffset + x, data.yDrawOffset + y);
}

void UI_Empire_drawBackground()
{
    data.xMin = Data_Screen.width <= MAX_WIDTH ? 0 : (Data_Screen.width - MAX_WIDTH) / 2;
    data.xMax = Data_Screen.width <= MAX_WIDTH ? Data_Screen.width : data.xMin + MAX_WIDTH;
    data.yMin = Data_Screen.height <= MAX_HEIGHT ? 0 : (Data_Screen.height - MAX_HEIGHT) / 2;
    data.yMax = Data_Screen.height <= MAX_HEIGHT ? Data_Screen.height : data.yMin + MAX_HEIGHT;

    if (data.xMin || data.yMin)
    {
        Graphics_clearScreen();
    }
    drawPaneling();
    drawPanelInfo();
}

void UI_Empire_drawForeground()
{
    drawEmpireMap();

    const empire_city *city = 0;
    int selected_object = empire_selected_object();
    if (selected_object)
    {
        const empire_object *object = empire_object_get(selected_object - 1);
        if (object->type == EMPIRE_OBJECT_CITY)
        {
            data.selectedCity = empire_city_get_for_object(object->id);
            city = empire_city_get(data.selectedCity);
        }
    }
    drawPanelInfoCityName(city);
    drawPanelButtons(city);
}

static void drawPaneling()
{
    int graphicBase = image_group(GROUP_EMPIRE_PANELS);
    // bottom panel background
    Graphics_setClipRectangle(data.xMin, data.yMin, data.xMax - data.xMin, data.yMax - data.yMin);
    for (int x = data.xMin; x < data.xMax; x += 70)
    {
        Graphics_drawImage(graphicBase + 3, x, data.yMax - 120);
        Graphics_drawImage(graphicBase + 3, x, data.yMax - 80);
        Graphics_drawImage(graphicBase + 3, x, data.yMax - 40);
    }

    // horizontal bar borders
    for (int x = data.xMin; x < data.xMax; x += 86)
    {
        Graphics_drawImage(graphicBase + 1, x, data.yMin);
        Graphics_drawImage(graphicBase + 1, x, data.yMax - 120);
        Graphics_drawImage(graphicBase + 1, x, data.yMax - 16);
    }

    // vertical bar borders
    for (int y = data.yMin + 16; y < data.yMax; y += 86)
    {
        Graphics_drawImage(graphicBase, data.xMin, y);
        Graphics_drawImage(graphicBase, data.xMax - 16, y);
    }

    // crossbars
    Graphics_drawImage(graphicBase + 2, data.xMin, data.yMin);
    Graphics_drawImage(graphicBase + 2, data.xMin, data.yMax - 120);
    Graphics_drawImage(graphicBase + 2, data.xMin, data.yMax - 16);
    Graphics_drawImage(graphicBase + 2, data.xMax - 16, data.yMin);
    Graphics_drawImage(graphicBase + 2, data.xMax - 16, data.yMax - 120);
    Graphics_drawImage(graphicBase + 2, data.xMax - 16, data.yMax - 16);

    Graphics_resetClipRectangle();
}

static void drawPanelInfo()
{
    int selectedObject = empire_selected_object();
    if (selectedObject)
    {
        const empire_object *object = empire_object_get(selectedObject - 1);
        switch (object->type)
        {
        case EMPIRE_OBJECT_CITY:
            drawPanelInfoCity(object);
            break;
        case EMPIRE_OBJECT_ROMAN_ARMY:
            drawPanelInfoRomanArmy(object);
            break;
        case EMPIRE_OBJECT_ENEMY_ARMY:
            drawPanelInfoEnemyArmy(object);
            break;
        }
    }
    else
    {
        Widget_GameText_drawCentered(47, 8, data.xMin, data.yMax - 48, data.xMax - data.xMin, FONT_NORMAL_GREEN);
    }
}

static void drawPanelInfoCity(const empire_object *object)
{
    int xOffset = (data.xMin + data.xMax - 240) / 2;
    int yOffset = data.yMax - 88;

    const empire_city *city = empire_city_get(data.selectedCity);
    if (city->type == EMPIRE_CITY_DISTANT_ROMAN)
    {
        Widget_GameText_drawCentered(47, 12, xOffset, yOffset + 42, 240, FONT_NORMAL_GREEN);
        return;
    }
    if (city->type == EMPIRE_CITY_VULNERABLE_ROMAN)
    {
        if (Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0)
        {
            Widget_GameText_drawCentered(47, 12, xOffset, yOffset + 42, 240, FONT_NORMAL_GREEN);
        }
        else
        {
            Widget_GameText_drawCentered(47, 13, xOffset, yOffset + 42, 240, FONT_NORMAL_GREEN);
        }
        return;
    }
    if (city->type == EMPIRE_CITY_FUTURE_TRADE ||
            city->type == EMPIRE_CITY_DISTANT_FOREIGN ||
            city->type == EMPIRE_CITY_FUTURE_ROMAN)
    {
        Widget_GameText_drawCentered(47, 0, xOffset, yOffset + 42, 240, FONT_NORMAL_GREEN);
        return;
    }
    if (city->type == EMPIRE_CITY_OURS)
    {
        Widget_GameText_drawCentered(47, 1, xOffset, yOffset + 42, 240, FONT_NORMAL_GREEN);
        return;
    }
    if (city->type != EMPIRE_CITY_TRADE)
    {
        return;
    }
    // trade city
    xOffset = (data.xMin + data.xMax - 500) / 2;
    yOffset = data.yMax - 108;
    if (city->is_open)
    {
        // city sells
        Widget_GameText_draw(47, 10, xOffset + 40, yOffset + 30, FONT_NORMAL_GREEN);
        int goodOffset = 0;
        for (int good = 1; good <= 15; good++)
        {
            if (!empire_object_city_sells_resource(object->id, good))
            {
                continue;
            }
            Graphics_drawInsetRect(xOffset + 100 * goodOffset + 120, yOffset + 21, 26, 26);
            int graphicId = good + image_group(GROUP_EMPIRE_RESOURCES);
            int resourceOffset = Resource_getGraphicIdOffset(good, 3);
            Graphics_drawImage(graphicId + resourceOffset, xOffset + 100 * goodOffset + 121, yOffset + 22);
            int tradeMax = trade_route_limit(city->route_id, (resource_type)good);
            switch (tradeMax)
            {
            case 15:
                Graphics_drawImage(image_group(GROUP_TRADE_AMOUNT),
                                   xOffset + 100 * goodOffset + 141, yOffset + 20);
                break;
            case 25:
                Graphics_drawImage(image_group(GROUP_TRADE_AMOUNT) + 1,
                                   xOffset + 100 * goodOffset + 137, yOffset + 20);
                break;
            case 40:
                Graphics_drawImage(image_group(GROUP_TRADE_AMOUNT) + 2,
                                   xOffset + 100 * goodOffset + 133, yOffset + 20);
                break;
            }
            int tradeNow = trade_route_traded(city->route_id, (resource_type)good);
            if (tradeNow > tradeMax)
            {
                tradeMax = tradeNow;
            }
            int textWidth = Widget::Text::drawNumber(tradeNow, '@', "",
                            xOffset + 100 * goodOffset + 150, yOffset + 30, FONT_NORMAL_GREEN);
            textWidth += Widget_GameText_draw(47, 11,
                                              xOffset + 100 * goodOffset + 148 + textWidth, yOffset + 30, FONT_NORMAL_GREEN);
            Widget::Text::drawNumber(tradeMax, '@', "",
                                     xOffset + 100 * goodOffset + 138 + textWidth, yOffset + 30, FONT_NORMAL_GREEN);
            goodOffset++;
        }
        // city buys
        Widget_GameText_draw(47, 9, xOffset + 40, yOffset + 60, FONT_NORMAL_GREEN);
        goodOffset = 0;
        for (int good = 1; good <= 15; good++)
        {
            if (!empire_object_city_buys_resource(object->id, good))
            {
                continue;
            }
            Graphics_drawInsetRect(xOffset + 100 * goodOffset + 120, yOffset + 51, 26, 26);
            int graphicId = good + image_group(GROUP_EMPIRE_RESOURCES);
            int resourceOffset = Resource_getGraphicIdOffset(good, 3);
            Graphics_drawImage(graphicId + resourceOffset, xOffset + 100 * goodOffset + 121, yOffset + 52);
            int tradeMax = trade_route_limit(city->route_id, (resource_type)good);
            switch (tradeMax)
            {
            case 15:
                Graphics_drawImage(image_group(GROUP_TRADE_AMOUNT),
                                   xOffset + 100 * goodOffset + 141, yOffset + 50);
                break;
            case 25:
                Graphics_drawImage(image_group(GROUP_TRADE_AMOUNT) + 1,
                                   xOffset + 100 * goodOffset + 137, yOffset + 50);
                break;
            case 40:
                Graphics_drawImage(image_group(GROUP_TRADE_AMOUNT) + 2,
                                   xOffset + 100 * goodOffset + 133, yOffset + 50);
                break;
            }
            int tradeNow = trade_route_traded(city->route_id, (resource_type)good);
            if (tradeNow > tradeMax)
            {
                tradeMax = tradeNow;
            }
            int textWidth = Widget::Text::drawNumber(tradeNow, '@', "",
                            xOffset + 100 * goodOffset + 150, yOffset + 60, FONT_NORMAL_GREEN);
            textWidth += Widget_GameText_draw(47, 11,
                                              xOffset + 100 * goodOffset + 148 + textWidth, yOffset + 60, FONT_NORMAL_GREEN);
            Widget::Text::drawNumber(tradeMax, '@', "",
                                     xOffset + 100 * goodOffset + 138 + textWidth, yOffset + 60, FONT_NORMAL_GREEN);
            goodOffset++;
        }
    }
    else     // trade is closed
    {
        int goodOffset = Widget_GameText_draw(47, 5, xOffset + 50, yOffset + 42, FONT_NORMAL_GREEN);
        for (int good = 1; good <= 15; good++)
        {
            if (!empire_object_city_sells_resource(object->id, good))
            {
                continue;
            }
            Graphics_drawInsetRect(xOffset + goodOffset + 60, yOffset + 33, 26, 26);
            int graphicId = good + image_group(GROUP_EMPIRE_RESOURCES);
            int resourceOffset = Resource_getGraphicIdOffset(good, 3);
            Graphics_drawImage(graphicId + resourceOffset, xOffset + goodOffset + 61, yOffset + 34);
            switch (trade_route_limit(city->route_id, (resource_type)good))
            {
            case 15:
                Graphics_drawImage(image_group(GROUP_TRADE_AMOUNT),
                                   xOffset + goodOffset + 81, yOffset + 32);
                break;
            case 25:
                Graphics_drawImage(image_group(GROUP_TRADE_AMOUNT) + 1,
                                   xOffset + goodOffset + 77, yOffset + 32);
                break;
            case 40:
                Graphics_drawImage(image_group(GROUP_TRADE_AMOUNT) + 2,
                                   xOffset + goodOffset + 73, yOffset + 32);
                break;
            }
            goodOffset += 32;
        }
        goodOffset += Widget_GameText_draw(47, 4, xOffset + goodOffset + 100, yOffset + 42, FONT_NORMAL_GREEN);
        for (int good = 1; good <= 15; good++)
        {
            if (!empire_object_city_buys_resource(object->id, (resource_type)good))
            {
                continue;
            }
            Graphics_drawInsetRect(xOffset + goodOffset + 110, yOffset + 33, 26, 26);
            int graphicId = good + image_group(GROUP_EMPIRE_RESOURCES);
            int resourceOffset = Resource_getGraphicIdOffset(good, 3);
            Graphics_drawImage(graphicId + resourceOffset, xOffset + goodOffset + 110, yOffset + 34);
            switch (trade_route_limit(city->route_id, (resource_type)good))
            {
            case 15:
                Graphics_drawImage(image_group(GROUP_TRADE_AMOUNT),
                                   xOffset + goodOffset + 130, yOffset + 32);
                break;
            case 25:
                Graphics_drawImage(image_group(GROUP_TRADE_AMOUNT) + 1,
                                   xOffset + goodOffset + 126, yOffset + 32);
                break;
            case 40:
                Graphics_drawImage(image_group(GROUP_TRADE_AMOUNT) + 2,
                                   xOffset + goodOffset + 122, yOffset + 32);
                break;
            }
            goodOffset += 32;
        }
        Widget::Panel::drawButtonBorder(xOffset + 50, yOffset + 68, 400, 20, data.selectedButton);
        goodOffset = Widget_GameText_drawNumberWithDescription(8, 0, city->cost_to_open,
                     xOffset + 60, yOffset + 73, FONT_NORMAL_GREEN);
        Widget_GameText_draw(47, 6, xOffset + goodOffset + 60, yOffset + 73, FONT_NORMAL_GREEN);
    }
}

static void drawPanelInfoRomanArmy(const empire_object *object)
{
    if (Data_CityInfo.distantBattleRomanMonthsToTravel > 0 ||
            Data_CityInfo.distantBattleRomanMonthsToReturn > 0)
    {
        if (Data_CityInfo.distantBattleRomanMonthsTraveled == object->distant_battle_travel_months)
        {
            int xOffset = (data.xMin + data.xMax - 240) / 2;
            int yOffset = data.yMax - 88;
            int textId;
            if (Data_CityInfo.distantBattleRomanMonthsToTravel)
            {
                textId = 15;
            }
            else
            {
                textId = 16;
            }
            Widget_GameText_drawMultiline(47, textId, xOffset, yOffset, 240, FONT_NORMAL_BLACK);
        }
    }
}

static void drawPanelInfoEnemyArmy(const empire_object *object)
{
    if (Data_CityInfo.distantBattleMonthsToBattle > 0)
    {
        if (Data_CityInfo.distantBattleEnemyMonthsTraveled == object->distant_battle_travel_months)
        {
            Widget_GameText_drawMultiline(47, 14,
                                          (data.xMin + data.xMax - 240) / 2,
                                          data.yMax - 68,
                                          240, FONT_NORMAL_BLACK);
        }
    }
}

static void drawPanelInfoCityName(const empire_city *city)
{
    int graphicBase = image_group(GROUP_EMPIRE_PANELS);
    Graphics_drawImage(graphicBase + 6, data.xMin + 2, data.yMax - 199);
    Graphics_drawImage(graphicBase + 7, data.xMax - 84, data.yMax - 199);
    Graphics_drawImage(graphicBase + 8, (data.xMin + data.xMax - 332) / 2, data.yMax - 181);
    if (city)
    {
        Widget_GameText_drawCentered(21, city->name_id,
                                     (data.xMin + data.xMax - 332) / 2 + 64, data.yMax - 118, 268, FONT_LARGE_BLACK);
    }
}

static void drawPanelButtons(const empire_city *city)
{
    Widget::Button::drawImageButtons(data.xMin + 20, data.yMax - 44, imageButtonHelp, 1);
    Widget::Button::drawImageButtons(data.xMax - 44, data.yMax - 44, imageButtonReturnToCity, 1);
    Widget::Button::drawImageButtons(data.xMax - 44, data.yMax - 100, imageButtonAdvisor, 1);
    if (city)
    {
        if (city->type == EMPIRE_CITY_TRADE && !city->is_open)
        {
            Widget::Panel::drawButtonBorder((data.xMin + data.xMax - 500) / 2 + 50, data.yMax - 40, 400, 20, data.selectedButton);
        }
    }
}

static void drawEmpireObject(const empire_object *obj)
{
    if (obj->type == EMPIRE_OBJECT_LAND_TRADE_ROUTE || obj->type == EMPIRE_OBJECT_SEA_TRADE_ROUTE)
    {
        if (!empire_city_is_trade_route_open(obj->trade_route_id))
        {
            return;
        }
    }
    int x, y, image_id;
    if (scenario_empire_is_expanded)
    {
        x = obj->expanded.x;
        y = obj->expanded.y;
        image_id = obj->expanded.image_id;
    }
    else
    {
        x = obj->x;
        y = obj->y;
        image_id = obj->image_id;
    }

    if (obj->type == EMPIRE_OBJECT_CITY)
    {
        const empire_city *city = empire_city_get(empire_city_get_for_object(obj->id));
        if (city->type == EMPIRE_CITY_DISTANT_FOREIGN ||
                city->type == EMPIRE_CITY_FUTURE_ROMAN)
        {
            image_id = image_group(GROUP_EMPIRE_FOREIGN_CITY);
        }
    }
    if (obj->type == EMPIRE_OBJECT_BATTLE_ICON)
    {
        // handled later
        return;
    }
    if (obj->type == EMPIRE_OBJECT_ENEMY_ARMY)
    {
        if (Data_CityInfo.distantBattleMonthsToBattle <= 0)
        {
            return;
        }
        if (Data_CityInfo.distantBattleEnemyMonthsTraveled != obj->distant_battle_travel_months)
        {
            return;
        }
    }
    if (obj->type == EMPIRE_OBJECT_ROMAN_ARMY)
    {
        if (Data_CityInfo.distantBattleRomanMonthsToTravel <= 0 &&
                Data_CityInfo.distantBattleRomanMonthsToReturn <= 0)
        {
            return;
        }
        if (Data_CityInfo.distantBattleRomanMonthsTraveled != obj->distant_battle_travel_months)
        {
            return;
        }
    }
    Graphics_drawImage(image_id, data.xDrawOffset + x, data.yDrawOffset + y);
    const image *img = image_get(image_id);
    if (img->animation_speed_id)
    {
        int new_animation = Animation::getIndexForEmpireMap(image_id, obj->animation_index);
        empire_object_update_animation(obj->id, new_animation);
        Graphics_drawImage(image_id + new_animation,
                           data.xDrawOffset + x + img->sprite_offset_x,
                           data.yDrawOffset + y + img->sprite_offset_y);
    }
}

static void drawEmpireMap()
{
    Graphics_setClipRectangle(data.xMin + 16, data.yMin + 16, data.xMax - data.xMin - 32, data.yMax - data.yMin - 136);

    empire_set_viewport(data.xMax - data.xMin - 32, data.yMax - data.yMin - 136);

    data.xDrawOffset = data.xMin + 16;
    data.yDrawOffset = data.yMin + 16;
    empire_adjust_scroll(&data.xDrawOffset, &data.yDrawOffset);
    Graphics_drawImage(image_group(GROUP_EMPIRE_MAP), data.xDrawOffset, data.yDrawOffset);

    empire_object_foreach(drawEmpireObject);

    scenario_invasion_foreach_warning(draw_invasion_warning);
    Graphics_resetClipRectangle();
}

static void determineSelectedObject(const mouse *m)
{
    if (!m->left.went_down)
    {
        return;
    }
    if (m->x < data.xMin + 16 || m->x >= data.xMax - 16 ||
            m->y < data.yMin + 16 || m->y >= data.yMax - 120)
    {
        return;
    }
    empire_select_object(m->x - data.xMin - 16, m->y - data.yMin - 16);
    UI_Window_requestRefresh();
}

void UI_Empire_handleMouse(const mouse *m)
{
    empire_scroll_map(scroll_get_direction(m));
    data.focusButtonId = 0;
    int buttonId;
    Widget::Button::handleImageButtons(data.xMin + 20, data.yMax - 44, imageButtonHelp, 1, &buttonId);
    if (buttonId)
    {
        data.focusButtonId = 1;
    }
    Widget::Button::handleImageButtons(data.xMax - 44, data.yMax - 44, imageButtonReturnToCity, 1, &buttonId);
    if (buttonId)
    {
        data.focusButtonId = 2;
    }
    Widget::Button::handleImageButtons(data.xMax - 44, data.yMax - 100, imageButtonAdvisor, 1, &buttonId);
    if (buttonId)
    {
        data.focusButtonId = 3;
    }
    if (data.focusButtonId)
    {
        return;
    }
    determineSelectedObject(m);
    if (m->right.went_down)
    {
        empire_clear_selected_object();
        UI_Window_requestRefresh();
    }
    int selectedObject = empire_selected_object();
    if (selectedObject)
    {
        if (empire_object_get(selectedObject-1)->type == EMPIRE_OBJECT_CITY)
        {
            data.selectedCity = empire_city_get_for_object(selectedObject-1);
            const empire_city *city = empire_city_get(data.selectedCity);
            if (city->type == EMPIRE_CITY_TRADE && !city->is_open)
            {
                Widget::Button::handleCustomButtons((data.xMin + data.xMax - 500) / 2, data.yMax - 105, customButtonOpenTrade, 1, &data.selectedButton);
            }
        }
    }
}

static int isMouseHit(struct TooltipContext *c, int x, int y, int size)
{
    int mx = c->mouse_x;
    int my = c->mouse_y;
    return x <= mx && mx < x + size && y <= my && my < y + size;
}

static int getTooltipResource(struct TooltipContext *c)
{
    const empire_city *city = empire_city_get(data.selectedCity);
    if (city->type != EMPIRE_CITY_TRADE)
    {
        return 0;
    }
    int objectId = empire_selected_object() - 1;
    int xOffset = (data.xMin + data.xMax - 500) / 2;
    int yOffset = data.yMax - 108;

    if (city->is_open)
    {
        for (int r = 1, index = 0; r <= 15; r++)
        {
            if (empire_object_city_sells_resource(objectId, r))
            {
                if (isMouseHit(c, xOffset + 120 + 100 * index, yOffset + 21, 26))
                {
                    return r;
                }
                index++;
            }
        }
        for (int r = 1, index = 0; r <= 15; r++)
        {
            if (empire_object_city_buys_resource(objectId, r))
            {
                if (isMouseHit(c, xOffset + 120 + 100 * index, yOffset + 51, 26))
                {
                    return r;
                }
                index++;
            }
        }
    }
    else
    {
        int itemOffset = Widget_GameText_getDrawWidth(47, 5, FONT_NORMAL_GREEN);
        for (int r = 1; r <= 15; r++)
        {
            if (empire_object_city_sells_resource(objectId, r))
            {
                if (isMouseHit(c, xOffset + 60 + itemOffset, yOffset + 35, 26))
                {
                    return r;
                }
                itemOffset += 32;
            }
        }
        itemOffset += Widget_GameText_getDrawWidth(47, 4, FONT_NORMAL_GREEN);
        for (int r = 1; r <= 15; r++)
        {
            if (empire_object_city_buys_resource(objectId, r))
            {
                if (isMouseHit(c, xOffset + 110 + itemOffset, yOffset + 35, 26))
                {
                    return r;
                }
                itemOffset += 32;
            }
        }
    }
    return 0;
}

void UI_EmpireMap_getTooltip(struct TooltipContext *c)
{
    int resource = getTooltipResource(c);
    if (resource)
    {
        c->type = TooltipType_Button;
        c->textId = 131 + resource;
    }
    else if (data.focusButtonId)
    {
        c->type = TooltipType_Button;
        switch (data.focusButtonId)
        {
        case 1:
            c->textId = 1;
            break;
        case 2:
            c->textId = 2;
            break;
        case 3:
            c->textId = 69;
            break;
        }
    }
}

static void buttonHelp(int param1, int param2)
{
    UI_MessageDialog_show(MessageDialog_EmpireMap, 1);
}

static void buttonReturnToCity(int param1, int param2)
{
    UI_Window_goTo(Window_City);
}

static void buttonAdvisor(int advisor, int param2)
{
    UI_Advisors_goToFromMessage(Advisor_Trade);
}

static void buttonOpenTrade(int param1, int param2)
{
    UI_PopupDialog_show(PopupDialog_OpenTrade, confirmOpenTrade, 2);
}

static void buttonEmpireMap(int param1, int param2)
{
    UI_Window_goTo(Window_Empire);
}

static void confirmOpenTrade(int accepted)
{
    if (accepted)
    {
        empire_city *city = empire_city_get(data.selectedCity);
        CityInfo_Finance_spendOnConstruction(city->cost_to_open);
        city->is_open = 1;
        SidebarMenu_enableBuildingMenuItemsAndButtons();
        UI_Window_goTo(Window_TradeOpenedDialog);
    }
}

void UI_TradeOpenedDialog_drawBackground()
{
    int xOffset = Data_Screen.offset640x480.x;
    int yOffset = Data_Screen.offset640x480.y;
    Widget::Panel::drawOuterPanel(xOffset + 80, yOffset + 64, 30, 14);
    Widget_GameText_drawCentered(142, 0, xOffset + 80, yOffset + 80, 480, FONT_LARGE_BLACK);
    if (empire_city_get(data.selectedCity)->is_sea_trade)
    {
        Widget_GameText_drawMultiline(142, 1, xOffset + 112, yOffset + 120, 416, FONT_NORMAL_BLACK);
        Widget_GameText_drawMultiline(142, 3, xOffset + 112, yOffset + 184, 416, FONT_NORMAL_BLACK);
    }
    else
    {
        Widget_GameText_drawMultiline(142, 1, xOffset + 112, yOffset + 152, 416, FONT_NORMAL_BLACK);
    }
    Widget_GameText_draw(142, 2, xOffset + 128, yOffset + 256, FONT_NORMAL_BLACK);
}

void UI_TradeOpenedDialog_drawForeground()
{
    Widget::Button::drawImageButtons(
        Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
        imageButtonsTradeOpened, 2);
}

void UI_TradeOpenedDialog_handleMouse(const mouse *m)
{
    Widget::Button::handleImageButtons(
        Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
        imageButtonsTradeOpened, 2, 0);
}
