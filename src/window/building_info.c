#include "building_info.h"

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
#include "window/building/common.h"

#include "Data/CityInfo.h"
#include "Data/CityView.h"
#include "UI/BuildingInfo.h"

static void button_help(int param1, int param2);
static void button_close(int param1, int param2);
static void button_advisor(int advisor, int param2);

static image_button image_buttons_help_close[] = {
    {14, 0, 27, 27, IB_NORMAL, 134, 0, button_help, button_none, 0, 0, 1},
    {424, 3, 24, 24, IB_NORMAL, 134, 4, button_close, button_none, 0, 0, 1}
};

static image_button image_buttons_advisor[] = {
    {350, -38, 28, 28, IB_NORMAL, 199, 9, button_advisor, button_none, ADVISOR_RATINGS, 0, 1}
};

static BuildingInfoContext context;
static int focus_image_button_id;

static int get_height_id()
{
    if (context.type == BUILDING_INFO_TERRAIN) {
        switch (context.terrainType) {
            case TERRAIN_INFO_AQUEDUCT:
                return 4;
            case TERRAIN_INFO_RUBBLE:
            case TERRAIN_INFO_WALL:
            case TERRAIN_INFO_GARDEN:
                return 1;
            default:
                return 0;
        }
    } else if (context.type == BUILDING_INFO_BUILDING) {
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

            default:
                return 0;
        }
    }
    return 0;
}

static void init(int grid_offset)
{
    context.canPlaySound = 1;
    context.storageShowSpecialOrders = 0;
    context.advisor = 0;
    context.buildingId = map_building_at(grid_offset);
    context.rubbleBuildingType = map_rubble_building_type(grid_offset);
    context.hasReservoirPipes = map_terrain_is(grid_offset, TERRAIN_RESERVOIR_RANGE);
    context.aqueductHasWater = map_aqueduct_at(grid_offset) && map_image_at(grid_offset) - image_group(GROUP_BUILDING_AQUEDUCT) < 15;

    city_resource_determine_available();
    context.type = BUILDING_INFO_TERRAIN;
    context.figure.drawn = 0;
    if (!context.buildingId && map_sprite_bridge_at(grid_offset) > 0) {
        if (map_terrain_is(grid_offset, TERRAIN_WATER)) {
            context.terrainType = TERRAIN_INFO_BRIDGE;
        } else {
            context.terrainType = TERRAIN_INFO_EMPTY;
        }
    } else if (map_property_is_plaza_or_earthquake(grid_offset)) {
        if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
            context.terrainType = TERRAIN_INFO_PLAZA;
        }
        if (map_terrain_is(grid_offset, TERRAIN_ROCK)) {
            context.terrainType = TERRAIN_INFO_EARTHQUAKE;
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_TREE)) {
        context.terrainType = TERRAIN_INFO_TREE;
    } else if (map_terrain_is(grid_offset, TERRAIN_ROCK)) {
        if (grid_offset == Data_CityInfo_Extra.entryPointFlag.gridOffset) {
            context.terrainType = TERRAIN_INFO_ENTRY_FLAG;
        } else if (grid_offset == Data_CityInfo_Extra.exitPointFlag.gridOffset) {
            context.terrainType = TERRAIN_INFO_EXIT_FLAG;
        } else {
            context.terrainType = TERRAIN_INFO_ROCK;
        }
    } else if ((map_terrain_get(grid_offset) & (TERRAIN_WATER|TERRAIN_BUILDING)) == TERRAIN_WATER) {
        context.terrainType = TERRAIN_INFO_WATER;
    } else if (map_terrain_is(grid_offset, TERRAIN_SCRUB)) {
        context.terrainType = TERRAIN_INFO_SCRUB;
    } else if (map_terrain_is(grid_offset, TERRAIN_GARDEN)) {
        context.terrainType = TERRAIN_INFO_GARDEN;
    } else if ((map_terrain_get(grid_offset) & (TERRAIN_ROAD|TERRAIN_BUILDING)) == TERRAIN_ROAD) {
        context.terrainType = TERRAIN_INFO_ROAD;
    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
        context.terrainType = TERRAIN_INFO_AQUEDUCT;
    } else if (map_terrain_is(grid_offset, TERRAIN_RUBBLE)) {
        context.terrainType = TERRAIN_INFO_RUBBLE;
    } else if (map_terrain_is(grid_offset, TERRAIN_WALL)) {
        context.terrainType = TERRAIN_INFO_WALL;
    } else if (!context.buildingId) {
        context.terrainType = TERRAIN_INFO_EMPTY;
    } else {
        building *b = building_get(context.buildingId);
        context.type = BUILDING_INFO_BUILDING;
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
                b = building_main(b);
                context.buildingId = b->id;
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
        int figureId = map_figure_at(grid_offset + figureOffsets[i]);
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
            context.type = BUILDING_INFO_LEGION;
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
    switch (get_height_id()) {
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
}

static void draw_background()
{
    window_city_draw_panels();
    window_city_draw();
    if (context.type == BUILDING_INFO_NONE) {
        UI_BuildingInfo_drawNoPeople(&context);
    } else if (context.type == BUILDING_INFO_TERRAIN) {
        UI_BuildingInfo_drawTerrain(&context);
    } else if (context.type == BUILDING_INFO_BUILDING) {
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
    } else if (context.type == BUILDING_INFO_LEGION) {
        UI_BuildingInfo_drawLegionInfo(&context);
    }
}

static void draw_foreground()
{
    // building-specific buttons
    if (context.type == BUILDING_INFO_BUILDING) {
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
    } else if (context.type == BUILDING_INFO_LEGION) {
        UI_BuildingInfo_drawLegionInfoForeground(&context);
    }
    // general buttons
    if (context.storageShowSpecialOrders) {
        image_buttons_draw(context.xOffset, 432,
                           image_buttons_help_close, 2);
    } else {
        image_buttons_draw(
            context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
            image_buttons_help_close, 2);
    }
    if (context.advisor) {
        image_buttons_draw(
            context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
            image_buttons_advisor, 1);
    }
}

static void handle_mouse(const mouse *m)
{
    if (m->right.went_up) {
        window_city_show();
        return;
    }
    // general buttons
    if (context.storageShowSpecialOrders) {
        image_buttons_handle_mouse(m, context.xOffset, 432,
                                   image_buttons_help_close, 2, &focus_image_button_id);
    } else {
        image_buttons_handle_mouse(
            m, context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
            image_buttons_help_close, 2, &focus_image_button_id);
    }
    if (context.advisor) {
        image_buttons_handle_mouse(
            m, context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
            image_buttons_advisor, 1, 0);
    }
    // building-specific buttons
    if (context.type == BUILDING_INFO_NONE) {
        return;
    }
    if (context.type == BUILDING_INFO_LEGION) {
        UI_BuildingInfo_handleMouseLegionInfo(m, &context);
    } else if (context.figure.drawn) {
        UI_BuildingInfo_handleMouseFigureList(m, &context);
    } else if (context.type == BUILDING_INFO_BUILDING) {
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

static void get_tooltip(tooltip_context *c)
{
    int textId = 0;
    if (focus_image_button_id) {
        textId = focus_image_button_id;
    } else if (context.type == BUILDING_INFO_LEGION) {
        textId = UI_BuildingInfo_getTooltipLegionInfo(&context);
    }
    if (textId) {
        c->type = TOOLTIP_BUTTON;
        c->text_id = textId;
    }
}

static void button_help(int param1, int param2)
{
    if (context.helpId > 0) {
        window_message_dialog_show(context.helpId, 0);
    } else {
        window_message_dialog_show(10, 0);
    }
    window_invalidate();
}

static void button_close(int param1, int param2)
{
    if (context.storageShowSpecialOrders) {
        context.storageShowSpecialOrders = 0;
        window_invalidate();
    } else {
        window_city_show();
    }
}

static void button_advisor(int advisor, int param2)
{
    window_advisors_show_advisor(advisor);
}

void window_building_info_show(int grid_offset)
{
    window_type window = {
        Window_BuildingInfo,
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip
    };
    init(grid_offset);
    window_show(&window);
}

int window_building_info_get_building_type()
{
    if (context.type == BUILDING_INFO_BUILDING) {
        return building_get(context.buildingId)->type;
    }
    return BUILDING_NONE;
}

void window_building_info_show_storage_orders()
{
    context.storageShowSpecialOrders = 1;
    window_invalidate();
}
