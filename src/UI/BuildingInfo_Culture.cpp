#include "BuildingInfo.h"
#include "../Graphics.h"
#include "../Sound.h"
#include "../Widget.h"
#include "../Data/Building.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"

static void drawInfo(BuildingInfoContext *c, int helpId, const char *soundFile, int groupId)
{
	c->helpId = helpId;
	PLAY_SOUND(soundFile);
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(groupId, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);

	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (Data_Buildings[c->buildingId].numWorkers <= 0) {
		DRAW_DESC(groupId, 2);
	} else {
		DRAW_DESC(groupId, 3);
	}
	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawClinic(BuildingInfoContext *c)
{
	drawInfo(c, 65, "wavs/clinic.wav", 81);
}

void UI_BuildingInfo_drawHospital(BuildingInfoContext *c)
{
	drawInfo(c, 66, "wavs/hospital.wav", 82);
}

void UI_BuildingInfo_drawBathhouse(BuildingInfoContext *c)
{
	c->helpId = 64;
	PLAY_SOUND("wavs/baths.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(83, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);

	if (!Data_Buildings[c->buildingId].hasWaterAccess) {
		DRAW_DESC(83, 4);
	} else if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (Data_Buildings[c->buildingId].numWorkers <= 0) {
		DRAW_DESC(83, 2);
	} else {
		DRAW_DESC(83, 3);
	}
	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawBarber(BuildingInfoContext *c)
{
	drawInfo(c, 63, "wavs/barber.wav", 84);
}

void UI_BuildingInfo_drawSchool(BuildingInfoContext *c)
{
	drawInfo(c, 68, "wavs/school.wav", 85);
}

void UI_BuildingInfo_drawAcademy(BuildingInfoContext *c)
{
	drawInfo(c, 69, "wavs/academy.wav", 86);
}

void UI_BuildingInfo_drawLibrary(BuildingInfoContext *c)
{
	drawInfo(c, 70, "wavs/library.wav", 87);
}

static void drawTemple(BuildingInfoContext *c, const char *soundFile, int groupId, int graphicOffset)
{
	c->helpId = 67;
	PLAY_SOUND(soundFile);
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(groupId, 0, c->xOffset, c->yOffset + 12, 16 * c->widthBlocks, Font_LargeBlack);
	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 56, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 62);
	if (c->hasRoadAccess) {
		Graphics_drawImage(graphicOffset + GraphicId(ID_Graphic_PanelWindows),
			c->xOffset + 190, c->yOffset + 16 * c->heightBlocks - 118);
	} else {
		Widget_GameText_drawMultiline(69, 25,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 128,
			16 * (c->widthBlocks - 4), Font_NormalBlack);
	}
}

void UI_BuildingInfo_drawTempleCeres(BuildingInfoContext *c)
{
	drawTemple(c, "wavs/temple_farm.wav", 92, 21);
}

void UI_BuildingInfo_drawTempleNeptune(BuildingInfoContext *c)
{
	drawTemple(c, "wavs/temple_ship.wav", 93, 22);
}

void UI_BuildingInfo_drawTempleMercury(BuildingInfoContext *c)
{
	drawTemple(c, "wavs/temple_comm.wav", 94, 23);
}

void UI_BuildingInfo_drawTempleMars(BuildingInfoContext *c)
{
	drawTemple(c, "wavs/temple_war.wav", 95, 24);
}

void UI_BuildingInfo_drawTempleVenus(BuildingInfoContext *c)
{
	drawTemple(c, "wavs/temple_love.wav", 96, 25);
}

void UI_BuildingInfo_drawOracle(BuildingInfoContext *c)
{
	c->helpId = 67;
	PLAY_SOUND("wavs/oracle.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(110, 0, c->xOffset, c->yOffset + 12, 16 * c->widthBlocks, Font_LargeBlack);
	Widget_GameText_drawCentered(13, 1,
		c->xOffset, c->yOffset + 16 * c->heightBlocks - 24,
		16 * c->widthBlocks, Font_NormalBlack);
	Widget_GameText_drawMultiline(110, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 143,
		16 * (c->widthBlocks - 4), Font_NormalBlack);
}

