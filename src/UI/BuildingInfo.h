#ifndef UI_BUILDINGINFO_H
#define UI_BUILDINGINFO_H

#define DRAW_DESC(g,n) Widget_GameText_drawMultiline(g, n, c->xOffset + 32, c->yOffset + 56,\
			16 * (c->widthBlocks - 4), Font_NormalBlack);

typedef struct {
	int xOffset;
	int yOffset;
	int widthBlocks;
	int heightBlocks;
	int helpId;
	int canPlaySound;
	int buildingId;
	int hasRoadAccess;
	int workerPercentage;
} BuildingInfoContext;

void UI_BuildingInfo_drawEmploymentInfo(BuildingInfoContext *c, int yOffset);

#endif
