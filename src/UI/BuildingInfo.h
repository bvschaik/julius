#ifndef UI_BUILDINGINFO_H
#define UI_BUILDINGINFO_H

#include "input/mouse.h"
#include "window/building/common.h"

void UI_BuildingInfo_drawFigureImagesLocal(BuildingInfoContext *c);
void UI_BuildingInfo_drawFigureList(BuildingInfoContext *c);
void UI_BuildingInfo_playFigurePhrase(BuildingInfoContext *c);
void UI_BuildingInfo_handleMouseFigureList(const mouse *m, BuildingInfoContext *c);

void UI_BuildingInfo_drawWall(BuildingInfoContext *c);
void UI_BuildingInfo_drawFort(BuildingInfoContext *c);
void UI_BuildingInfo_drawGatehouse(BuildingInfoContext *c);
void UI_BuildingInfo_drawTower(BuildingInfoContext *c);
void UI_BuildingInfo_drawMilitaryAcademy(BuildingInfoContext *c);
void UI_BuildingInfo_drawBarracks(BuildingInfoContext *c);
void UI_BuildingInfo_drawLegionInfo(BuildingInfoContext *c);
void UI_BuildingInfo_drawLegionInfoForeground(BuildingInfoContext *c);
void UI_BuildingInfo_handleMouseLegionInfo(const mouse *m, BuildingInfoContext *c);
int UI_BuildingInfo_getTooltipLegionInfo(BuildingInfoContext *c);

void UI_BuildingInfo_drawTerrain(BuildingInfoContext *c);
void UI_BuildingInfo_drawNoPeople(BuildingInfoContext *c);

#endif
