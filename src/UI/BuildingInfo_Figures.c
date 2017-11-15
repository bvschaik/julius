#include "BuildingInfo.h"

#include "CityBuildings.h"
#include "Window.h"
#include "../CityView.h"
#include "../Figure.h"
#include "../Graphics.h"
#include "../Resource.h"
#include "../Widget.h"

#include "../Data/Building.h"
#include "../Data/Constants.h"
#include "../Data/Figure.h"
#include "../Data/Settings.h"

#include "empire/city.h"
#include "figure/formation.h"
#include "figure/trader.h"
#include "scenario/property.h"

static void selectFigure(int param1, int param2);

static const int figureTypeToBigPeopleGraphicId[] = {
	8, 13, 13, 9, 4, 13, 8, 16, 7, 4,
	18, 42, 26, 41, 8, 1, 33, 10, 11, 25,
	8, 25, 15, 15, 15, 60, 12, 14, 5, 52,
	52, 2, 3, 6, 6, 13, 8, 8, 17, 12,
	58, 21, 50, 8, 8, 8, 28, 30, 23, 8,
	8, 8, 34, 39, 33, 43, 27, 48, 63, 8,
	8, 8, 8, 8, 53, 8, 38, 62, 54, 55,
	56, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	57, 74, 75, 76, 90, 58, 72, 99, 88, 89,
	10, 11, 59, 32, 33, 34, 35, 37, 12, 13,
	14, 15, 80, 71, 94, 100, 101, 102, 103, 104,
	105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
	70, 55, 51, 47, 91, 16, 17
};

#define BigPeopleGraphic(t) (image_group(ID_Graphic_BigPeople) + figureTypeToBigPeopleGraphicId[t] - 1)

static CustomButton figureButtons[] = {
	{26, 46, 76, 96, CustomButton_Immediate, selectFigure, Widget_Button_doNothing, 0, 0},
	{86, 46, 136, 96, CustomButton_Immediate, selectFigure, Widget_Button_doNothing, 1, 0},
	{146, 46, 196, 96, CustomButton_Immediate, selectFigure, Widget_Button_doNothing, 2, 0},
	{206, 46, 256, 96, CustomButton_Immediate, selectFigure, Widget_Button_doNothing, 3, 0},
	{266, 46, 316, 96, CustomButton_Immediate, selectFigure, Widget_Button_doNothing, 4, 0},
	{326, 46, 376, 96, CustomButton_Immediate, selectFigure, Widget_Button_doNothing, 5, 0},
	{386, 46, 436, 96, CustomButton_Immediate, selectFigure, Widget_Button_doNothing, 6, 0},
};

static color_t figureImages[7][48*48];

static int focusButtonId;
static BuildingInfoContext *contextForCallback;

static int collectingItemIdToResourceId(int c)
{
	switch (c) {
		case 0: return Resource_Wheat;
		case 1: return Resource_Vegetables;
		case 2: return Resource_Fruit;
		case 3: return Resource_Meat;
		case 4: return Resource_Wine;
		case 5: return Resource_Oil;
		case 6: return Resource_Furniture;
		case 7: return Resource_Pottery;
		default: return 0;
	}
}

static void drawFigureInfoTrade(BuildingInfoContext *c, int figureId)
{
	while (Data_Figures[figureId].type == FIGURE_TRADE_CARAVAN_DONKEY) {
		figureId = Data_Figures[figureId].inFrontFigureId;
	}
	struct Data_Figure *f = &Data_Figures[figureId];
	const empire_city *city = empire_city_get(f->empireCityId);
	int width = Widget_GameText_draw(64, f->type,
		c->xOffset + 40, c->yOffset + 110, FONT_SMALL_BLACK);
	Widget_GameText_draw(21, city->name_id,
		c->xOffset + 40 + width, c->yOffset + 110, FONT_SMALL_BLACK);
	
	width = Widget_GameText_draw(129, 1,
		c->xOffset + 40, c->yOffset + 130, FONT_SMALL_BLACK);
	Widget_GameText_drawNumberWithDescription(8, 10, f->type == FIGURE_TRADE_SHIP ? 12 : 8,
		c->xOffset + 40 + width, c->yOffset + 130, FONT_SMALL_BLACK);
	
	int traderId = f->traderId;
	if (f->type == FIGURE_TRADE_SHIP) {
		int textId;
		switch (f->actionState) {
			case FigureActionState_114_TradeShipAnchored: textId = 6; break;
			case FigureActionState_112_TradeShipMoored: textId = 7; break;
			case FigureActionState_115_TradeShipLeaving: textId = 8; break;
			default: textId = 9; break;
		}
		Widget_GameText_draw(129, textId, c->xOffset + 40, c->yOffset + 150, FONT_SMALL_BLACK);
	} else {
		int textId;
		switch (f->actionState) {
			case FigureActionState_101_TradeCaravanArriving:
				textId = 12;
				break;
			case FigureActionState_102_TradeCaravanTrading:
				textId = 10;
				break;
			case FigureActionState_103_TradeCaravanLeaving:
				if (trader_has_traded(traderId)) {
					textId = 11;
				} else {
					textId = 13;
				}
				break;
			default:
				textId = 11;
				break;
		}
		Widget_GameText_draw(129, textId, c->xOffset + 40, c->yOffset + 150, FONT_SMALL_BLACK);
	}
	if (trader_has_traded(traderId)) {
		// bought
		width = Widget_GameText_draw(129, 4, c->xOffset + 40, c->yOffset + 170, FONT_SMALL_BLACK);
		for (int r = Resource_Min; r < Resource_Max; r++) {
			if (trader_bought_resources(traderId, r)) {
				width += Widget_Text_drawNumber(trader_bought_resources(traderId, r),
					'@', " ", c->xOffset + 40 + width, c->yOffset + 170, FONT_SMALL_BLACK);
				int graphicId = image_group(ID_Graphic_ResourceIcons) + r + Resource_getGraphicIdOffset(r, 3);
				Graphics_drawImage(graphicId, c->xOffset + 40 + width, c->yOffset + 167);
				width += 25;
			}
		}
		// sold
		width = Widget_GameText_draw(129, 5, c->xOffset + 40, c->yOffset + 200, FONT_SMALL_BLACK);
		for (int r = Resource_Min; r < Resource_Max; r++) {
			if (trader_sold_resources(traderId, r)) {
				width += Widget_Text_drawNumber(trader_sold_resources(traderId, r),
					'@', " ", c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
				int graphicId = image_group(ID_Graphic_ResourceIcons) + r + Resource_getGraphicIdOffset(r, 3);
				Graphics_drawImage(graphicId, c->xOffset + 40 + width, c->yOffset + 197);
				width += 25;
			}
		}
	} else { // nothing sold/bought (yet)
		// buying
		width = Widget_GameText_draw(129, 2, c->xOffset + 40, c->yOffset + 170, FONT_SMALL_BLACK);
		for (int r = Resource_Min; r < Resource_Max; r++) {
			if (city->buys_resource[r]) {
				int graphicId = image_group(ID_Graphic_ResourceIcons) + r + Resource_getGraphicIdOffset(r, 3);
				Graphics_drawImage(graphicId, c->xOffset + 40 + width, c->yOffset + 167);
				width += 25;
			}
		}
		// selling
		width = Widget_GameText_draw(129, 3, c->xOffset + 40, c->yOffset + 200, FONT_SMALL_BLACK);
		for (int r = Resource_Min; r < Resource_Max; r++) {
			if (city->sells_resource[r]) {
				int graphicId = image_group(ID_Graphic_ResourceIcons) + r + Resource_getGraphicIdOffset(r, 3);
				Graphics_drawImage(graphicId, c->xOffset + 40 + width, c->yOffset + 197);
				width += 25;
			}
		}
	}
}

static void drawFigureInfoEnemy(BuildingInfoContext *c, int figureId)
{
	int graphicId = 8;
	int formationId = Data_Figures[figureId].formationId;
    int enemy_type = formation_get(formationId)->enemy_type;
	switch (Data_Figures[figureId].type) {
		case FIGURE_ENEMY43_SPEAR:
			switch (enemy_type) {
				case EnemyType_5_Pergamum: graphicId = 44; break;
				case EnemyType_6_Seleucid: graphicId = 46; break;
				case EnemyType_7_Etruscan: graphicId = 32; break;
				case EnemyType_8_Greek: graphicId = 36; break;
			}
			break;
		case FIGURE_ENEMY44_SWORD:
			switch (enemy_type) {
				case EnemyType_5_Pergamum: graphicId = 45; break;
				case EnemyType_6_Seleucid: graphicId = 47; break;
				case EnemyType_9_Egyptian: graphicId = 29; break;
			}
			break;
		case FIGURE_ENEMY45_SWORD:
			switch (enemy_type) {
				case EnemyType_7_Etruscan: graphicId = 31; break;
				case EnemyType_8_Greek: graphicId = 37; break;
				case EnemyType_10_Carthaginian: graphicId = 22; break;
			}
			break;
		case FIGURE_ENEMY49_FAST_SWORD:
			switch (enemy_type) {
				case EnemyType_0_Barbarian: graphicId = 21; break;
				case EnemyType_1_Numidian: graphicId = 20; break;
				case EnemyType_4_Goth: graphicId = 35; break;
			}
			break;
		case FIGURE_ENEMY50_SWORD:
			switch (enemy_type) {
				case EnemyType_2_Gaul: graphicId = 40; break;
				case EnemyType_3_Celt: graphicId = 24; break;
			}
			break;
		case FIGURE_ENEMY51_SPEAR:
			switch (enemy_type) {
				case EnemyType_1_Numidian: graphicId = 20; break;
			}
			break;
	}
	Graphics_drawImage(image_group(ID_Graphic_BigPeople) + graphicId - 1,
		c->xOffset + 28, c->yOffset + 112);
	
	Widget_GameText_draw(65, Data_Figures[figureId].name,
		c->xOffset + 90, c->yOffset + 108, FONT_LARGE_BROWN);
	Widget_GameText_draw(37, scenario_property_enemy() + 20,
		c->xOffset + 92, c->yOffset + 149, FONT_SMALL_BLACK);
}

static void drawFigureInfoBoatAnimal(BuildingInfoContext *c, int figureId)
{
	Graphics_drawImage(BigPeopleGraphic(Data_Figures[figureId].type),
		c->xOffset + 28, c->yOffset + 112);
	
	Widget_GameText_draw(64, Data_Figures[figureId].type,
		c->xOffset + 92, c->yOffset + 139, FONT_SMALL_BLACK);
}

static void drawFigureInfoCartpusher(BuildingInfoContext *c, int figureId)
{
	Graphics_drawImage(BigPeopleGraphic(Data_Figures[figureId].type),
		c->xOffset + 28, c->yOffset + 112);
	
	Widget_GameText_draw(65, Data_Figures[figureId].name,
		c->xOffset + 90, c->yOffset + 108, FONT_LARGE_BROWN);
	int width = Widget_GameText_draw(64, Data_Figures[figureId].type,
		c->xOffset + 92, c->yOffset + 139, FONT_SMALL_BLACK);
	
	if (Data_Figures[figureId].actionState != FigureActionState_132_DockerIdling &&
		Data_Figures[figureId].resourceId) {
		int resource = Data_Figures[figureId].resourceId;
		Graphics_drawImage(image_group(ID_Graphic_ResourceIcons) +
			resource + Resource_getGraphicIdOffset(resource, 3),
			c->xOffset + 92 + width, c->yOffset + 135);
	}
	
	Widget_GameText_drawMultiline(130, 21 * c->figure.soundId + c->figure.phraseId + 1,
		c->xOffset + 90, c->yOffset + 160, 16 * (c->widthBlocks - 9), FONT_SMALL_BLACK);
	
	if (!Data_Figures[figureId].buildingId) {
		return;
	}
	int sourceBuildingId = Data_Figures[figureId].buildingId;
	int targetBuildingId = Data_Figures[figureId].destinationBuildingId;
	int isReturning = 0;
	switch (Data_Figures[figureId].actionState) {
		case FigureActionState_27_CartpusherReturning:
		case FigureActionState_53_WarehousemanReturningEmpty:
		case FigureActionState_56_WarehousemanReturningWithFood:
		case FigureActionState_59_WarehousemanReturningWithResource:
		case FigureActionState_134_DockerExportQueue:
		case FigureActionState_137_DockerExportReturning:
		case FigureActionState_138_DockerImportReturning:
			isReturning = 1;
			break;
	}
	if (Data_Figures[figureId].actionState != FigureActionState_132_DockerIdling) {
		if (isReturning) {
			width = Widget_GameText_draw(129, 16,
				c->xOffset + 40, c->yOffset + 200, FONT_SMALL_BLACK);
			width += Widget_GameText_draw(41, Data_Buildings[sourceBuildingId].type,
				c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
			width += Widget_GameText_draw(129, 14,
				c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
			width += Widget_GameText_draw(41, Data_Buildings[targetBuildingId].type,
				c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
		} else {
			width = Widget_GameText_draw(129, 15,
				c->xOffset + 40, c->yOffset + 200, FONT_SMALL_BLACK);
			width += Widget_GameText_draw(41, Data_Buildings[targetBuildingId].type,
				c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
			width += Widget_GameText_draw(129, 14,
				c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
			width += Widget_GameText_draw(41, Data_Buildings[sourceBuildingId].type,
				c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
		}
	}
}

static void drawFigureInfoMarketBuyer(BuildingInfoContext *c, int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	Graphics_drawImage(BigPeopleGraphic(f->type),
		c->xOffset + 28, c->yOffset + 112);
	
	Widget_GameText_draw(65, f->name,
		c->xOffset + 90, c->yOffset + 108, FONT_LARGE_BROWN);
	int width = Widget_GameText_draw(64, f->type,
		c->xOffset + 92, c->yOffset + 139, FONT_SMALL_BLACK);
	
	if (f->actionState == FigureActionState_145_MarketBuyerGoingToStorage) {
		width += Widget_GameText_draw(129, 17,
			c->xOffset + 90 + width, c->yOffset + 139, FONT_SMALL_BLACK);
		int resourceId = collectingItemIdToResourceId(f->collectingItemId);
		Graphics_drawImage(
			image_group(ID_Graphic_ResourceIcons) + resourceId + Resource_getGraphicIdOffset(resourceId, 3),
			c->xOffset + 90 + width, c->yOffset + 135);
	} else if (f->actionState == FigureActionState_146_MarketBuyerReturning) {
		width += Widget_GameText_draw(129, 18,
			c->xOffset + 90 + width, c->yOffset + 139, FONT_SMALL_BLACK);
		int resourceId = collectingItemIdToResourceId(f->collectingItemId);
		Graphics_drawImage(
			image_group(ID_Graphic_ResourceIcons) + resourceId + Resource_getGraphicIdOffset(resourceId, 3),
			c->xOffset + 90 + width, c->yOffset + 135);
	}
	if (c->figure.phraseId >= 0) {
		Widget_GameText_drawMultiline(130, 21 * c->figure.soundId + c->figure.phraseId + 1,
			c->xOffset + 90, c->yOffset + 160, 16 * (c->widthBlocks - 9), FONT_SMALL_BLACK);
	}
}

static void drawFigureInfoNormal(BuildingInfoContext *c, int figureId)
{
	int graphicId = BigPeopleGraphic(Data_Figures[figureId].type);
	if (Data_Figures[figureId].actionState == FigureActionState_74_PrefectGoingToFire ||
		Data_Figures[figureId].actionState == FigureActionState_75_PrefectAtFire) {
		graphicId = image_group(ID_Graphic_BigPeople) + 18;
	}
	Graphics_drawImage(graphicId, c->xOffset + 28, c->yOffset + 112);
	
	Widget_GameText_draw(65, Data_Figures[figureId].name,
		c->xOffset + 90, c->yOffset + 108, FONT_LARGE_BROWN);
	Widget_GameText_draw(64, Data_Figures[figureId].type,
		c->xOffset + 92, c->yOffset + 139, FONT_SMALL_BLACK);
	
	if (c->figure.phraseId >= 0) {
		Widget_GameText_drawMultiline(130, 21 * c->figure.soundId + c->figure.phraseId + 1,
			c->xOffset + 90, c->yOffset + 160, 16 * (c->widthBlocks - 9), FONT_SMALL_BLACK);
	}
}

static void drawFigureInfo(BuildingInfoContext *c, int figureId)
{
	Widget_Panel_drawButtonBorder(c->xOffset + 24, c->yOffset + 102, 16 * (c->widthBlocks - 3), 122, 0);
	
	int type = Data_Figures[figureId].type;
	if (type == FIGURE_TRADE_CARAVAN || type == FIGURE_TRADE_CARAVAN_DONKEY || type == FIGURE_TRADE_SHIP) {
		drawFigureInfoTrade(c, figureId);
	} else if (FigureIsEnemy(type)) {
		drawFigureInfoEnemy(c, figureId);
	} else if (type == FIGURE_FISHING_BOAT || type == FIGURE_SHIPWRECK ||
			type == FIGURE_SHEEP || type == FIGURE_WOLF || type == FIGURE_ZEBRA) {
		drawFigureInfoBoatAnimal(c, figureId);
	} else if (type == FIGURE_CART_PUSHER || type == FIGURE_WAREHOUSEMAN || type == FIGURE_DOCKMAN) {
		drawFigureInfoCartpusher(c, figureId);
	} else if (type == FIGURE_MARKET_BUYER) {
		drawFigureInfoMarketBuyer(c, figureId);
	} else {
		drawFigureInfoNormal(c, figureId);
	}
}

void UI_BuildingInfo_drawFigureList(BuildingInfoContext *c)
{
	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 40,
		c->widthBlocks - 2, 12);
	if (c->figure.count <= 0) {
		Widget_GameText_drawCentered(70, 0, c->xOffset, c->yOffset + 120,
			16 * c->widthBlocks, FONT_SMALL_BLACK);
	} else {
		for (int i = 0; i < c->figure.count; i++) {
			Widget_Panel_drawButtonBorder(
				c->xOffset + 60 * i + 25, c->yOffset + 45,
				52, 52, i == c->figure.selectedIndex);
			Graphics_loadFromBuffer(
				c->xOffset + 27 + 60 * i, c->yOffset + 47,
				48, 48, figureImages[i]);
		}
		drawFigureInfo(c, c->figure.figureIds[c->figure.selectedIndex]);
	}
	c->figure.drawn = 1;
}

static void drawFigureInCity(int figureId, struct UI_CityPixelCoordinate *coord)
{
	int xCam = Data_Settings_Map.camera.x;
	int yCam = Data_Settings_Map.camera.y;

	int gridOffset = Data_Figures[figureId].gridOffset;
	int x, y;
	CityView_gridOffsetToXYCoords(gridOffset, &x, &y);
	Data_Settings_Map.camera.x = x - 2;
	Data_Settings_Map.camera.y = y - 6;
	CityView_checkCameraBoundaries();
	UI_CityBuildings_drawForegroundForFigure(
		Data_Settings_Map.camera.x, Data_Settings_Map.camera.y,
		figureId, coord);

	Data_Settings_Map.camera.x = xCam;
	Data_Settings_Map.camera.y = yCam;
}

void UI_BuildingInfo_drawFigureImagesLocal(BuildingInfoContext *c)
{
	if (c->figure.count > 0) {
		struct UI_CityPixelCoordinate coord = {0, 0};
		for (int i = 0; i < c->figure.count; i++) {
			drawFigureInCity(c->figure.figureIds[i], &coord);
			Graphics_saveToBuffer(coord.x, coord.y, 48, 48, figureImages[i]);
		}
		UI_CityBuildings_drawForeground(Data_Settings_Map.camera.x, Data_Settings_Map.camera.y);
	}
}

void UI_BuildingInfo_playFigurePhrase(BuildingInfoContext *c)
{
	int figureId = c->figure.figureIds[c->figure.selectedIndex];
	c->figure.soundId = Figure_playPhrase(figureId);
	c->figure.phraseId = Data_Figures[figureId].phraseId;
}

void UI_BuildingInfo_handleMouseFigureList(BuildingInfoContext *c)
{
	contextForCallback = c;
	Widget_Button_handleCustomButtons(c->xOffset, c->yOffset,
		figureButtons, c->figure.count, &focusButtonId);
	contextForCallback = 0;
}

static void selectFigure(int index, int param2)
{
	contextForCallback->figure.selectedIndex = index;
	UI_BuildingInfo_playFigurePhrase(contextForCallback);
	UI_Window_requestRefresh();
}
