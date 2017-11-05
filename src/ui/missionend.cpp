#include "allwindows.h"
#include "window.h"

#include "cityinfo.h"
#include "graphics.h"
#include "game/game_settings.h"
#include "sound.h"
#include "widget.h"

#include "data/cityinfo.hpp"
#include "data/constants.hpp"
#include "data/screen.hpp"
#include "data/settings.hpp"
#include "data/state.hpp"

#include "game/settings.h"
#include "graphics/image.h"

static void victoryAccept(int param1, int param2);
static void victoryContinueGoverning(int param1, int param2);
static void firedAccept(int param1, int param2);

static CustomButton victoryButtons[] =
{
    {32, 112, 416, 132, CustomButton_OnMouseUp, victoryAccept, Widget_Button_doNothing, 0, 0},
    {32, 144, 416, 164, CustomButton_Immediate, victoryContinueGoverning, Widget_Button_doNothing, 1, 0},
    {32, 176, 416, 196, CustomButton_Immediate, victoryContinueGoverning, Widget_Button_doNothing, 2, 0},
};
static CustomButton firedButtons[] =
{
    {64, 208, 384, 228, CustomButton_Immediate, firedAccept, Widget_Button_doNothing, 0, 0},
};

static int focusButtonId = 0;

void UI_VictoryIntermezzo_init()
{
    Sound_Music_reset();
    UI_Intermezzo_show(Intermezzo_Won, Window_MissionEnd, 1000);
}

void UI_MissionEnd_drawBackground()
{
    int xOffset = Data_Screen.offset640x480.x + 48;
    int yOffset = Data_Screen.offset640x480.y + 128;
    if (Data_State.winState != WinState_Win)
    {
        // lost mission
        Widget_Panel_drawOuterPanel(xOffset, yOffset - 112, 34, 16);
        Widget_GameText_drawCentered(62, 1, xOffset, yOffset - 96, 544, FONT_LARGE_BLACK);
        Widget_GameText_drawMultiline(62, 16, xOffset + 16, yOffset - 56, 496, FONT_NORMAL_BLACK);
        return;
    }
    // won mission
    Widget_Panel_drawOuterPanel(xOffset, yOffset, 34, 18);
    Widget_GameText_drawCentered(62, 0, xOffset, yOffset + 16, 544, FONT_LARGE_BLACK);

    Widget_Panel_drawInnerPanel(xOffset + 16, yOffset + 56, 32, 7);

    if (Data_Settings.isCustomScenario)
    {
        Widget_GameText_drawMultiline(147, 20, xOffset + 32, yOffset + 64, 496, FONT_NORMAL_WHITE);
    }
    else
    {
        Widget_GameText_drawMultiline(147, Data_Settings.saveGameMissionId, xOffset + 32, yOffset + 64, 496, FONT_NORMAL_WHITE);
    }
    int width = Widget_GameText_draw(148, 0, xOffset + 40, yOffset + 180, FONT_NORMAL_BLACK);
    Widget_Text_drawNumber(Data_CityInfo.ratingCulture, '@', " ",
                           xOffset + 40 + width, yOffset + 180, FONT_NORMAL_BLACK);

    width = Widget_GameText_draw(148, 1, xOffset + 300, yOffset + 180, FONT_NORMAL_BLACK);
    Widget_Text_drawNumber(Data_CityInfo.ratingProsperity, '@', " ",
                           xOffset + 300 + width, yOffset + 180, FONT_NORMAL_BLACK);

    width = Widget_GameText_draw(148, 2, xOffset + 40, yOffset + 200, FONT_NORMAL_BLACK);
    Widget_Text_drawNumber(Data_CityInfo.ratingPeace, '@', " ",
                           xOffset + 40 + width, yOffset + 200, FONT_NORMAL_BLACK);

    width = Widget_GameText_draw(148, 3, xOffset + 300, yOffset + 200, FONT_NORMAL_BLACK);
    Widget_Text_drawNumber(Data_CityInfo.ratingFavor, '@', " ",
                           xOffset + 300 + width, yOffset + 200, FONT_NORMAL_BLACK);

    width = Widget_GameText_draw(148, 4, xOffset + 40, yOffset + 220, FONT_NORMAL_BLACK);
    Widget_Text_drawNumber(Data_CityInfo.population, '@', " ",
                           xOffset + 40 + width, yOffset + 220, FONT_NORMAL_BLACK);

    width = Widget_GameText_draw(148, 5, xOffset + 300, yOffset + 220, FONT_NORMAL_BLACK);
    Widget_Text_drawNumber(Data_CityInfo.treasury, '@', " ",
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
    Data_Settings.startingFavor = Data_CityInfo.ratingFavor;
    Data_Settings.personalSavingsLastMission = Data_CityInfo.personalSavings;
    setting_set_personal_savings_for_mission(Data_Settings.currentMissionId + 1, Data_CityInfo.personalSavings);
    Data_Settings.currentMissionId++;

    Data_CityInfo.victoryHasWonScenario = 0;
    Data_CityInfo.victoryContinueMonths = 0;
    Data_CityInfo.victoryContinueMonthsChosen = 0;

    Data_State.undoAvailable = 0;
    Data_State.currentOverlay = 0;

    if (Data_Settings.currentMissionId >= 11 || Data_Settings.isCustomScenario)
    {
        UI_Window_goTo(Window_MainMenu);
        if (!Data_Settings.isCustomScenario)
        {
            Settings_clearMissionSettings();
            Data_Settings.currentMissionId = 2;
        }
    }
    else
    {
        Data_Settings.saveGameMissionId = Constant_MissionIds[Data_Settings.currentMissionId].peaceful;
        UI_MissionStart_show();
    }
}

void UI_MissionEnd_handleMouse(const mouse *m)
{
    if (Data_State.winState == WinState_Win)
    {
        if (m->right.went_up)
        {
            Sound_stopMusic();
            Sound_stopSpeech();
            advanceToNextMission();
        }
    }
    else
    {
        int xOffset = Data_Screen.offset640x480.x + 48;
        int yOffset = Data_Screen.offset640x480.y + 128;

        Widget_Button_handleCustomButtons(xOffset, yOffset - 112,
                                          firedButtons, 1, &focusButtonId);
    }
}

void UI_VictoryDialog_drawBackground()
{
    int xOffset = Data_Screen.offset640x480.x + 48;
    int yOffset = Data_Screen.offset640x480.y + 128;

    Widget_Panel_drawOuterPanel(xOffset, yOffset, 34, 15);
    if (Data_Settings.currentMissionId < 10 || Data_Settings.isCustomScenario)
    {
        Widget_GameText_drawCentered(62, 0, xOffset, yOffset + 16, 544, FONT_LARGE_BLACK);
        Widget_GameText_drawCentered(62, 2, xOffset, yOffset + 47, 544, FONT_NORMAL_BLACK);
        Widget_GameText_drawCentered(32, Data_CityInfo.playerRank + 1, xOffset, yOffset + 66, 544, FONT_LARGE_BLACK);
    }
    else
    {
        Widget_Text_drawCentered(Data_Settings.playerName, xOffset, yOffset + 16, 512, FONT_LARGE_BLACK, 0);
        Widget_GameText_drawMultiline(62, 26, xOffset + 16, yOffset + 47, 480, FONT_NORMAL_BLACK);
    }
}

void UI_VictoryDialog_drawForeground()
{
    int xOffset = Data_Screen.offset640x480.x + 48;
    int yOffset = Data_Screen.offset640x480.y + 128;

    if (Data_State.winState == WinState_Win)
    {
        Widget_Panel_drawLargeLabelButton(xOffset + 32, yOffset + 112, 30, focusButtonId == 1);
        if (Data_Settings.currentMissionId < 10 || Data_Settings.isCustomScenario)
        {
            Widget_GameText_drawCentered(62, 3,
                                         xOffset + 32, yOffset + 118, 480, FONT_NORMAL_GREEN);
        }
        else
        {
            Widget_GameText_drawCentered(62, 27,
                                         xOffset + 32, yOffset + 118, 480, FONT_NORMAL_GREEN);
        }
        if (Data_Settings.currentMissionId >= 2 || Data_Settings.isCustomScenario)
        {
            // Continue for 2/5 years
            Widget_Panel_drawLargeLabelButton(xOffset + 32, yOffset + 144, 30, focusButtonId == 2);
            Widget_GameText_drawCentered(62, 4, xOffset + 32, yOffset + 150, 480, FONT_NORMAL_GREEN);

            Widget_Panel_drawLargeLabelButton(xOffset + 32, yOffset + 176, 30, focusButtonId == 3);
            Widget_GameText_drawCentered(62, 5, xOffset + 32, yOffset + 182, 480, FONT_NORMAL_GREEN);
        }
    }
    else
    {
        // lost
        Widget_Panel_drawLargeLabelButton(xOffset + 32, yOffset + 96, 30, focusButtonId == 1);
        Widget_GameText_drawCentered(62, 6, xOffset + 32, yOffset + 102, 480, FONT_NORMAL_GREEN);
    }
}

void UI_VictoryDialog_handleMouse(const mouse *m)
{
    int xOffset = Data_Screen.offset640x480.x + 48;
    int yOffset = Data_Screen.offset640x480.y + 128;

    int numButtons;
    if (Data_Settings.currentMissionId >= 2 || Data_Settings.isCustomScenario)
    {
        numButtons = 3;
    }
    else
    {
        numButtons = 1;
    }
    Widget_Button_handleCustomButtons(xOffset, yOffset, victoryButtons, numButtons, &focusButtonId);
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
    Sound_Music_reset();
    Sound_Music_update();
}

static void firedAccept(int param1, int param2)
{
    Data_CityInfo.victoryHasWonScenario = 0;
    Data_CityInfo.victoryContinueMonths = 0;
    Data_CityInfo.victoryContinueMonthsChosen = 0;
    Data_State.undoAvailable = 0;
    if (Data_Settings.isCustomScenario)
    {
        UI_Window_goTo(Window_MainMenu);
    }
    else
    {
        UI_MissionStart_show();
    }
}
