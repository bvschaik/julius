#include "Minimap.h"
#include "Sidebar.h"
#include "../CityView.h"
#include "../Graphics.h"
#include "../Data/Building.h"
#include "../Data/CityView.h"
#include "../Data/Constants.h"
#include "../Data/Figure.h"
#include "../Data/Graphics.h"
#include "../Data/Grid.h"
#include "../Data/Mouse.h"
#include "../Data/Scenario.h"
#include "../Data/Settings.h"

#include "figure/type.h"

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
static int getMouseGridOffset(int xOffset, int yOffset, int widthTiles, int heightTiles);

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
	if (Data_Scenario.climate == Climate_Central) {
		enemyColor = COLOR_ENEMY_CENTRAL;
	} else if (Data_Scenario.climate == Climate_Northern) {
		enemyColor = COLOR_ENEMY_NORTHERN;
	} else {
		enemyColor = COLOR_ENEMY_DESERT;
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

	if ((Data_Scenario.mapSizeX - widthTiles) / 2 > 0) {
		if (Data_CityView.xInTiles < minimapAbsoluteX) {
			minimapAbsoluteX = Data_CityView.xInTiles;
		} else if (Data_CityView.xInTiles > widthTiles + minimapAbsoluteX - Data_CityView.widthInTiles) {
			minimapAbsoluteX = Data_CityView.widthInTiles + Data_CityView.xInTiles - widthTiles;
		}
	}
	if ((2 * Data_Scenario.mapSizeY - heightTiles) / 2 > 0) {
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

static int drawFigure(int xView, int yView, int gridOffset)
{
	color_t color = COLOR_BLACK;
	int hasFigure = 0;

	int figureId = Data_Grid_figureIds[gridOffset];
	while (figureId > 0) {
		int type = Data_Figures[figureId].type;
		if (FigureIsLegion(type)) {
			hasFigure = 1;
			color = soldierColor;
			break;
		}
		if (FigureIsEnemy(type)) {
			hasFigure = 1;
			color = enemyColor;
			break;
		}
		if (type == FIGURE_INDIGENOUS_NATIVE &&
			Data_Figures[figureId].actionState == FigureActionState_159_NativeAttacking) {
			hasFigure = 1;
			color = enemyColor;
			break;
		}
		if (type == FIGURE_WOLF) {
			hasFigure = 1;
			color = COLOR_BLACK;
			break;
		}
		figureId = Data_Figures[figureId].nextFigureIdOnSameTile;
	}
	if (hasFigure) {
		Graphics_drawLine(xView, yView, xView+1, yView, color);
		return 1;
	} else {
		return 0;
	}
}

static void drawTile(int xView, int yView, int gridOffset)
{
	if (gridOffset < 0) {
		Graphics_drawImage(GraphicId(ID_Graphic_MinimapBlack), xView, yView);
		return;
	}

	if (drawFigure(xView, yView, gridOffset)) {
		return;
	}
	
	int terrain = Data_Grid_terrain[gridOffset];
	// exception for fort ground: display as empty land
	if (terrain & Terrain_Building) {
		if (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type == BUILDING_FORT_GROUND) {
			terrain = 0;
		}
	}

	if (terrain & Terrain_Building) {
		if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
			int graphicId;
			int buildingId = Data_Grid_buildingIds[gridOffset];
			if (Data_Buildings[buildingId].houseSize) {
				graphicId = GraphicId(ID_Graphic_MinimapHouse);
			} else if (Data_Buildings[buildingId].type == BUILDING_RESERVOIR) {
				graphicId = GraphicId(ID_Graphic_MinimapAqueduct) - 1;
			} else {
				graphicId = GraphicId(ID_Graphic_MinimapBuilding);
			}
			switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
				case 0:
					Graphics_drawImage(graphicId, xView, yView); break;
				case 1:
					Graphics_drawImage(graphicId + 1, xView, yView - 1); break;
				case 2:
					Graphics_drawImage(graphicId + 2, xView, yView - 2); break;
				case 4:
					Graphics_drawImage(graphicId + 3, xView, yView - 3); break;
				case 8:
					Graphics_drawImage(graphicId + 4, xView, yView - 4); break;
			}
		}
	} else {
		int rand = Data_Grid_random[gridOffset];
		int graphicId;
		if (terrain & Terrain_Water) {
			graphicId = GraphicId(ID_Graphic_MinimapWater) + (rand & 3);
		} else if (terrain & Terrain_Scrub) {
			graphicId = GraphicId(ID_Graphic_MinimapTree) + (rand & 3);
		} else if (terrain & Terrain_Tree) {
			graphicId = GraphicId(ID_Graphic_MinimapTree) + (rand & 3);
		} else if (terrain & Terrain_Rock) {
			graphicId = GraphicId(ID_Graphic_MinimapRock) + (rand & 3);
		} else if (terrain & Terrain_Elevation) {
			graphicId = GraphicId(ID_Graphic_MinimapRock) + (rand & 3);
		} else if (terrain & Terrain_Road) {
			graphicId = GraphicId(ID_Graphic_MinimapRoad);
		} else if (terrain & Terrain_Aqueduct) {
			graphicId = GraphicId(ID_Graphic_MinimapAqueduct);
		} else if (terrain & Terrain_Wall) {
			graphicId = GraphicId(ID_Graphic_MinimapWall);
		} else if (terrain & Terrain_Meadow) {
			graphicId = GraphicId(ID_Graphic_MinimapMeadow) + (rand & 3);
		} else {
			graphicId = GraphicId(ID_Graphic_MinimapEmptyLand) + (rand & 7);
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

static int getMouseGridOffset(int xOffset, int yOffset, int widthTiles, int heightTiles)
{
	setBounds(xOffset, yOffset, widthTiles, heightTiles);
	FOREACH_XY_VIEW(
		if (Data_Mouse.y == yView && (Data_Mouse.x == xView || Data_Mouse.x == xView + 1)) {
			int gridOffset = ViewToGridOffset(xAbs, yAbs);
			return gridOffset < 0 ? 0 : gridOffset;
		}
	);
	return 0;
}

static int isMinimapClick()
{
	if (Data_Mouse.x >= minimapLeft && Data_Mouse.x < minimapRight &&
		Data_Mouse.y >= minimapTop && Data_Mouse.y < minimapBottom) {
		return 1;
	}
	return 0;
}

int UI_Minimap_handleClick()
{
	if (isMinimapClick()) {
		if (Data_Mouse.left.wentDown || Data_Mouse.right.wentDown) {
			int gridOffset = getMouseGridOffset(minimapLeft, minimapTop, 73, 111);
			if (gridOffset > 0) {
				CityView_goToGridOffset(gridOffset);
				UI_Sidebar_requestMinimapRefresh();
				return 1;
			}
		}
	}
	return 0;
}
