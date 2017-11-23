#include "advisors_private.h"
#include "window.h"
#include "cityinfo.h"

#include "core/calc.h"

#include <ui>

#define VAL(val,x,y) Widget::Text::drawNumber(val, '@', " ", baseOffsetX + x, baseOffsetY + y, FONT_NORMAL_BLACK)
#define ROW(tgr,tid,y,valLy,valTy) \
	Widget_GameText_draw(tgr, tid, baseOffsetX + 80, baseOffsetY + y, FONT_NORMAL_BLACK);\
	Widget::Text::drawNumber(valLy, '@', " ", baseOffsetX + 290, baseOffsetY + y, FONT_NORMAL_BLACK);\
	Widget::Text::drawNumber(valTy, '@', " ", baseOffsetX + 430, baseOffsetY + y, FONT_NORMAL_BLACK)

static void buttonChangeTaxes(int param1, int param2);

static ArrowButton arrowButtonsTaxes[] =
{
    {180, 70, 17, 24, buttonChangeTaxes, 1, 0},
    {204, 70, 15, 24, buttonChangeTaxes, 0, 0}
};

static int arrowButtonFocus;

void UI_Advisor_Financial_drawBackground(int *advisorHeight)
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    *advisorHeight = 26;
    Widget::Panel::drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
    Graphics_drawImage(image_group(ID_Graphic_AdvisorIcons) + 10, baseOffsetX + 10, baseOffsetY + 10);

    Widget_GameText_draw(60, 0, baseOffsetX + 60, baseOffsetY + 12, FONT_LARGE_BLACK);
    Widget::Panel::drawInnerPanel(baseOffsetX + 64, baseOffsetY + 48, 34, 5);
    //Widget::Panel::drawInnerPanelBottom(baseOffsetX + 64, baseOffsetY + 104, 34);

    int width;
    if (Data_CityInfo.treasury < 0)
    {
        width = Widget_GameText_draw(60, 3, baseOffsetX + 70, baseOffsetY + 58, FONT_NORMAL_RED);
        Widget_GameText_drawNumberWithDescription(8, 0,
                -Data_CityInfo.treasury, baseOffsetX + 72 + width, baseOffsetY + 58, FONT_NORMAL_RED
                                                 );
    }
    else
    {
        width = Widget_GameText_draw(60, 2, baseOffsetX + 70, baseOffsetY + 58, FONT_NORMAL_WHITE);
        Widget_GameText_drawNumberWithDescription(8, 0,
                Data_CityInfo.treasury, baseOffsetX + 72 + width, baseOffsetY + 58, FONT_NORMAL_WHITE
                                                 );
    }

    // tax percentage and estimated income
    Widget_GameText_draw(60, 1, baseOffsetX + 70, baseOffsetY + 81, FONT_NORMAL_WHITE);
    width = Widget::Text::drawPercentage(Data_CityInfo.taxPercentage,
                                         baseOffsetX + 240, baseOffsetY + 81, FONT_NORMAL_WHITE
                                        );
    width += Widget_GameText_draw(60, 4, baseOffsetX + 240 + width, baseOffsetY + 81, FONT_NORMAL_WHITE);
    Widget_GameText_drawNumberWithDescription(8, 0,
            Data_CityInfo.estimatedTaxIncome, baseOffsetX + 240 + width, baseOffsetY + 81, FONT_NORMAL_WHITE
                                             );

    // percentage taxpayers
    width = Widget::Text::drawPercentage(Data_CityInfo.percentageTaxedPeople,
                                         baseOffsetX + 70, baseOffsetY + 103, FONT_NORMAL_WHITE
                                        );
    Widget_GameText_draw(60, 5, baseOffsetX + 70 + width, baseOffsetY + 103, FONT_NORMAL_WHITE);

    // table headers
    Widget_GameText_draw(60, 6, baseOffsetX + 270, baseOffsetY + 133, FONT_NORMAL_BLACK);
    Widget_GameText_draw(60, 7, baseOffsetX + 400, baseOffsetY + 133, FONT_NORMAL_BLACK);

    // income
    ROW(60, 8, 155, Data_CityInfo.financeTaxesLastYear, Data_CityInfo.financeTaxesThisYear);
    ROW(60, 9, 170, Data_CityInfo.financeExportsLastYear, Data_CityInfo.financeExportsThisYear);
    ROW(60, 20, 185, Data_CityInfo.financeDonatedLastYear, Data_CityInfo.financeDonatedThisYear);

    Graphics_drawLine(baseOffsetX + 280, baseOffsetY + 198, baseOffsetX + 350, baseOffsetY + 198, COLOR_BLACK);
    Graphics_drawLine(baseOffsetX + 420, baseOffsetY + 198, baseOffsetX + 490, baseOffsetY + 198, COLOR_BLACK);

    ROW(60, 10, 203, Data_CityInfo.financeTotalIncomeLastYear, Data_CityInfo.financeTotalIncomeThisYear);

    // expenses
    ROW(60, 11, 227, Data_CityInfo.financeImportsLastYear, Data_CityInfo.financeImportsThisYear);
    ROW(60, 12, 242, Data_CityInfo.financeWagesLastYear, Data_CityInfo.financeWagesThisYear);
    ROW(60, 13, 257, Data_CityInfo.financeConstructionLastYear, Data_CityInfo.financeConstructionThisYear);

    // interest (with percentage)
    width = Widget_GameText_draw(60, 14, baseOffsetX + 80, baseOffsetY + 272, FONT_NORMAL_BLACK);
    Widget::Text::drawPercentage(10, baseOffsetX + 80 + width, baseOffsetY + 272, FONT_NORMAL_BLACK);
    VAL(Data_CityInfo.financeInterestLastYear, 290, 272);
    VAL(Data_CityInfo.financeInterestThisYear, 430, 272);

    ROW(60, 15, 287, Data_CityInfo.financeSalaryLastYear, Data_CityInfo.financeSalaryThisYear);
    ROW(60, 16, 302, Data_CityInfo.financeSundriesLastYear, Data_CityInfo.financeSundriesThisYear);
    ROW(60, 21, 317, Data_CityInfo.financeTributeLastYear, Data_CityInfo.financeTributeThisYear);

    Graphics_drawLine(baseOffsetX + 280, baseOffsetY + 330, baseOffsetX + 350, baseOffsetY + 330, COLOR_BLACK);
    Graphics_drawLine(baseOffsetX + 420, baseOffsetY + 330, baseOffsetX + 490, baseOffsetY + 330, COLOR_BLACK);

    ROW(60, 17, 335, Data_CityInfo.financeTotalExpensesLastYear, Data_CityInfo.financeTotalExpensesThisYear);
    ROW(60, 18, 358, Data_CityInfo.financeNetInOutLastYear, Data_CityInfo.financeNetInOutThisYear);
    ROW(60, 19, 381, Data_CityInfo.financeBalanceLastYear, Data_CityInfo.financeBalanceThisYear);
}

void UI_Advisor_Financial_drawForeground()
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;
    Widget::Button::drawArrowButtons(baseOffsetX, baseOffsetY, arrowButtonsTaxes, 2);
}

void UI_Advisor_Financial_handleMouse()
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;
    arrowButtonFocus = Widget::Button::handleArrowButtons(baseOffsetX, baseOffsetY, arrowButtonsTaxes, 2);
}

static void buttonChangeTaxes(int isDown, int param2)
{
    if (isDown)
    {
        --Data_CityInfo.taxPercentage;
    }
    else
    {
        ++Data_CityInfo.taxPercentage;
    }
    Data_CityInfo.taxPercentage = calc_bound(Data_CityInfo.taxPercentage, 0, 25);

    CityInfo_Finance_calculateEstimatedTaxes();
    CityInfo_Finance_calculateTotals();
    UI_Window_requestRefresh();
}

int UI_Advisor_Financial_getTooltip()
{
    if (arrowButtonFocus)
    {
        return 120;
    }
    else
    {
        return 0;
    }
}
