#include "BuildingInfo.h"

#include "CityBuildings.h"
#include "Window.h"
#include "../CityView.h"
#include "../Graphics.h"
#include "../Resource.h"
#include "../Walker.h"
#include "../Widget.h"

#include "../Data/Building.h"
#include "../Data/Constants.h"
#include "../Data/Empire.h"
#include "../Data/Formation.h"
#include "../Data/Scenario.h"
#include "../Data/Settings.h"
#include "../Data/Walker.h"

static void selectWalker(int param1, int param2);

static const int walkerTypeToBigPeopleGraphicId[] = {
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

#define BigPeopleGraphic(t) (GraphicId(ID_Graphic_BigPeople) + walkerTypeToBigPeopleGraphicId[t] - 1)

static CustomButton walkerButtons[] = {
	{26, 46, 76, 96, selectWalker, Widget_Button_doNothing, 1, 0, 0},
	{86, 46, 136, 96, selectWalker, Widget_Button_doNothing, 1, 1, 0},
	{146, 46, 196, 96, selectWalker, Widget_Button_doNothing, 1, 2, 0},
	{206, 46, 256, 96, selectWalker, Widget_Button_doNothing, 1, 3, 0},
	{266, 46, 316, 96, selectWalker, Widget_Button_doNothing, 1, 4, 0},
	{326, 46, 376, 96, selectWalker, Widget_Button_doNothing, 1, 5, 0},
	{386, 46, 436, 96, selectWalker, Widget_Button_doNothing, 1, 6, 0},
};

static ScreenColor walkerImages[7][48*48];

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

static void drawWalkerInfoTrade(BuildingInfoContext *c, int walkerId)
{
	while (Data_Walkers[walkerId].type == Walker_TradeCaravanDonkey) {
		walkerId = Data_Walkers[walkerId].inFrontWalkerId;
	}
	int cityId = Data_Walkers[walkerId].empireCityId;
	int width = Widget_GameText_draw(64, Data_Walkers[walkerId].type,
		c->xOffset + 40, c->yOffset + 110, Font_SmallBlack);
	Widget_GameText_draw(21, Data_Empire_Cities[cityId].cityNameId,
		c->xOffset + 40 + width, c->yOffset + 110, Font_SmallBlack);
	
	width = Widget_GameText_draw(129, 1,
		c->xOffset + 40, c->yOffset + 130, Font_SmallBlack);
	Widget_GameText_drawNumberWithDescription(8, 10,
		Data_Walkers[walkerId].type == Walker_TradeShip ? 12 : 8,
		c->xOffset + 40 + width, c->yOffset + 130, Font_SmallBlack);
	
	int traderId = Data_Walkers[walkerId].traderId;
	if (Data_Walkers[walkerId].type == Walker_TradeShip) {
		int textId;
		switch (Data_Walkers[walkerId].actionState) {
			case WalkerActionState_114_TradeShipAnchored: textId = 6; break;
			case WalkerActionState_112_TradeShipMoored: textId = 7; break;
			case WalkerActionState_115_TradeShipLeaving: textId = 8; break;
			default: textId = 9; break;
		}
		Widget_GameText_draw(129, textId, c->xOffset + 40, c->yOffset + 150, Font_SmallBlack);
	} else {
		int textId;
		switch (Data_Walkers[walkerId].actionState) {
			case WalkerActionState_101_TradeCaravanArriving:
				textId = 12;
				break;
			case WalkerActionState_102_TradeCaravanTrading:
				textId = 10;
				break;
			case WalkerActionState_103_TradeCaravanLeaving:
				if (Data_Walker_Traders[traderId].totalSold || Data_Walker_Traders[traderId].totalBought) {
					textId = 11;
				} else {
					textId = 13;
				}
				break;
			default:
				textId = 11;
				break;
		}
		Widget_GameText_draw(129, textId, c->xOffset + 40, c->yOffset + 150, Font_SmallBlack);
	}
	if (Data_Walker_Traders[traderId].totalSold || Data_Walker_Traders[traderId].totalBought) {
		// bought
		width = Widget_GameText_draw(129, 4, c->xOffset + 40, c->yOffset + 170, Font_SmallBlack);
		for (int r = 1; r < 16; r++) {
			if (Data_Walker_Traders[traderId].boughtResources[r]) {
				width += Widget_Text_drawNumber(Data_Walker_Traders[traderId].boughtResources[r],
					'@', " ", c->xOffset + 40 + width, c->yOffset + 170, Font_SmallBlack);
				int graphicId = GraphicId(ID_Graphic_ResourceIcons) + r + Resource_getGraphicIdOffset(r, 3);
				Graphics_drawImage(graphicId, c->xOffset + 40 + width, c->yOffset + 167);
				width += 25;
			}
		}
		// sold
		width = Widget_GameText_draw(129, 5, c->xOffset + 40, c->yOffset + 200, Font_SmallBlack);
		for (int r = 1; r < 16; r++) {
			if (Data_Walker_Traders[traderId].soldResources[r]) {
				width += Widget_Text_drawNumber(Data_Walker_Traders[traderId].soldResources[r],
					'@', " ", c->xOffset + 40 + width, c->yOffset + 200, Font_SmallBlack);
				int graphicId = GraphicId(ID_Graphic_ResourceIcons) + r + Resource_getGraphicIdOffset(r, 3);
				Graphics_drawImage(graphicId, c->xOffset + 40 + width, c->yOffset + 197);
				width += 25;
			}
		}
	} else { // nothing sold/bought (yet)
		// buying
		width = Widget_GameText_draw(129, 2, c->xOffset + 40, c->yOffset + 170, Font_SmallBlack);
		for (int r = 1; r < 16; r++) {
			if (Data_Empire_Cities[cityId].buysResourceFlag[r]) {
				int graphicId = GraphicId(ID_Graphic_ResourceIcons) + r + Resource_getGraphicIdOffset(r, 3);
				Graphics_drawImage(graphicId, c->xOffset + 40 + width, c->yOffset + 167);
				width += 25;
			}
		}
		// selling
		width = Widget_GameText_draw(129, 3, c->xOffset + 40, c->yOffset + 200, Font_SmallBlack);
		for (int r = 1; r < 16; r++) {
			if (Data_Empire_Cities[cityId].sellsResourceFlag[r]) {
				int graphicId = GraphicId(ID_Graphic_ResourceIcons) + r + Resource_getGraphicIdOffset(r, 3);
				Graphics_drawImage(graphicId, c->xOffset + 40 + width, c->yOffset + 197);
				width += 25;
			}
		}
	}
}

static void drawWalkerInfoEnemy(BuildingInfoContext *c, int walkerId)
{
	int graphicId = 8;
	int formationId = Data_Walkers[walkerId].formationId;
	switch (Data_Walkers[walkerId].type) {
		case Walker_Enemy43:
		switch (Data_Formations[formationId].enemyType) {
			case 5: graphicId = 44; break;
			case 6: graphicId = 46; break;
			case 7: graphicId = 32; break;
			case 8: graphicId = 36; break;
		}
		break;
		case Walker_Enemy44:
		switch (Data_Formations[formationId].enemyType) {
			case 5: graphicId = 45; break;
			case 6: graphicId = 47; break;
			case 9: graphicId = 29; break;
		}
		break;
		case Walker_Enemy45:
		switch (Data_Formations[formationId].enemyType) {
			case 7: graphicId = 31; break;
			case 8: graphicId = 37; break;
			case 10:graphicId = 22; break;
		}
		break;
		case Walker_Enemy49:
		switch (Data_Formations[formationId].enemyType) {
			case 0: graphicId = 21; break;
			case 1: graphicId = 20; break;
			case 4: graphicId = 35; break;
		}
		break;
		case Walker_Enemy50:
		switch (Data_Formations[formationId].enemyType) {
			case 2: graphicId = 40; break;
			case 3: graphicId = 24; break;
		}
		break;
		case Walker_Enemy51:
		switch (Data_Formations[formationId].enemyType) {
			case 1: graphicId = 20; break;
		}
		break;
	}
	Graphics_drawImage(GraphicId(ID_Graphic_BigPeople) + graphicId - 1,
		c->xOffset + 28, c->yOffset + 112);
	
	Widget_GameText_draw(65, Data_Walkers[walkerId].name,
		c->xOffset + 90, c->yOffset + 108, Font_LargeBrown);
	Widget_GameText_draw(37, Data_Scenario.enemyId + 20,
		c->xOffset + 92, c->yOffset + 149, Font_SmallBlack);
}

static void drawWalkerInfoBoatAnimal(BuildingInfoContext *c, int walkerId)
{
	Graphics_drawImage(BigPeopleGraphic(Data_Walkers[walkerId].type),
		c->xOffset + 28, c->yOffset + 112);
	
	Widget_GameText_draw(64, Data_Walkers[walkerId].type,
		c->xOffset + 92, c->yOffset + 139, Font_SmallBlack);
}

static void drawWalkerInfoCartpusher(BuildingInfoContext *c, int walkerId)
{
	Graphics_drawImage(BigPeopleGraphic(Data_Walkers[walkerId].type),
		c->xOffset + 28, c->yOffset + 112);
	
	Widget_GameText_draw(65, Data_Walkers[walkerId].name,
		c->xOffset + 90, c->yOffset + 108, Font_LargeBrown);
	int width = Widget_GameText_draw(64, Data_Walkers[walkerId].type,
		c->xOffset + 92, c->yOffset + 139, Font_SmallBlack);
	
	if (Data_Walkers[walkerId].actionState != WalkerActionState_132_ResourceCarrier &&
		Data_Walkers[walkerId].resourceId) {
		int resource = Data_Walkers[walkerId].resourceId;
		Graphics_drawImage(GraphicId(ID_Graphic_ResourceIcons) + resource + Resource_getGraphicIdOffset(resource, 3),
			c->xOffset + 92 + width, c->yOffset + 135);
	}
	
	Widget_GameText_drawMultiline(130, 21 * c->walker.soundId + c->walker.phraseId + 1,
		c->xOffset + 90, c->yOffset + 160, 16 * (c->widthBlocks - 9), Font_SmallBlack);
	
	if (!Data_Walkers[walkerId].buildingId) {
		return;
	}
	int sourceBuildingId = Data_Walkers[walkerId].buildingId;
	int targetBuildingId = Data_Walkers[walkerId].destinationBuildingId;
	int isReturning = 0;
	switch (Data_Walkers[walkerId].actionState) {
		case WalkerActionState_27_MarketBuyer:
		case WalkerActionState_53_MarketBuyer:
		case WalkerActionState_56_MarketBuyer:
		case WalkerActionState_59_MarketBuyer:
		case WalkerActionState_134_Dockman_MarketBuyer:
		case WalkerActionState_137_Dockman_MarketBuyer:
		case WalkerActionState_138_Dockman_MarketBuyer:
			isReturning = 1;
			break;
	}
	if (Data_Walkers[walkerId].actionState != WalkerActionState_132_ResourceCarrier) {
		if (isReturning) {
			width = Widget_GameText_draw(129, 16,
				c->xOffset + 40 + width, c->yOffset + 200, Font_SmallBlack);
			width += Widget_GameText_draw(41, Data_Buildings[sourceBuildingId].type,
				c->xOffset + 40 + width, c->yOffset + 200, Font_SmallBlack);
			width += Widget_GameText_draw(129, 14,
				c->xOffset + 40 + width, c->yOffset + 200, Font_SmallBlack);
			width += Widget_GameText_draw(41, Data_Buildings[targetBuildingId].type,
				c->xOffset + 40 + width, c->yOffset + 200, Font_SmallBlack);
		} else {
			width = Widget_GameText_draw(129, 15,
				c->xOffset + 40 + width, c->yOffset + 200, Font_SmallBlack);
			width += Widget_GameText_draw(41, Data_Buildings[targetBuildingId].type,
				c->xOffset + 40 + width, c->yOffset + 200, Font_SmallBlack);
			width += Widget_GameText_draw(129, 14,
				c->xOffset + 40 + width, c->yOffset + 200, Font_SmallBlack);
			width += Widget_GameText_draw(41, Data_Buildings[sourceBuildingId].type,
				c->xOffset + 40 + width, c->yOffset + 200, Font_SmallBlack);
		}
	}
}

static void drawWalkerInfoMarketBuyer(BuildingInfoContext *c, int walkerId)
{
	Graphics_drawImage(BigPeopleGraphic(Data_Walkers[walkerId].type),
		c->xOffset + 28, c->yOffset + 112);
	
	Widget_GameText_draw(65, Data_Walkers[walkerId].name,
		c->xOffset + 90, c->yOffset + 108, Font_LargeBrown);
	int width = Widget_GameText_draw(64, Data_Walkers[walkerId].type,
		c->xOffset + 92, c->yOffset + 139, Font_SmallBlack);
	
	if (Data_Walkers[walkerId].actionState == WalkerActionState_145_MarketBuyerCreated) {
		width += Widget_GameText_draw(129, 17,
			c->xOffset + 90 + width, c->yOffset + 139, Font_SmallBlack);
		int resourceId = collectingItemIdToResourceId(Data_Walkers[walkerId].collectingItemId);
		Graphics_drawImage(
			GraphicId(ID_Graphic_ResourceIcons) + resourceId + Resource_getGraphicIdOffset(resourceId, 3),
			c->xOffset + 90 + width, c->yOffset + 135);
	} else if (Data_Walkers[walkerId].actionState == WalkerActionState_146_MarketBuyer) {
		width += Widget_GameText_draw(129, 18,
			c->xOffset + 90 + width, c->yOffset + 139, Font_SmallBlack);
		int resourceId = collectingItemIdToResourceId(Data_Walkers[walkerId].collectingItemId);
		Graphics_drawImage(
			GraphicId(ID_Graphic_ResourceIcons) + resourceId + Resource_getGraphicIdOffset(resourceId, 3),
			c->xOffset + 90 + width, c->yOffset + 135);
	}
	if (c->walker.phraseId >= 0) {
		Widget_GameText_drawMultiline(130, 21 * c->walker.soundId + c->walker.phraseId + 1,
			c->xOffset + 90, c->yOffset + 160, 16 * (c->widthBlocks - 9), Font_SmallBlack);
	}
}

static void drawWalkerInfoNormal(BuildingInfoContext *c, int walkerId)
{
	int graphicId = BigPeopleGraphic(Data_Walkers[walkerId].type);
	if (Data_Walkers[walkerId].actionState == WalkerActionState_74_PrefectGoingToFire ||
		Data_Walkers[walkerId].actionState == WalkerActionState_75_PrefectFightingFire) {
		graphicId = GraphicId(ID_Graphic_BigPeople) + 18;
	}
	Graphics_drawImage(graphicId, c->xOffset + 28, c->yOffset + 112);
	
	Widget_GameText_draw(65, Data_Walkers[walkerId].name,
		c->xOffset + 90, c->yOffset + 108, Font_LargeBrown);
	Widget_GameText_draw(64, Data_Walkers[walkerId].type,
		c->xOffset + 92, c->yOffset + 139, Font_SmallBlack);
	
	if (c->walker.phraseId >= 0) {
		Widget_GameText_drawMultiline(130, 21 * c->walker.soundId + c->walker.phraseId + 1,
			c->xOffset + 90, c->yOffset + 160, 16 * (c->widthBlocks - 9), Font_SmallBlack);
	}
}

static void drawWalkerInfo(BuildingInfoContext *c, int walkerId)
{
	Widget_Panel_drawButtonBorder(c->xOffset + 24, c->yOffset + 102, 16 * (c->widthBlocks - 3), 122, 0);
	
	int type = Data_Walkers[walkerId].type;
	if (type == Walker_TradeCaravan || type == Walker_TradeCaravanDonkey || type == Walker_TradeShip) {
		drawWalkerInfoTrade(c, walkerId);
	} else if (WalkerIsEnemy(type)) {
		drawWalkerInfoEnemy(c, walkerId);
	} else if (type == Walker_FishingBoat || type == Walker_Shipwreck ||
			type == Walker_Sheep || type == Walker_Wolf || type == Walker_Zebra) {
		drawWalkerInfoBoatAnimal(c, walkerId);
	} else if (type == Walker_CartPusher && type == Walker_Warehouseman && type == Walker_Dockman) {
		drawWalkerInfoCartpusher(c, walkerId);
	} else if (type == Walker_MarketBuyer) {
		drawWalkerInfoMarketBuyer(c, walkerId);
	} else {
		drawWalkerInfoNormal(c, walkerId);
	}
}

void UI_BuildingInfo_drawWalkerList(BuildingInfoContext *c)
{
	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 40,
		c->widthBlocks - 2, 12);
	if (c->walker.count <= 0) {
		Widget_GameText_drawCentered(70, 0, c->xOffset, c->yOffset + 120,
			16 * c->widthBlocks, Font_SmallBlack);
	} else {
		for (int i = 0; i < c->walker.count; i++) {
			Widget_Panel_drawButtonBorder(
				c->xOffset + 60 * i + 25, c->yOffset + 45,
				52, 52, i == c->walker.selectedIndex);
			Graphics_loadFromBuffer(
				c->xOffset + 27 + 60 * i, c->yOffset + 47,
				48, 48, walkerImages[i]);
		}
		drawWalkerInfo(c, c->walker.walkerIds[c->walker.selectedIndex]);
	}
	c->walker.drawn = 1;
}

static void drawWalkerInCity(int walkerId, UI_CityPixelCoordinate *coord)
{
	int xCam = Data_Settings_Map.camera.x;
	int yCam = Data_Settings_Map.camera.y;

	int gridOffset = Data_Walkers[walkerId].gridOffset;
	int x, y;
	CityView_gridOffsetToXYCoords(gridOffset, &x, &y);
	Data_Settings_Map.camera.x = x - 2;
	Data_Settings_Map.camera.y = y - 6;
	CityView_checkCameraBoundaries();
	// TODO setIsometricViewportForWalker()
	UI_CityBuildings_drawForegroundForWalker(
		Data_Settings_Map.camera.x, Data_Settings_Map.camera.y,
		walkerId, coord);


	Data_Settings_Map.camera.x = xCam;
	Data_Settings_Map.camera.y = yCam;
	// TODO setCityViewport_(with|without)Panel?
}

void UI_BuildingInfo_drawWalkerImagesLocal(BuildingInfoContext *c)
{
	if (c->walker.count > 0) {
		UI_CityPixelCoordinate coord;
		for (int i = 0; i < c->walker.count; i++) {
			drawWalkerInCity(c->walker.walkerIds[i], &coord);
			Graphics_saveToBuffer(coord.x, coord.y, 48, 48, walkerImages[i]);
		}
		UI_CityBuildings_drawForeground(Data_Settings_Map.camera.x, Data_Settings_Map.camera.y);
	}
}

void UI_BuildingInfo_playWalkerPhrase(BuildingInfoContext *c)
{
	int walkerId = c->walker.walkerIds[c->walker.selectedIndex];
	c->walker.soundId = Walker_playPhrase(walkerId);
	c->walker.phraseId = Data_Walkers[walkerId].phraseId;
}

void UI_BuildingInfo_handleMouseWalkerList(BuildingInfoContext *c)
{
	contextForCallback = c;
	Widget_Button_handleCustomButtons(c->xOffset, c->yOffset,
		walkerButtons, c->walker.count, &focusButtonId);
	contextForCallback = 0;
}

static void selectWalker(int index, int param2)
{
	contextForCallback->walker.selectedIndex = index;
	UI_BuildingInfo_playWalkerPhrase(contextForCallback);
	UI_Window_requestRefresh();
}
