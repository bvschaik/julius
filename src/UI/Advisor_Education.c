#include "Advisors_private.h"

#include "building/count.h"
#include "city/culture.h"

void UI_Advisor_Education_drawBackground(int *advisorHeight)
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	*advisorHeight = 16;
	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
	Graphics_drawImage(image_group(ID_Graphic_AdvisorIcons) + 7, baseOffsetX + 10, baseOffsetY + 10);
	Widget_GameText_draw(57, 0, baseOffsetX + 60, baseOffsetY + 12, FONT_LARGE_BLACK);

	// x population, y school age, z academy age
	int width = Widget_Text_drawNumber(Data_CityInfo.population, '@', " ",
		baseOffsetX + 60, baseOffsetY + 50, FONT_NORMAL_BLACK
	);
	width += Widget_GameText_draw(57, 1,
		baseOffsetX + 60 + width, baseOffsetY + 50, FONT_NORMAL_BLACK
	);
	width += Widget_Text_drawNumber(Data_CityInfo.populationSchoolAge, '@', " ",
		baseOffsetX + 60 + width, baseOffsetY + 50, FONT_NORMAL_BLACK
	);
	width += Widget_GameText_draw(57, 2,
		baseOffsetX + 60 + width, baseOffsetY + 50, FONT_NORMAL_BLACK
	);
	width += Widget_Text_drawNumber(Data_CityInfo.populationAcademyAge, '@', " ",
		baseOffsetX + 60 + width, baseOffsetY + 50, FONT_NORMAL_BLACK
	);
	width += Widget_GameText_draw(57, 3,
		baseOffsetX + 60 + width, baseOffsetY + 50, FONT_NORMAL_BLACK
	);

	// table headers
	Widget_GameText_draw(57, 4, baseOffsetX + 180, baseOffsetY + 86, FONT_SMALL_PLAIN);
	Widget_GameText_draw(57, 5, baseOffsetX + 290, baseOffsetY + 86, FONT_SMALL_PLAIN);
	Widget_GameText_draw(57, 6, baseOffsetX + 478, baseOffsetY + 86, FONT_SMALL_PLAIN);

	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 100, 36, 4);
	
	// schools
	Widget_GameText_drawNumberWithDescription(
		8, 18, building_count_total(BUILDING_SCHOOL),
		baseOffsetX + 40, baseOffsetY + 105, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(building_count_active(BUILDING_SCHOOL),
		baseOffsetX + 150, baseOffsetY + 105, 100, FONT_NORMAL_WHITE
	);

	width = Widget_Text_drawNumber(75 * building_count_active(BUILDING_SCHOOL), '@', " ",
		baseOffsetX + 280, baseOffsetY + 105, FONT_NORMAL_WHITE
	);
	Widget_GameText_draw(57, 7, baseOffsetX + 280 + width, baseOffsetY + 105, FONT_NORMAL_WHITE);

	int pct_school = city_culture_coverage_school();
	if (pct_school == 0) {
		Widget_GameText_drawCentered(57, 10, baseOffsetX + 420, baseOffsetY + 105, 200, FONT_NORMAL_WHITE);
	} else if (pct_school < 100) {
		Widget_GameText_drawCentered(57, pct_school / 10 + 11,
			baseOffsetX + 420, baseOffsetY + 105, 200, FONT_NORMAL_WHITE
		);
	} else {
		Widget_GameText_drawCentered(57, 21, baseOffsetX + 420, baseOffsetY + 105, 200, FONT_NORMAL_WHITE);
	}

	// academies
	Widget_GameText_drawNumberWithDescription(
		8, 20, building_count_total(BUILDING_ACADEMY),
		baseOffsetX + 40, baseOffsetY + 125, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(building_count_active(BUILDING_ACADEMY),
		baseOffsetX + 150, baseOffsetY + 125, 100, FONT_NORMAL_WHITE
	);

	width = Widget_Text_drawNumber(100 * building_count_active(BUILDING_ACADEMY), '@', " ",
		baseOffsetX + 280, baseOffsetY + 125, FONT_NORMAL_WHITE
	);
	Widget_GameText_draw(57, 8, baseOffsetX + 280 + width, baseOffsetY + 125, FONT_NORMAL_WHITE);

	int pct_academy = city_culture_coverage_academy();
	if (pct_academy == 0) {
		Widget_GameText_drawCentered(57, 10, baseOffsetX + 420, baseOffsetY + 125, 200, FONT_NORMAL_WHITE);
	} else if (pct_academy < 100) {
		Widget_GameText_drawCentered(57, pct_academy / 10 + 11,
			baseOffsetX + 420, baseOffsetY + 125, 200, FONT_NORMAL_WHITE
		);
	} else {
		Widget_GameText_drawCentered(57, 21, baseOffsetX + 420, baseOffsetY + 125, 200, FONT_NORMAL_WHITE);
	}

	// libraries
	Widget_GameText_drawNumberWithDescription(
		8, 22, building_count_total(BUILDING_LIBRARY),
		baseOffsetX + 40, baseOffsetY + 145, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(building_count_active(BUILDING_LIBRARY),
		baseOffsetX + 150, baseOffsetY + 145, 100, FONT_NORMAL_WHITE
	);

	width = Widget_Text_drawNumber(800 * building_count_active(BUILDING_LIBRARY), '@', " ",
		baseOffsetX + 280, baseOffsetY + 145, FONT_NORMAL_WHITE
	);
	Widget_GameText_draw(57, 9, baseOffsetX + 280 + width, baseOffsetY + 145, FONT_NORMAL_WHITE);

	int pct_library = city_culture_coverage_library();
	if (pct_library == 0) {
		Widget_GameText_drawCentered(57, 10, baseOffsetX + 420, baseOffsetY + 145, 200, FONT_NORMAL_WHITE);
	} else if (pct_library < 100) {
		Widget_GameText_drawCentered(57, pct_library / 10 + 11,
			baseOffsetX + 420, baseOffsetY + 145, 200, FONT_NORMAL_WHITE
		);
	} else {
		Widget_GameText_drawCentered(57, 21, baseOffsetX + 420, baseOffsetY + 145, 200, FONT_NORMAL_WHITE);
	}

	int adviceId;
	if (Data_CityInfo.educationDemand == 1) {
		adviceId = Data_CityInfo.housesRequiringSchool ? 1 : 0;
	} else if (Data_CityInfo.educationDemand == 2) {
		adviceId = Data_CityInfo.housesRequiringLibrary ? 3 : 2;
	} else if (Data_CityInfo.educationDemand == 3) {
		adviceId = 4;
	} else {
		int coverageSchool = city_culture_coverage_school();
		int coverageAcademy = city_culture_coverage_academy();
		int coverageLibrary = city_culture_coverage_library();
		if (!Data_CityInfo.housesRequiringSchool) {
			adviceId = 5; // no demands yet
		} else if (!Data_CityInfo.housesRequiringLibrary) {
			if (coverageSchool >= 100 && coverageAcademy >= 100) {
				adviceId = 6; // education is perfect
			} else if (coverageSchool <= coverageAcademy) {
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
			} else if (coverageLibrary <= coverageSchool && coverageLibrary <= coverageAcademy) {
				adviceId = 9; // build more libraries
			} else {
				adviceId = 6; // unlikely event that all coverages are equal
			}
		}
	}
	Widget_GameText_drawMultiline(57, 22 + adviceId, baseOffsetX + 60, baseOffsetY + 180, 512, FONT_NORMAL_BLACK);
}
