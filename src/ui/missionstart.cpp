
#include "allwindows.h"
#include "window.h"

#include "graphics.h"
#include "scenario.h"

#include <data>
#include <sound>
#include <ui>
#include <game>

#include "core/lang.h"
#include "scenario/criteria.h"

static void startMission(int param1, int param2);
static void briefingBack(int param1, int param2);

static const int backgroundGraphicOffset[] =
{
    0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0
};
static const int campaignHasChoice[] =
{
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
struct CampaignSelection
{
    int xPeaceful;
    int yPeaceful;
    int xMilitary;
    int yMilitary;
};
static const struct CampaignSelection campaignSelection[12] =
{
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {292, 182, 353, 232},
    {118, 202, 324, 286},
    {549, 285, 224, 121},
    {173, 109, 240, 292},
    {576, 283, 19, 316},
    {97, 240, 156, 59},
    {127, 300, 579, 327},
    {103, 35, 410, 109},
    {191, 153, 86, 8},
    {200, 300, 400, 300},
};

static const int goalOffsetsX[] = {32, 288, 32, 288, 288, 288};
static const int goalOffsetsY[] = {95, 95, 117, 117, 73, 135};

static ImageButton imageButtonStartMission =
{
    0, 0, 27, 27, ImageButton_Normal, 92, 56, startMission, Widget::Button::doNothing, 1, 0, 1
};
static ImageButton imageButtonBackToSelection =
{
    0, 0, 31, 20, ImageButton_Normal, 90, 8, briefingBack, Widget::Button::doNothing, 0, 0, 1
};

static int focusButton = 0;

static struct
{
    int choice;
} data;

void UI_MissionStart_show()
{
    int select = 1;
    if (UI_Window_getId() == Window_MissionSelection)
    {
        select = 0;
    }
    if (!campaignHasChoice[Data_Settings.currentMissionId])
    {
        select = 0;
    }
    if (select)
    {
        data.choice = 0;
        UI_Window_goTo(Window_MissionSelection);
    }
    else
    {
        UI_Intermezzo_show(Intermezzo_MissionBriefing, Window_MissionBriefingInitial, 1000);
        Data_State.missionBriefingShown = 0;
    }
}

void UI_MissionStart_Selection_drawBackground()
{
    int xOffset = Data_Screen.offset640x480.x;
    int yOffset = Data_Screen.offset640x480.y;
    int missionId = Data_Settings.currentMissionId;

    Graphics_drawFullScreenImage(image_group(ID_Graphic_SelectMissionBackground));
    Graphics_drawImage(image_group(ID_Graphic_SelectMission) +
                       backgroundGraphicOffset[missionId],
                       xOffset, yOffset);
    Widget_GameText_draw(144, 1 + 3 * missionId, xOffset + 20, yOffset + 410, FONT_LARGE_BLACK);
    if (data.choice)
    {
        Widget_GameText_drawMultiline(144, 1 + 3 * missionId + data.choice,
                                      xOffset + 20, yOffset + 440, 560, FONT_NORMAL_BLACK);
    }
    else
    {
        Widget_GameText_drawMultiline(144, 0,
                                      xOffset + 20, yOffset + 440, 560, FONT_NORMAL_BLACK);
    }
}

static int isMouseHit(const mouse *m, int x, int y, int size)
{
    return
        x <= m->x && m->x < x + size &&
        y <= m->y && m->y < y + size;
}

void UI_MissionStart_Selection_drawForeground()
{
    int xOffset = Data_Screen.offset640x480.x;
    int yOffset = Data_Screen.offset640x480.y;

    if (data.choice > 0)
    {
        Widget::Button::drawImageButtons(xOffset + 580, yOffset + 410, &imageButtonStartMission, 1);
    }

    int missionId = Data_Settings.currentMissionId;
    int xPeaceful = xOffset + campaignSelection[missionId].xPeaceful - 4;
    int yPeaceful = yOffset + campaignSelection[missionId].yPeaceful - 4;
    int xMilitary = xOffset + campaignSelection[missionId].xMilitary - 4;
    int yMilitary = yOffset + campaignSelection[missionId].yMilitary - 4;
    int graphicId = image_group(ID_Graphic_SelectMissionButton);
    if (data.choice == 0)
    {
        Graphics_drawImage(focusButton == 1 ? graphicId + 1 : graphicId, xPeaceful, yPeaceful);
        Graphics_drawImage(focusButton == 2 ? graphicId + 1 : graphicId, xMilitary, yMilitary);
    }
    else if (data.choice == 1)
    {
        Graphics_drawImage(focusButton == 1 ? graphicId + 1 : graphicId + 2, xPeaceful, yPeaceful);
        Graphics_drawImage(focusButton == 2 ? graphicId + 1 : graphicId, xMilitary, yMilitary);
    }
    else
    {
        Graphics_drawImage(focusButton == 1 ? graphicId + 1 : graphicId, xPeaceful, yPeaceful);
        Graphics_drawImage(focusButton == 2 ? graphicId + 1 : graphicId + 2, xMilitary, yMilitary);
    }
}

void UI_MissionStart_Selection_handleMouse(const mouse *m)
{
    int xOffset = Data_Screen.offset640x480.x;
    int yOffset = Data_Screen.offset640x480.y;

    int missionId = Data_Settings.currentMissionId;
    int xPeaceful = xOffset + campaignSelection[missionId].xPeaceful - 4;
    int yPeaceful = yOffset + campaignSelection[missionId].yPeaceful - 4;
    int xMilitary = xOffset + campaignSelection[missionId].xMilitary - 4;
    int yMilitary = yOffset + campaignSelection[missionId].yMilitary - 4;
    focusButton = 0;
    if (isMouseHit(m, xPeaceful, yPeaceful, 44))
    {
        focusButton = 1;
    }
    if (isMouseHit(m, xMilitary, yMilitary, 44))
    {
        focusButton = 2;
    }

    if (m->right.went_up)
    {
        UI_MissionStart_show();
    }
    if (data.choice > 0)
    {
        if (Widget::Button::handleImageButtons(xOffset + 580, yOffset + 410, &imageButtonStartMission, 1, 0))
        {
            return;
        }
    }
    if (m->left.went_up)
    {
        if (isMouseHit(m, xPeaceful, yPeaceful, 44))
        {
            Data_Settings.saveGameMissionId = Constant_MissionIds[missionId].peaceful;
            data.choice = 1;
            UI_Window_requestRefresh();
            sound_speech_play_file("wavs/fanfare_nu1.wav");
        }
        if (isMouseHit(m, xMilitary, yMilitary, 44))
        {
            Data_Settings.saveGameMissionId = Constant_MissionIds[missionId].military;
            data.choice = 2;
            UI_Window_requestRefresh();
            sound_speech_play_file("wavs/fanfare_nu5.wav");
        }
    }
}

void UI_MissionStart_Briefing_init()
{
    Widget_RichText_reset(0);
}

void UI_MissionStart_Briefing_drawBackground()
{
    if (!Data_State.missionBriefingShown)
    {
        Data_State.missionBriefingShown = 1;
        Scenario_initialize(Data_FileList.selectedScenario);
        if (UI_Window_getId() == Window_City)
        {
            return;
        }
    }

    int textId = 200 + Data_Settings.saveGameMissionId;
    int xOffset = Data_Screen.offset640x480.x + 16;
    int yOffset = Data_Screen.offset640x480.y + 32;

    Widget::Panel::drawOuterPanel(xOffset, yOffset, 38, 27);
    Widget::Text::draw(lang_get_message(textId)->title.text, xOffset + 16, yOffset + 16, FONT_LARGE_BLACK, 0);
    Widget_GameText_draw(62, 7, xOffset + 360, yOffset + 401, FONT_NORMAL_BLACK);
    if (UI_Window_getId() == Window_MissionBriefingInitial && Data_Settings.currentMissionId >= 2)
    {
        Widget_GameText_draw(13, 4, xOffset + 50, yOffset + 403, FONT_NORMAL_BLACK);
    }

    Widget::Panel::drawInnerPanel(xOffset + 16, yOffset + 64, 33, 5);
    Widget_GameText_draw(62, 10, xOffset + 32, yOffset + 72, FONT_NORMAL_WHITE);
    int goalIndex = 0;
    if (scenario_criteria_population_enabled())
    {
        int x = goalOffsetsX[goalIndex];
        int y = goalOffsetsY[goalIndex];
        goalIndex++;
        Widget::Panel::drawSmallLabelButton(xOffset + x, yOffset + y, 15, 1);
        int width = Widget_GameText_draw(62, 11, xOffset + x + 8, yOffset + y + 3, FONT_NORMAL_RED);
        Widget::Text::drawNumber(scenario_criteria_population(), '@', " ",
                                 xOffset + x + 8 + width, yOffset + y + 3, FONT_NORMAL_RED);
    }
    if (scenario_criteria_culture_enabled())
    {
        int x = goalOffsetsX[goalIndex];
        int y = goalOffsetsY[goalIndex];
        goalIndex++;
        Widget::Panel::drawSmallLabelButton(xOffset + x, yOffset + y, 15, 1);
        int width = Widget_GameText_draw(62, 12, xOffset + x + 8, yOffset + y + 3, FONT_NORMAL_RED);
        Widget::Text::drawNumber(scenario_criteria_culture(), '@', " ",
                                 xOffset + x + 8 + width, yOffset + y + 3, FONT_NORMAL_RED);
    }
    if (scenario_criteria_prosperity_enabled())
    {
        int x = goalOffsetsX[goalIndex];
        int y = goalOffsetsY[goalIndex];
        goalIndex++;
        Widget::Panel::drawSmallLabelButton(xOffset + x, yOffset + y, 15, 1);
        int width = Widget_GameText_draw(62, 13, xOffset + x + 8, yOffset + y + 3, FONT_NORMAL_RED);
        Widget::Text::drawNumber(scenario_criteria_prosperity(), '@', " ",
                                 xOffset + x + 8 + width, yOffset + y + 3, FONT_NORMAL_RED);
    }
    if (scenario_criteria_peace_enabled())
    {
        int x = goalOffsetsX[goalIndex];
        int y = goalOffsetsY[goalIndex];
        goalIndex++;
        Widget::Panel::drawSmallLabelButton(xOffset + x, yOffset + y, 15, 1);
        int width = Widget_GameText_draw(62, 14, xOffset + x + 8, yOffset + y + 3, FONT_NORMAL_RED);
        Widget::Text::drawNumber(scenario_criteria_peace(), '@', " ",
                                 xOffset + x + 8 + width, yOffset + y + 3, FONT_NORMAL_RED);
    }
    if (scenario_criteria_favor_enabled())
    {
        int x = goalOffsetsX[goalIndex];
        int y = goalOffsetsY[goalIndex];
        goalIndex++;
        Widget::Panel::drawSmallLabelButton(xOffset + x, yOffset + y, 15, 1);
        int width = Widget_GameText_draw(62, 15, xOffset + x + 8, yOffset + y + 3, FONT_NORMAL_RED);
        Widget::Text::drawNumber(scenario_criteria_favor(), '@', " ",
                                 xOffset + x + 8 + width, yOffset + y + 3, FONT_NORMAL_RED);
    }
    int immediateGoalText = Tutorial::get_immediate_goal_text();
    if (immediateGoalText)
    {
        int x = goalOffsetsX[2];
        int y = goalOffsetsY[2];
        goalIndex++;
        Widget::Panel::drawSmallLabelButton(xOffset + x, yOffset + y, 31, 1);
        Widget_GameText_draw(62, immediateGoalText, xOffset + x + 8, yOffset + y + 3, FONT_NORMAL_RED);
    }

    Widget::Panel::drawInnerPanel(xOffset + 16, yOffset + 152, 33, 15);

    Widget_RichText_setFonts(FONT_NORMAL_WHITE, FONT_NORMAL_RED);
    Widget_RichText_init(lang_get_message(textId)->content.text,
                         xOffset + 48, yOffset + 152, 31, 15, 0);

    Graphics_setClipRectangle(xOffset + 19, yOffset + 155, 522, 234);
    Widget_RichText_draw(lang_get_message(textId)->content.text,
                         xOffset + 32, yOffset + 164, 496, 14, 0);
    Graphics_resetClipRectangle();
}

void UI_MissionStart_BriefingInitial_drawForeground()
{
    int xOffset = Data_Screen.offset640x480.x + 16;
    int yOffset = Data_Screen.offset640x480.y + 32;

    Widget_RichText_drawScrollbar();
    Widget::Button::drawImageButtons(xOffset + 500, yOffset + 394, &imageButtonStartMission, 1);
    if (Data_Settings.currentMissionId >= 2)
    {
        Widget::Button::drawImageButtons(xOffset + 10, yOffset + 396, &imageButtonBackToSelection, 1);
    }
}

void UI_MissionStart_BriefingReview_drawForeground()
{
    int xOffset = Data_Screen.offset640x480.x + 16;
    int yOffset = Data_Screen.offset640x480.y + 32;

    Widget_RichText_drawScrollbar();
    Widget::Button::drawImageButtons(xOffset + 500, yOffset + 394, &imageButtonStartMission, 1);
}

void UI_MissionStart_BriefingInitial_handleMouse(const mouse *m)
{
    int xOffset = Data_Screen.offset640x480.x + 16;
    int yOffset = Data_Screen.offset640x480.y + 32;

    if (Widget::Button::handleImageButtons(xOffset + 500, yOffset + 394, &imageButtonStartMission, 1, 0))
    {
        return;
    }
    if (Data_Settings.currentMissionId >= 2)
    {
        if (Widget::Button::handleImageButtons(xOffset + 10, yOffset + 396, &imageButtonBackToSelection, 1, 0))
        {
            return;
        }
    }
    Widget_RichText_handleScrollbar(m);
}

void UI_MissionStart_BriefingReview_handleMouse(const mouse *m)
{
    int xOffset = Data_Screen.offset640x480.x + 16;
    int yOffset = Data_Screen.offset640x480.y + 32;

    if (Widget::Button::handleImageButtons(xOffset + 500, yOffset + 394, &imageButtonStartMission, 1, 0))
    {
        return;
    }
    Widget_RichText_handleScrollbar(m);
}

static void briefingBack(int param1, int param2)
{
    if (UI_Window_getId() == Window_MissionBriefingInitial)
    {
        sound_speech_stop();
        UI_Window_goTo(Window_MissionSelection);
    }
}

static void startMission(int param1, int param2)
{
    Data_State.previousOverlay = 0;
    switch (UI_Window_getId())
    {
    case Window_NewCareerDialog:
    case Window_MissionSelection:
        UI_MissionStart_show();
        break;
    default:
        sound_speech_stop();
        sound_music_reset();
        UI_Window_goTo(Window_City);
        Data_CityInfo.missionSavedGameWritten = 0;
        break;
    }
    UI_Window_requestRefresh();
}
