#include "AllWindows.hpp"
#include "Window.hpp"
#include "Advisors_private.hpp"
#include "../CityInfo.hpp"
#include "../Data/Constants.hpp"

static void buttonCancel(int param1, int param2);
static void buttonSetSalary(int param1, int param2);

static CustomButton buttons[] =
{
    {240, 395, 400, 415, CustomButton_Immediate, buttonCancel, Widget_Button_doNothing, 0, 0},
    {144, 85, 432, 105, CustomButton_Immediate, buttonSetSalary, Widget_Button_doNothing, 0, 0},
    {144, 105, 432, 125, CustomButton_Immediate, buttonSetSalary, Widget_Button_doNothing, 1, 0},
    {144, 125, 432, 145, CustomButton_Immediate, buttonSetSalary, Widget_Button_doNothing, 2, 0},
    {144, 145, 432, 165, CustomButton_Immediate, buttonSetSalary, Widget_Button_doNothing, 3, 0},
    {144, 165, 432, 185, CustomButton_Immediate, buttonSetSalary, Widget_Button_doNothing, 4, 0},
    {144, 185, 432, 205, CustomButton_Immediate, buttonSetSalary, Widget_Button_doNothing, 5, 0},
    {144, 205, 432, 225, CustomButton_Immediate, buttonSetSalary, Widget_Button_doNothing, 6, 0},
    {144, 225, 432, 245, CustomButton_Immediate, buttonSetSalary, Widget_Button_doNothing, 7, 0},
    {144, 245, 432, 265, CustomButton_Immediate, buttonSetSalary, Widget_Button_doNothing, 8, 0},
    {144, 265, 432, 285, CustomButton_Immediate, buttonSetSalary, Widget_Button_doNothing, 9, 0},
    {144, 285, 432, 305, CustomButton_Immediate, buttonSetSalary, Widget_Button_doNothing, 10, 0},
};

static int focusButtonId;

void UI_SetSalaryDialog_drawBackground()
{
    UI_Advisor_drawGeneralBackground();
    UI_SetSalaryDialog_drawForeground();
}

void UI_SetSalaryDialog_drawForeground()
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    Widget_Panel_drawOuterPanel(baseOffsetX + 128, baseOffsetY + 32, 24, 25);
    Graphics_drawImage(image_group(ID_Graphic_ResourceIcons) + 16,
                       baseOffsetX + 144, baseOffsetY + 48);
    Widget_GameText_drawCentered(52, 15,
                                 baseOffsetX + 144, baseOffsetY + 48, 368, FONT_LARGE_BLACK);

    Widget_Panel_drawInnerPanel(baseOffsetX + 144, baseOffsetY + 80, 22, 15);

    for (int rank = 0; rank < 11; rank++)
    {
        font_t font = (focusButtonId == rank + 2) ? FONT_NORMAL_RED : FONT_NORMAL_WHITE;
        int width = Widget_GameText_draw(52, rank + 4,
                                         baseOffsetX + 176, baseOffsetY + 90 + 20 * rank, font);
        Widget_Text_drawMoney(Constant_SalaryForRank[rank],
                              baseOffsetX + 176 + width, baseOffsetY + 90 + 20 * rank, font);
    }

    if (Data_CityInfo.victoryContinueMonths <= 0)
    {
        if (Data_CityInfo.salaryRank <= Data_CityInfo.playerRank)
        {
            Widget_GameText_drawMultiline(52, 76,
                                          baseOffsetX + 152, baseOffsetY + 336, 336, FONT_NORMAL_BLACK);
        }
        else
        {
            Widget_GameText_drawMultiline(52, 71,
                                          baseOffsetX + 152, baseOffsetY + 336, 336, FONT_NORMAL_BLACK);
        }
    }
    else
    {
        Widget_GameText_drawMultiline(52, 77,
                                      baseOffsetX + 152, baseOffsetY + 336, 336, FONT_NORMAL_BLACK);
    }
    Widget_Panel_drawButtonBorder(baseOffsetX + 240, baseOffsetY + 395,
                                  160, 20, focusButtonId == 1);
    Widget_GameText_drawCentered(13, 4, baseOffsetX + 176, baseOffsetY + 400, 288, FONT_NORMAL_BLACK);
}

void UI_SetSalaryDialog_handleMouse(const mouse *m)
{
    if (m->right.went_up)
    {
        UI_Window_goTo(Window_Advisors);
    }
    else
    {
        int offsetX = Data_Screen.offset640x480.x;
        int offsetY = Data_Screen.offset640x480.y;
        Widget_Button_handleCustomButtons(offsetX, offsetY,
                                          buttons, 12, &focusButtonId);
    }
}

static void buttonCancel(int param1, int param2)
{
    UI_Window_goTo(Window_Advisors);
}

static void buttonSetSalary(int rank, int param2)
{
    if (Data_CityInfo.victoryContinueMonths <= 0)
    {
        Data_CityInfo.salaryRank = rank;
        Data_CityInfo.salaryAmount = Constant_SalaryForRank[rank];
        CityInfo_Finance_updateSalary();
        CityInfo_Ratings_updateFavorExplanation();
        UI_Window_goTo(Window_Advisors);
    }
}
