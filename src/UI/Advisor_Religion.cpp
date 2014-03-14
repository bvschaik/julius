#include "Advisors_private.h"
#include "../Data/Settings.h"
#include "../CityInfo.h"

enum {
	Ceres = 1,
	Neptune = 2,
	Mercury = 3,
	Mars = 4,
	Venus = 5
};

static char wrathOfGod(int godId);

void UI_Advisor_Religion_drawBackground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	if (Data_Settings.godsEnabled) {
		Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, 17);
	} else {
		Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, 20);
		Widget_GameText_drawMultiline(59, 43,
			baseOffsetX + 60, baseOffsetY + 256, 520, Font_NormalBlack
		);
	}
	
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons) + 9, baseOffsetX + 10, baseOffsetY + 10);
	
	Widget_GameText_draw(59, 0, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack);
	
	// table header
	Widget_GameText_draw(59, 5, baseOffsetX + 270, baseOffsetY + 32, Font_SmallPlain);
	Widget_GameText_draw(59, 1, baseOffsetX + 240, baseOffsetY + 46, Font_SmallPlain);
	Widget_GameText_draw(59, 2, baseOffsetX + 300, baseOffsetY + 46, Font_SmallPlain);
	Widget_GameText_draw(59, 3, baseOffsetX + 450, baseOffsetY + 46, Font_SmallPlain);
	Widget_GameText_draw(59, 6, baseOffsetX + 370, baseOffsetY + 18, Font_SmallPlain);
	Widget_GameText_draw(59, 9, baseOffsetX + 370, baseOffsetY + 32, Font_SmallPlain);
	Widget_GameText_draw(59, 7, baseOffsetX + 370, baseOffsetY + 46, Font_SmallPlain);
	
	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 60, 36, 8);
	
	int graphicIdBolt = GraphicId(ID_Graphic_GodBolt);
	int width;
	
	// Ceres
	Widget_GameText_draw(59, 11, baseOffsetX + 40, baseOffsetY + 66, Font_NormalWhite);
	Widget_GameText_draw(59, 16, baseOffsetX + 120, baseOffsetY + 67, Font_SmallPlain);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.smallTempleCeres.total, '@', " ",
		baseOffsetX + 230, baseOffsetY + 66, 50, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.largeTempleCeres.total, '@', " ",
		baseOffsetX + 290, baseOffsetY + 66, 50, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo.monthsSinceFestivalCeres, '@', " ",
		baseOffsetX + 360, baseOffsetY + 66, 50, Font_NormalWhite
	);
	width = Widget_GameText_draw(59, Data_CityInfo.godHappinessCeres / 10 + 32,
		baseOffsetX + 460, baseOffsetY + 66, Font_NormalWhite
	);
	for (int i = 0; i < Data_CityInfo.godWrathCeres / 10; i++) {
		Graphics_drawImage(graphicIdBolt,
			10 * i + baseOffsetX + width + 460, baseOffsetY + 62
		);
	}
	
	// Neptune
	Widget_GameText_draw(59, 12, baseOffsetX + 40, baseOffsetY + 86, Font_NormalWhite);
	Widget_GameText_draw(59, 17, baseOffsetX + 120, baseOffsetY + 87, Font_SmallPlain);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.smallTempleNeptune.total, '@', " ",
		baseOffsetX + 230, baseOffsetY + 86, 50, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.largeTempleNeptune.total, '@', " ",
		baseOffsetX + 290, baseOffsetY + 86, 50, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo.monthsSinceFestivalNeptune, '@', " ",
		baseOffsetX + 360, baseOffsetY + 86, 50, Font_NormalWhite
	);
	width = Widget_GameText_draw(59, Data_CityInfo.godHappinessNeptune / 10 + 32,
		baseOffsetX + 460, baseOffsetY + 86, Font_NormalWhite
	);
	for (int i = 0; i < Data_CityInfo.godWrathNeptune / 10; i++) {
		Graphics_drawImage(graphicIdBolt,
			10 * i + baseOffsetX + width + 460, baseOffsetY + 82
		);
	}
	
	// Mercury
	Widget_GameText_draw(59, 13, baseOffsetX + 40, baseOffsetY + 106, Font_NormalWhite);
	Widget_GameText_draw(59, 18, baseOffsetX + 120, baseOffsetY + 107, Font_SmallPlain);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.smallTempleMercury.total, '@', " ",
		baseOffsetX + 230, baseOffsetY + 106, 50, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.largeTempleMercury.total, '@', " ",
		baseOffsetX + 290, baseOffsetY + 106, 50, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo.monthsSinceFestivalMercury, '@', " ",
		baseOffsetX + 360, baseOffsetY + 106, 50, Font_NormalWhite
	);
	width = Widget_GameText_draw(59, Data_CityInfo.godHappinessMercury / 10 + 32,
		baseOffsetX + 460, baseOffsetY + 106, Font_NormalWhite
	);
	for (int i = 0; i < Data_CityInfo.godWrathMercury / 10; i++) {
		Graphics_drawImage(graphicIdBolt,
			10 * i + baseOffsetX + width + 460, baseOffsetY + 102
		);
	}
	
	// Mars
	Widget_GameText_draw(59, 14, baseOffsetX + 40, baseOffsetY + 126, Font_NormalWhite);
	Widget_GameText_draw(59, 19, baseOffsetX + 120, baseOffsetY + 127, Font_SmallPlain);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.smallTempleMars.total, '@', " ",
		baseOffsetX + 230, baseOffsetY + 126, 50, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.largeTempleMars.total, '@', " ",
		baseOffsetX + 290, baseOffsetY + 126, 50, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo.monthsSinceFestivalMars, '@', " ",
		baseOffsetX + 360, baseOffsetY + 126, 50, Font_NormalWhite
	);
	width = Widget_GameText_draw(59, Data_CityInfo.godHappinessMars / 10 + 32,
		baseOffsetX + 460, baseOffsetY + 126, Font_NormalWhite
	);
	for (int i = 0; i < Data_CityInfo.godWrathMars / 10; i++) {
		Graphics_drawImage(graphicIdBolt,
			10 * i + baseOffsetX + width + 460, baseOffsetY + 122
		);
	}
	
	// Venus
	Widget_GameText_draw(59, 15, baseOffsetX + 40, baseOffsetY + 146, Font_NormalWhite);
	Widget_GameText_draw(59, 20, baseOffsetX + 120, baseOffsetY + 147, Font_SmallPlain);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.smallTempleVenus.total, '@', " ",
		baseOffsetX + 230, baseOffsetY + 146, 50, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.largeTempleVenus.total, '@', " ",
		baseOffsetX + 290, baseOffsetY + 146, 50, Font_NormalWhite
	);
	Widget_Text_drawNumberCentered(
		Data_CityInfo.monthsSinceFestivalVenus, '@', " ",
		baseOffsetX + 360, baseOffsetY + 146, 50, Font_NormalWhite
	);
	width = Widget_GameText_draw(59, Data_CityInfo.godHappinessVenus / 10 + 32,
		baseOffsetX + 460, baseOffsetY + 146, Font_NormalWhite
	);
	for (int i = 0; i < Data_CityInfo.godWrathVenus / 10; i++) {
		Graphics_drawImage(graphicIdBolt,
			10 * i + baseOffsetX + width + 460, baseOffsetY + 142
		);
	}

	// oracles
	Widget_GameText_draw(59, 8, baseOffsetX + 40, baseOffsetY + 166, Font_NormalWhite);
	Widget_Text_drawNumberCentered(
		Data_CityInfo_Buildings.oracle.total, '@', " ",
		baseOffsetX + 230, baseOffsetY + 166, 50, Font_NormalWhite
	);
	
	CityInfo_Gods_calculateLeastHappy();

	int adviceId;
	if (Data_CityInfo.godLeastHappy <= 0 || wrathOfGod(Data_CityInfo.godLeastHappy) <= 4) {
		if (Data_CityInfo.religionDemand == 1) {
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
	} else {
		adviceId = 5 + Data_CityInfo.godLeastHappy;
	}
	Widget_GameText_drawMultiline(59, 21 + adviceId,
		baseOffsetX + 60, baseOffsetY + 196, 512, Font_NormalBlack);
}

static char wrathOfGod(int godId)
{
	switch (godId) {
		case Ceres:   return Data_CityInfo.godWrathCeres;
		case Neptune: return Data_CityInfo.godWrathNeptune;
		case Mercury: return Data_CityInfo.godWrathMercury;
		case Mars:    return Data_CityInfo.godWrathMars;
		case Venus:   return Data_CityInfo.godWrathVenus;
	}
	return 0;
}
