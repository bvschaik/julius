#ifndef UI_CITYBUILDINGS_PRIVATE_H
#define UI_CITYBUILDINGS_PRIVATE_H

#include "CityBuildings.hpp"

#include "../Data/Building.hpp"
#include "../Data/CityInfo.hpp"
#include "../Data/CityView.hpp"
#include "../Data/Constants.hpp"
#include "../Data/Figure.hpp"
#include "../Data/Grid.hpp"
#include "../Data/Settings.hpp"
#include "../Data/State.hpp"

#include "../Animation.hpp"
#include "core/calc.hpp"
#include "../Graphics.hpp"

#include "building/model.hpp"
#include "graphics/image.hpp"

#define DRAWFOOT_SIZE1(g,x,y) Graphics_drawIsometricFootprint(g, x, y, 0)
#define DRAWFOOT_SIZE2(g,x,y) Graphics_drawIsometricFootprint(g, x + 30, y - 15, 0)
#define DRAWFOOT_SIZE3(g,x,y) Graphics_drawIsometricFootprint(g, x + 60, y - 30, 0)
#define DRAWFOOT_SIZE4(g,x,y) Graphics_drawIsometricFootprint(g, x + 90, y - 45, 0)
#define DRAWFOOT_SIZE5(g,x,y) Graphics_drawIsometricFootprint(g, x + 120, y - 60, 0)

#define DRAWTOP_SIZE1(g,x,y) Graphics_drawIsometricTop(g, x, y, 0)
#define DRAWTOP_SIZE2(g,x,y) Graphics_drawIsometricTop(g, x + 30, y - 15, 0)
#define DRAWTOP_SIZE3(g,x,y) Graphics_drawIsometricTop(g, x + 60, y - 30, 0)
#define DRAWTOP_SIZE4(g,x,y) Graphics_drawIsometricTop(g, x + 90, y - 45, 0)
#define DRAWTOP_SIZE5(g,x,y) Graphics_drawIsometricTop(g, x + 120, y - 60, 0)

#define DRAWTOP_SIZE1_C(g,x,y,c) Graphics_drawIsometricTop(g, x, y, c)
#define DRAWTOP_SIZE2_C(g,x,y,c) Graphics_drawIsometricTop(g, x + 30, y - 15, c)
#define DRAWTOP_SIZE3_C(g,x,y,c) Graphics_drawIsometricTop(g, x + 60, y - 30, c)
#define DRAWTOP_SIZE4_C(g,x,y,c) Graphics_drawIsometricTop(g, x + 90, y - 45, c)
#define DRAWTOP_SIZE5_C(g,x,y,c) Graphics_drawIsometricTop(g, x + 120, y - 60, c)

#define FOREACH_XY_VIEW \
	int odd = 0;\
	int yView = Data_CityView.yInTiles - 8;\
	int yGraphic = Data_CityView.yOffsetInPixels - 9*15;\
	for (int y = 0; y < Data_CityView.heightInTiles + 14; y++) {\
		int xGraphic = -(4*58 + 8);\
		if (odd) {\
			xGraphic += Data_CityView.xOffsetInPixels - 30;\
			odd = 0;\
		} else {\
			xGraphic += Data_CityView.xOffsetInPixels;\
			odd = 1;\
		}\
		int xView = Data_CityView.xInTiles - 4;\
		for (int x = 0; x < Data_CityView.widthInTiles + 7; x++) {\
			if (xView >= 0 && xView < VIEW_X_MAX &&\
				yView >= 0 && yView < VIEW_Y_MAX) {

#define END_FOREACH_XY_VIEW \
			}\
			xGraphic += 60;\
			xView++;\
		}\
		yGraphic += 15;\
		yView++;\
	}

#define FOREACH_Y_VIEW \
	int odd = 0;\
	int yView = Data_CityView.yInTiles - 8;\
	int yGraphic = Data_CityView.yOffsetInPixels - 9*15;\
	int xGraphic, xView;\
	for (int y = 0; y < Data_CityView.heightInTiles + 14; y++) {\
		if (yView >= 0 && yView < VIEW_Y_MAX) {

#define END_FOREACH_Y_VIEW \
		}\
		odd = 1 - odd;\
		yGraphic += 15;\
		yView++;\
	}

#define FOREACH_X_VIEW \
	xGraphic = -(4*58 + 8);\
	if (odd) {\
		xGraphic += Data_CityView.xOffsetInPixels - 30;\
	} else {\
		xGraphic += Data_CityView.xOffsetInPixels;\
	}\
	xView = Data_CityView.xInTiles - 4;\
	for (int x = 0; x < Data_CityView.widthInTiles + 7; x++) {\
		if (xView >= 0 && xView < VIEW_X_MAX) {\
			int gridOffset = ViewToGridOffset(xView, yView);\
			if (gridOffset >= 0) {

#define END_FOREACH_X_VIEW \
			}\
		}\
		xGraphic += 60;\
		xView++;\
	}

void UI_CityBuildings_drawOverlayFootprints();
void UI_CityBuildings_drawOverlayTopsFiguresAnimation(int overlay);

void UI_CityBuildings_drawFigure(int figureId, int xOffset, int yOffset,
                                 int selectedFigureId, struct UI_CityPixelCoordinate *coord);
void UI_CityBuildings_drawBridge(int gridOffset, int xOffset, int yOffset);

void UI_CityBuildings_drawSelectedBuildingGhost();

#endif
