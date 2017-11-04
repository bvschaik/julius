#ifndef UI_CITYBUILDINGS_H
#define UI_CITYBUILDINGS_H

#include "Tooltip.hpp"

#include "graphics/mouse.hpp"

struct UI_CityPixelCoordinate {
	int x;
	int y;
};

void UI_CityBuildings_drawForeground(int x, int y);
void UI_CityBuildings_drawForegroundForFigure(int x, int y, int figureId, struct UI_CityPixelCoordinate *coord);
void UI_CityBuildings_drawBuildingCost();

void UI_CityBuildings_handleMouse(const mouse *m);
void UI_CityBuildings_handleMouseMilitary(const mouse *m);
void UI_CityBuildings_getTooltip(struct TooltipContext *c);

void UI_CityBuildings_checkCameraWithinBounds();

void UI_CityBuildings_scrollMap(int direction);

#endif
