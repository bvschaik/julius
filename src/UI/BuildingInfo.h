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
	int warehouseSpaceText;
	int type; // 1 = terrain; 2 = building
	int terrainType;
	int advisor;
	int rubbleBuildingType;
	int storageShowSpecialOrders;
	struct {
		int soundId;
		int phraseId;
		int selectedIndex;
		int count;
		int drawn;
		int walkerIds[7];
	} walker;
} BuildingInfoContext;

void UI_BuildingInfo_init();
void UI_BuildingInfo_drawBackground();
void UI_BuildingInfo_drawForeground();
void UI_BuildingInfo_handleMouse();

void UI_BuildingInfo_drawEmploymentInfo(BuildingInfoContext *c, int yOffset);
void UI_BuildingInfo_drawWalkerImagesLocal(BuildingInfoContext *c);
void UI_BuildingInfo_drawWalkerList(BuildingInfoContext *c);
void UI_BuildingInfo_playWalkerPhrase(BuildingInfoContext *c);
void UI_BuildingInfo_handleMouseWalkerList(BuildingInfoContext *c);

void UI_BuildingInfo_showStorageOrders(int param1, int param2);

void UI_BuildingInfo_houseDetermineWorstDesirabilityBuilding(BuildingInfoContext *c);

void UI_BuildingInfo_drawHouse(BuildingInfoContext *c);

void UI_BuildingInfo_drawWheatFarm(BuildingInfoContext *c);
void UI_BuildingInfo_drawVegetableFarm(BuildingInfoContext *c);
void UI_BuildingInfo_drawFruitFarm(BuildingInfoContext *c);
void UI_BuildingInfo_drawOliveFarm(BuildingInfoContext *c);
void UI_BuildingInfo_drawVinesFarm(BuildingInfoContext *c);
void UI_BuildingInfo_drawPigFarm(BuildingInfoContext *c);

void UI_BuildingInfo_drawMarbleQuarry(BuildingInfoContext *c);
void UI_BuildingInfo_drawIronMine(BuildingInfoContext *c);
void UI_BuildingInfo_drawTimberYard(BuildingInfoContext *c);
void UI_BuildingInfo_drawClayPit(BuildingInfoContext *c);

void UI_BuildingInfo_drawWineWorkshop(BuildingInfoContext *c);
void UI_BuildingInfo_drawOilWorkshop(BuildingInfoContext *c);
void UI_BuildingInfo_drawWeaponsWorkshop(BuildingInfoContext *c);
void UI_BuildingInfo_drawFurnitureWorkshop(BuildingInfoContext *c);
void UI_BuildingInfo_drawPotteryWorkshop(BuildingInfoContext *c);

void UI_BuildingInfo_drawMarket(BuildingInfoContext *c);
void UI_BuildingInfo_drawGranary(BuildingInfoContext *c);
void UI_BuildingInfo_drawGranaryOrders(BuildingInfoContext *c);
void UI_BuildingInfo_drawWarehouse(BuildingInfoContext *c);
void UI_BuildingInfo_drawWarehouseOrders(BuildingInfoContext *c);

void UI_BuildingInfo_drawGranaryForeground(BuildingInfoContext *c);
void UI_BuildingInfo_drawGranaryOrdersForeground(BuildingInfoContext *c);
void UI_BuildingInfo_drawWarehouseForeground(BuildingInfoContext *c);
void UI_BuildingInfo_drawWarehouseOrdersForeground(BuildingInfoContext *c);

void UI_BuildingInfo_handleMouseGranary(BuildingInfoContext *c);
void UI_BuildingInfo_handleMouseGranaryOrders(BuildingInfoContext *c);
void UI_BuildingInfo_handleMouseWarehouse(BuildingInfoContext *c);
void UI_BuildingInfo_handleMouseWarehouseOrders(BuildingInfoContext *c);

void UI_BuildingInfo_drawAmphitheater(BuildingInfoContext *c);
void UI_BuildingInfo_drawTheater(BuildingInfoContext *c);
void UI_BuildingInfo_drawHippodrome(BuildingInfoContext *c);
void UI_BuildingInfo_drawColosseum(BuildingInfoContext *c);
void UI_BuildingInfo_drawGladiatorSchool(BuildingInfoContext *c);
void UI_BuildingInfo_drawLionHouse(BuildingInfoContext *c);
void UI_BuildingInfo_drawActorColony(BuildingInfoContext *c);
void UI_BuildingInfo_drawChariotMaker(BuildingInfoContext *c);

void UI_BuildingInfo_drawClinic(BuildingInfoContext *c);
void UI_BuildingInfo_drawHospital(BuildingInfoContext *c);
void UI_BuildingInfo_drawBathhouse(BuildingInfoContext *c);
void UI_BuildingInfo_drawBarber(BuildingInfoContext *c);

void UI_BuildingInfo_drawSchool(BuildingInfoContext *c);
void UI_BuildingInfo_drawAcademy(BuildingInfoContext *c);
void UI_BuildingInfo_drawLibrary(BuildingInfoContext *c);

void UI_BuildingInfo_drawTempleCeres(BuildingInfoContext *c);
void UI_BuildingInfo_drawTempleNeptune(BuildingInfoContext *c);
void UI_BuildingInfo_drawTempleMercury(BuildingInfoContext *c);
void UI_BuildingInfo_drawTempleMars(BuildingInfoContext *c);
void UI_BuildingInfo_drawTempleVenus(BuildingInfoContext *c);
void UI_BuildingInfo_drawOracle(BuildingInfoContext *c);

void UI_BuildingInfo_drawForum(BuildingInfoContext *c);
void UI_BuildingInfo_drawSenate(BuildingInfoContext *c);
void UI_BuildingInfo_drawGovernorsHome(BuildingInfoContext *c);

void UI_BuildingInfo_drawEngineersPost(BuildingInfoContext *c);
void UI_BuildingInfo_drawShipyard(BuildingInfoContext *c);
void UI_BuildingInfo_drawDock(BuildingInfoContext *c);
void UI_BuildingInfo_drawWharf(BuildingInfoContext *c);

void UI_BuildingInfo_drawBurningRuin(BuildingInfoContext *c);
void UI_BuildingInfo_drawRubble(BuildingInfoContext *c);

void UI_BuildingInfo_drawAqueduct(BuildingInfoContext *c);
void UI_BuildingInfo_drawReservoir(BuildingInfoContext *c);
void UI_BuildingInfo_drawFountain(BuildingInfoContext *c);
void UI_BuildingInfo_drawWell(BuildingInfoContext *c);

void UI_BuildingInfo_drawWall(BuildingInfoContext *c);
void UI_BuildingInfo_drawPrefect(BuildingInfoContext *c);
void UI_BuildingInfo_drawFort(BuildingInfoContext *c);
void UI_BuildingInfo_drawGatehouse(BuildingInfoContext *c);
void UI_BuildingInfo_drawTower(BuildingInfoContext *c);
void UI_BuildingInfo_drawMilitaryAcademy(BuildingInfoContext *c);
void UI_BuildingInfo_drawBarracks(BuildingInfoContext *c);
void UI_BuildingInfo_drawLegionInfo(BuildingInfoContext *c);
void UI_BuildingInfo_drawLegionInfoForeground(BuildingInfoContext *c);
void UI_BuildingInfo_handleMouseLegionInfo(BuildingInfoContext *c);

void UI_BuildingInfo_drawGarden(BuildingInfoContext *c);
void UI_BuildingInfo_drawPlaza(BuildingInfoContext *c);
void UI_BuildingInfo_drawStatue(BuildingInfoContext *c);
void UI_BuildingInfo_drawTriumphalArch(BuildingInfoContext *c);

void UI_BuildingInfo_drawNativeHut(BuildingInfoContext *c);
void UI_BuildingInfo_drawNativeMeeting(BuildingInfoContext *c);
void UI_BuildingInfo_drawNativeCrops(BuildingInfoContext *c);
void UI_BuildingInfo_drawMissionPost(BuildingInfoContext *c);

void UI_BuildingInfo_drawTerrain(BuildingInfoContext *c);
void UI_BuildingInfo_drawNoPeople(BuildingInfoContext *c);

#endif
