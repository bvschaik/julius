#include "BuildingInfo.h"

#include "CityBuildings.h"
#include "Window.h"
#include "../Graphics.h"

#include "../Data/State.h"

#include "building/building.h"
#include "city/view.h"
#include "empire/city.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "figure/phrase.h"
#include "figure/trader.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "scenario/property.h"

static void selectFigure(int index, int param2);

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

#define BigPeopleGraphic(t) (image_group(GROUP_BIG_PEOPLE) + figureTypeToBigPeopleGraphicId[t] - 1)

static generic_button figureButtons[] = {
	{26, 46, 76, 96, GB_IMMEDIATE, selectFigure, button_none, 0, 0},
	{86, 46, 136, 96, GB_IMMEDIATE, selectFigure, button_none, 1, 0},
	{146, 46, 196, 96, GB_IMMEDIATE, selectFigure, button_none, 2, 0},
	{206, 46, 256, 96, GB_IMMEDIATE, selectFigure, button_none, 3, 0},
	{266, 46, 316, 96, GB_IMMEDIATE, selectFigure, button_none, 4, 0},
	{326, 46, 376, 96, GB_IMMEDIATE, selectFigure, button_none, 5, 0},
	{386, 46, 436, 96, GB_IMMEDIATE, selectFigure, button_none, 6, 0},
};

static color_t figureImages[7][48*48];

static int focusButtonId;
static BuildingInfoContext *contextForCallback;

static int collectingItemIdToResourceId(int c)
{
	switch (c) {
		case 0: return RESOURCE_WHEAT;
		case 1: return RESOURCE_VEGETABLES;
		case 2: return RESOURCE_FRUIT;
		case 3: return RESOURCE_MEAT;
		case 4: return RESOURCE_WINE;
		case 5: return RESOURCE_OIL;
		case 6: return RESOURCE_FURNITURE;
		case 7: return RESOURCE_POTTERY;
		default: return 0;
	}
}

static figure *get_head_of_caravan(figure *f)
{
    while (f->type == FIGURE_TRADE_CARAVAN_DONKEY) {
        f = figure_get(f->inFrontFigureId);
    }
    return f;
}

static void drawFigureInfoTrade(BuildingInfoContext *c, figure *f)
{
	f = get_head_of_caravan(f);
	const empire_city *city = empire_city_get(f->empireCityId);
	int width = lang_text_draw(64, f->type,
		c->xOffset + 40, c->yOffset + 110, FONT_SMALL_BLACK);
	lang_text_draw(21, city->name_id,
		c->xOffset + 40 + width, c->yOffset + 110, FONT_SMALL_BLACK);
	
	width = lang_text_draw(129, 1,
		c->xOffset + 40, c->yOffset + 130, FONT_SMALL_BLACK);
	lang_text_draw_amount(8, 10, f->type == FIGURE_TRADE_SHIP ? 12 : 8,
		c->xOffset + 40 + width, c->yOffset + 130, FONT_SMALL_BLACK);
	
	int traderId = f->traderId;
	if (f->type == FIGURE_TRADE_SHIP) {
		int textId;
		switch (f->actionState) {
			case FIGURE_ACTION_114_TRADE_SHIP_ANCHORED: textId = 6; break;
			case FIGURE_ACTION_112_TRADE_SHIP_MOORED: textId = 7; break;
			case FIGURE_ACTION_115_TRADE_SHIP_LEAVING: textId = 8; break;
			default: textId = 9; break;
		}
		lang_text_draw(129, textId, c->xOffset + 40, c->yOffset + 150, FONT_SMALL_BLACK);
	} else {
		int textId;
		switch (f->actionState) {
			case FIGURE_ACTION_101_TRADE_CARAVAN_ARRIVING:
				textId = 12;
				break;
			case FIGURE_ACTION_102_TRADE_CARAVAN_TRADING:
				textId = 10;
				break;
			case FIGURE_ACTION_103_TRADE_CARAVAN_LEAVING:
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
		lang_text_draw(129, textId, c->xOffset + 40, c->yOffset + 150, FONT_SMALL_BLACK);
	}
	if (trader_has_traded(traderId)) {
		// bought
		width = lang_text_draw(129, 4, c->xOffset + 40, c->yOffset + 170, FONT_SMALL_BLACK);
		for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
			if (trader_bought_resources(traderId, r)) {
				width += text_draw_number(trader_bought_resources(traderId, r),
					'@', " ", c->xOffset + 40 + width, c->yOffset + 170, FONT_SMALL_BLACK);
				int graphicId = image_group(GROUP_RESOURCE_ICONS) + r + resource_image_offset(r, RESOURCE_IMAGE_ICON);
				image_draw(graphicId, c->xOffset + 40 + width, c->yOffset + 167);
				width += 25;
			}
		}
		// sold
		width = lang_text_draw(129, 5, c->xOffset + 40, c->yOffset + 200, FONT_SMALL_BLACK);
		for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
			if (trader_sold_resources(traderId, r)) {
				width += text_draw_number(trader_sold_resources(traderId, r),
					'@', " ", c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
				int graphicId = image_group(GROUP_RESOURCE_ICONS) + r + resource_image_offset(r, RESOURCE_IMAGE_ICON);
				image_draw(graphicId, c->xOffset + 40 + width, c->yOffset + 197);
				width += 25;
			}
		}
	} else { // nothing sold/bought (yet)
		// buying
		width = lang_text_draw(129, 2, c->xOffset + 40, c->yOffset + 170, FONT_SMALL_BLACK);
		for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
			if (city->buys_resource[r]) {
				int graphicId = image_group(GROUP_RESOURCE_ICONS) + r + resource_image_offset(r, RESOURCE_IMAGE_ICON);
				image_draw(graphicId, c->xOffset + 40 + width, c->yOffset + 167);
				width += 25;
			}
		}
		// selling
		width = lang_text_draw(129, 3, c->xOffset + 40, c->yOffset + 200, FONT_SMALL_BLACK);
		for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
			if (city->sells_resource[r]) {
				int graphicId = image_group(GROUP_RESOURCE_ICONS) + r + resource_image_offset(r, RESOURCE_IMAGE_ICON);
				image_draw(graphicId, c->xOffset + 40 + width, c->yOffset + 197);
				width += 25;
			}
		}
	}
}

static void drawFigureInfoEnemy(BuildingInfoContext *c, figure *f)
{
	int graphicId = 8;
    int enemy_type = formation_get(f->formationId)->enemy_type;
	switch (f->type) {
		case FIGURE_ENEMY43_SPEAR:
			switch (enemy_type) {
				case ENEMY_5_PERGAMUM: graphicId = 44; break;
				case ENEMY_6_SELEUCID: graphicId = 46; break;
				case ENEMY_7_ETRUSCAN: graphicId = 32; break;
				case ENEMY_8_GREEK: graphicId = 36; break;
			}
			break;
		case FIGURE_ENEMY44_SWORD:
			switch (enemy_type) {
				case ENEMY_5_PERGAMUM: graphicId = 45; break;
				case ENEMY_6_SELEUCID: graphicId = 47; break;
				case ENEMY_9_EGYPTIAN: graphicId = 29; break;
			}
			break;
		case FIGURE_ENEMY45_SWORD:
			switch (enemy_type) {
				case ENEMY_7_ETRUSCAN: graphicId = 31; break;
				case ENEMY_8_GREEK: graphicId = 37; break;
				case ENEMY_10_CARTHAGINIAN: graphicId = 22; break;
			}
			break;
		case FIGURE_ENEMY49_FAST_SWORD:
			switch (enemy_type) {
				case ENEMY_0_BARBARIAN: graphicId = 21; break;
				case ENEMY_1_NUMIDIAN: graphicId = 20; break;
				case ENEMY_4_GOTH: graphicId = 35; break;
			}
			break;
		case FIGURE_ENEMY50_SWORD:
			switch (enemy_type) {
				case ENEMY_2_GAUL: graphicId = 40; break;
				case ENEMY_3_CELT: graphicId = 24; break;
			}
			break;
		case FIGURE_ENEMY51_SPEAR:
			switch (enemy_type) {
				case ENEMY_1_NUMIDIAN: graphicId = 20; break;
			}
			break;
	}
	image_draw(image_group(GROUP_BIG_PEOPLE) + graphicId - 1,
		c->xOffset + 28, c->yOffset + 112);
	
	lang_text_draw(65, f->name,
		c->xOffset + 90, c->yOffset + 108, FONT_LARGE_BROWN);
	lang_text_draw(37, scenario_property_enemy() + 20,
		c->xOffset + 92, c->yOffset + 149, FONT_SMALL_BLACK);
}

static void drawFigureInfoBoatAnimal(BuildingInfoContext *c, figure *f)
{
	image_draw(BigPeopleGraphic(f->type),
		c->xOffset + 28, c->yOffset + 112);
	
	lang_text_draw(64, f->type,
		c->xOffset + 92, c->yOffset + 139, FONT_SMALL_BLACK);
}

static void drawFigureInfoCartpusher(BuildingInfoContext *c, figure *f)
{
	image_draw(BigPeopleGraphic(f->type),
		c->xOffset + 28, c->yOffset + 112);
	
	lang_text_draw(65, f->name,
		c->xOffset + 90, c->yOffset + 108, FONT_LARGE_BROWN);
	int width = lang_text_draw(64, f->type,
		c->xOffset + 92, c->yOffset + 139, FONT_SMALL_BLACK);
	
	if (f->actionState != FIGURE_ACTION_132_DOCKER_IDLING && f->resourceId) {
		int resource = f->resourceId;
		image_draw(image_group(GROUP_RESOURCE_ICONS) +
			resource + resource_image_offset(resource, RESOURCE_IMAGE_ICON),
			c->xOffset + 92 + width, c->yOffset + 135);
	}
	
	lang_text_draw_multiline(130, 21 * c->figure.soundId + c->figure.phraseId + 1,
		c->xOffset + 90, c->yOffset + 160, 16 * (c->widthBlocks - 9), FONT_SMALL_BLACK);
	
	if (!f->buildingId) {
		return;
	}
	building *sourceBuilding = building_get(f->buildingId);
	building *targetBuilding = building_get(f->destinationBuildingId);
	int isReturning = 0;
	switch (f->actionState) {
		case FIGURE_ACTION_27_CARTPUSHER_RETURNING:
		case FIGURE_ACTION_53_WAREHOUSEMAN_RETURNING_EMPTY:
		case FIGURE_ACTION_56_WAREHOUSEMAN_RETURNING_WITH_FOOD:
		case FIGURE_ACTION_59_WAREHOUSEMAN_RETURNING_WITH_RESOURCE:
		case FIGURE_ACTION_134_DOCKER_EXPORT_QUEUE:
		case FIGURE_ACTION_137_DOCKER_EXPORT_RETURNING:
		case FIGURE_ACTION_138_DOCKER_IMPORT_RETURNING:
			isReturning = 1;
			break;
	}
	if (f->actionState != FIGURE_ACTION_132_DOCKER_IDLING) {
		if (isReturning) {
			width = lang_text_draw(129, 16,
				c->xOffset + 40, c->yOffset + 200, FONT_SMALL_BLACK);
			width += lang_text_draw(41, sourceBuilding->type,
				c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
			width += lang_text_draw(129, 14,
				c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
			width += lang_text_draw(41, targetBuilding->type,
				c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
		} else {
			width = lang_text_draw(129, 15,
				c->xOffset + 40, c->yOffset + 200, FONT_SMALL_BLACK);
			width += lang_text_draw(41, targetBuilding->type,
				c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
			width += lang_text_draw(129, 14,
				c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
			width += lang_text_draw(41, sourceBuilding->type,
				c->xOffset + 40 + width, c->yOffset + 200, FONT_SMALL_BLACK);
		}
	}
}

static void drawFigureInfoMarketBuyer(BuildingInfoContext *c, figure *f)
{
	image_draw(BigPeopleGraphic(f->type),
		c->xOffset + 28, c->yOffset + 112);
	
	lang_text_draw(65, f->name,
		c->xOffset + 90, c->yOffset + 108, FONT_LARGE_BROWN);
	int width = lang_text_draw(64, f->type,
		c->xOffset + 92, c->yOffset + 139, FONT_SMALL_BLACK);
	
	if (f->actionState == FIGURE_ACTION_145_MARKET_BUYER_GOING_TO_STORAGE) {
		width += lang_text_draw(129, 17,
			c->xOffset + 90 + width, c->yOffset + 139, FONT_SMALL_BLACK);
		int resourceId = collectingItemIdToResourceId(f->collectingItemId);
		image_draw(
			image_group(GROUP_RESOURCE_ICONS) + resourceId + resource_image_offset(resourceId, RESOURCE_IMAGE_ICON),
			c->xOffset + 90 + width, c->yOffset + 135);
	} else if (f->actionState == FIGURE_ACTION_146_MARKET_BUYER_RETURNING) {
		width += lang_text_draw(129, 18,
			c->xOffset + 90 + width, c->yOffset + 139, FONT_SMALL_BLACK);
		int resourceId = collectingItemIdToResourceId(f->collectingItemId);
		image_draw(
			image_group(GROUP_RESOURCE_ICONS) + resourceId + resource_image_offset(resourceId, RESOURCE_IMAGE_ICON),
			c->xOffset + 90 + width, c->yOffset + 135);
	}
	if (c->figure.phraseId >= 0) {
		lang_text_draw_multiline(130, 21 * c->figure.soundId + c->figure.phraseId + 1,
			c->xOffset + 90, c->yOffset + 160, 16 * (c->widthBlocks - 9), FONT_SMALL_BLACK);
	}
}

static void drawFigureInfoNormal(BuildingInfoContext *c, figure *f)
{
	int graphicId = BigPeopleGraphic(f->type);
	if (f->actionState == FIGURE_ACTION_74_PREFECT_GOING_TO_FIRE ||
		f->actionState == FIGURE_ACTION_75_PREFECT_AT_FIRE) {
		graphicId = image_group(GROUP_BIG_PEOPLE) + 18;
	}
	image_draw(graphicId, c->xOffset + 28, c->yOffset + 112);
	
	lang_text_draw(65, f->name,
		c->xOffset + 90, c->yOffset + 108, FONT_LARGE_BROWN);
	lang_text_draw(64, f->type,
		c->xOffset + 92, c->yOffset + 139, FONT_SMALL_BLACK);
	
	if (c->figure.phraseId >= 0) {
		lang_text_draw_multiline(130, 21 * c->figure.soundId + c->figure.phraseId + 1,
			c->xOffset + 90, c->yOffset + 160, 16 * (c->widthBlocks - 9), FONT_SMALL_BLACK);
	}
}

static void drawFigureInfo(BuildingInfoContext *c, int figureId)
{
	button_border_draw(c->xOffset + 24, c->yOffset + 102, 16 * (c->widthBlocks - 3), 122, 0);

    figure *f = figure_get(figureId);
	int type = f->type;
	if (type == FIGURE_TRADE_CARAVAN || type == FIGURE_TRADE_CARAVAN_DONKEY || type == FIGURE_TRADE_SHIP) {
		drawFigureInfoTrade(c, f);
	} else if (type >= FIGURE_ENEMY43_SPEAR && type <= FIGURE_ENEMY53_AXE) {
		drawFigureInfoEnemy(c, f);
	} else if (type == FIGURE_FISHING_BOAT || type == FIGURE_SHIPWRECK ||
			type == FIGURE_SHEEP || type == FIGURE_WOLF || type == FIGURE_ZEBRA) {
		drawFigureInfoBoatAnimal(c, f);
	} else if (type == FIGURE_CART_PUSHER || type == FIGURE_WAREHOUSEMAN || type == FIGURE_DOCKER) {
		drawFigureInfoCartpusher(c, f);
	} else if (type == FIGURE_MARKET_BUYER) {
		drawFigureInfoMarketBuyer(c, f);
	} else {
		drawFigureInfoNormal(c, f);
	}
}

void UI_BuildingInfo_drawFigureList(BuildingInfoContext *c)
{
	inner_panel_draw(c->xOffset + 16, c->yOffset + 40,
		c->widthBlocks - 2, 12);
	if (c->figure.count <= 0) {
		lang_text_draw_centered(70, 0, c->xOffset, c->yOffset + 120,
			16 * c->widthBlocks, FONT_SMALL_BLACK);
	} else {
		for (int i = 0; i < c->figure.count; i++) {
			button_border_draw(
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
    int x_cam, y_cam;
    city_view_get_camera(&x_cam, &y_cam);

	int gridOffset = figure_get(figureId)->gridOffset;
	int x, y;
	city_view_grid_offset_to_xy_view(gridOffset, &x, &y);

	city_view_set_camera(x - 2, y - 6);
    city_view_get_camera(&x, &y); // camera might have shifted because of map boundaries

	UI_CityBuildings_drawForegroundForFigure(x, y, figureId, coord);

    city_view_set_camera(x_cam, y_cam);
}

void UI_BuildingInfo_drawFigureImagesLocal(BuildingInfoContext *c)
{
	if (c->figure.count > 0) {
		struct UI_CityPixelCoordinate coord = {0, 0};
		for (int i = 0; i < c->figure.count; i++) {
			drawFigureInCity(c->figure.figureIds[i], &coord);
			Graphics_saveToBuffer(coord.x, coord.y, 48, 48, figureImages[i]);
		}
		int x, y;
		city_view_get_camera(&x, &y);
		UI_CityBuildings_drawForeground(x, y);
	}
}

void UI_BuildingInfo_playFigurePhrase(BuildingInfoContext *c)
{
	int figureId = c->figure.figureIds[c->figure.selectedIndex];
    figure *f = figure_get(figureId);
	c->figure.soundId = figure_phrase_play(f);
	c->figure.phraseId = f->phraseId;
}

void UI_BuildingInfo_handleMouseFigureList(const mouse *m, BuildingInfoContext *c)
{
	contextForCallback = c;
	generic_buttons_handle_mouse(m, c->xOffset, c->yOffset,
		figureButtons, c->figure.count, &focusButtonId);
	contextForCallback = 0;
}

static void selectFigure(int index, int param2)
{
	contextForCallback->figure.selectedIndex = index;
	UI_BuildingInfo_playFigurePhrase(contextForCallback);
	UI_Window_requestRefresh();
}
