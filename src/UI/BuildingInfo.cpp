#include "BuildingInfo.h"

#include "AllWindows.h"
#include "MessageDialog.h"
#include "Window.h"

#include "../Building.h"
#include "../Calc.h"
#include "../CityInfo.h"
#include "../Formation.h"
#include "../Graphics.h"
#include "../HouseEvolution.h"
#include "../Resource.h"
#include "../Terrain.h"
#include "../Widget.h"

#include "../Data/Building.h"
#include "../Data/CityInfo.h"
#include "../Data/CityView.h"
#include "../Data/Constants.h"
#include "../Data/Grid.h"
#include "../Data/Model.h"
#include "../Data/Mouse.h"

static void buttonHelp(int param1, int param2);
static void buttonExit(int param1, int param2);
static void buttonAdvisor(int param1, int param2);

static ImageButton imageButtonsHelpExit[] = {
	{14, 0, 27, 27, 4, 134, 0, buttonHelp, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0},
	{424, 3, 24, 24, 4, 134, 4, buttonExit, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0}
};

static ImageButton imageButtonsAdvisor[] = {
	{350, -38, 28, 28, 4, 199, 9, buttonAdvisor, Widget_Button_doNothing, 1, 0, 0, 0, 4, 0}
};

static BuildingInfoContext context;

static int getHeightId()
{
	if (context.type == 1) {
		switch (context.terrainType) {
			case 7:
				return 4; // aqueduct
			case 8:
			case 9:
			case 12:
				return 1; // rubble, wall, garden
			default:
				return 0;
		}
	} else if (context.type == 2) {
		switch (Data_Buildings[context.buildingId].type) {
			case Building_SmallTempleCeres:
			case Building_SmallTempleNeptune:
			case Building_SmallTempleMercury:
			case Building_SmallTempleMars:
			case Building_SmallTempleVenus:
			case Building_LargeTempleCeres:
			case Building_LargeTempleNeptune:
			case Building_LargeTempleMercury:
			case Building_LargeTempleMars:
			case Building_LargeTempleVenus:
			case Building_Oracle:
			case Building_SmallStatue:
			case Building_MediumStatue:
			case Building_LargeStatue:
			case Building_GladiatorSchool:
			case Building_LionHouse:
			case Building_ActorColony:
			case Building_ChariotMaker:
			case Building_Doctor:
			case Building_Hospital:
			case Building_Bathhouse:
			case Building_Barber:
			case Building_BurningRuin:
			case Building_Reservoir:
			case Building_NativeHut:
			case Building_NativeMeeting:
			case Building_NativeCrops:
			case Building_MissionPost:
			case Building_Prefecture:
			case Building_EngineersPost:
			case Building_School:
			case Building_Academy:
			case Building_Library:
			case Building_Gatehouse:
			case Building_Tower:
			case Building_FortGround__:
			case Building_MilitaryAcademy:
			case Building_Barracks:
			case Building_Market:
			case Building_Granary:
			case Building_Shipyard:
			case Building_Dock:
			case Building_Wharf:
			case Building_GovernorsHouse:
			case Building_GovernorsVilla:
			case Building_GovernorsPalace:
			case Building_Forum:
			case Building_ForumUpgraded:
			case Building_WineWorkshop:
			case Building_OilWorkshop:
			case Building_WeaponsWorkshop:
			case Building_FurnitureWorkshop:
			case Building_PotteryWorkshop:
				return 1;

			case Building_Theater:
			case Building_Hippodrome:
			case Building_Colosseum:
			case Building_Senate:
			case Building_SenateUpgraded:
				return 2;

			case Building_Amphitheater:
				return 3;

			case Building_Fountain:
			case Building_Well:
				return 4;
		}
	}
	return 0;
}

void UI_BuildingInfo_init()
{
	int gridOffset = Data_CityView.selectedTile.gridOffset;
	int terrain = Data_Grid_terrain[gridOffset];
	context.canPlaySound = 1;
	context.storageShowSpecialOrders = 0;
	context.buildingId = Data_Grid_buildingIds[gridOffset];
	context.rubbleBuildingType = Data_Grid_rubbleBuildingType[gridOffset];
	context.hasReservoirPipes = terrain & Terrain_ReservoirRange;
	context.aqueductHasWater = Data_Grid_aqueducts[gridOffset];

	CityInfo_Resource_calculateAvailableResources();
	context.type = 1;
	context.walker.drawn = 0;
	if (!Data_Grid_buildingIds[gridOffset] && Data_Grid_spriteOffsets[gridOffset] > 0) {
		if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
			context.terrainType = 11;
		} else {
			context.terrainType = 10;
		}
	} else if (Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake) {
		if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
			context.terrainType = 13;
		}
		if (Data_Grid_terrain[gridOffset] & Terrain_Rock) {
			context.terrainType = 5;
		}
	} else if (Data_Grid_terrain[gridOffset] & Terrain_Tree) {
		context.terrainType = 1;
	} else if (Data_Grid_terrain[gridOffset] & Terrain_Rock) {
		if (gridOffset == Data_CityInfo_Extra.gridOffsetEntryPoint) {
			context.terrainType = 14;
		} else if (gridOffset == Data_CityInfo_Extra.gridOffsetExitPoint) {
			context.terrainType = 15;
		} else {
			context.terrainType = 2;
		}
	} else if ((Data_Grid_terrain[gridOffset] & (Terrain_Water|Terrain_Building)) == Terrain_Water) {
		context.terrainType = 3;
	} else if (Data_Grid_terrain[gridOffset] & Terrain_Scrub) {
		context.terrainType = 4;
	} else if (Data_Grid_terrain[gridOffset] & Terrain_Garden) {
		context.terrainType = 12;
	} else if ((Data_Grid_terrain[gridOffset] & (Terrain_Road|Terrain_Building)) == Terrain_Road) {
		context.terrainType = 6;
	} else if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
		context.terrainType = 7;
	} else if (Data_Grid_terrain[gridOffset] & Terrain_Rubble) {
		context.terrainType = 8;
	} else if (Data_Grid_terrain[gridOffset] & Terrain_Wall) {
		context.terrainType = 9;
	} else if (!context.buildingId) {
		context.terrainType = 10;
	} else {
		context.type = 2;
		context.workerPercentage = Calc_getPercentage(
			Data_Buildings[context.buildingId].numWorkers,
			Data_Model_Buildings[Data_Buildings[context.buildingId].type].laborers);
		switch (Data_Buildings[context.buildingId].type) {
			case Building_FortGround:
				context.buildingId = Data_Buildings[context.buildingId].prevPartBuildingId;
				// fallthrough
			case Building_FortGround__:
				context.formationId = Data_Buildings[context.buildingId].formationId;
				break;
			case Building_WarehouseSpace:
			case Building_Hippodrome:
				context.buildingId = Building_getMainBuildingId(context.buildingId);
				break;
			case Building_Barracks:
				context.barracksSoldiersRequested = Formation_anyLegionNeedsSoldiers();
				context.barracksSoldiersRequested += Data_Buildings_Extra.barracksTowerSentryRequested;
				break;
			default:
				if (Data_Buildings[context.buildingId].houseSize) {
					UI_BuildingInfo_houseDetermineWorstDesirabilityBuilding(&context);
					HouseEvolution_determineEvolveText(context.buildingId, context.worstDesirabilityBuildingId);
				}
				break;
		}
		context.hasRoadAccess = 0;
		switch (Data_Buildings[context.buildingId].type) {
			case Building_Granary:
				if (Terrain_hasRoadAccessGranary(
					Data_Buildings[context.buildingId].x,
					Data_Buildings[context.buildingId].y, 0, 0)) {
					context.hasRoadAccess = 1;
				}
				break;
			case Building_Hippodrome:
				if (Terrain_hasRoadAccessHippodrome(
					Data_Buildings[context.buildingId].x,
					Data_Buildings[context.buildingId].y, 0, 0)) {
					context.hasRoadAccess = 1;
				}
				break;
			case Building_Warehouse:
				if (Terrain_hasRoadAccess(
					Data_Buildings[context.buildingId].x,
					Data_Buildings[context.buildingId].y, 3, 0, 0)) {
					context.hasRoadAccess = 1;
				}
				context.warehouseSpaceText = Resource_getWarehouseSpaceInfo(context.buildingId);
				break;
			default:
				if (Terrain_hasRoadAccess(
					Data_Buildings[context.buildingId].x,
					Data_Buildings[context.buildingId].y,
					Data_Buildings[context.buildingId].size, 0, 0)) {
					context.hasRoadAccess = 1;
				}
				break;
		}
	}
	// TODO walkers
	context.xOffset = 8;
	context.yOffset = 32;
	context.widthBlocks = 29;
	switch (getHeightId()) {
		case 1: context.heightBlocks = 16; break;
		case 2: context.heightBlocks = 18; break;
		case 3: context.heightBlocks = 19; break;
		case 4: context.heightBlocks = 14; break;
		default: context.heightBlocks = 22; break;
	}
	// TODO window placement
}

void UI_BuildingInfo_drawBackground()
{
	UI_City_drawBackground();
	UI_City_drawForeground();//?do we want this?
	if (context.type == 0) {
		UI_BuildingInfo_drawNoPeople(&context);
	} else if (context.type == 1) {
		UI_BuildingInfo_drawTerrain(&context);
	} else if (context.type == 2) {
		int btype = Data_Buildings[context.buildingId].type;
		if (btype >= Building_HouseVacantLot && btype <= Building_HouseLuxuryPalace) {
			UI_BuildingInfo_drawHouse(&context);
		} else if (btype == Building_WheatFarm) {
			UI_BuildingInfo_drawWheatFarm(&context);
		} else if (btype == Building_VegetableFarm) {
			UI_BuildingInfo_drawVegetableFarm(&context);
		} else if (btype == Building_FruitFarm) {
			UI_BuildingInfo_drawFruitFarm(&context);
		} else if (btype == Building_OliveFarm) {
			UI_BuildingInfo_drawOliveFarm(&context);
		} else if (btype == Building_VinesFarm) {
			UI_BuildingInfo_drawVinesFarm(&context);
		} else if (btype == Building_PigFarm) {
			UI_BuildingInfo_drawPigFarm(&context);
		} else if (btype == Building_MarbleQuarry) {
			UI_BuildingInfo_drawMarbleQuarry(&context);
		} else if (btype == Building_IronMine) {
			UI_BuildingInfo_drawIronMine(&context);
		} else if (btype == Building_TimberYard) {
			UI_BuildingInfo_drawTimberYard(&context);
		} else if (btype == Building_ClayPit) {
			UI_BuildingInfo_drawClayPit(&context);
		} else if (btype == Building_WineWorkshop) {
			UI_BuildingInfo_drawWineWorkshop(&context);
		} else if (btype == Building_OilWorkshop) {
			UI_BuildingInfo_drawOilWorkshop(&context);
		} else if (btype == Building_WeaponsWorkshop) {
			UI_BuildingInfo_drawWeaponsWorkshop(&context);
		} else if (btype == Building_FurnitureWorkshop) {
			UI_BuildingInfo_drawFurnitureWorkshop(&context);
		} else if (btype == Building_PotteryWorkshop) {
			UI_BuildingInfo_drawPotteryWorkshop(&context);
		} else if (btype == Building_Market) {
			UI_BuildingInfo_drawMarket(&context);
		} else if (btype == Building_Granary) {
			if (context.storageShowSpecialOrders) {
				UI_BuildingInfo_drawGranaryOrders(&context);
			} else {
				UI_BuildingInfo_drawGranary(&context);
			}
		} else if (btype == Building_Warehouse) {
			if (context.storageShowSpecialOrders) {
				UI_BuildingInfo_drawWarehouseOrders(&context);
			} else {
				UI_BuildingInfo_drawWarehouse(&context);
			}
		} else if (btype == Building_Amphitheater) {
			UI_BuildingInfo_drawAmphitheater(&context);
		} else if (btype == Building_Theater) {
			UI_BuildingInfo_drawTheater(&context);
		} else if (btype == Building_Hippodrome) {
			UI_BuildingInfo_drawHippodrome(&context);
		} else if (btype == Building_Colosseum) {
			UI_BuildingInfo_drawColosseum(&context);
		} else if (btype == Building_GladiatorSchool) {
			UI_BuildingInfo_drawGladiatorSchool(&context);
		} else if (btype == Building_LionHouse) {
			UI_BuildingInfo_drawLionHouse(&context);
		} else if (btype == Building_ActorColony) {
			UI_BuildingInfo_drawActorColony(&context);
		} else if (btype == Building_ChariotMaker) {
			UI_BuildingInfo_drawChariotMaker(&context);
		} else if (btype == Building_Doctor) {
			UI_BuildingInfo_drawClinic(&context);
		} else if (btype == Building_Hospital) {
			UI_BuildingInfo_drawHospital(&context);
		} else if (btype == Building_Bathhouse) {
			UI_BuildingInfo_drawBathhouse(&context);
		} else if (btype == Building_Barber) {
			UI_BuildingInfo_drawBarber(&context);
		} else if (btype == Building_School) {
			UI_BuildingInfo_drawSchool(&context);
		} else if (btype == Building_Academy) {
			UI_BuildingInfo_drawAcademy(&context);
		} else if (btype == Building_Library) {
			UI_BuildingInfo_drawLibrary(&context);
		} else if (btype == Building_SmallTempleCeres || btype == Building_LargeTempleCeres) {
			UI_BuildingInfo_drawTempleCeres(&context);
		} else if (btype == Building_SmallTempleNeptune || btype == Building_LargeTempleNeptune) {
			UI_BuildingInfo_drawTempleNeptune(&context);
		} else if (btype == Building_SmallTempleMercury || btype == Building_LargeTempleMercury) {
			UI_BuildingInfo_drawTempleMercury(&context);
		} else if (btype == Building_SmallTempleMars || btype == Building_LargeTempleMars) {
			UI_BuildingInfo_drawTempleMars(&context);
		} else if (btype == Building_SmallTempleVenus || btype == Building_LargeTempleVenus) {
			UI_BuildingInfo_drawTempleVenus(&context);
		} else if (btype == Building_Oracle) {
			UI_BuildingInfo_drawOracle(&context);
		} else if (btype == Building_GovernorsHouse || btype == Building_GovernorsVilla || btype == Building_GovernorsPalace) {
			UI_BuildingInfo_drawGovernorsHome(&context);
		} else if (btype == Building_Forum || btype == Building_ForumUpgraded) {
			UI_BuildingInfo_drawForum(&context);
		} else if (btype == Building_Senate || btype == Building_SenateUpgraded) {
			UI_BuildingInfo_drawSenate(&context);
		} else if (btype == Building_EngineersPost) {
			UI_BuildingInfo_drawEngineersPost(&context);
		} else if (btype == Building_Shipyard) {
			UI_BuildingInfo_drawShipyard(&context);
		} else if (btype == Building_Dock) {
			UI_BuildingInfo_drawDock(&context);
		} else if (btype == Building_Wharf) {
			UI_BuildingInfo_drawWharf(&context);
		} else if (btype == Building_Reservoir) {
			UI_BuildingInfo_drawReservoir(&context);
		} else if (btype == Building_Fountain) {
			UI_BuildingInfo_drawFountain(&context);
		} else if (btype == Building_Well) {
			UI_BuildingInfo_drawWell(&context);
		} else if (btype == Building_SmallStatue || btype == Building_MediumStatue || btype == Building_LargeStatue) {
			UI_BuildingInfo_drawStatue(&context);
		} else if (btype == Building_TriumphalArch) {
			UI_BuildingInfo_drawTriumphalArch(&context);
		} else if (btype == Building_Prefecture) {
			UI_BuildingInfo_drawPrefect(&context);
		} else if (btype == Building_Gatehouse) {
			UI_BuildingInfo_drawGatehouse(&context);
		} else if (btype == Building_Tower) {
			UI_BuildingInfo_drawTower(&context);
		} else if (btype == Building_MilitaryAcademy) {
			UI_BuildingInfo_drawMilitaryAcademy(&context);
		} else if (btype == Building_Barracks) {
			UI_BuildingInfo_drawBarracks(&context);
		} else if (btype == Building_FortGround__) {
			UI_BuildingInfo_drawFort(&context);
		} else if (btype == Building_BurningRuin) {
			UI_BuildingInfo_drawBurningRuin(&context);
		} else if (btype == Building_NativeHut) {
			UI_BuildingInfo_drawNativeHut(&context);
		} else if (btype == Building_NativeMeeting) {
			UI_BuildingInfo_drawNativeMeeting(&context);
		} else if (btype == Building_NativeCrops) {
			UI_BuildingInfo_drawNativeCrops(&context);
		} else if (btype == Building_MissionPost) {
			UI_BuildingInfo_drawMissionPost(&context);
		}
	} else if (context.type == 4) {
		UI_BuildingInfo_drawLegionInfo(&context);
	}
}

void UI_BuildingInfo_drawForeground()
{
	// building-specific buttons
	if (context.type == 2) {
		int btype = Data_Buildings[context.buildingId].type;
		if (btype == Building_Granary) {
			if (context.storageShowSpecialOrders) {
				UI_BuildingInfo_drawGranaryOrdersForeground(&context);
			} else {
				UI_BuildingInfo_drawGranaryForeground(&context);
			}
		} else if (btype == Building_Warehouse) {
			if (context.storageShowSpecialOrders) {
				UI_BuildingInfo_drawWarehouseOrdersForeground(&context);
			} else {
				UI_BuildingInfo_drawWarehouseForeground(&context);
			}
		}
	} else if (context.type == 4) {
		UI_BuildingInfo_drawLegionInfoForeground(&context);
	}
	// general buttons
	if (context.storageShowSpecialOrders) {
		Widget_Button_drawImageButtons(context.xOffset, 432,
			imageButtonsHelpExit, 2);
	} else {
		Widget_Button_drawImageButtons(
			context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
			imageButtonsHelpExit, 2);
	}
	if (context.advisor) {
		Widget_Button_drawImageButtons(
			context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
			imageButtonsAdvisor, 1);
	}
}

void UI_BuildingInfo_handleMouse()
{
	if (Data_Mouse.right.wentUp) {
		UI_Window_goTo(Window_City);
		return;
	}
	// general buttons
	if (context.storageShowSpecialOrders) {
		Widget_Button_handleImageButtons(context.xOffset, 432,
			imageButtonsHelpExit, 2);
	} else {
		Widget_Button_handleImageButtons(
			context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
			imageButtonsHelpExit, 2);
	}
	if (context.advisor) {
		Widget_Button_handleImageButtons(
			context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
			imageButtonsAdvisor, 1);
	}
	// building-specific buttons
	if (context.type == 0) {
		return;
	}
	if (context.type == 4) {
		UI_BuildingInfo_handleMouseLegionInfo(&context);
	} else if (context.walker.drawn) {
		UI_BuildingInfo_handleMouseWalkerList(&context);
	} else if (context.type == 2) {
		int btype = Data_Buildings[context.buildingId].type;
		if (btype == Building_Granary) {
			if (context.storageShowSpecialOrders) {
				UI_BuildingInfo_handleMouseGranaryOrders(&context);
			} else {
				UI_BuildingInfo_handleMouseGranary(&context);
			}
		} else if (btype == Building_Warehouse) {
			if (context.storageShowSpecialOrders) {
				UI_BuildingInfo_handleMouseWarehouseOrders(&context);
			} else {
				UI_BuildingInfo_handleMouseWarehouse(&context);
			}
		}
	}
}

void UI_BuildingInfo_showStorageOrders(int param1, int param2)
{
	context.storageShowSpecialOrders = 1;
	UI_Window_requestRefresh();
}

void UI_BuildingInfo_drawEmploymentInfo(BuildingInfoContext *c, int yOffset)
{
	Data_Building *b = &Data_Buildings[c->buildingId];
	int textId;
	if (b->numWorkers >= Data_Model_Buildings[b->type].laborers) {
		textId = 0;
	} else if (Data_CityInfo.population <= 0) {
		textId = 16; // no people in city
	} else if (b->housesCovered <= 0) {
		textId = 17; // no employees nearby
	} else if (b->housesCovered < 40) {
		textId = 20; // poor access to employees
	} else if (Data_CityInfo.laborCategory[b->laborCategory].workersAllocated > 0) {
		textId = 18; // no people allocated
	} else {
		textId = 19; // too few people allocated
	}
	if (!textId && b->housesCovered < 40) {
		textId = 20; // poor access to employees
	}
	Graphics_drawImage(GraphicId(ID_Graphic_ContextIcons) + 14,
		c->xOffset + 40, yOffset + 6);
	if (textId) {
		int width = Widget_GameText_drawNumberWithDescription(8, 12, b->numWorkers,
			c->xOffset + 60, yOffset + 10, Font_SmallBlack);
		width += Widget_Text_drawNumber(Data_Model_Buildings[b->type].laborers, '(', "",
			c->xOffset + 70 + width, yOffset + 10, Font_SmallBlack);
		Widget_GameText_draw(69, 0, c->xOffset + 70 + width, yOffset + 10, Font_SmallBlack);
		Widget_GameText_draw(69, textId, c->xOffset + 70, yOffset + 26, Font_SmallBlack);
	} else {
		int width = Widget_GameText_drawNumberWithDescription(8, 12, b->numWorkers,
			c->xOffset + 60, yOffset + 16, Font_SmallBlack);
		width += Widget_Text_drawNumber(Data_Model_Buildings[b->type].laborers, '(', "",
			c->xOffset + 70 + width, yOffset + 16, Font_SmallBlack);
		Widget_GameText_draw(69, 0, c->xOffset + 70 + width, yOffset + 16, Font_SmallBlack);
	}
}

static void buttonHelp(int param1, int param2)
{
	if (context.helpId > 0) {
		UI_MessageDialog_show(context.helpId, 0);
	} else {
		UI_MessageDialog_show(10, 0);
	}
	UI_Window_requestRefresh();
}

static void buttonExit(int param1, int param2)
{
	if (context.storageShowSpecialOrders) {
		context.storageShowSpecialOrders = 0;
		UI_Window_requestRefresh();
	} else {
		UI_Window_goTo(Window_City);
	}
}

static void buttonAdvisor(int param1, int param2)
{
	// TODO
}
