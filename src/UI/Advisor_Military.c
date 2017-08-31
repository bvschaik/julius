#include "Advisors_private.h"
#include "Window.h"

#include "../CityView.h"
#include "../Event.h"
#include "../Formation.h"

#include "../Data/Figure.h"
#include "../Data/Settings.h"

#include "figure/formation.h"

static void buttonGoToLegion(int param1, int param2);
static void buttonReturnToFort(int param1, int param2);
static void buttonEmpireService(int param1, int param2);

static CustomButton fortCustomButtons[] = {
	{400, 83, 430, 113, CustomButton_Immediate, buttonGoToLegion, Widget_Button_doNothing, 1, 0},
	{480, 83, 510, 113, CustomButton_Immediate, buttonReturnToFort, Widget_Button_doNothing, 1, 0},
	{560, 83, 590, 113, CustomButton_Immediate, buttonEmpireService, Widget_Button_doNothing, 1, 0},
	{400, 127, 430, 157, CustomButton_Immediate, buttonGoToLegion, Widget_Button_doNothing, 2, 0},
	{480, 127, 510, 157, CustomButton_Immediate, buttonReturnToFort, Widget_Button_doNothing, 2, 0},
	{560, 127, 590, 157, CustomButton_Immediate, buttonEmpireService, Widget_Button_doNothing, 2, 0},
	{400, 171, 430, 201, CustomButton_Immediate, buttonGoToLegion, Widget_Button_doNothing, 3, 0},
	{480, 171, 510, 201, CustomButton_Immediate, buttonReturnToFort, Widget_Button_doNothing, 3, 0},
	{560, 171, 590, 201, CustomButton_Immediate, buttonEmpireService, Widget_Button_doNothing, 3, 0},
	{400, 215, 430, 245, CustomButton_Immediate, buttonGoToLegion, Widget_Button_doNothing, 4, 0},
	{480, 215, 510, 245, CustomButton_Immediate, buttonReturnToFort, Widget_Button_doNothing, 4, 0},
	{560, 215, 590, 245, CustomButton_Immediate, buttonEmpireService, Widget_Button_doNothing, 4, 0},
	{400, 259, 430, 289, CustomButton_Immediate, buttonGoToLegion, Widget_Button_doNothing, 5, 0},
	{480, 259, 510, 289, CustomButton_Immediate, buttonReturnToFort, Widget_Button_doNothing, 5, 0},
	{560, 259, 590, 289, CustomButton_Immediate, buttonEmpireService, Widget_Button_doNothing, 5, 0},
	{400, 303, 430, 333, CustomButton_Immediate, buttonGoToLegion, Widget_Button_doNothing, 6, 0},
	{480, 303, 510, 333, CustomButton_Immediate, buttonReturnToFort, Widget_Button_doNothing, 6, 0},
	{560, 303, 590, 333, CustomButton_Immediate, buttonEmpireService, Widget_Button_doNothing, 6, 0},
};

static int focusButtonId;
static int numLegions;

void UI_Advisor_Military_drawBackground(int *advisorHeight)
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	*advisorHeight = 26;
	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons) + 1, baseOffsetX + 10, baseOffsetY + 10);
	Widget_GameText_draw(51, 0, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack);

	Widget_GameText_draw(51, 1, baseOffsetX + 390, baseOffsetY + 43, Font_SmallPlain);
	Widget_GameText_draw(51, 2, baseOffsetX + 390, baseOffsetY + 58, Font_SmallPlain);
	Widget_GameText_draw(51, 3, baseOffsetX + 470, baseOffsetY + 43, Font_SmallPlain);
	Widget_GameText_draw(51, 4, baseOffsetX + 470, baseOffsetY + 58, Font_SmallPlain);
	Widget_GameText_draw(51, 5, baseOffsetX + 550, baseOffsetY + 43, Font_SmallPlain);
	Widget_GameText_draw(51, 6, baseOffsetX + 550, baseOffsetY + 58, Font_SmallPlain);
	Widget_GameText_draw(138, 36, baseOffsetX + 290, baseOffsetY + 58, Font_SmallPlain);

	numLegions = formation_get_num_legions();

    int enemy_text_id;
    if (Data_CityInfo.numEnemiesInCity) {
        enemy_text_id = 10;
    } else if (Data_CityInfo.numImperialSoldiersInCity) {
        enemy_text_id = 11;
    } else if (Event_existsUpcomingInvasion()) {
        enemy_text_id = 9;
    } else {
        enemy_text_id = 8;
    }
    int distant_battle_text_id;
    if (Data_CityInfo.distantBattleRomanMonthsToReturn > 0) {
        distant_battle_text_id = 15;
    } else if (Data_CityInfo.distantBattleRomanMonthsToTravel > 0) {
        distant_battle_text_id = 14;
    } else if (Data_CityInfo.distantBattleMonthsToBattle > 0) {
        distant_battle_text_id = 13;
    } else {
        distant_battle_text_id = 12;
    }
	if (numLegions <= 0) {
		Graphics_drawImage(GraphicId(ID_Graphic_Bullet), baseOffsetX + 100, baseOffsetY + 359);
		Widget_GameText_draw(51, enemy_text_id, baseOffsetX + 120, baseOffsetY + 358, Font_NormalBlack);

		Graphics_drawImage(GraphicId(ID_Graphic_Bullet), baseOffsetX + 100, baseOffsetY + 379);
		Widget_GameText_draw(51, distant_battle_text_id, baseOffsetX + 120, baseOffsetY + 378, Font_NormalBlack);
	} else {
		// has forts
		Graphics_drawImage(GraphicId(ID_Graphic_Bullet), baseOffsetX + 100, baseOffsetY + 349);
		int width = Widget_GameText_drawNumberWithDescription(8, 46, Data_CityInfo.militaryTotalSoldiers,
			baseOffsetX + 120, baseOffsetY + 348, Font_NormalBlack);
		width += Widget_GameText_draw(51, 7, baseOffsetX + 120 + width, baseOffsetY + 348, Font_NormalBlack);
		Widget_GameText_drawNumberWithDescription(8, 48, Data_CityInfo.militaryTotalLegions,
			baseOffsetX + 120 + width, baseOffsetY + 348, Font_NormalBlack);

		Graphics_drawImage(GraphicId(ID_Graphic_Bullet), baseOffsetX + 100, baseOffsetY + 369);
		Widget_GameText_draw(51, enemy_text_id, baseOffsetX + 120, baseOffsetY + 368, Font_NormalBlack);

		Graphics_drawImage(GraphicId(ID_Graphic_Bullet), baseOffsetX + 100, baseOffsetY + 389);
		Widget_GameText_draw(51, distant_battle_text_id, baseOffsetX + 120, baseOffsetY + 388, Font_NormalBlack);
	}

	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 70, 36, 17);
	if (numLegions <= 0) {
		Widget_GameText_drawMultiline(51, 16, baseOffsetX + 64, baseOffsetY + 200, 496, Font_NormalGreen);
		return;
	}

	for (int i = 0; i < numLegions; i++) {
		int formationId = formation_for_legion(i + 1);
		const formation *m = formation_get(formationId);
		Widget_Panel_drawButtonBorder(baseOffsetX + 38, baseOffsetY + 77 + 44 * i, 560, 40, 0);
		Graphics_drawImage(GraphicId(ID_Graphic_FortStandardIcons) + m->legion_id,
			baseOffsetX + 48, baseOffsetY + 82 + 44 * i);
		Widget_GameText_draw(138, m->legion_id,
			baseOffsetX + 100, baseOffsetY + 83 + 44 * i, Font_NormalWhite);
		int width = Widget_Text_drawNumber(m->num_figures, '@', " ",
			baseOffsetX + 100, baseOffsetY + 100 + 44 * i, Font_NormalGreen);
		switch (m->figure_type) {
			case FIGURE_FORT_LEGIONARY:
				Widget_GameText_draw(138, 33, baseOffsetX + 100 + width, baseOffsetY + 100 + 44 * i, Font_NormalGreen);
				break;
			case FIGURE_FORT_MOUNTED:
				Widget_GameText_draw(138, 34, baseOffsetX + 100 + width, baseOffsetY + 100 + 44 * i, Font_NormalGreen);
				break;
			case FIGURE_FORT_JAVELIN:
				Widget_GameText_draw(138, 35, baseOffsetX + 100 + width, baseOffsetY + 100 + 44 * i, Font_NormalGreen);
				break;
		}
		Widget_GameText_drawCentered(138, 37 + m->morale / 5,
			baseOffsetX + 240, baseOffsetY + 91 + 44 * i, 150, Font_NormalGreen);

		int graphicId = GraphicId(ID_Graphic_FortIcons);
		Widget_Panel_drawButtonBorder(baseOffsetX + 400, baseOffsetY + 83 + 44 * i, 30, 30, 0);
		Graphics_drawImage(graphicId, baseOffsetX + 403, baseOffsetY + 86 + 44 * i);

		Widget_Panel_drawButtonBorder(baseOffsetX + 480, baseOffsetY + 83 + 44 * i, 30, 30, 0);
		if (m->is_at_fort) {
			Graphics_drawImage(graphicId + 2, baseOffsetX + 483, baseOffsetY + 86 + 44 * i);
		} else {
			Graphics_drawImage(graphicId + 1, baseOffsetX + 483, baseOffsetY + 86 + 44 * i);
		}

		Widget_Panel_drawButtonBorder(baseOffsetX + 560, baseOffsetY + 83 + 44 * i, 30, 30, 0);
		if (m->empire_service) {
			Graphics_drawImage(graphicId + 3, baseOffsetX + 563, baseOffsetY + 86 + 44 * i);
		} else {
			Graphics_drawImage(graphicId + 4, baseOffsetX + 563, baseOffsetY + 86 + 44 * i);
		}
	}
}

void UI_Advisor_Military_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	numLegions = formation_get_num_legions();
	for (int i = 0; i < numLegions; i++) {
		Widget_Panel_drawButtonBorder(baseOffsetX + 400, baseOffsetY + 83 + 44 * i, 30, 30,
			focusButtonId == 3 * i + 1);
		Widget_Panel_drawButtonBorder(baseOffsetX + 480, baseOffsetY + 83 + 44 * i, 30, 30,
			focusButtonId == 3 * i + 2);
		Widget_Panel_drawButtonBorder(baseOffsetX + 560, baseOffsetY + 83 + 44 * i, 30, 30,
			focusButtonId == 3 * i + 3);
	}
}

void UI_Advisor_Military_handleMouse()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Button_handleCustomButtons(baseOffsetX, baseOffsetY, fortCustomButtons, 3 * numLegions, &focusButtonId);
}

static void buttonGoToLegion(int legionId, int param2)
{
	const formation *m = formation_get(formation_for_legion(legionId));
	CityView_goToGridOffset(GridOffset(m->x_home, m->y_home));
	UI_Window_goTo(Window_City);
}

static void buttonReturnToFort(int legionId, int param2)
{
	int formationId = formation_for_legion(legionId);
	if (!formation_get(formationId)->in_distant_battle) {
		Formation_legionReturnHome(formationId);
		UI_Window_requestRefresh();
	}
}

static void buttonEmpireService(int legionId, int param2)
{
	int formationId = formation_for_legion(legionId);
    formation_toggle_empire_service(formationId);
	Formation_calculateFigures();
	UI_Window_requestRefresh();
}

