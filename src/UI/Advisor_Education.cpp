#include "Advisors_private.h"

void UI_Advisor_Education_drawBackground(int *advisorHeight)
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	*advisorHeight = 16;
	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons) + 7, baseOffsetX + 10, baseOffsetY + 10);
	Widget_GameText_draw(57, 0, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack);

	// x population, y school age, z academy age
	int width = Widget_Text_drawNumber(Data_CityInfo.population, '@', " ",
		baseOffsetX + 60, baseOffsetY + 50, Font_NormalBlack
	);
	width += Widget_GameText_draw(57, 1,
		baseOffsetX + 60 + width, baseOffsetY + 50, Font_NormalBlack
	);
	width += Widget_Text_drawNumber(Data_CityInfo.populationSchoolAge, '@', " ",
		baseOffsetX + 60 + width, baseOffsetY + 50, Font_NormalBlack
	);
	width += Widget_GameText_draw(57, 2,
		baseOffsetX + 60 + width, baseOffsetY + 50, Font_NormalBlack
	);
	width += Widget_Text_drawNumber(Data_CityInfo.populationAcademyAge, '@', " ",
		baseOffsetX + 60 + width, baseOffsetY + 50, Font_NormalBlack
	);
	width += Widget_GameText_draw(57, 3,
		baseOffsetX + 60 + width, baseOffsetY + 50, Font_NormalBlack
	);

	// table headers
	Widget_GameText_draw(57, 4, baseOffsetX + 180, baseOffsetY + 86, Font_SmallPlain);
	Widget_GameText_draw(57, 5, baseOffsetX + 290, baseOffsetY + 86, Font_SmallPlain);
	Widget_GameText_draw(57, 6, baseOffsetX + 478, baseOffsetY + 86, Font_SmallPlain);

	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 100, 36, 4);
	
	// schools
	Widget_GameText_drawNumberWithDescription(
		8, 18, Data_CityInfo_Buildings.school.total,
		baseOffsetX + 40, baseOffsetY + 105, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(Data_CityInfo_Buildings.school.working, '@', " ",
		baseOffsetX + 150, baseOffsetY + 105, 100, Font_NormalWhite
	);

	width = Widget_Text_drawNumber(75 * Data_CityInfo_Buildings.school.working, '@', " ",
		baseOffsetX + 280, baseOffsetY + 105, Font_NormalWhite
	);
	Widget_GameText_draw(57, 7, baseOffsetX + 280 + width, baseOffsetY + 105, Font_NormalWhite);

	if (Data_CityInfo_CultureCoverage.school == 0) {
		Widget_GameText_drawCentered(57, 10, baseOffsetX + 420, baseOffsetY + 105, 200, Font_NormalWhite);
	} else if (Data_CityInfo_CultureCoverage.school < 100) {
		Widget_GameText_drawCentered(57, Data_CityInfo_CultureCoverage.school / 10 + 11,
			baseOffsetX + 420, baseOffsetY + 105, 200, Font_NormalWhite
		);
	} else {
		Widget_GameText_drawCentered(57, 21, baseOffsetX + 420, baseOffsetY + 105, 200, Font_NormalWhite);
	}

	// academies
	Widget_GameText_drawNumberWithDescription(
		8, 20, Data_CityInfo_Buildings.academy.total,
		baseOffsetX + 40, baseOffsetY + 125, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(Data_CityInfo_Buildings.academy.working, '@', " ",
		baseOffsetX + 150, baseOffsetY + 125, 100, Font_NormalWhite
	);

	width = Widget_Text_drawNumber(100 * Data_CityInfo_Buildings.academy.working, '@', " ",
		baseOffsetX + 280, baseOffsetY + 125, Font_NormalWhite
	);
	Widget_GameText_draw(57, 8, baseOffsetX + 280 + width, baseOffsetY + 125, Font_NormalWhite);

	if (Data_CityInfo_CultureCoverage.academy == 0) {
		Widget_GameText_drawCentered(57, 10, baseOffsetX + 420, baseOffsetY + 125, 200, Font_NormalWhite);
	} else if (Data_CityInfo_CultureCoverage.academy < 100) {
		Widget_GameText_drawCentered(57, Data_CityInfo_CultureCoverage.academy / 10 + 11,
			baseOffsetX + 420, baseOffsetY + 125, 200, Font_NormalWhite
		);
	} else {
		Widget_GameText_drawCentered(57, 21, baseOffsetX + 420, baseOffsetY + 125, 200, Font_NormalWhite);
	}

	// libraries
	Widget_GameText_drawNumberWithDescription(
		8, 22, Data_CityInfo_Buildings.library.total,
		baseOffsetX + 40, baseOffsetY + 145, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(Data_CityInfo_Buildings.library.working, '@', " ",
		baseOffsetX + 150, baseOffsetY + 145, 100, Font_NormalWhite
	);

	width = Widget_Text_drawNumber(800 * Data_CityInfo_Buildings.library.working, '@', " ",
		baseOffsetX + 280, baseOffsetY + 145, Font_NormalWhite
	);
	Widget_GameText_draw(57, 9, baseOffsetX + 280 + width, baseOffsetY + 145, Font_NormalWhite);

	if (Data_CityInfo_CultureCoverage.library == 0) {
		Widget_GameText_drawCentered(57, 10, baseOffsetX + 420, baseOffsetY + 145, 200, Font_NormalWhite);
	} else if (Data_CityInfo_CultureCoverage.library < 100) {
		Widget_GameText_drawCentered(57, Data_CityInfo_CultureCoverage.library / 10 + 11,
			baseOffsetX + 420, baseOffsetY + 145, 200, Font_NormalWhite
		);
	} else {
		Widget_GameText_drawCentered(57, 21, baseOffsetX + 420, baseOffsetY + 145, 200, Font_NormalWhite);
	}

	int adviceId;
	if (Data_CityInfo.educationDemand == 1) {
		adviceId = Data_CityInfo.housesRequiringSchool ? 1 : 0;
	} else if (Data_CityInfo.educationDemand == 2) {
		adviceId = Data_CityInfo.housesRequiringLibrary ? 3 : 2;
	} else if (Data_CityInfo.educationDemand == 3) {
		adviceId = 4;
	} else {
		int coverageSchool = Data_CityInfo_CultureCoverage.school;
		int coverageAcademy = Data_CityInfo_CultureCoverage.academy;
		int coverageLibrary = Data_CityInfo_CultureCoverage.library;
		if (!Data_CityInfo.housesRequiringSchool) {
			adviceId = 5; // no demands yet
		} else if (!Data_CityInfo.housesRequiringLibrary) {
			if (coverageSchool >= 100 && coverageAcademy >= 100) {
				adviceId = 6; // education is perfect
			} else if (coverageSchool < coverageAcademy) {
				adviceId = 7; // build more schools
			} else {
				adviceId = 8; // build more academies
			}
		} else {
			// all education needed
			if (coverageSchool >= 100 && coverageAcademy >= 100 && coverageLibrary >= 100) {
				adviceId = 6;
			} else if (coverageSchool <= coverageAcademy && coverageSchool <= coverageLibrary) {
				adviceId = 7; // build more schools
			} else if (coverageAcademy <= coverageSchool && coverageAcademy <= coverageLibrary) {
				adviceId = 8; // build more academies
			} else {
				adviceId = 9; // build more libraries
			}
		}
	}
	Widget_GameText_drawMultiline(57, 22 + adviceId, baseOffsetX + 60, baseOffsetY + 180, 512, Font_NormalBlack);
}
