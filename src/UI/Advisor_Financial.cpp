#include "Advisors_private.h"

#define VAL(val,x,y) Widget_Text_drawNumber(val, '@', " ", baseOffsetX + x, baseOffsetY + y, Font_NormalBlack, 0)
#define ROW(tgr,tid,y,valLy,valTy) \
	Widget_GameText_draw(tgr, tid, baseOffsetX + 80, baseOffsetY + y, Font_NormalBlack, 0);\
	Widget_Text_drawNumber(valLy, '@', " ", baseOffsetX + 290, baseOffsetY + y, Font_NormalBlack, 0);\
	Widget_Text_drawNumber(valTy, '@', " ", baseOffsetX + 430, baseOffsetY + y, Font_NormalBlack, 0)

void UI_Advisor_Financial_drawBackground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, 26);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons) + 10, baseOffsetX + 10, baseOffsetY + 10);

	Widget_GameText_draw(60, 0, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack, 0);
	Widget_Panel_drawInnerPanel(baseOffsetX + 64, baseOffsetY + 48, 34, 4);
	Widget_Panel_drawInnerPanelBottom(baseOffsetX + 64, baseOffsetY + 104, 34);

	int width;
	if (Data_CityInfo.treasury < 0) {
		width = Widget_GameText_draw(60, 3, baseOffsetX + 70, baseOffsetY + 53, Font_NormalRed, 0);
		Widget_GameText_drawNumberWithDescription(
			8, 0, -Data_CityInfo.treasury, baseOffsetX + 72 + width, baseOffsetY + 53, Font_NormalRed, 0
		);
	} else {
		width = Widget_GameText_draw(60, 2, baseOffsetX + 70, baseOffsetY + 53, Font_NormalWhite, 0);
		Widget_GameText_drawNumberWithDescription(8, 0,
			Data_CityInfo.treasury, baseOffsetX + 70 + width, baseOffsetY + 53, Font_NormalWhite, 0
		);
	}

	// tax percentage and estimated income
	Widget_GameText_draw(60, 1, baseOffsetX + 70, baseOffsetY + 76, Font_NormalWhite, 0);
	width = Widget_Text_drawNumber(Data_CityInfo.taxPercentage, '@', "%",
		baseOffsetX + 240, baseOffsetY + 76, Font_NormalWhite, 0
	);
	width += Widget_GameText_draw(60, 4, baseOffsetX + 240 + width, baseOffsetY + 76, Font_NormalWhite, 0);
	Widget_GameText_drawNumberWithDescription(8, 0,
		Data_CityInfo.estimatedTaxIncome, baseOffsetX + 240 + width, baseOffsetY + 76, Font_NormalWhite, 0
	);

	// percentage taxpayers
	width = Widget_Text_drawNumber(Data_CityInfo.percentageRegisteredForTax, '@', "%",
		baseOffsetX + 70, baseOffsetY + 98, Font_NormalWhite, 0
	);
	Widget_GameText_draw(60, 5, baseOffsetX + 70 + width, baseOffsetY + 98, Font_NormalWhite, 0);

	// table headers
	Widget_GameText_draw(60, 6, baseOffsetX + 270, baseOffsetY + 133, Font_NormalBlack, 0);
	Widget_GameText_draw(60, 7, baseOffsetX + 400, baseOffsetY + 133, Font_NormalBlack, 0);

	// income
	ROW(60, 8, 155, Data_CityInfo.financeTaxesLastYear, Data_CityInfo.financeTaxesThisYear);
	ROW(60, 9, 170, Data_CityInfo.financeExportsLastYear, Data_CityInfo.financeExportsThisYear);
	ROW(60, 20, 185, Data_CityInfo.financeDonatedLastYear, Data_CityInfo.financeDonatedThisYear);

	Graphics_drawLine(baseOffsetX + 280, baseOffsetY + 198, baseOffsetX + 350, baseOffsetY + 198, Color_Black);
	Graphics_drawLine(baseOffsetX + 420, baseOffsetY + 198, baseOffsetX + 490, baseOffsetY + 198, Color_Black);
	
	ROW(60, 10, 203, Data_CityInfo.financeTotalIncomeLastYear, Data_CityInfo.financeTotalIncomeThisYear);

	// expenses
	ROW(60, 11, 227, Data_CityInfo.financeExportsLastYear, Data_CityInfo.financeExportsThisYear);
	ROW(60, 12, 242, Data_CityInfo.financeWagesLastYear, Data_CityInfo.financeWagesThisYear);
	ROW(60, 13, 257, Data_CityInfo.financeConstructionLastYear, Data_CityInfo.financeConstructionThisYear);

	// interest (with percentage)
	width = Widget_GameText_draw(60, 14, baseOffsetX + 80, baseOffsetY + 272, Font_NormalBlack, 0);
	Widget_Text_drawNumber(10, '@', "%", baseOffsetX + 80 + width, baseOffsetY + 272, Font_NormalBlack, 0);
	VAL(Data_CityInfo.financeInterestLastYear, 290, 272);
	VAL(Data_CityInfo.financeInterestThisYear, 430, 272);

	ROW(60, 15, 287, Data_CityInfo.financeSalaryLastYear, Data_CityInfo.financeSalaryThisYear);
	ROW(60, 16, 302, Data_CityInfo.financeSundriesLastYear, Data_CityInfo.financeSundriesThisYear);
	ROW(60, 21, 317, Data_CityInfo.financeTributeLastYear, Data_CityInfo.financeTributeThisYear);

	Graphics_drawLine(baseOffsetX + 280, baseOffsetY + 330, baseOffsetX + 350, baseOffsetY + 330, Color_Black);
	Graphics_drawLine(baseOffsetX + 420, baseOffsetY + 330, baseOffsetX + 490, baseOffsetY + 330, Color_Black);
	
	ROW(60, 17, 335, Data_CityInfo.financeTotalExpensesLastYear, Data_CityInfo.financeTotalExpensesThisYear);
	ROW(60, 18, 358, Data_CityInfo.financeNetInOutLastYear, Data_CityInfo.financeNetInOutThisYear);
	ROW(60, 19, 381, Data_CityInfo.financeBalanceLastYear, Data_CityInfo.financeBalanceThisYear);
}
