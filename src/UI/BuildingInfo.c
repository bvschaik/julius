#include "BuildingInfo.h"

#include "../Data/CityInfo.h"
#include "../Data/CityView.h"
#include "../Data/State.h"

#include "building/barracks.h"
#include "building/house_evolution.h"
#include "building/model.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "figure/formation_legion.h"
#include "figure/phrase.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "map/aqueduct.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/image.h"
#include "map/property.h"
#include "map/road_access.h"
#include "map/sprite.h"
#include "map/terrain.h"
#include "window/advisors.h"
#include "window/city.h"
#include "window/message_dialog.h"

static void buttonHelp(int param1, int param2);
static void buttonExit(int param1, int param2);
static void buttonAdvisor(int param1, int param2);

static image_button imageButtonsHelpExit[] = {
	{14, 0, 27, 27, IB_NORMAL, 134, 0, buttonHelp, button_none, 0, 0, 1},
	{424, 3, 24, 24, IB_NORMAL, 134, 4, buttonExit, button_none, 0, 0, 1}
};

static image_button imageButtonsAdvisor[] = {
	{350, -38, 28, 28, IB_NORMAL, 199, 9, buttonAdvisor, button_none, ADVISOR_RATINGS, 0, 1}
};

static BuildingInfoContext context;
static int focusImageButtonId;

static int getHeightId()
{
	if (context.type == BuildingInfoType_Terrain) {
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
	} else if (context.type == BuildingInfoType_Building) {
		switch (building_get(context.buildingId)->type) {
			case BUILDING_SMALL_TEMPLE_CERES:
			case BUILDING_SMALL_TEMPLE_NEPTUNE:
			case BUILDING_SMALL_TEMPLE_MERCURY:
			case BUILDING_SMALL_TEMPLE_MARS:
			case BUILDING_SMALL_TEMPLE_VENUS:
			case BUILDING_LARGE_TEMPLE_CERES:
			case BUILDING_LARGE_TEMPLE_NEPTUNE:
			case BUILDING_LARGE_TEMPLE_MERCURY:
			case BUILDING_LARGE_TEMPLE_MARS:
			case BUILDING_LARGE_TEMPLE_VENUS:
			case BUILDING_ORACLE:
			case BUILDING_SMALL_STATUE:
			case BUILDING_MEDIUM_STATUE:
			case BUILDING_LARGE_STATUE:
			case BUILDING_GLADIATOR_SCHOOL:
			case BUILDING_LION_HOUSE:
			case BUILDING_ACTOR_COLONY:
			case BUILDING_CHARIOT_MAKER:
			case BUILDING_DOCTOR:
			case BUILDING_HOSPITAL:
			case BUILDING_BATHHOUSE:
			case BUILDING_BARBER:
			case BUILDING_BURNING_RUIN:
			case BUILDING_RESERVOIR:
			case BUILDING_NATIVE_HUT:
			case BUILDING_NATIVE_MEETING:
			case BUILDING_NATIVE_CROPS:
			case BUILDING_MISSION_POST:
			case BUILDING_PREFECTURE:
			case BUILDING_ENGINEERS_POST:
			case BUILDING_SCHOOL:
			case BUILDING_ACADEMY:
			case BUILDING_LIBRARY:
			case BUILDING_GATEHOUSE:
			case BUILDING_TOWER:
			case BUILDING_FORT:
			case BUILDING_MILITARY_ACADEMY:
			case BUILDING_BARRACKS:
			case BUILDING_MARKET:
			case BUILDING_GRANARY:
			case BUILDING_SHIPYARD:
			case BUILDING_DOCK:
			case BUILDING_WHARF:
			case BUILDING_GOVERNORS_HOUSE:
			case BUILDING_GOVERNORS_VILLA:
			case BUILDING_GOVERNORS_PALACE:
			case BUILDING_FORUM:
			case BUILDING_FORUM_UPGRADED:
			case BUILDING_WINE_WORKSHOP:
			case BUILDING_OIL_WORKSHOP:
			case BUILDING_WEAPONS_WORKSHOP:
			case BUILDING_FURNITURE_WORKSHOP:
			case BUILDING_POTTERY_WORKSHOP:
				return 1;

			case BUILDING_THEATER:
			case BUILDING_HIPPODROME:
			case BUILDING_COLOSSEUM:
			case BUILDING_SENATE:
			case BUILDING_SENATE_UPGRADED:
				return 2;

			case BUILDING_AMPHITHEATER:
				return 3;

			case BUILDING_FOUNTAIN:
			case BUILDING_WELL:
				return 4;
		}
	}
	return 0;
}

void UI_BuildingInfo_show(int gridOffset)
{
	context.canPlaySound = 1;
	context.storageShowSpecialOrders = 0;
	context.advisor = 0;
	context.buildingId = map_building_at(gridOffset);
	context.rubbleBuildingType = map_rubble_building_type(gridOffset);
	context.hasReservoirPipes = map_terrain_is(gridOffset, TERRAIN_RESERVOIR_RANGE);
	context.aqueductHasWater = map_aqueduct_at(gridOffset) && map_image_at(gridOffset) - image_group(GROUP_BUILDING_AQUEDUCT) < 15;

	city_resource_determine_available();
	context.type = BuildingInfoType_Terrain;
	context.figure.drawn = 0;
	if (!context.buildingId && map_sprite_bridge_at(gridOffset) > 0) {
		if (map_terrain_is(gridOffset, TERRAIN_WATER)) {
			context.terrainType = 11;
		} else {
			context.terrainType = 10;
		}
	} else if (map_property_is_plaza_or_earthquake(gridOffset)) {
		if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
			context.terrainType = 13;
		}
		if (map_terrain_is(gridOffset, TERRAIN_ROCK)) {
			context.terrainType = 5;
		}
	} else if (map_terrain_is(gridOffset, TERRAIN_TREE)) {
		context.terrainType = 1;
	} else if (map_terrain_is(gridOffset, TERRAIN_ROCK)) {
		if (gridOffset == Data_CityInfo_Extra.entryPointFlag.gridOffset) {
			context.terrainType = 14;
		} else if (gridOffset == Data_CityInfo_Extra.exitPointFlag.gridOffset) {
			context.terrainType = 15;
		} else {
			context.terrainType = 2;
		}
	} else if ((map_terrain_get(gridOffset) & (TERRAIN_WATER|TERRAIN_BUILDING)) == TERRAIN_WATER) {
		context.terrainType = 3;
	} else if (map_terrain_is(gridOffset, TERRAIN_SCRUB)) {
		context.terrainType = 4;
	} else if (map_terrain_is(gridOffset, TERRAIN_GARDEN)) {
		context.terrainType = 12;
	} else if ((map_terrain_get(gridOffset) & (TERRAIN_ROAD|TERRAIN_BUILDING)) == TERRAIN_ROAD) {
		context.terrainType = 6;
	} else if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT)) {
		context.terrainType = 7;
	} else if (map_terrain_is(gridOffset, TERRAIN_RUBBLE)) {
		context.terrainType = 8;
	} else if (map_terrain_is(gridOffset, TERRAIN_WALL)) {
		context.terrainType = 9;
	} else if (!context.buildingId) {
		context.terrainType = 10;
	} else {
        building *b = building_get(context.buildingId);
		context.type = BuildingInfoType_Building;
		context.workerPercentage = calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
		switch (b->type) {
			case BUILDING_FORT_GROUND:
				context.buildingId = b->prevPartBuildingId;
				// fallthrough
			case BUILDING_FORT:
				context.formationId = b->formationId;
				break;
			case BUILDING_WAREHOUSE_SPACE:
			case BUILDING_HIPPODROME:
				context.buildingId = building_main(b)->id;
				break;
			case BUILDING_BARRACKS:
				context.barracksSoldiersRequested = formation_legion_recruits_needed();
				context.barracksSoldiersRequested += building_barracks_has_tower_sentry_request();
				break;
			default:
				if (b->houseSize) {
					context.worstDesirabilityBuildingId = building_house_determine_worst_desirability_building(b);
					building_house_determine_evolve_text(b, context.worstDesirabilityBuildingId);
				}
				break;
		}
		context.hasRoadAccess = 0;
		switch (b->type) {
			case BUILDING_GRANARY:
				if (map_has_road_access_granary(b->x, b->y, 0, 0)) {
					context.hasRoadAccess = 1;
				}
				break;
			case BUILDING_HIPPODROME:
				if (map_has_road_access_hippodrome(b->x, b->y, 0, 0)) {
					context.hasRoadAccess = 1;
				}
				break;
			case BUILDING_WAREHOUSE:
				if (map_has_road_access(b->x, b->y, 3, 0, 0)) {
					context.hasRoadAccess = 1;
				}
				context.warehouseSpaceText = building_warehouse_get_space_info(b);
				break;
			default:
				if (map_has_road_access(b->x, b->y, b->size, 0, 0)) {
					context.hasRoadAccess = 1;
				}
				break;
		}
	}
	// figures
	context.figure.selectedIndex = 0;
	context.figure.count = 0;
	for (int i = 0; i < 7; i++) {
		context.figure.figureIds[i] = 0;
	}
	static const int figureOffsets[] = {0, -162, 162, 1, -1, -163, -161, 161, 163};
	for (int i = 0; i < 9 && context.figure.count < 7; i++) {
		int figureId = map_figure_at(gridOffset + figureOffsets[i]);
		while (figureId > 0 && context.figure.count < 7) {
            figure *figure = figure_get(figureId);
			if (figure->state != FigureState_Dead &&
				figure->actionState != FIGURE_ACTION_149_CORPSE) {
				switch (figure->type) {
					case FIGURE_NONE:
					case FIGURE_EXPLOSION:
					case FIGURE_MAP_FLAG:
					case FIGURE_FLOTSAM:
					case FIGURE_ARROW:
					case FIGURE_JAVELIN:
					case FIGURE_BOLT:
					case FIGURE_BALLISTA:
					case FIGURE_CREATURE:
					case FIGURE_FISH_GULLS:
					case FIGURE_SPEAR:
					case FIGURE_HIPPODROME_HORSES:
						break;
					default:
						context.figure.figureIds[context.figure.count++] = figureId;
						figure_phrase_determine(figure);
						break;
				}
			}
			figureId = figure->nextFigureIdOnSameTile;
		}
	}
	// check for legion figures
	for (int i = 0; i < 7; i++) {
		int figureId = context.figure.figureIds[i];
		if (figureId <= 0) {
			continue;
		}
        figure *f = figure_get(figureId);
		int type = f->type;
		if (type == FIGURE_FORT_STANDARD || FigureIsLegion(type)) {
			context.type = BuildingInfoType_Legion;
			context.formationId = f->formationId;
            const formation *m = formation_get(context.formationId);
			if (m->figure_type != FIGURE_FORT_LEGIONARY) {
				context.formationTypes = 5;
			} else if (m->has_military_training) {
				context.formationTypes = 4;
			} else {
				context.formationTypes = 3;
			}
			break;
		}
	}
	// dialog size
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
	// dialog placement
	int s_height = screen_height();
	if (s_height >= 600) {
		if (mouse_get()->y <= (s_height - 24) / 2 + 24) {
			context.yOffset = s_height - 16 * context.heightBlocks - 16;
		} else {
			context.yOffset = 32;
		}
	}
	int border = (Data_CityView.widthInPixels - 16 * context.widthBlocks) / 2;
	context.xOffset = Data_CityView.xOffsetInPixels + border;
    UI_Window_goTo(Window_BuildingInfo);
}

int UI_BuildingInfo_getBuildingType()
{
	if (context.type == BuildingInfoType_Building) {
		return building_get(context.buildingId)->type;
	}
	return BUILDING_WELL;
}

void UI_BuildingInfo_drawBackground()
{
	window_city_draw_panels();
	window_city_draw();
	if (context.type == BuildingInfoType_None) {
		UI_BuildingInfo_drawNoPeople(&context);
	} else if (context.type == BuildingInfoType_Terrain) {
		UI_BuildingInfo_drawTerrain(&context);
	} else if (context.type == BuildingInfoType_Building) {
		int btype = building_get(context.buildingId)->type;
		if (building_is_house(btype)) {
			UI_BuildingInfo_drawHouse(&context);
		} else if (btype == BUILDING_WHEAT_FARM) {
			UI_BuildingInfo_drawWheatFarm(&context);
		} else if (btype == BUILDING_VEGETABLE_FARM) {
			UI_BuildingInfo_drawVegetableFarm(&context);
		} else if (btype == BUILDING_FRUIT_FARM) {
			UI_BuildingInfo_drawFruitFarm(&context);
		} else if (btype == BUILDING_OLIVE_FARM) {
			UI_BuildingInfo_drawOliveFarm(&context);
		} else if (btype == BUILDING_VINES_FARM) {
			UI_BuildingInfo_drawVinesFarm(&context);
		} else if (btype == BUILDING_PIG_FARM) {
			UI_BuildingInfo_drawPigFarm(&context);
		} else if (btype == BUILDING_MARBLE_QUARRY) {
			UI_BuildingInfo_drawMarbleQuarry(&context);
		} else if (btype == BUILDING_IRON_MINE) {
			UI_BuildingInfo_drawIronMine(&context);
		} else if (btype == BUILDING_TIMBER_YARD) {
			UI_BuildingInfo_drawTimberYard(&context);
		} else if (btype == BUILDING_CLAY_PIT) {
			UI_BuildingInfo_drawClayPit(&context);
		} else if (btype == BUILDING_WINE_WORKSHOP) {
			UI_BuildingInfo_drawWineWorkshop(&context);
		} else if (btype == BUILDING_OIL_WORKSHOP) {
			UI_BuildingInfo_drawOilWorkshop(&context);
		} else if (btype == BUILDING_WEAPONS_WORKSHOP) {
			UI_BuildingInfo_drawWeaponsWorkshop(&context);
		} else if (btype == BUILDING_FURNITURE_WORKSHOP) {
			UI_BuildingInfo_drawFurnitureWorkshop(&context);
		} else if (btype == BUILDING_POTTERY_WORKSHOP) {
			UI_BuildingInfo_drawPotteryWorkshop(&context);
		} else if (btype == BUILDING_MARKET) {
			UI_BuildingInfo_drawMarket(&context);
		} else if (btype == BUILDING_GRANARY) {
			if (context.storageShowSpecialOrders) {
				UI_BuildingInfo_drawGranaryOrders(&context);
			} else {
				UI_BuildingInfo_drawGranary(&context);
			}
		} else if (btype == BUILDING_WAREHOUSE) {
			if (context.storageShowSpecialOrders) {
				UI_BuildingInfo_drawWarehouseOrders(&context);
			} else {
				UI_BuildingInfo_drawWarehouse(&context);
			}
		} else if (btype == BUILDING_AMPHITHEATER) {
			UI_BuildingInfo_drawAmphitheater(&context);
		} else if (btype == BUILDING_THEATER) {
			UI_BuildingInfo_drawTheater(&context);
		} else if (btype == BUILDING_HIPPODROME) {
			UI_BuildingInfo_drawHippodrome(&context);
		} else if (btype == BUILDING_COLOSSEUM) {
			UI_BuildingInfo_drawColosseum(&context);
		} else if (btype == BUILDING_GLADIATOR_SCHOOL) {
			UI_BuildingInfo_drawGladiatorSchool(&context);
		} else if (btype == BUILDING_LION_HOUSE) {
			UI_BuildingInfo_drawLionHouse(&context);
		} else if (btype == BUILDING_ACTOR_COLONY) {
			UI_BuildingInfo_drawActorColony(&context);
		} else if (btype == BUILDING_CHARIOT_MAKER) {
			UI_BuildingInfo_drawChariotMaker(&context);
		} else if (btype == BUILDING_DOCTOR) {
			UI_BuildingInfo_drawClinic(&context);
		} else if (btype == BUILDING_HOSPITAL) {
			UI_BuildingInfo_drawHospital(&context);
		} else if (btype == BUILDING_BATHHOUSE) {
			UI_BuildingInfo_drawBathhouse(&context);
		} else if (btype == BUILDING_BARBER) {
			UI_BuildingInfo_drawBarber(&context);
		} else if (btype == BUILDING_SCHOOL) {
			UI_BuildingInfo_drawSchool(&context);
		} else if (btype == BUILDING_ACADEMY) {
			UI_BuildingInfo_drawAcademy(&context);
		} else if (btype == BUILDING_LIBRARY) {
			UI_BuildingInfo_drawLibrary(&context);
		} else if (btype == BUILDING_SMALL_TEMPLE_CERES || btype == BUILDING_LARGE_TEMPLE_CERES) {
			UI_BuildingInfo_drawTempleCeres(&context);
		} else if (btype == BUILDING_SMALL_TEMPLE_NEPTUNE || btype == BUILDING_LARGE_TEMPLE_NEPTUNE) {
			UI_BuildingInfo_drawTempleNeptune(&context);
		} else if (btype == BUILDING_SMALL_TEMPLE_MERCURY || btype == BUILDING_LARGE_TEMPLE_MERCURY) {
			UI_BuildingInfo_drawTempleMercury(&context);
		} else if (btype == BUILDING_SMALL_TEMPLE_MARS || btype == BUILDING_LARGE_TEMPLE_MARS) {
			UI_BuildingInfo_drawTempleMars(&context);
		} else if (btype == BUILDING_SMALL_TEMPLE_VENUS || btype == BUILDING_LARGE_TEMPLE_VENUS) {
			UI_BuildingInfo_drawTempleVenus(&context);
		} else if (btype == BUILDING_ORACLE) {
			UI_BuildingInfo_drawOracle(&context);
		} else if (btype == BUILDING_GOVERNORS_HOUSE || btype == BUILDING_GOVERNORS_VILLA || btype == BUILDING_GOVERNORS_PALACE) {
			UI_BuildingInfo_drawGovernorsHome(&context);
		} else if (btype == BUILDING_FORUM || btype == BUILDING_FORUM_UPGRADED) {
			UI_BuildingInfo_drawForum(&context);
		} else if (btype == BUILDING_SENATE || btype == BUILDING_SENATE_UPGRADED) {
			UI_BuildingInfo_drawSenate(&context);
		} else if (btype == BUILDING_ENGINEERS_POST) {
			UI_BuildingInfo_drawEngineersPost(&context);
		} else if (btype == BUILDING_SHIPYARD) {
			UI_BuildingInfo_drawShipyard(&context);
		} else if (btype == BUILDING_DOCK) {
			UI_BuildingInfo_drawDock(&context);
		} else if (btype == BUILDING_WHARF) {
			UI_BuildingInfo_drawWharf(&context);
		} else if (btype == BUILDING_RESERVOIR) {
			UI_BuildingInfo_drawReservoir(&context);
		} else if (btype == BUILDING_FOUNTAIN) {
			UI_BuildingInfo_drawFountain(&context);
		} else if (btype == BUILDING_WELL) {
			UI_BuildingInfo_drawWell(&context);
		} else if (btype == BUILDING_SMALL_STATUE || btype == BUILDING_MEDIUM_STATUE || btype == BUILDING_LARGE_STATUE) {
			UI_BuildingInfo_drawStatue(&context);
		} else if (btype == BUILDING_TRIUMPHAL_ARCH) {
			UI_BuildingInfo_drawTriumphalArch(&context);
		} else if (btype == BUILDING_PREFECTURE) {
			UI_BuildingInfo_drawPrefect(&context);
		} else if (btype == BUILDING_GATEHOUSE) {
			UI_BuildingInfo_drawGatehouse(&context);
		} else if (btype == BUILDING_TOWER) {
			UI_BuildingInfo_drawTower(&context);
		} else if (btype == BUILDING_MILITARY_ACADEMY) {
			UI_BuildingInfo_drawMilitaryAcademy(&context);
		} else if (btype == BUILDING_BARRACKS) {
			UI_BuildingInfo_drawBarracks(&context);
		} else if (btype == BUILDING_FORT) {
			UI_BuildingInfo_drawFort(&context);
		} else if (btype == BUILDING_BURNING_RUIN) {
			UI_BuildingInfo_drawBurningRuin(&context);
		} else if (btype == BUILDING_NATIVE_HUT) {
			UI_BuildingInfo_drawNativeHut(&context);
		} else if (btype == BUILDING_NATIVE_MEETING) {
			UI_BuildingInfo_drawNativeMeeting(&context);
		} else if (btype == BUILDING_NATIVE_CROPS) {
			UI_BuildingInfo_drawNativeCrops(&context);
		} else if (btype == BUILDING_MISSION_POST) {
			UI_BuildingInfo_drawMissionPost(&context);
		}
	} else if (context.type == BuildingInfoType_Legion) {
		UI_BuildingInfo_drawLegionInfo(&context);
	}
}

void UI_BuildingInfo_drawForeground()
{
	// building-specific buttons
	if (context.type == BuildingInfoType_Building) {
		int btype = building_get(context.buildingId)->type;
		if (btype == BUILDING_GRANARY) {
			if (context.storageShowSpecialOrders) {
				UI_BuildingInfo_drawGranaryOrdersForeground(&context);
			} else {
				UI_BuildingInfo_drawGranaryForeground(&context);
			}
		} else if (btype == BUILDING_WAREHOUSE) {
			if (context.storageShowSpecialOrders) {
				UI_BuildingInfo_drawWarehouseOrdersForeground(&context);
			} else {
				UI_BuildingInfo_drawWarehouseForeground(&context);
			}
		}
	} else if (context.type == BuildingInfoType_Legion) {
		UI_BuildingInfo_drawLegionInfoForeground(&context);
	}
	// general buttons
	if (context.storageShowSpecialOrders) {
		image_buttons_draw(context.xOffset, 432,
			imageButtonsHelpExit, 2);
	} else {
		image_buttons_draw(
			context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
			imageButtonsHelpExit, 2);
	}
	if (context.advisor) {
		image_buttons_draw(
			context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
			imageButtonsAdvisor, 1);
	}
}

void UI_BuildingInfo_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		window_city_show();
		return;
	}
	// general buttons
	if (context.storageShowSpecialOrders) {
		image_buttons_handle_mouse(m, context.xOffset, 432,
			imageButtonsHelpExit, 2, &focusImageButtonId);
	} else {
		image_buttons_handle_mouse(
			m, context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
			imageButtonsHelpExit, 2, &focusImageButtonId);
	}
	if (context.advisor) {
		image_buttons_handle_mouse(
			m, context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
			imageButtonsAdvisor, 1, 0);
	}
	// building-specific buttons
	if (context.type == BuildingInfoType_None) {
		return;
	}
	if (context.type == BuildingInfoType_Legion) {
		UI_BuildingInfo_handleMouseLegionInfo(m, &context);
	} else if (context.figure.drawn) {
		UI_BuildingInfo_handleMouseFigureList(m, &context);
	} else if (context.type == BuildingInfoType_Building) {
		int btype = building_get(context.buildingId)->type;
		if (btype == BUILDING_GRANARY) {
			if (context.storageShowSpecialOrders) {
				UI_BuildingInfo_handleMouseGranaryOrders(m, &context);
			} else {
				UI_BuildingInfo_handleMouseGranary(m, &context);
			}
		} else if (btype == BUILDING_WAREHOUSE) {
			if (context.storageShowSpecialOrders) {
				UI_BuildingInfo_handleMouseWarehouseOrders(m, &context);
			} else {
				UI_BuildingInfo_handleMouseWarehouse(m, &context);
			}
		}
	}
}

void UI_BuildingInfo_getTooltip(tooltip_context *c)
{
	int textId = 0;
	if (focusImageButtonId) {
		textId = focusImageButtonId;
	} else if (context.type == BuildingInfoType_Legion) {
		textId = UI_BuildingInfo_getTooltipLegionInfo(&context);
	}
	if (textId) {
		c->type = TOOLTIP_BUTTON;
		c->text_id = textId;
	}
}

void UI_BuildingInfo_showStorageOrders(int param1, int param2)
{
	context.storageShowSpecialOrders = 1;
	window_invalidate();
}

void UI_BuildingInfo_drawEmploymentInfo(BuildingInfoContext *c, int yOffset)
{
	building *b = building_get(c->buildingId);
	int textId;
	if (b->numWorkers >= model_get_building(b->type)->laborers) {
		textId = 0;
	} else if (Data_CityInfo.population <= 0) {
		textId = 16; // no people in city
	} else if (b->housesCovered <= 0) {
		textId = 17; // no employees nearby
	} else if (b->housesCovered < 40) {
		textId = 20; // poor access to employees
	} else if (Data_CityInfo.laborCategory[b->laborCategory].workersAllocated <= 0) {
		textId = 18; // no people allocated
	} else {
		textId = 19; // too few people allocated
	}
	if (!textId && b->housesCovered < 40) {
		textId = 20; // poor access to employees
	}
	image_draw(image_group(GROUP_CONTEXT_ICONS) + 14,
		c->xOffset + 40, yOffset + 6);
	if (textId) {
		int width = lang_text_draw_amount(8, 12, b->numWorkers,
			c->xOffset + 60, yOffset + 10, FONT_SMALL_BLACK);
		width += text_draw_number(model_get_building(b->type)->laborers, '(', "",
			c->xOffset + 70 + width, yOffset + 10, FONT_SMALL_BLACK);
		lang_text_draw(69, 0, c->xOffset + 70 + width, yOffset + 10, FONT_SMALL_BLACK);
		lang_text_draw(69, textId, c->xOffset + 70, yOffset + 26, FONT_SMALL_BLACK);
	} else {
		int width = lang_text_draw_amount(8, 12, b->numWorkers,
			c->xOffset + 60, yOffset + 16, FONT_SMALL_BLACK);
		width += text_draw_number(model_get_building(b->type)->laborers, '(', "",
			c->xOffset + 70 + width, yOffset + 16, FONT_SMALL_BLACK);
		lang_text_draw(69, 0, c->xOffset + 70 + width, yOffset + 16, FONT_SMALL_BLACK);
	}
}

static void buttonHelp(int param1, int param2)
{
	if (context.helpId > 0) {
		window_message_dialog_show(context.helpId, 0);
	} else {
		window_message_dialog_show(10, 0);
	}
	window_invalidate();
}

static void buttonExit(int param1, int param2)
{
	if (context.storageShowSpecialOrders) {
		context.storageShowSpecialOrders = 0;
		window_invalidate();
	} else {
		window_city_show();
	}
}

static void buttonAdvisor(int param1, int param2)
{
	window_advisors_show_advisor(ADVISOR_RATINGS);
}
