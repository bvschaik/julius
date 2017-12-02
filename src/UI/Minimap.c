#include "Minimap.h"
#include "Sidebar.h"
#include "../CityView.h"
#include "../Graphics.h"
#include "../Data/Building.h"
#include "../Data/CityView.h"
#include "../Data/Grid.h"
#include "../Data/State.h"

#include "figure/figure.h"
#include "figure/type.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/property.h"
#include "map/random.h"
#include "scenario/property.h"

#define FOREACH_XY_VIEW(block)\
	int odd = 0;\
	int yAbs = minimapAbsoluteY - 4;\
	int yView = yOffset - 4;\
	for (int yRel = -4; yRel < heightTiles + 4; yRel++, yAbs++, yView++) {\
		int xView;\
		if (odd) {\
			xView = xOffset - 9;\
			odd = 0;\
		} else {\
			xView = xOffset - 8;\
			odd = 1;\
		}\
		int xAbs = minimapAbsoluteX - 4;\
		for (int xRel = -4; xRel < widthTiles; xRel++, xAbs++, xView += 2) {\
			if (xAbs < 0 || xAbs >= VIEW_X_MAX) continue;\
			if (yAbs < 0 || yAbs >= VIEW_Y_MAX) continue;\
			block;\
		}\
	}

static void setBounds(int xOffset, int yOffset, int widthTiles, int heightTiles);
static void drawMinimap(int xOffset, int yOffset, int widthTiles, int heightTiles);
static int drawFigure(int xView, int yView, int gridOffset);
static void drawTile(int xView, int yView, int gridOffset);
static void drawViewportRectangle(int xView, int yView, int widthTiles, int heightTiles);
static int getMouseGridOffset(const mouse *m, int xOffset, int yOffset, int widthTiles, int heightTiles);

static int minimapAbsoluteX;
static int minimapAbsoluteY;
static int minimapLeft;
static int minimapTop;
static int minimapRight;
static int minimapBottom;
static color_t soldierColor;
static color_t enemyColor;

void UI_Minimap_draw(int xOffset, int yOffset, int widthTiles, int heightTiles)
{
	Graphics_setClipRectangle(xOffset, yOffset, 2 * widthTiles, heightTiles);
	
	soldierColor = COLOR_SOLDIER;
    switch (scenario_property_climate()) {
        case CLIMATE_CENTRAL: enemyColor = COLOR_ENEMY_CENTRAL; break;
        case CLIMATE_NORTHERN: enemyColor = COLOR_ENEMY_NORTHERN; break;
        default: enemyColor = COLOR_ENEMY_DESERT; break;
    }

	setBounds(xOffset, yOffset, widthTiles, heightTiles);
	drawMinimap(xOffset, yOffset, widthTiles, heightTiles);
	drawViewportRectangle(xOffset, yOffset, widthTiles, heightTiles);

	Graphics_resetClipRectangle();
}

static void setBounds(int xOffset, int yOffset, int widthTiles, int heightTiles)
{
	minimapAbsoluteX = (VIEW_X_MAX - widthTiles) / 2;
	minimapAbsoluteY = (VIEW_Y_MAX - heightTiles) / 2;
	minimapLeft = xOffset;
	minimapTop = yOffset;
	minimapRight = xOffset + 2 * widthTiles;
	minimapBottom = yOffset + heightTiles;

	if ((Data_State.map.width - widthTiles) / 2 > 0) {
		if (Data_CityView.xInTiles < minimapAbsoluteX) {
			minimapAbsoluteX = Data_CityView.xInTiles;
		} else if (Data_CityView.xInTiles > widthTiles + minimapAbsoluteX - Data_CityView.widthInTiles) {
			minimapAbsoluteX = Data_CityView.widthInTiles + Data_CityView.xInTiles - widthTiles;
		}
	}
	if ((2 * Data_State.map.height - heightTiles) / 2 > 0) {
		if (Data_CityView.yInTiles < minimapAbsoluteY) {
			minimapAbsoluteY = Data_CityView.yInTiles;
		} else if (Data_CityView.yInTiles > heightTiles + minimapAbsoluteY - Data_CityView.heightInTiles) {
			minimapAbsoluteY = Data_CityView.heightInTiles + Data_CityView.yInTiles - heightTiles;
		}
	}
	// ensure even height
	minimapAbsoluteY &= ~1;
}

static void drawMinimap(int xOffset, int yOffset, int widthTiles, int heightTiles)
{
	FOREACH_XY_VIEW(
		int gridOffset = ViewToGridOffset(xAbs, yAbs);
		drawTile(xView, yView, gridOffset);
	);
}

enum {
    FIGURE_COLOR_NONE = 0,
    FIGURE_COLOR_SOLDIER = 1,
    FIGURE_COLOR_ENEMY = 2,
    FIGURE_COLOR_WOLF = 3
};

static int has_figure_color(figure *f)
{
    int type = f->type;
    if (FigureIsLegion(f->type)) {
        return FIGURE_COLOR_SOLDIER;
    }
    if (FigureIsEnemy(f->type)) {
        return FIGURE_COLOR_ENEMY;
    }
    if (f->type == FIGURE_INDIGENOUS_NATIVE &&
        f->actionState == FigureActionState_159_NativeAttacking) {
        return FIGURE_COLOR_ENEMY;
    }
    if (type == FIGURE_WOLF) {
        return FIGURE_COLOR_WOLF;
    }
    return FIGURE_COLOR_NONE;
}

static int drawFigure(int xView, int yView, int gridOffset)
{
    int color_type = map_figure_foreach_until(gridOffset, has_figure_color);
    if (color_type == FIGURE_COLOR_NONE) {
        return 0;
    }
    color_t color = COLOR_BLACK;
    if (color_type == FIGURE_COLOR_SOLDIER) {
        color = soldierColor;
    } else if (color_type == FIGURE_COLOR_ENEMY) {
        color = enemyColor;
    }
    Graphics_drawLine(xView, yView, xView+1, yView, color);
    return 1;
}

static void drawTile(int xView, int yView, int gridOffset)
{
	if (gridOffset < 0) {
		Graphics_drawImage(image_group(GROUP_MINIMAP_BLACK), xView, yView);
		return;
	}

	if (drawFigure(xView, yView, gridOffset)) {
		return;
	}
	
	int terrain = Data_Grid_terrain[gridOffset];
	// exception for fort ground: display as empty land
	if (terrain & Terrain_Building) {
		if (Data_Buildings[map_building_at(gridOffset)].type == BUILDING_FORT_GROUND) {
			terrain = 0;
		}
	}

	if (terrain & Terrain_Building) {
		if (map_property_is_draw_tile(gridOffset)) {
			int graphicId;
			int buildingId = map_building_at(gridOffset);
			if (Data_Buildings[buildingId].houseSize) {
				graphicId = image_group(GROUP_MINIMAP_HOUSE);
			} else if (Data_Buildings[buildingId].type == BUILDING_RESERVOIR) {
				graphicId = image_group(GROUP_MINIMAP_AQUEDUCT) - 1;
			} else {
				graphicId = image_group(GROUP_MINIMAP_BUILDING);
			}
			switch (map_property_multi_tile_size(gridOffset)) {
				case 1:
					Graphics_drawImage(graphicId, xView, yView); break;
				case 2:
					Graphics_drawImage(graphicId + 1, xView, yView - 1); break;
				case 3:
					Graphics_drawImage(graphicId + 2, xView, yView - 2); break;
				case 4:
					Graphics_drawImage(graphicId + 3, xView, yView - 3); break;
				case 5:
					Graphics_drawImage(graphicId + 4, xView, yView - 4); break;
			}
		}
	} else {
		int rand = map_random_get(gridOffset);
		int graphicId;
		if (terrain & Terrain_Water) {
			graphicId = image_group(GROUP_MINIMAP_WATER) + (rand & 3);
		} else if (terrain & Terrain_Scrub) {
			graphicId = image_group(GROUP_MINIMAP_TREE) + (rand & 3);
		} else if (terrain & Terrain_Tree) {
			graphicId = image_group(GROUP_MINIMAP_TREE) + (rand & 3);
		} else if (terrain & Terrain_Rock) {
			graphicId = image_group(GROUP_MINIMAP_ROCK) + (rand & 3);
		} else if (terrain & Terrain_Elevation) {
			graphicId = image_group(GROUP_MINIMAP_ROCK) + (rand & 3);
		} else if (terrain & Terrain_Road) {
			graphicId = image_group(GROUP_MINIMAP_ROAD);
		} else if (terrain & Terrain_Aqueduct) {
			graphicId = image_group(GROUP_MINIMAP_AQUEDUCT);
		} else if (terrain & Terrain_Wall) {
			graphicId = image_group(GROUP_MINIMAP_WALL);
		} else if (terrain & Terrain_Meadow) {
			graphicId = image_group(GROUP_MINIMAP_MEADOW) + (rand & 3);
		} else {
			graphicId = image_group(GROUP_MINIMAP_EMPTY_LAND) + (rand & 7);
		}
		Graphics_drawImage(graphicId, xView, yView);
	}
}

static void drawViewportRectangle(int xView, int yView, int widthTiles, int heightTiles)
{
	int xOffset = xView + 2 * (Data_CityView.xInTiles - minimapAbsoluteX) - 2;
	if (xOffset < xView) {
		xOffset = xView;
	}
	if (xOffset + 2 * Data_CityView.widthInTiles + 4 > xView + widthTiles) {
		xOffset -= 2;
	}
	int yOffset = yView + Data_CityView.yInTiles - minimapAbsoluteY + 2;
	Graphics_drawRect(xOffset, yOffset,
		Data_CityView.widthInTiles * 2 + 4,
		Data_CityView.heightInTiles - 4,
		COLOR_YELLOW);
}

static int getMouseGridOffset(const mouse *m, int xOffset, int yOffset, int widthTiles, int heightTiles)
{
	setBounds(xOffset, yOffset, widthTiles, heightTiles);
	FOREACH_XY_VIEW(
		if (m->y == yView && (m->x == xView || m->x == xView + 1)) {
			int gridOffset = ViewToGridOffset(xAbs, yAbs);
			return gridOffset < 0 ? 0 : gridOffset;
		}
	);
	return 0;
}

static int isMinimapClick(const mouse *m)
{
	if (m->x >= minimapLeft && m->x < minimapRight &&
		m->y >= minimapTop && m->y < minimapBottom) {
		return 1;
	}
	return 0;
}

int UI_Minimap_handleClick(const mouse *m)
{
	if (isMinimapClick(m)) {
		if (m->left.went_down || m->right.went_down) {
			int gridOffset = getMouseGridOffset(m, minimapLeft, minimapTop, 73, 111);
			if (gridOffset > 0) {
				CityView_goToGridOffset(gridOffset);
				UI_Sidebar_requestMinimapRefresh();
				return 1;
			}
		}
	}
	return 0;
}
