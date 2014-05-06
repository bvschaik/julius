#ifndef UI_CITYBUILDINGS_H
#define UI_CITYBUILDINGS_H

#include "Tooltip.h"

void UI_CityBuildings_drawForeground(int x, int y);
void UI_CityBuildings_handleMouse();
void UI_CityBuildings_getTooltip(struct TooltipContext *c);

void UI_CityBuildings_checkCameraWithinBounds();

void UI_CityBuildings_scrollMap(int direction);

#endif
