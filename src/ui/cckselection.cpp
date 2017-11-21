#include "window.h"

#include "sound/speech.h"

#include "gamefile.h"
#include "graphics.h"

#include <data>
#include <core>
#include <ui>
#include <scenario>

#include <string.h>

static void drawScenarioList();
static void drawScrollbarDot();
static void drawScenarioInfo();
static int handleScrollbarClick(const mouse *m);
static void buttonSelectItem(int param1, int param2);
static void buttonScroll(int param1, int param2);
static void buttonStartScenario(int param1, int param2);

static ImageButton imageButtons[] =
{
    {276, 210, 34, 34, ImageButton_Normal, 96, 8, buttonScroll, Widget::Button::doNothing, 0, 1, 1},
    {276, 440, 34, 34, ImageButton_Normal, 96, 12, buttonScroll, Widget::Button::doNothing, 1, 1, 1},
    {600, 440, 27, 27, ImageButton_Normal, 92, 56, buttonStartScenario, Widget::Button::doNothing, 1, 0, 1},
};

static CustomButton customButtons[] =
{
    {28, 220, 280, 236, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 0, 0},
    {28, 236, 280, 252, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 1, 0},
    {28, 252, 280, 268, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 2, 0},
    {28, 268, 280, 284, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 3, 0},
    {28, 284, 280, 300, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 4, 0},
    {28, 300, 280, 316, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 5, 0},
    {28, 316, 280, 332, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 6, 0},
    {28, 332, 280, 348, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 7, 0},
    {28, 348, 280, 364, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 8, 0},
    {28, 364, 280, 380, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 9, 0},
    {28, 380, 280, 396, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 10, 0},
    {28, 396, 280, 412, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 11, 0},
    {28, 412, 280, 428, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 12, 0},
    {28, 428, 280, 444, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 13, 0},
    {28, 444, 280, 460, CustomButton_Immediate, buttonSelectItem, Widget::Button::doNothing, 14, 0},
};

static int scrollPosition;
static int focusButtonId;
static int selectedItem;

static const dir_listing *scenarios;

void UI_CCKSelection_init()
{
    Data_Settings.isCustomScenario = 2;
    scenarios = dir_find_files_with_extension("map");
    scrollPosition = 0;
    focusButtonId = 0;
    buttonSelectItem(0, 0);
}

void UI_CCKSelection_drawBackground()
{
    Graphics_drawFullScreenImage(image_group(ID_Graphic_CCKBackground));
    Widget_Panel_drawInnerPanel(
        Data_Screen.offset640x480.x + 280,
        Data_Screen.offset640x480.y + 242, 2, 12);
    drawScenarioList();
    drawScrollbarDot();
    drawScenarioInfo();
}

static void drawScenarioList()
{
    Widget_Panel_drawInnerPanel(Data_Screen.offset640x480.x + 16,
                                Data_Screen.offset640x480.y + 210, 16, 16);
    for (int i = 0; i < 15; i++)
    {
        font_t font = FONT_NORMAL_GREEN;
        if (focusButtonId == i + 1)
        {
            font = FONT_NORMAL_WHITE;
        }
        else if (!focusButtonId && selectedItem == i + scrollPosition)
        {
            font = FONT_NORMAL_WHITE;
        }
        char file[FILENAME_LENGTH];
        strcpy(file, scenarios->files[i + scrollPosition]);
        file_remove_extension(file);
        Widget::Text::draw(file, Data_Screen.offset640x480.x + 24,
                           Data_Screen.offset640x480.y + 220 + 16 * i, font, 0);
    }
}

static void drawScrollbarDot()
{
    if (scenarios->num_files > 15)
    {
        int pct;
        if (scrollPosition <= 0)
        {
            pct = 0;
        }
        else if (scrollPosition + 15 >= scenarios->num_files)
        {
            pct = 100;
        }
        else
        {
            pct = calc_percentage(scrollPosition, scenarios->num_files - 15);
        }
        int yOffset = calc_adjust_with_percentage(164, pct);
        Graphics_drawImage(image_group(ID_Graphic_PanelButton) + 39,
                           Data_Screen.offset640x480.x + 284,
                           Data_Screen.offset640x480.y + 245 + yOffset);
    }
}

static void drawScenarioInfo()
{
    int baseOffsetX = Data_Screen.offset640x480.x + 320;
    int baseOffsetY = Data_Screen.offset640x480.y + 20;

    Graphics_drawImage(image_group(ID_Graphic_ScenarioImage) + scenario_image_id(),
                       Data_Screen.offset640x480.x + 78, Data_Screen.offset640x480.y + 36);

    Widget::Text::drawCentered(Data_FileList.selectedScenario,
                               baseOffsetX + 15, baseOffsetY + 5, 260, FONT_LARGE_BLACK, 0);
    Widget::Text::drawCentered(scenario_brief_description(),
                               baseOffsetX + 15, baseOffsetY + 40, 260, FONT_NORMAL_WHITE, 0);
    Widget_GameText_drawYear(scenario_property_start_year(),
                             baseOffsetX + 90, baseOffsetY + 70, FONT_LARGE_BLACK);
    Widget_GameText_drawCentered(44, 77 + scenario_property_climate(),
                                 baseOffsetX + 15, baseOffsetY + 130, 260, FONT_NORMAL_BLACK);

    // map size
    int textId;
    switch (Data_Scenario.mapSizeX)
    {
    case 40:
        textId = 121;
        break;
    case 60:
        textId = 122;
        break;
    case 80:
        textId = 123;
        break;
    case 100:
        textId = 124;
        break;
    case 120:
        textId = 125;
        break;
    default:
        textId = 126;
        break;
    }
    Widget_GameText_drawCentered(44, textId,
                                 baseOffsetX + 15, baseOffsetY + 150, 260, FONT_NORMAL_BLACK);

    // military
    int numInvasions = 0;
    for (int i = 0; i < 20; i++)
    {
        if (Data_Scenario.invasions.type[i])
        {
            numInvasions++;
        }
    }
    if (numInvasions <= 0)
    {
        textId = 112;
    }
    else if (numInvasions <= 2)
    {
        textId = 113;
    }
    else if (numInvasions <= 4)
    {
        textId = 114;
    }
    else if (numInvasions <= 10)
    {
        textId = 115;
    }
    else
    {
        textId = 116;
    }
    Widget_GameText_drawCentered(44, textId,
                                 baseOffsetX + 15, baseOffsetY + 170, 260, FONT_NORMAL_BLACK);

    Widget_GameText_drawCentered(32, 11 + scenario_property_player_rank(),
                                 baseOffsetX + 15, baseOffsetY + 190, 260, FONT_NORMAL_BLACK);
    if (scenario_is_open_play())
    {
        Widget_GameText_drawMultiline(145, scenario_open_play_id(),
                                      baseOffsetX + 25, baseOffsetY + 250, 260, FONT_NORMAL_BLACK);
    }
    else
    {
        Widget_GameText_drawCentered(44, 127,
                                     baseOffsetX + 15, baseOffsetY + 242, 260, FONT_NORMAL_BLACK);
        int width;
        if (scenario_criteria_culture_enabled())
        {
            width = Widget::Text::drawNumber(scenario_criteria_culture(), '@', " ",
                                             baseOffsetX + 90, baseOffsetY + 270, FONT_NORMAL_BLACK);
            Widget_GameText_draw(44, 129,
                                 baseOffsetX + 90 + width, baseOffsetY + 270, FONT_NORMAL_BLACK);
        }
        if (scenario_criteria_prosperity_enabled())
        {
            width = Widget::Text::drawNumber(scenario_criteria_prosperity(), '@', " ",
                                             baseOffsetX + 90, baseOffsetY + 286, FONT_NORMAL_BLACK);
            Widget_GameText_draw(44, 130,
                                 baseOffsetX + 90 + width, baseOffsetY + 286, FONT_NORMAL_BLACK);
        }
        if (scenario_criteria_peace_enabled())
        {
            width = Widget::Text::drawNumber(scenario_criteria_peace(), '@', " ",
                                             baseOffsetX + 90, baseOffsetY + 302, FONT_NORMAL_BLACK);
            Widget_GameText_draw(44, 131,
                                 baseOffsetX + 90 + width, baseOffsetY + 302, FONT_NORMAL_BLACK);
        }
        if (scenario_criteria_favor_enabled())
        {
            width = Widget::Text::drawNumber(scenario_criteria_favor(), '@', " ",
                                             baseOffsetX + 90, baseOffsetY + 318, FONT_NORMAL_BLACK);
            Widget_GameText_draw(44, 132,
                                 baseOffsetX + 90 + width, baseOffsetY + 318, FONT_NORMAL_BLACK);
        }
        if (scenario_criteria_population_enabled())
        {
            width = Widget::Text::drawNumber(scenario_criteria_population(), '@', " ",
                                             baseOffsetX + 90, baseOffsetY + 334, FONT_NORMAL_BLACK);
            Widget_GameText_draw(44, 133,
                                 baseOffsetX + 90 + width, baseOffsetY + 334, FONT_NORMAL_BLACK);
        }
        if (scenario_criteria_time_limit_enabled())
        {
            width = Widget::Text::drawNumber(scenario_criteria_time_limit_years(), '@', " ",
                                             baseOffsetX + 90, baseOffsetY + 350, FONT_NORMAL_BLACK);
            Widget_GameText_draw(44, 134,
                                 baseOffsetX + 90 + width, baseOffsetY + 350, FONT_NORMAL_BLACK);
        }
        if (scenario_criteria_survival_enabled())
        {
            width = Widget::Text::drawNumber(scenario_criteria_survival_years(), '@', " ",
                                             baseOffsetX + 90, baseOffsetY + 366, FONT_NORMAL_BLACK);
            Widget_GameText_draw(44, 135,
                                 baseOffsetX + 90 + width, baseOffsetY + 366, FONT_NORMAL_BLACK);
        }
    }
    Widget_GameText_draw(44, 136, baseOffsetX + 100, baseOffsetY + 426, FONT_NORMAL_BLACK);
}

void UI_CCKSelection_drawForeground()
{
    Widget::Button::drawImageButtons(Data_Screen.offset640x480.x, Data_Screen.offset640x480.y, imageButtons, 3);
    drawScenarioList();
}

void UI_CCKSelection_handleMouse(const mouse *m)
{
    if (m->scrolled == SCROLL_DOWN)
    {
        buttonScroll(1, 3);
    }
    else if (m->scrolled == SCROLL_UP)
    {
        buttonScroll(0, 3);
    }
    if (handleScrollbarClick(m))
    {
        return;
    }
    if (Widget::Button::handleImageButtons(
                Data_Screen.offset640x480.x, Data_Screen.offset640x480.y, imageButtons, 3, 0))
    {
        return;
    }
    Widget::Button::handleCustomButtons(
        Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
        customButtons, 15, &focusButtonId);
}

static int handleScrollbarClick(const mouse *m)
{
    if (scenarios->num_files <= 15)
    {
        return 0;
    }
    if (!m->left.is_down)
    {
        return 0;
    }
    int x = Data_Screen.offset640x480.x;
    int y = Data_Screen.offset640x480.y;
    if (m->x >= x + 280 && m->x <= x + 312 &&
            m->y >= y + 245 && m->y <= y + 434)
    {
        int yOffset = m->y - (y + 245);
        if (yOffset > 164)
        {
            yOffset = 164;
        }
        int pct = calc_percentage(yOffset, 164);
        scrollPosition = calc_adjust_with_percentage(scenarios->num_files - 15, pct);
        UI_Window_requestRefresh();
        return 1;
    }
    return 0;
}

static void buttonSelectItem(int index, int param2)
{
    if (index >= scenarios->num_files)
    {
        return;
    }
    selectedItem = scrollPosition + index;
    strcpy(Data_FileList.selectedScenario, scenarios->files[selectedItem]);
    GameFile_loadScenario(Data_FileList.selectedScenario);
    file_remove_extension(Data_FileList.selectedScenario);
    UI_Window_requestRefresh();
}

static void buttonScroll(int isDown, int numLines)
{
    if (scenarios->num_files > 15)
    {
        if (isDown)
        {
            scrollPosition += numLines;
            if (scrollPosition > scenarios->num_files - 15)
            {
                scrollPosition = scenarios->num_files - 15;
            }
        }
        else
        {
            scrollPosition -= numLines;
            if (scrollPosition < 0)
            {
                scrollPosition = 0;
            }
        }
    }
}

static void buttonStartScenario(int param1, int param2)
{
    sound_speech_stop();
    Scenario_initialize(Data_FileList.selectedScenario);
    UI_Window_goTo(Window_City);
}
