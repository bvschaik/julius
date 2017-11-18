#include "allwindows.h"
#include "citybuildings.h"
#include "sidebar.h"
#include "topmenu.h"
#include "window.h"
#include "widget_text.h"

#include "graphics.h"
#include "city/message.h"
#include "widget.h"

#include "game/time.h"
#include "graphics/image.h"

#include <scenario>
#include <data>

void UI_City_drawBackground()
{
    //Graphics_clearScreen();
    UI_Sidebar_drawBackground();
    UI_TopMenu_drawBackground();
}

void UI_City_drawForeground()
{
    UI_TopMenu_drawBackgroundIfNecessary();
    UI_City_drawCity();
    UI_Sidebar_drawForeground();
    UI_City_drawPausedAndTimeLeft();
    UI_CityBuildings_drawBuildingCost();
    if (UI_Window_getId() == Window_City)
    {
        city_message_process_queue();
    }
}

void UI_City_drawForegroundMilitary()
{
    UI_TopMenu_drawBackgroundIfNecessary();
    UI_City_drawCity();
    UI_Sidebar_drawMinimap(0);
    UI_City_drawPausedAndTimeLeft();
}

void UI_City_drawCity()
{
    UI_CityBuildings_drawForeground(
        Data_Settings_Map.camera.x, Data_Settings_Map.camera.y);
}

void UI_City_drawPausedAndTimeLeft()
{
    if (scenario_criteria_time_limit_enabled())
    {
        int years;
        if (Data_Event.timeLimitMaxGameYear <= game_time_year() + 1)
        {
            years = 0;
        }
        else
        {
            years = Data_Event.timeLimitMaxGameYear - game_time_year() - 1;
        }
        int totalMonths = 12 - game_time_month() + 12 * years;
        Widget_Panel_drawSmallLabelButton(1, 25, 15, 1);
        int width = Widget_GameText_draw(6, 2, 6, 29, FONT_NORMAL_BLACK);
        Widget::Text::drawNumber(totalMonths, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK);
    }
    else if (scenario_criteria_survival_enabled())
    {
        int years;
        if (Data_Event.timeLimitMaxGameYear <= game_time_year() + 1)
        {
            years = 0;
        }
        else
        {
            years = Data_Event.timeLimitMaxGameYear - game_time_year() - 1;
        }
        int totalMonths = 12 - game_time_month() + 12 * years;
        Widget_Panel_drawSmallLabelButton(1, 25, 15, 1);
        int width = Widget_GameText_draw(6, 3, 6, 29, FONT_NORMAL_BLACK);
        Widget::Text::drawNumber(totalMonths, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK);
    }
    if (Data_Settings.gamePaused)
    {
        int width = Data_CityView.widthInPixels;
        Widget_Panel_drawOuterPanel((width - 448) / 2, 40, 28, 3);
        Widget_GameText_drawCentered(13, 2,
                                     (width - 448) / 2, 58, 448, FONT_NORMAL_BLACK);
    }
}

void UI_City_handleMouse(const mouse *m)
{
    if (UI_TopMenu_handleMouseWidget(m))
    {
        return;
    }
    if (UI_Sidebar_handleMouse(m))
    {
        return;
    }
    UI_CityBuildings_handleMouse(m);
}

void UI_City_getTooltip(struct TooltipContext *c)
{
    int textId = UI_TopMenu_getTooltipText(c);
    if (!textId)
    {
        textId = UI_Sidebar_getTooltipText();
    }
    if (textId)
    {
        c->type = TooltipType_Button;
        c->textId = textId;
        return;
    }
    UI_CityBuildings_getTooltip(c);
}

void UI_City_handleMouseMilitary(const mouse *m)
{
    UI_CityBuildings_handleMouseMilitary(m);
}
