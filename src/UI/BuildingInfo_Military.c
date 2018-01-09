#include "BuildingInfo.h"

#include "../Graphics.h"
#include "../UI/Window.h"

#include "../Data/State.h"

#include "building/count.h"
#include "city/view.h"
#include "core/calc.h"
#include "core/debug.h"
#include "figure/formation_legion.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

static void buttonReturnToFort(int param1, int param2);
static void buttonLayout(int index, int param2);

static generic_button layoutButtons[] = {
	{19, 139, 103, 223, GB_IMMEDIATE, buttonLayout, button_none, 0, 0},
	{104, 139, 188, 223, GB_IMMEDIATE, buttonLayout, button_none, 1, 0},
	{189, 139, 273, 223, GB_IMMEDIATE, buttonLayout, button_none, 2, 0},
	{274, 139, 358, 223, GB_IMMEDIATE, buttonLayout, button_none, 3, 0},
	{359, 139, 443, 223, GB_IMMEDIATE, buttonLayout, button_none, 4, 0}
};

static generic_button returnButtons[] = {
	{0, 0, 288, 32, GB_IMMEDIATE, buttonReturnToFort, button_none, 0, 0},
};

static int focusButtonId;
static int returnButtonId;

static BuildingInfoContext *contextForCallback;

void UI_BuildingInfo_drawWall(BuildingInfoContext *c)
{
	c->helpId = 85;
	PLAY_SOUND("wavs/wall.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(139, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	lang_text_draw_multiline(139, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawPrefect(BuildingInfoContext *c)
{
	c->helpId = 86;
	PLAY_SOUND("wavs/prefecture.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(88, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	lang_text_draw_centered(13, 1,
		c->xOffset, c->yOffset + 16 * c->heightBlocks - 24,
		16 * c->widthBlocks, FONT_NORMAL_BLACK);

	building *b = building_get(c->buildingId);
	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (b->numWorkers <= 0) {
		DRAW_DESC(88, 9);
	} else {
		if (b->figureId) {
			DRAW_DESC(88, 2);
		} else {
			DRAW_DESC(88, 3);
		}
		if (c->workerPercentage >= 100) {
			DRAW_DESC_AT(72, 88, 4);
		} else if (c->workerPercentage >= 75) {
			DRAW_DESC_AT(72, 88, 5);
		} else if (c->workerPercentage >= 50) {
			DRAW_DESC_AT(72, 88, 6);
		} else if (c->workerPercentage >= 25) {
			DRAW_DESC_AT(72, 88, 7);
		} else {
			DRAW_DESC_AT(72, 88, 8);
		}
	}

	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawFort(BuildingInfoContext *c)
{
	c->helpId = 87;
	PLAY_SOUND("wavs/fort.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(89, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	if (formation_get(c->formationId)->cursed_by_mars) {
		lang_text_draw_multiline(89, 1,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	} else {
		lang_text_draw_multiline(89, 2,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	}
}

void UI_BuildingInfo_drawGatehouse(BuildingInfoContext *c)
{
	c->helpId = 85;
	PLAY_SOUND("wavs/gatehouse.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(90, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	lang_text_draw_multiline(90, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawTower(BuildingInfoContext *c)
{
	c->helpId = 85;
	PLAY_SOUND("wavs/tower.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(91, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

    building *b = building_get(c->buildingId);
	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (b->numWorkers <= 0) {
		DRAW_DESC(91, 2);
	} else if (b->figureId) {
		DRAW_DESC(91, 3);
	} else {
		DRAW_DESC(91, 4);
	}
	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawMilitaryAcademy(BuildingInfoContext *c)
{
	c->helpId = 88;
	PLAY_SOUND("wavs/mil_acad.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(135, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	building *b = building_get(c->buildingId);
	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (b->numWorkers <= 0) {
		DRAW_DESC(135, 2);
	} else if (c->workerPercentage >= 100) {
		DRAW_DESC(135, 1);
	} else {
		DRAW_DESC(135, 3);
	}
	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawBarracks(BuildingInfoContext *c)
{
	c->helpId = 37;
	PLAY_SOUND("wavs/barracks.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(136, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	Graphics_drawImage(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WEAPONS,
		c->xOffset + 64, c->yOffset + 38);

	building *b = building_get(c->buildingId);
	if (b->loadsStored < 1) {
		lang_text_draw_amount(8, 10, 0,
			c->xOffset + 92, c->yOffset + 44, FONT_NORMAL_BLACK);
	} else {
		lang_text_draw_amount(8, 10, b->loadsStored,
			c->xOffset + 92, c->yOffset + 44, FONT_NORMAL_BLACK);
	}

	if (!c->hasRoadAccess) {
		DRAW_DESC_AT(70, 69, 25);
	} else if (b->numWorkers <= 0) {
		DRAW_DESC_AT(70, 136, 3);
	} else if (!c->barracksSoldiersRequested) {
		DRAW_DESC_AT(70, 136, 4);
	} else {
		int offset = 0;
		if (b->loadsStored > 0) {
			offset = 4;
		}
		if (c->workerPercentage >= 100) {
			DRAW_DESC_AT(70, 136, 5 + offset);
		} else if (c->workerPercentage >= 66) {
			DRAW_DESC_AT(70, 136, 6 + offset);
		} else if (c->workerPercentage >= 33) {
			DRAW_DESC_AT(70, 136, 7 + offset);
		} else {
			DRAW_DESC_AT(70, 136, 8 + offset);
		}
	}
	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawLegionInfo(BuildingInfoContext *c)
{
	int textId, groupId;
    const formation *m = formation_get(c->formationId);
	c->helpId = 87;
	outer_panel_draw(c->xOffset, c->yOffset,
		c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(138, m->legion_id,
		c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	// standard icon at the top
	int graphicId = image_group(GROUP_FIGURE_FORT_STANDARD_ICONS) + m->legion_id;
	int iconHeight = image_get(graphicId)->height;
	Graphics_drawImage(graphicId,
		c->xOffset + 16 + (40 - image_get(graphicId)->width) / 2,
		c->yOffset + 16);
	// standard flag
	graphicId = image_group(GROUP_FIGURE_FORT_FLAGS);
	if (m->figure_type == FIGURE_FORT_JAVELIN) {
		graphicId += 9;
	} else if (m->figure_type == FIGURE_FORT_MOUNTED) {
		graphicId += 18;
	}
	if (m->is_halted) {
		graphicId += 8;
	}
	int flagHeight = image_get(graphicId)->height;
	Graphics_drawImage(graphicId,
		c->xOffset + 16 + (40 - image_get(graphicId)->width) / 2,
		c->yOffset + 16 + iconHeight);
	// standard pole and morale ball
	graphicId = image_group(GROUP_FIGURE_FORT_STANDARD_POLE) + 20 - m->morale / 5;
	Graphics_drawImage(graphicId,
		c->xOffset + 16 + (40 - image_get(graphicId)->width) / 2,
		c->yOffset + 16 + iconHeight + flagHeight);

	// number of soldiers
	lang_text_draw(138, 23, c->xOffset + 100, c->yOffset + 60, FONT_NORMAL_BLACK);
	text_draw_number(m->num_figures, '@', " ",
		c->xOffset + 294, c->yOffset + 60, FONT_NORMAL_BLACK);
	// health
	lang_text_draw(138, 24, c->xOffset + 100, c->yOffset + 80, FONT_NORMAL_BLACK);
	int health = calc_percentage(m->total_damage, m->max_total_damage);
	if (health <= 0) {
		textId = 26;
	} else if (health <= 20) {
		textId = 27;
	} else if (health <= 40) {
		textId = 28;
	} else if (health <= 55) {
		textId = 29;
	} else if (health <= 70) {
		textId = 30;
	} else if (health <= 90) {
		textId = 31;
	} else {
		textId = 32;
	}
	lang_text_draw(138, textId, c->xOffset + 300, c->yOffset + 80, FONT_NORMAL_BLACK);
	// military training
	lang_text_draw(138, 25, c->xOffset + 100, c->yOffset + 100, FONT_NORMAL_BLACK);
	lang_text_draw(18, m->has_military_training,
		c->xOffset + 300, c->yOffset + 100, FONT_NORMAL_BLACK);
	// morale
	if (m->cursed_by_mars) {
		lang_text_draw(138, 59, c->xOffset + 100, c->yOffset + 120, FONT_NORMAL_BLACK);
	} else {
		lang_text_draw(138, 36, c->xOffset + 100, c->yOffset + 120, FONT_NORMAL_BLACK);
		lang_text_draw(138, 37 + m->morale / 5,
			c->xOffset + 300, c->yOffset + 120, FONT_NORMAL_BLACK);
	}
	if (m->num_figures) {
		// layout
		static const int offsetsLegionary[2][5] = {
			{0, 0, 2, 3, 4}, {0, 0, 3, 2, 4},
		};
		static const int offsetsOther[2][5] = {
			{5, 6, 2, 3, 4}, {6, 5, 3, 2, 4},
		};
		const int *offsets;
		int index = 0;
		if (city_view_orientation() == DIR_6_LEFT || city_view_orientation() == DIR_2_RIGHT) {
			index = 1;
		}
		if (m->figure_type == FIGURE_FORT_LEGIONARY) {
			offsets = offsetsLegionary[index];
		} else {
			offsets = offsetsOther[index];
		}
		for (int i = 5 - c->formationTypes; i < 5; i++) {
			Graphics_drawImage(image_group(GROUP_FORT_FORMATIONS) + offsets[i],
				c->xOffset + 21 + 85 * i, c->yOffset + 141);
		}
		UI_BuildingInfo_drawLegionInfoForeground(c);
	} else {
		// no soldiers
		if (m->cursed_by_mars) {
			groupId = 89; textId = 1;
		} else if (building_count_active(BUILDING_BARRACKS)) {
			groupId = 138; textId = 10;
		} else {
			groupId = 138; textId = 11;
		}
		lang_text_draw_multiline(groupId, textId,
			c->xOffset + 32, c->yOffset + 172,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	}
}

void UI_BuildingInfo_drawLegionInfoForeground(BuildingInfoContext *c)
{
    const formation *m = formation_get(c->formationId);
	if (!m->num_figures) {
		return;
	}
	for (int i = 5 - c->formationTypes; i < 5; i++) {
		int hasFocus = 0;
		if (focusButtonId) {
			if (focusButtonId - 1 == i) {
				hasFocus = 1;
			}
		} else if (m->figure_type == FIGURE_FORT_LEGIONARY) {
			if (i == 0 && m->layout == FORMATION_TORTOISE) {
				hasFocus = 1;
			} else if (i == 1 && m->layout == FORMATION_COLUMN) {
				hasFocus = 1;
			} else if (i == 2 && m->layout == FORMATION_DOUBLE_LINE_1) {
				hasFocus = 1;
			} else if (i == 3 && m->layout == FORMATION_DOUBLE_LINE_2) {
				hasFocus = 1;
			} else if (i == 4 && m->layout == FORMATION_MOP_UP) {
				hasFocus = 1;
			}
		} else { // mounted/javelin
			if (i == 0 && m->layout == FORMATION_SINGLE_LINE_1) {
				hasFocus = 1;
			} else if (i == 1 && m->layout == FORMATION_SINGLE_LINE_2) {
				hasFocus = 1;
			} else if (i == 2 && m->layout == FORMATION_DOUBLE_LINE_1) {
				hasFocus = 1;
			} else if (i == 3 && m->layout == FORMATION_DOUBLE_LINE_2) {
				hasFocus = 1;
			} else if (i == 4 && m->layout == FORMATION_MOP_UP) {
				hasFocus = 1;
			}
		}
		button_border_draw(c->xOffset + 19 + 85 * i,
			c->yOffset + 139, 84, 84, hasFocus);
	}
	inner_panel_draw(c->xOffset + 16, c->yOffset + 230,
		c->widthBlocks - 2, 4);

	int titleId;
	int textId;
	switch (focusButtonId) {
		// single line or testudo
		case 1:
			if (m->figure_type == FIGURE_FORT_LEGIONARY) {
				titleId = 12;
				textId = m->has_military_training ? 18 : 17;
			} else {
				titleId = 16;
				textId = 22;
			}
			break;
		case 2:
			if (m->figure_type == FIGURE_FORT_LEGIONARY) {
				titleId = 13;
				textId = m->has_military_training ? 19 : 17;
			} else {
				titleId = 16;
				textId = 22;
			}
			break;
		// double line
		case 3:
		case 4:
			titleId = 14;
			textId = 20;
			break;
		case 5: // mop up
			titleId = 15;
			textId = 21;
			break;
		default:
			// no button selected: go for formation layout
			switch (m->layout) {
				case FORMATION_SINGLE_LINE_1:
				case FORMATION_SINGLE_LINE_2:
					titleId = 16;
					textId = 22;
					break;
				case FORMATION_DOUBLE_LINE_1:
				case FORMATION_DOUBLE_LINE_2:
					titleId = 14;
					textId = 20;
					break;
				case FORMATION_TORTOISE:
					titleId = 12;
					textId = 18;
					break;
				case FORMATION_MOP_UP:
					titleId = 15;
					textId = 21;
					break;
				case FORMATION_COLUMN:
					titleId = 13;
					textId = 19;
					break;
				default:
					titleId = 16;
					textId = 22;
					debug_log("Unknown formation", 0, m->layout);
					break;
			}
			break;
	}
	lang_text_draw(138, titleId,
		c->xOffset + 24, c->yOffset + 236, FONT_NORMAL_WHITE);
	lang_text_draw_multiline(138, textId,
		c->xOffset + 24, c->yOffset + 252,
		16 * (c->widthBlocks - 4), FONT_NORMAL_GREEN);

	if (!m->is_at_fort) {
		button_border_draw(
			c->xOffset + 16 * (c->widthBlocks - 18) / 2,
			c->yOffset + 16 * c->heightBlocks - 48,
			288, 32, returnButtonId == 1);
		lang_text_draw_centered(138, 58,
			c->xOffset + 16 * (c->widthBlocks - 18) / 2,
			c->yOffset + 16 * c->heightBlocks - 39,
			288, FONT_NORMAL_BLACK);
	}
}

void UI_BuildingInfo_handleMouseLegionInfo(const mouse *m, BuildingInfoContext *c)
{
	contextForCallback = c;
    int handled = generic_buttons_handle_mouse(
			m, c->xOffset, c->yOffset, layoutButtons, 5, &focusButtonId);
    if (formation_get(c->formationId)->figure_type == FIGURE_FORT_LEGIONARY) {
        if (focusButtonId == 1 || (focusButtonId == 2 && c->formationTypes == 3)) {
            focusButtonId = 0;
        }
    }
	if (!handled) {
		generic_buttons_handle_mouse(
			m, c->xOffset + 16 * (c->widthBlocks - 18) / 2,
			c->yOffset + 16 * c->heightBlocks - 48,
			returnButtons, 1, &returnButtonId);
	}
	contextForCallback = 0;
}

int UI_BuildingInfo_getTooltipLegionInfo(BuildingInfoContext *c)
{
	return focusButtonId ? 147 : 0;
}

static void buttonReturnToFort(int param1, int param2)
{
    formation *m = formation_get(contextForCallback->formationId);
	if (!m->in_distant_battle && m->is_at_fort != 1) {
		formation_legion_return_home(m);
		UI_Window_goTo(Window_City);
	}
}

static void buttonLayout(int index, int param2)
{
    formation *m = formation_get(contextForCallback->formationId);
	if (m->in_distant_battle) {
		return;
	}
	if (index == 0 && contextForCallback->formationTypes < 5) {
		return;
	}
	if (index == 1 && contextForCallback->formationTypes < 4) {
		return;
	}
	// store layout in case of mop up
	int new_layout = m->layout;
	if (m->figure_type == FIGURE_FORT_LEGIONARY) {
		switch (index) {
			case 0: new_layout = FORMATION_TORTOISE; break;
			case 1: new_layout = FORMATION_COLUMN; break;
			case 2: new_layout = FORMATION_DOUBLE_LINE_1; break;
			case 3: new_layout = FORMATION_DOUBLE_LINE_2; break;
			case 4: new_layout = FORMATION_MOP_UP; break;
		}
	} else {
		switch (index) {
			case 0: new_layout = FORMATION_SINGLE_LINE_1; break;
			case 1: new_layout = FORMATION_SINGLE_LINE_2; break;
			case 2: new_layout = FORMATION_DOUBLE_LINE_1; break;
			case 3: new_layout = FORMATION_DOUBLE_LINE_2; break;
			case 4: new_layout = FORMATION_MOP_UP; break;
		}
	}
	formation_legion_change_layout(m, new_layout);
	switch (index) {
		case 0: sound_speech_play_file("wavs/cohort1.wav"); break;
		case 1: sound_speech_play_file("wavs/cohort2.wav"); break;
		case 2: sound_speech_play_file("wavs/cohort3.wav"); break;
		case 3: sound_speech_play_file("wavs/cohort4.wav"); break;
		case 4: sound_speech_play_file("wavs/cohort5.wav"); break;
	}
	Data_State.selectedLegionFormationId = contextForCallback->formationId;
	UI_Window_goTo(Window_CityMilitary);
}
