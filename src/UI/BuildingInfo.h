#ifndef UI_BUILDINGINFO_H
#define UI_BUILDINGINFO_H

#define PLAY_SOUND(f) \
	if (c->canPlaySound) {\
		Sound_Speech_playFile(f);\
		c->canPlaySound = 0;\
	}

#define DRAW_DESC(g,n) Widget_GameText_drawMultiline(g, n, c->xOffset + 32, c->yOffset + 56,\
			16 * (c->widthBlocks - 4), Font_NormalBlack);
#define DRAW_DESC_AT(y,g,n) Widget_GameText_drawMultiline(g, n, c->xOffset + 32, c->yOffset + y,\
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
	int hasReservoirPipes;
	int aqueductHasWater;
	int formationId;
	int barracksSoldiersRequested;
	int worstDesirabilityBuildingId;
} BuildingInfoContext;

void UI_BuildingInfo_drawEmploymentInfo(BuildingInfoContext *c, int yOffset);
void UI_BuildingInfo_drawWalkerListLocal(BuildingInfoContext *c);
void UI_BuildingInfo_drawWalkerList(BuildingInfoContext *c);

#endif
