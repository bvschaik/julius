#include "Advisors_private.h"
#include "../CityInfo.h"

#include "building/count.h"
#include "city/culture.h"
#include "city/gods.h"
#include "game/settings.h"

void UI_Advisor_Religion_drawBackground(int *advisorHeight)
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	if (setting_gods_enabled()) {
		*advisorHeight = 17;
		Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
	} else {
		*advisorHeight = 20;
		Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
		Widget_GameText_drawMultiline(59, 43,
			baseOffsetX + 60, baseOffsetY + 256, 520, FONT_NORMAL_BLACK
		);
	}
	
	Graphics_drawImage(image_group(GROUP_ADVISOR_ICONS) + 9, baseOffsetX + 10, baseOffsetY + 10);
	
	Widget_GameText_draw(59, 0, baseOffsetX + 60, baseOffsetY + 12, FONT_LARGE_BLACK);
	
	// table header
	Widget_GameText_draw(59, 5, baseOffsetX + 270, baseOffsetY + 32, FONT_SMALL_PLAIN);
	Widget_GameText_draw(59, 1, baseOffsetX + 240, baseOffsetY + 46, FONT_SMALL_PLAIN);
	Widget_GameText_draw(59, 2, baseOffsetX + 300, baseOffsetY + 46, FONT_SMALL_PLAIN);
	Widget_GameText_draw(59, 3, baseOffsetX + 450, baseOffsetY + 46, FONT_SMALL_PLAIN);
	Widget_GameText_draw(59, 6, baseOffsetX + 370, baseOffsetY + 18, FONT_SMALL_PLAIN);
	Widget_GameText_draw(59, 9, baseOffsetX + 370, baseOffsetY + 32, FONT_SMALL_PLAIN);
	Widget_GameText_draw(59, 7, baseOffsetX + 370, baseOffsetY + 46, FONT_SMALL_PLAIN);
	
	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 60, 36, 8);
	
	int graphicIdBolt = image_group(GROUP_GOD_BOLT);
	int width;
	
	// Ceres
	Widget_GameText_draw(59, 11, baseOffsetX + 40, baseOffsetY + 66, FONT_NORMAL_WHITE);
	Widget_GameText_draw(59, 16, baseOffsetX + 120, baseOffsetY + 67, FONT_SMALL_PLAIN);
	Widget_Text_drawNumberCentered(
		building_count_total(BUILDING_SMALL_TEMPLE_CERES),
		baseOffsetX + 230, baseOffsetY + 66, 50, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(
		building_count_total(BUILDING_LARGE_TEMPLE_CERES),
		baseOffsetX + 290, baseOffsetY + 66, 50, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo.godMonthsSinceFestival[GOD_CERES],
		baseOffsetX + 360, baseOffsetY + 66, 50, FONT_NORMAL_WHITE
	);
	width = Widget_GameText_draw(59, 32 + Data_CityInfo.godHappiness[GOD_CERES] / 10,
		baseOffsetX + 460, baseOffsetY + 66, FONT_NORMAL_WHITE
	);
	for (int i = 0; i < Data_CityInfo.godWrathBolts[GOD_CERES] / 10; i++) {
		Graphics_drawImage(graphicIdBolt,
			10 * i + baseOffsetX + width + 460, baseOffsetY + 62
		);
	}
	
	// Neptune
	Widget_GameText_draw(59, 12, baseOffsetX + 40, baseOffsetY + 86, FONT_NORMAL_WHITE);
	Widget_GameText_draw(59, 17, baseOffsetX + 120, baseOffsetY + 87, FONT_SMALL_PLAIN);
	Widget_Text_drawNumberCentered(
		building_count_total(BUILDING_SMALL_TEMPLE_NEPTUNE),
		baseOffsetX + 230, baseOffsetY + 86, 50, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(
		building_count_total(BUILDING_LARGE_TEMPLE_NEPTUNE),
		baseOffsetX + 290, baseOffsetY + 86, 50, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo.godMonthsSinceFestival[GOD_NEPTUNE],
		baseOffsetX + 360, baseOffsetY + 86, 50, FONT_NORMAL_WHITE
	);
	width = Widget_GameText_draw(59, 32 + Data_CityInfo.godHappiness[GOD_NEPTUNE] / 10,
		baseOffsetX + 460, baseOffsetY + 86, FONT_NORMAL_WHITE
	);
	for (int i = 0; i < Data_CityInfo.godWrathBolts[GOD_NEPTUNE] / 10; i++) {
		Graphics_drawImage(graphicIdBolt,
			10 * i + baseOffsetX + width + 460, baseOffsetY + 82
		);
	}
	
	// Mercury
	Widget_GameText_draw(59, 13, baseOffsetX + 40, baseOffsetY + 106, FONT_NORMAL_WHITE);
	Widget_GameText_draw(59, 18, baseOffsetX + 120, baseOffsetY + 107, FONT_SMALL_PLAIN);
	Widget_Text_drawNumberCentered(
		building_count_total(BUILDING_SMALL_TEMPLE_MERCURY),
		baseOffsetX + 230, baseOffsetY + 106, 50, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(
		building_count_total(BUILDING_LARGE_TEMPLE_MERCURY),
		baseOffsetX + 290, baseOffsetY + 106, 50, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo.godMonthsSinceFestival[GOD_MERCURY],
		baseOffsetX + 360, baseOffsetY + 106, 50, FONT_NORMAL_WHITE
	);
	width = Widget_GameText_draw(59, 32 + Data_CityInfo.godHappiness[GOD_MERCURY] / 10,
		baseOffsetX + 460, baseOffsetY + 106, FONT_NORMAL_WHITE
	);
	for (int i = 0; i < Data_CityInfo.godWrathBolts[GOD_MERCURY] / 10; i++) {
		Graphics_drawImage(graphicIdBolt,
			10 * i + baseOffsetX + width + 460, baseOffsetY + 102
		);
	}
	
	// Mars
	Widget_GameText_draw(59, 14, baseOffsetX + 40, baseOffsetY + 126, FONT_NORMAL_WHITE);
	Widget_GameText_draw(59, 19, baseOffsetX + 120, baseOffsetY + 127, FONT_SMALL_PLAIN);
	Widget_Text_drawNumberCentered(
		building_count_total(BUILDING_SMALL_TEMPLE_MARS),
		baseOffsetX + 230, baseOffsetY + 126, 50, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(
		building_count_total(BUILDING_LARGE_TEMPLE_MARS),
		baseOffsetX + 290, baseOffsetY + 126, 50, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo.godMonthsSinceFestival[GOD_MARS],
		baseOffsetX + 360, baseOffsetY + 126, 50, FONT_NORMAL_WHITE
	);
	width = Widget_GameText_draw(59, 32 + Data_CityInfo.godHappiness[GOD_MARS] / 10,
		baseOffsetX + 460, baseOffsetY + 126, FONT_NORMAL_WHITE
	);
	for (int i = 0; i < Data_CityInfo.godWrathBolts[GOD_MARS] / 10; i++) {
		Graphics_drawImage(graphicIdBolt,
			10 * i + baseOffsetX + width + 460, baseOffsetY + 122
		);
	}
	
	// Venus
	Widget_GameText_draw(59, 15, baseOffsetX + 40, baseOffsetY + 146, FONT_NORMAL_WHITE);
	Widget_GameText_draw(59, 20, baseOffsetX + 120, baseOffsetY + 147, FONT_SMALL_PLAIN);
	Widget_Text_drawNumberCentered(
		building_count_total(BUILDING_SMALL_TEMPLE_VENUS),
		baseOffsetX + 230, baseOffsetY + 146, 50, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(
		building_count_total(BUILDING_LARGE_TEMPLE_VENUS),
		baseOffsetX + 290, baseOffsetY + 146, 50, FONT_NORMAL_WHITE
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo.godMonthsSinceFestival[GOD_VENUS],
		baseOffsetX + 360, baseOffsetY + 146, 50, FONT_NORMAL_WHITE
	);
	width = Widget_GameText_draw(59, 32 + Data_CityInfo.godHappiness[GOD_VENUS] / 10,
		baseOffsetX + 460, baseOffsetY + 146, FONT_NORMAL_WHITE
	);
	for (int i = 0; i < Data_CityInfo.godWrathBolts[GOD_VENUS] / 10; i++) {
		Graphics_drawImage(graphicIdBolt,
			10 * i + baseOffsetX + width + 460, baseOffsetY + 142
		);
	}

	// oracles
	Widget_GameText_draw(59, 8, baseOffsetX + 40, baseOffsetY + 166, FONT_NORMAL_WHITE);
	Widget_Text_drawNumberCentered(
		building_count_total(BUILDING_ORACLE),
		baseOffsetX + 230, baseOffsetY + 166, 50, FONT_NORMAL_WHITE
	);
	
	city_gods_calculate_least_happy();

	int adviceId;
	if (Data_CityInfo.godLeastHappy > 0 && Data_CityInfo.godWrathBolts[Data_CityInfo.godLeastHappy - 1] > 4) {
		adviceId = 5 + Data_CityInfo.godLeastHappy;
	} else if (Data_CityInfo.religionDemand == 1) {
		adviceId = Data_CityInfo.housesRequiringReligion ? 1 : 0;
	} else if (Data_CityInfo.religionDemand == 2) {
		adviceId = 2;
	} else if (Data_CityInfo.religionDemand == 3) {
		adviceId = 3;
	} else if (!Data_CityInfo.housesRequiringReligion) {
		adviceId = 4;
	} else if (Data_CityInfo.godLeastHappy) {
		adviceId = 5 + Data_CityInfo.godLeastHappy;
	} else {
		adviceId = 5;
	}
	Widget_GameText_drawMultiline(59, 21 + adviceId,
		baseOffsetX + 60, baseOffsetY + 196, 512, FONT_NORMAL_BLACK);
}

