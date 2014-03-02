#include "Advisors_private.h"

void UI_Advisor_Health_drawBackground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, 18);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons) + 6, baseOffsetX + 10, baseOffsetY + 10);

	Widget_GameText_draw(56, 0, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack);
	if (Data_CityInfo.population >= 200) {
		Widget_GameText_drawMultiline(56,
			Data_CityInfo.healthRate / 10 + 16,
			baseOffsetX + 60, baseOffsetY + 46,
			512, Font_NormalBlack
		);
	} else {
		Widget_GameText_drawMultiline(56, 15,
			baseOffsetX + 60, baseOffsetY + 46,
			512, Font_NormalBlack
		);
	}
	Widget_GameText_draw(56, 3, baseOffsetX + 180, baseOffsetY + 94, Font_SmallPlain);
	Widget_GameText_draw(56, 4, baseOffsetX + 290, baseOffsetY + 94, Font_SmallPlain);
	Widget_GameText_draw(56, 5, baseOffsetX + 478, baseOffsetY + 94, Font_SmallPlain);

	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 108, 36, 5);

	// bathhouses
	Widget_GameText_drawNumberWithDescription(8, 24,
		Data_CityInfo_Buildings.bathhouse.total,
		baseOffsetX + 40, baseOffsetY + 112, Font_NormalGreen
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.bathhouse.total, '@', " ",
		baseOffsetX + 150, baseOffsetY + 112, 100, Font_NormalGreen
	);
	Widget_GameText_draw(56, 2, baseOffsetX + 300, baseOffsetY + 112, Font_NormalGreen);
	Widget_GameText_draw(56, 2, baseOffsetX + 504, baseOffsetY + 112, Font_NormalGreen);

	// barbers
	Widget_GameText_drawNumberWithDescription(8, 26,
		Data_CityInfo_Buildings.barber.total,
		baseOffsetX + 40, baseOffsetY + 132, Font_NormalGreen
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.barber.working, '@', " ",
		baseOffsetX + 150, baseOffsetY + 132, 100, Font_NormalGreen
	);
	Widget_GameText_draw(56, 2, baseOffsetX + 300, baseOffsetY + 132, Font_NormalGreen);
	Widget_GameText_draw(56, 2, baseOffsetX + 504, baseOffsetY + 132, Font_NormalGreen);

	// clinics
	Widget_GameText_drawNumberWithDescription(8, 28,
		Data_CityInfo_Buildings.clinic.total,
		baseOffsetX + 40, baseOffsetY + 152, Font_NormalGreen
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.clinic.working, '@', " ",
		baseOffsetX + 150, baseOffsetY + 152, 100, Font_NormalGreen
	);
	Widget_GameText_draw(56, 2, baseOffsetX + 300, baseOffsetY + 152, Font_NormalGreen);
	Widget_GameText_draw(56, 2, baseOffsetX + 504, baseOffsetY + 152, Font_NormalGreen);

	// hospitals
	Widget_GameText_drawNumberWithDescription(8, 30,
		Data_CityInfo_Buildings.hospital.total,
		baseOffsetX + 40, baseOffsetY + 172, Font_NormalGreen
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.hospital.working, '@', " ",
		baseOffsetX + 150, baseOffsetY + 172, 100, Font_NormalGreen
	);

	int width = Widget_Text_drawNumber(
		1000 * Data_CityInfo_Buildings.hospital.working,
		'@', " ", baseOffsetX + 280, baseOffsetY + 172, Font_NormalGreen
	);
	Widget_GameText_draw(56, 6, baseOffsetX + 280 + width, baseOffsetY + 172, Font_NormalGreen);

	if (Data_CityInfo_CultureCoverage.hospital == 0) {
		Widget_GameText_drawCentered(57, 10,
			baseOffsetX + 420, baseOffsetY + 172, 200, Font_NormalGreen
		);
	} else if (Data_CityInfo_CultureCoverage.hospital < 100) {
		Widget_GameText_drawCentered(57,
			Data_CityInfo_CultureCoverage.hospital / 10 + 11,
			baseOffsetX + 420, baseOffsetY + 172, 200, Font_NormalGreen
		);
	} else {
		Widget_GameText_drawCentered(57, 21,
			baseOffsetX + 420, baseOffsetY + 172, 200, Font_NormalGreen
		);
	}

	int adviceId;

	switch (Data_CityInfo.healthDemand) {
		case 1:
			adviceId = Data_CityInfo.housesRequiringBathhouse ? 1 : 0;
			break;
		case 2:
			adviceId = Data_CityInfo.housesRequiringBarber ? 3 : 2;
			break;
		case 3:
			adviceId = Data_CityInfo.housesRequiringClinic ? 5 : 4;
			break;
		case 4:
			adviceId = 6;
			break;
		default:
			adviceId = 7;
			break;
	}
	Widget_GameText_drawMultiline(56, 7 + adviceId,
		baseOffsetX + 60, baseOffsetY + 194, 512, Font_NormalBlack
	);
}
