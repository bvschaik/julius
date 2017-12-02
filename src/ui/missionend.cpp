#include "cityinfo.h"
#include "graphics.h"

#include <sound>
#include <data>
#include <ui>
#include <scenario>

#include "game/game_settings.h"
#include "game/settings.h"
#include "graphics/image.h"

static void victoryAccept(int param1, int param2);
static void victoryContinueGoverning(int param1, int param2);
static void firedAccept(int param1, int param2);

static CustomButton victoryButtons[] =
{
    {32, 112, 416, 132, CustomButton_OnMouseUp, victoryAccept, Widget::Button::doNothing, 0, 0},
    {32, 144, 416, 164, CustomButton_Immediate, victoryContinueGoverning, Widget::Button::doNothing, 1, 0},
    {32, 176, 416, 196, CustomButton_Immediate, victoryContinueGoverning, Widget::Button::doNothing, 2, 0},
};
static CustomButton firedButtons[] =
{
    {64, 208, 384, 228, CustomButton_Immediate, firedAccept, Widget::Button::doNothing, 0, 0},
};

static int focusButtonId = 0;

void UI_VictoryIntermezzo_init()
{
    sound_music_reset();
    UI_Intermezzo_show(Intermezzo_Won, Window_MissionEnd, 1000);
}

void UI_MissionEnd_drawBackground()
{
    int xOffset = Data_Screen.offset640x480.x + 48;
    int yOffset = Data_Screen.offset640x480.y + 128;
    if (Data_State.winState != WinState_Win)
    {
        // lost mission
        Widget::Panel::drawOuterPanel(xOffset, yOffset - 112, 34, 16);
        Widget_GameText_drawCentered(62, 1, xOffset, yOffset - 96, 544, FONT_LARGE_BLACK);
        Widget_GameText_drawMultiline(62, 16, xOffset + 16, yOffset - 56, 496, FONT_NORMAL_BLACK);
        return;
    }
    // won mission
    Widget::Panel::drawOuterPanel(xOffset, yOffset, 34, 18);
    Widget_GameText_drawCentered(62, 0, xOffset, yOffset + 16, 544, FONT_LARGE_BLACK);

    Widget::Panel::drawInnerPanel(xOffset + 16, yOffset + 56, 32, 7);

    if (scenario_is_custom())
    {
        Widget_GameText_drawMultiline(147, 20, xOffset + 32, yOffset + 64, 496, FONT_NORMAL_WHITE);
    }
    else
    {
        Widget_GameText_drawMultiline(147, scenario_campaign_mission(), xOffset + 32, yOffset + 64, 496, FONT_NORMAL_WHITE);
    }
    int width = Widget_GameText_draw(148, 0, xOffset + 40, yOffset + 180, FONT_NORMAL_BLACK);
    Widget::Text::drawNumber(Data_CityInfo.ratingCulture, '@', " ",
                             xOffset + 40 + width, yOffset + 180, FONT_NORMAL_BLACK);

    width = Widget_GameText_draw(148, 1, xOffset + 300, yOffset + 180, FONT_NORMAL_BLACK);
    Widget::Text::drawNumber(Data_CityInfo.ratingProsperity, '@', " ",
                             xOffset + 300 + width, yOffset + 180, FONT_NORMAL_BLACK);

    width = Widget_GameText_draw(148, 2, xOffset + 40, yOffset + 200, FONT_NORMAL_BLACK);
    Widget::Text::drawNumber(Data_CityInfo.ratingPeace, '@', " ",
                             xOffset + 40 + width, yOffset + 200, FONT_NORMAL_BLACK);

    width = Widget_GameText_draw(148, 3, xOffset + 300, yOffset + 200, FONT_NORMAL_BLACK);
    Widget::Text::drawNumber(Data_CityInfo.ratingFavor, '@', " ",
                             xOffset + 300 + width, yOffset + 200, FONT_NORMAL_BLACK);

    width = Widget_GameText_draw(148, 4, xOffset + 40, yOffset + 220, FONT_NORMAL_BLACK);
    Widget::Text::drawNumber(Data_CityInfo.population, '@', " ",
                             xOffset + 40 + width, yOffset + 220, FONT_NORMAL_BLACK);

    width = Widget_GameText_draw(148, 5, xOffset + 300, yOffset + 220, FONT_NORMAL_BLACK);
    Widget::Text::drawNumber(Data_CityInfo.treasury, '@', " ",
                             xOffset + 300 + width, yOffset + 220, FONT_NORMAL_BLACK);

    Widget_GameText_drawCentered(13, 1, xOffset + 16, yOffset + 260, 512, FONT_NORMAL_BLACK);
}

void UI_MissionEnd_drawForeground()
{
    if (Data_State.winState != WinState_Win)
    {
        UI_VictoryDialog_drawForeground();
    }
}

static void advanceToNextMission()
{
    // TODO move out of UI code
    setting_set_personal_savings_for_mission(scenario_campaign_rank() + 1, Data_CityInfo.personalSavings);
    scenario_set_campaign_rank(scenario_campaign_rank() + 1);

    Data_CityInfo.victoryHasWonScenario = 0;
    Data_CityInfo.victoryContinueMonths = 0;
    Data_CityInfo.victoryContinueMonthsChosen = 0;

    Data_State.undoAvailable = 0;
    Data_State.currentOverlay = 0;

    if (scenario_campaign_rank() >= 11 || scenario_is_custom())
    {
        UI_Window_goTo(Window_MainMenu);
        if (!scenario_is_custom())
        {
            setting_clear_personal_savings();
            scenario_settings_init();
            scenario_set_campaign_rank(2);
        }
    }
    else
    {
        scenario_set_campaign_mission(Constant_MissionIds[scenario_campaign_rank()].peaceful);
        UI_MissionStart_show();
    }
}

void UI_MissionEnd_handleMouse(const mouse *m)
{
    if (Data_State.winState == WinState_Win)
    {
        if (m->right.went_up)
        {
            sound_music_stop();
            sound_speech_stop();
            advanceToNextMission();
        }
    }
    else
    {
        int xOffset = Data_Screen.offset640x480.x + 48;
        int yOffset = Data_Screen.offset640x480.y + 128;

        Widget::Button::handleCustomButtons(xOffset, yOffset - 112,
                                            firedButtons, 1, &focusButtonId);
    }
}

void UI_VictoryDialog_drawBackground()
{
    int xOffset = Data_Screen.offset640x480.x + 48;
    int yOffset = Data_Screen.offset640x480.y + 128;

    Widget::Panel::drawOuterPanel(xOffset, yOffset, 34, 15);
    if (scenario_campaign_rank() < 10 || scenario_is_custom())
    {
        Widget_GameText_drawCentered(62, 0, xOffset, yOffset + 16, 544, FONT_LARGE_BLACK);
        Widget_GameText_drawCentered(62, 2, xOffset, yOffset + 47, 544, FONT_NORMAL_BLACK);
        Widget_GameText_drawCentered(32, Data_CityInfo.playerRank + 1, xOffset, yOffset + 66, 544, FONT_LARGE_BLACK);
    }
    else
    {
        Widget::Text::drawCentered(scenario_player_name(), xOffset, yOffset + 16, 512, FONT_LARGE_BLACK, 0);
        Widget_GameText_drawMultiline(62, 26, xOffset + 16, yOffset + 47, 480, FONT_NORMAL_BLACK);
    }
}

void UI_VictoryDialog_drawForeground()
{
    int xOffset = Data_Screen.offset640x480.x + 48;
    int yOffset = Data_Screen.offset640x480.y + 128;

    if (Data_State.winState == WinState_Win)
    {
        Widget::Panel::drawLargeLabelButton(xOffset + 32, yOffset + 112, 30, focusButtonId == 1);
        if (scenario_campaign_rank() < 10 || scenario_is_custom())
        {
            Widget_GameText_drawCentered(62, 3,
                                         xOffset + 32, yOffset + 118, 480, FONT_NORMAL_GREEN);
        }
        else
        {
            Widget_GameText_drawCentered(62, 27,
                                         xOffset + 32, yOffset + 118, 480, FONT_NORMAL_GREEN);
        }
        if (scenario_campaign_rank() >= 2 || scenario_is_custom())
        {
            // Continue for 2/5 years
            Widget::Panel::drawLargeLabelButton(xOffset + 32, yOffset + 144, 30, focusButtonId == 2);
            Widget_GameText_drawCentered(62, 4, xOffset + 32, yOffset + 150, 480, FONT_NORMAL_GREEN);

            Widget::Panel::drawLargeLabelButton(xOffset + 32, yOffset + 176, 30, focusButtonId == 3);
            Widget_GameText_drawCentered(62, 5, xOffset + 32, yOffset + 182, 480, FONT_NORMAL_GREEN);
        }
    }
    else
    {
        // lost
        Widget::Panel::drawLargeLabelButton(xOffset + 32, yOffset + 96, 30, focusButtonId == 1);
        Widget_GameText_drawCentered(62, 6, xOffset + 32, yOffset + 102, 480, FONT_NORMAL_GREEN);
    }
}

void UI_VictoryDialog_handleMouse(const mouse *m)
{
    int xOffset = Data_Screen.offset640x480.x + 48;
    int yOffset = Data_Screen.offset640x480.y + 128;

    int numButtons;
    if (scenario_campaign_rank() >= 2 || scenario_is_custom())
    {
        numButtons = 3;
    }
    else
    {
        numButtons = 1;
    }
    Widget::Button::handleCustomButtons(xOffset, yOffset, victoryButtons, numButtons, &focusButtonId);
}

static void victoryAccept(int param1, int param2)
{
    UI_Window_goTo(Window_City);
}

static void victoryContinueGoverning(int duration, int param2)
{
    Data_CityInfo.victoryHasWonScenario = 1;
    if (duration == 1)
    {
        Data_CityInfo.victoryContinueMonths += 24;
        Data_CityInfo.victoryContinueMonthsChosen = 24;
        Data_CityInfo.salaryRank = 0;
        Data_CityInfo.salaryAmount = 0;
        CityInfo_Finance_updateSalary();
    }
    else if (duration == 2)
    {
        Data_CityInfo.victoryContinueMonths += 60;
        Data_CityInfo.victoryContinueMonthsChosen = 60;
        Data_CityInfo.salaryRank = 0;
        Data_CityInfo.salaryAmount = 0;
        CityInfo_Finance_updateSalary();
    }
    UI_Window_goTo(Window_City);
    Data_State.winState = WinState_None;
    Data_State.forceWinCheat = 0;
    sound_music_reset();
    sound_music_update();
}

static void firedAccept(int param1, int param2)
{
    Data_CityInfo.victoryHasWonScenario = 0;
    Data_CityInfo.victoryContinueMonths = 0;
    Data_CityInfo.victoryContinueMonthsChosen = 0;
    Data_State.undoAvailable = 0;
    if (scenario_is_custom())
    {
        UI_Window_goTo(Window_MainMenu);
    }
    else
    {
        UI_MissionStart_show();
    }
}
