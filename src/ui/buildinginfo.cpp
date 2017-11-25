#include "buildinginfo.h"

#include "building.h"
#include "core/calc.h"
#include "cityinfo.h"
#include "figure.h"
#include "formation.h"
#include "graphics.h"
#include "houseevolution.h"
#include "resource.h"
#include "terrain.h"

#include <ui>
#include <data>
#include <game>

#include "building/model.h"
#include "figure/formation.h"

static void buttonHelp(int param1, int param2);
static void buttonExit(int param1, int param2);
static void buttonAdvisor(int param1, int param2);

static ImageButton imageButtonsHelpExit[] =
{
    {14, 0, 27, 27, ImageButton_Normal, 134, 0, buttonHelp, Widget::Button::doNothing, 0, 0, 1},
    {424, 3, 24, 24, ImageButton_Normal, 134, 4, buttonExit, Widget::Button::doNothing, 0, 0, 1}
};

static ImageButton imageButtonsAdvisor[] =
{
    {350, -38, 28, 28, ImageButton_Normal, 199, 9, buttonAdvisor, Widget::Button::doNothing, Advisor_Ratings, 0, 1}
};

static BuildingInfoContext context;
static int focusImageButtonId;

static int getHeightId()
{
    if (context.type == BuildingInfoType_Terrain)
    {
        switch (context.terrainType)
        {
        case 7:
            return 4; // aqueduct
        case 8:
        case 9:
        case 12:
            return 1; // rubble, wall, garden
        default:
            return 0;
        }
    }
    else if (context.type == BuildingInfoType_Building)
    {
        switch (Data_Buildings[context.buildingId].type)
        {
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

void UI_BuildingInfo_init()
{
    int gridOffset = Data_Settings_Map.current.gridOffset;
    int terrain = Data_Grid_terrain[gridOffset];
    context.canPlaySound = 1;
    context.storageShowSpecialOrders = 0;
    context.advisor = 0;
    context.buildingId = Data_Grid_buildingIds[gridOffset];
    context.rubbleBuildingType = Data_Grid_rubbleBuildingType[gridOffset];
    context.hasReservoirPipes = terrain & Terrain_ReservoirRange;
    context.aqueductHasWater = Data_Grid_aqueducts[gridOffset];

    CityInfo_Resource_calculateAvailableResources();
    context.type = BuildingInfoType_Terrain;
    context.figure.drawn = 0;
    if (!Data_Grid_buildingIds[gridOffset] && Data_Grid_spriteOffsets[gridOffset] > 0)
    {
        if (Data_Grid_terrain[gridOffset] & Terrain_Water)
        {
            context.terrainType = 11;
        }
        else
        {
            context.terrainType = 10;
        }
    }
    else if (Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake)
    {
        if (Data_Grid_terrain[gridOffset] & Terrain_Road)
        {
            context.terrainType = 13;
        }
        if (Data_Grid_terrain[gridOffset] & Terrain_Rock)
        {
            context.terrainType = 5;
        }
    }
    else if (Data_Grid_terrain[gridOffset] & Terrain_Tree)
    {
        context.terrainType = 1;
    }
    else if (Data_Grid_terrain[gridOffset] & Terrain_Rock)
    {
        if (gridOffset == Data_CityInfo_Extra.entryPointFlag.gridOffset)
        {
            context.terrainType = 14;
        }
        else if (gridOffset == Data_CityInfo_Extra.exitPointFlag.gridOffset)
        {
            context.terrainType = 15;
        }
        else
        {
            context.terrainType = 2;
        }
    }
    else if ((Data_Grid_terrain[gridOffset] & (Terrain_Water|Terrain_Building)) == Terrain_Water)
    {
        context.terrainType = 3;
    }
    else if (Data_Grid_terrain[gridOffset] & Terrain_Scrub)
    {
        context.terrainType = 4;
    }
    else if (Data_Grid_terrain[gridOffset] & Terrain_Garden)
    {
        context.terrainType = 12;
    }
    else if ((Data_Grid_terrain[gridOffset] & (Terrain_Road|Terrain_Building)) == Terrain_Road)
    {
        context.terrainType = 6;
    }
    else if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct)
    {
        context.terrainType = 7;
    }
    else if (Data_Grid_terrain[gridOffset] & Terrain_Rubble)
    {
        context.terrainType = 8;
    }
    else if (Data_Grid_terrain[gridOffset] & Terrain_Wall)
    {
        context.terrainType = 9;
    }
    else if (!context.buildingId)
    {
        context.terrainType = 10;
    }
    else
    {
        context.type = BuildingInfoType_Building;
        context.workerPercentage = calc_percentage(
                                       Data_Buildings[context.buildingId].numWorkers,
                                       model_get_building((building_type)Data_Buildings[context.buildingId].type)->laborers);
        switch (Data_Buildings[context.buildingId].type)
        {
        case BUILDING_FORT_GROUND:
            context.buildingId = Data_Buildings[context.buildingId].prevPartBuildingId;
        // fallthrough
        case BUILDING_FORT:
            context.formationId = Data_Buildings[context.buildingId].formationId;
            break;
        case BUILDING_WAREHOUSE_SPACE:
        case BUILDING_HIPPODROME:
            context.buildingId = Building_getMainBuildingId(context.buildingId);
            break;
        case BUILDING_BARRACKS:
            context.barracksSoldiersRequested = Formation_anyLegionNeedsSoldiers();
            context.barracksSoldiersRequested += Data_Buildings_Extra.barracksTowerSentryRequested;
            break;
        default:
            if (Data_Buildings[context.buildingId].houseSize)
            {
                UI_BuildingInfo_houseDetermineWorstDesirabilityBuilding(&context);
                HouseEvolution_determineEvolveText(context.buildingId, context.worstDesirabilityBuildingId);
            }
            break;
        }
        context.hasRoadAccess = 0;
        switch (Data_Buildings[context.buildingId].type)
        {
        case BUILDING_GRANARY:
            if (Terrain_hasRoadAccessGranary(
                        Data_Buildings[context.buildingId].x,
                        Data_Buildings[context.buildingId].y, 0, 0))
            {
                context.hasRoadAccess = 1;
            }
            break;
        case BUILDING_HIPPODROME:
            if (Terrain_hasRoadAccessHippodrome(
                        Data_Buildings[context.buildingId].x,
                        Data_Buildings[context.buildingId].y, 0, 0))
            {
                context.hasRoadAccess = 1;
            }
            break;
        case BUILDING_WAREHOUSE:
            if (Terrain_hasRoadAccess(
                        Data_Buildings[context.buildingId].x,
                        Data_Buildings[context.buildingId].y, 3, 0, 0))
            {
                context.hasRoadAccess = 1;
            }
            context.warehouseSpaceText = Resource_getWarehouseSpaceInfo(context.buildingId);
            break;
        default:
            if (Terrain_hasRoadAccess(
                        Data_Buildings[context.buildingId].x,
                        Data_Buildings[context.buildingId].y,
                        Data_Buildings[context.buildingId].size, 0, 0))
            {
                context.hasRoadAccess = 1;
            }
            break;
        }
    }
    // figures
    context.figure.selectedIndex = 0;
    context.figure.count = 0;
    for (int i = 0; i < 7; i++)
    {
        context.figure.figureIds[i] = 0;
    }
    static const int figureOffsets[] = {0, -162, 162, 1, -1, -163, -161, 161, 163};
    for (int i = 0; i < 9 && context.figure.count < 7; i++)
    {
        int figureId = Data_Grid_figureIds[gridOffset + figureOffsets[i]];
        while (figureId > 0 && context.figure.count < 7)
        {
            if (Data_Figures[figureId].state != FigureState_Dead &&
                    Data_Figures[figureId].actionState != FigureActionState_149_Corpse)
            {
                switch (Data_Figures[figureId].type)
                {
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
                    Figure_determinePhrase(figureId);
                    break;
                }
            }
            figureId = Data_Figures[figureId].nextFigureIdOnSameTile;
        }
    }
    // check for legion figures
    for (int i = 0; i < 7; i++)
    {
        int figureId = context.figure.figureIds[i];
        if (figureId <= 0)
        {
            continue;
        }
        int type = Data_Figures[figureId].type;
        if (type == FIGURE_FORT_STANDARD || FigureIsLegion(type))
        {
            context.type = BuildingInfoType_Legion;
            context.formationId = Data_Figures[figureId].formationId;
            const formation *m = formation_get(context.formationId);
            if (m->figure_type != FIGURE_FORT_LEGIONARY)
            {
                context.formationTypes = 5;
            }
            else if (m->has_military_training)
            {
                context.formationTypes = 4;
            }
            else
            {
                context.formationTypes = 3;
            }
            break;
        }
    }
    // dialog size
    context.xOffset = 8;
    context.yOffset = 32;
    context.widthBlocks = 29;
    switch (getHeightId())
    {
    case 1:
        context.heightBlocks = 16;
        break;
    case 2:
        context.heightBlocks = 18;
        break;
    case 3:
        context.heightBlocks = 19;
        break;
    case 4:
        context.heightBlocks = 14;
        break;
    default:
        context.heightBlocks = 22;
        break;
    }
    // dialog placement
    if (Data_Screen.height >= 600)
    {
        if (mouse_get()->y <= (Data_Screen.height - 24) / 2 + 24)
        {
            context.yOffset = Data_Screen.height - 16 * context.heightBlocks - 16;
        }
        else
        {
            context.yOffset = 32;
        }
    }
    int border = (Data_CityView.widthInPixels - 16 * context.widthBlocks) / 2;
    context.xOffset = Data_CityView.xOffsetInPixels + border;
}

int UI_BuildingInfo_getBuildingType()
{
    if (context.type == BuildingInfoType_Building)
    {
        return Data_Buildings[context.buildingId].type;
    }
    return BUILDING_WELL;
}

void UI_BuildingInfo_drawBackground()
{
    UI_City_drawBackground();
    UI_City_drawCity();//?do we want this?
    if (context.type == BuildingInfoType_None)
    {
        UI_BuildingInfo_drawNoPeople(&context);
    }
    else if (context.type == BuildingInfoType_Terrain)
    {
        UI_BuildingInfo_drawTerrain(&context);
    }
    else if (context.type == BuildingInfoType_Building)
    {
        int btype = Data_Buildings[context.buildingId].type;
        if (BuildingIsHouse(btype))
        {
            UI_BuildingInfo_drawHouse(&context);
        }
        else if (btype == BUILDING_WHEAT_FARM)
        {
            UI_BuildingInfo_drawWheatFarm(&context);
        }
        else if (btype == BUILDING_VEGETABLE_FARM)
        {
            UI_BuildingInfo_drawVegetableFarm(&context);
        }
        else if (btype == BUILDING_FRUIT_FARM)
        {
            UI_BuildingInfo_drawFruitFarm(&context);
        }
        else if (btype == BUILDING_OLIVE_FARM)
        {
            UI_BuildingInfo_drawOliveFarm(&context);
        }
        else if (btype == BUILDING_VINES_FARM)
        {
            UI_BuildingInfo_drawVinesFarm(&context);
        }
        else if (btype == BUILDING_PIG_FARM)
        {
            UI_BuildingInfo_drawPigFarm(&context);
        }
        else if (btype == BUILDING_MARBLE_QUARRY)
        {
            UI_BuildingInfo_drawMarbleQuarry(&context);
        }
        else if (btype == BUILDING_IRON_MINE)
        {
            UI_BuildingInfo_drawIronMine(&context);
        }
        else if (btype == BUILDING_TIMBER_YARD)
        {
            UI_BuildingInfo_drawTimberYard(&context);
        }
        else if (btype == BUILDING_CLAY_PIT)
        {
            UI_BuildingInfo_drawClayPit(&context);
        }
        else if (btype == BUILDING_WINE_WORKSHOP)
        {
            UI_BuildingInfo_drawWineWorkshop(&context);
        }
        else if (btype == BUILDING_OIL_WORKSHOP)
        {
            UI_BuildingInfo_drawOilWorkshop(&context);
        }
        else if (btype == BUILDING_WEAPONS_WORKSHOP)
        {
            UI_BuildingInfo_drawWeaponsWorkshop(&context);
        }
        else if (btype == BUILDING_FURNITURE_WORKSHOP)
        {
            UI_BuildingInfo_drawFurnitureWorkshop(&context);
        }
        else if (btype == BUILDING_POTTERY_WORKSHOP)
        {
            UI_BuildingInfo_drawPotteryWorkshop(&context);
        }
        else if (btype == BUILDING_MARKET)
        {
            UI_BuildingInfo_drawMarket(&context);
        }
        else if (btype == BUILDING_GRANARY)
        {
            if (context.storageShowSpecialOrders)
            {
                UI_BuildingInfo_drawGranaryOrders(&context);
            }
            else
            {
                UI_BuildingInfo_drawGranary(&context);
            }
        }
        else if (btype == BUILDING_WAREHOUSE)
        {
            if (context.storageShowSpecialOrders)
            {
                UI_BuildingInfo_drawWarehouseOrders(&context);
            }
            else
            {
                UI_BuildingInfo_drawWarehouse(&context);
            }
        }
        else if (btype == BUILDING_AMPHITHEATER)
        {
            UI_BuildingInfo_drawAmphitheater(&context);
        }
        else if (btype == BUILDING_THEATER)
        {
            UI_BuildingInfo_drawTheater(&context);
        }
        else if (btype == BUILDING_HIPPODROME)
        {
            UI_BuildingInfo_drawHippodrome(&context);
        }
        else if (btype == BUILDING_COLOSSEUM)
        {
            UI_BuildingInfo_drawColosseum(&context);
        }
        else if (btype == BUILDING_GLADIATOR_SCHOOL)
        {
            UI_BuildingInfo_drawGladiatorSchool(&context);
        }
        else if (btype == BUILDING_LION_HOUSE)
        {
            UI_BuildingInfo_drawLionHouse(&context);
        }
        else if (btype == BUILDING_ACTOR_COLONY)
        {
            UI_BuildingInfo_drawActorColony(&context);
        }
        else if (btype == BUILDING_CHARIOT_MAKER)
        {
            UI_BuildingInfo_drawChariotMaker(&context);
        }
        else if (btype == BUILDING_DOCTOR)
        {
            UI_BuildingInfo_drawClinic(&context);
        }
        else if (btype == BUILDING_HOSPITAL)
        {
            UI_BuildingInfo_drawHospital(&context);
        }
        else if (btype == BUILDING_BATHHOUSE)
        {
            UI_BuildingInfo_drawBathhouse(&context);
        }
        else if (btype == BUILDING_BARBER)
        {
            UI_BuildingInfo_drawBarber(&context);
        }
        else if (btype == BUILDING_SCHOOL)
        {
            UI_BuildingInfo_drawSchool(&context);
        }
        else if (btype == BUILDING_ACADEMY)
        {
            UI_BuildingInfo_drawAcademy(&context);
        }
        else if (btype == BUILDING_LIBRARY)
        {
            UI_BuildingInfo_drawLibrary(&context);
        }
        else if (btype == BUILDING_SMALL_TEMPLE_CERES || btype == BUILDING_LARGE_TEMPLE_CERES)
        {
            UI_BuildingInfo_drawTempleCeres(&context);
        }
        else if (btype == BUILDING_SMALL_TEMPLE_NEPTUNE || btype == BUILDING_LARGE_TEMPLE_NEPTUNE)
        {
            UI_BuildingInfo_drawTempleNeptune(&context);
        }
        else if (btype == BUILDING_SMALL_TEMPLE_MERCURY || btype == BUILDING_LARGE_TEMPLE_MERCURY)
        {
            UI_BuildingInfo_drawTempleMercury(&context);
        }
        else if (btype == BUILDING_SMALL_TEMPLE_MARS || btype == BUILDING_LARGE_TEMPLE_MARS)
        {
            UI_BuildingInfo_drawTempleMars(&context);
        }
        else if (btype == BUILDING_SMALL_TEMPLE_VENUS || btype == BUILDING_LARGE_TEMPLE_VENUS)
        {
            UI_BuildingInfo_drawTempleVenus(&context);
        }
        else if (btype == BUILDING_ORACLE)
        {
            UI_BuildingInfo_drawOracle(&context);
        }
        else if (btype == BUILDING_GOVERNORS_HOUSE || btype == BUILDING_GOVERNORS_VILLA || btype == BUILDING_GOVERNORS_PALACE)
        {
            UI_BuildingInfo_drawGovernorsHome(&context);
        }
        else if (btype == BUILDING_FORUM || btype == BUILDING_FORUM_UPGRADED)
        {
            UI_BuildingInfo_drawForum(&context);
        }
        else if (btype == BUILDING_SENATE || btype == BUILDING_SENATE_UPGRADED)
        {
            UI_BuildingInfo_drawSenate(&context);
        }
        else if (btype == BUILDING_ENGINEERS_POST)
        {
            UI_BuildingInfo_drawEngineersPost(&context);
        }
        else if (btype == BUILDING_SHIPYARD)
        {
            UI_BuildingInfo_drawShipyard(&context);
        }
        else if (btype == BUILDING_DOCK)
        {
            UI_BuildingInfo_drawDock(&context);
        }
        else if (btype == BUILDING_WHARF)
        {
            UI_BuildingInfo_drawWharf(&context);
        }
        else if (btype == BUILDING_RESERVOIR)
        {
            UI_BuildingInfo_drawReservoir(&context);
        }
        else if (btype == BUILDING_FOUNTAIN)
        {
            UI_BuildingInfo_drawFountain(&context);
        }
        else if (btype == BUILDING_WELL)
        {
            UI_BuildingInfo_drawWell(&context);
        }
        else if (btype == BUILDING_SMALL_STATUE || btype == BUILDING_MEDIUM_STATUE || btype == BUILDING_LARGE_STATUE)
        {
            UI_BuildingInfo_drawStatue(&context);
        }
        else if (btype == BUILDING_TRIUMPHAL_ARCH)
        {
            UI_BuildingInfo_drawTriumphalArch(&context);
        }
        else if (btype == BUILDING_PREFECTURE)
        {
            UI_BuildingInfo_drawPrefect(&context);
        }
        else if (btype == BUILDING_GATEHOUSE)
        {
            UI_BuildingInfo_drawGatehouse(&context);
        }
        else if (btype == BUILDING_TOWER)
        {
            UI_BuildingInfo_drawTower(&context);
        }
        else if (btype == BUILDING_MILITARY_ACADEMY)
        {
            UI_BuildingInfo_drawMilitaryAcademy(&context);
        }
        else if (btype == BUILDING_BARRACKS)
        {
            UI_BuildingInfo_drawBarracks(&context);
        }
        else if (btype == BUILDING_FORT)
        {
            UI_BuildingInfo_drawFort(&context);
        }
        else if (btype == BUILDING_BURNING_RUIN)
        {
            UI_BuildingInfo_drawBurningRuin(&context);
        }
        else if (btype == BUILDING_NATIVE_HUT)
        {
            UI_BuildingInfo_drawNativeHut(&context);
        }
        else if (btype == BUILDING_NATIVE_MEETING)
        {
            UI_BuildingInfo_drawNativeMeeting(&context);
        }
        else if (btype == BUILDING_NATIVE_CROPS)
        {
            UI_BuildingInfo_drawNativeCrops(&context);
        }
        else if (btype == BUILDING_MISSION_POST)
        {
            UI_BuildingInfo_drawMissionPost(&context);
        }
    }
    else if (context.type == BuildingInfoType_Legion)
    {
        UI_BuildingInfo_drawLegionInfo(&context);
    }
}

void UI_BuildingInfo_drawForeground()
{
    // building-specific buttons
    if (context.type == BuildingInfoType_Building)
    {
        int btype = Data_Buildings[context.buildingId].type;
        if (btype == BUILDING_GRANARY)
        {
            if (context.storageShowSpecialOrders)
            {
                UI_BuildingInfo_drawGranaryOrdersForeground(&context);
            }
            else
            {
                UI_BuildingInfo_drawGranaryForeground(&context);
            }
        }
        else if (btype == BUILDING_WAREHOUSE)
        {
            if (context.storageShowSpecialOrders)
            {
                UI_BuildingInfo_drawWarehouseOrdersForeground(&context);
            }
            else
            {
                UI_BuildingInfo_drawWarehouseForeground(&context);
            }
        }
    }
    else if (context.type == BuildingInfoType_Legion)
    {
        UI_BuildingInfo_drawLegionInfoForeground(&context);
    }
    // general buttons
    if (context.storageShowSpecialOrders)
    {
        Widget::Button::drawImageButtons(context.xOffset, 432,
                                         imageButtonsHelpExit, 2);
    }
    else
    {
        Widget::Button::drawImageButtons(
            context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
            imageButtonsHelpExit, 2);
    }
    if (context.advisor)
    {
        Widget::Button::drawImageButtons(
            context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
            imageButtonsAdvisor, 1);
    }
}

void UI_BuildingInfo_handleMouse(const mouse *m)
{
    if (m->right.went_up)
    {
        UI_Window_goTo(Window_City);
        return;
    }
    // general buttons
    if (context.storageShowSpecialOrders)
    {
        Widget::Button::handleImageButtons(context.xOffset, 432,
                                           imageButtonsHelpExit, 2, &focusImageButtonId);
    }
    else
    {
        Widget::Button::handleImageButtons(
            context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
            imageButtonsHelpExit, 2, &focusImageButtonId);
    }
    if (context.advisor)
    {
        Widget::Button::handleImageButtons(
            context.xOffset, context.yOffset + 16 * context.heightBlocks - 40,
            imageButtonsAdvisor, 1, 0);
    }
    // building-specific buttons
    if (context.type == BuildingInfoType_None)
    {
        return;
    }
    if (context.type == BuildingInfoType_Legion)
    {
        UI_BuildingInfo_handleMouseLegionInfo(&context);
    }
    else if (context.figure.drawn)
    {
        UI_BuildingInfo_handleMouseFigureList(&context);
    }
    else if (context.type == BuildingInfoType_Building)
    {
        int btype = Data_Buildings[context.buildingId].type;
        if (btype == BUILDING_GRANARY)
        {
            if (context.storageShowSpecialOrders)
            {
                UI_BuildingInfo_handleMouseGranaryOrders(&context);
            }
            else
            {
                UI_BuildingInfo_handleMouseGranary(&context);
            }
        }
        else if (btype == BUILDING_WAREHOUSE)
        {
            if (context.storageShowSpecialOrders)
            {
                UI_BuildingInfo_handleMouseWarehouseOrders(&context);
            }
            else
            {
                UI_BuildingInfo_handleMouseWarehouse(&context);
            }
        }
    }
}

void UI_BuildingInfo_getTooltip(struct TooltipContext *c)
{
    int textId = 0;
    if (focusImageButtonId)
    {
        textId = focusImageButtonId;
    }
    else if (context.type == BuildingInfoType_Legion)
    {
        textId = UI_BuildingInfo_getTooltipLegionInfo(&context);
    }
    if (textId)
    {
        c->type = TooltipType_Button;
        c->textId = textId;
    }
}

void UI_BuildingInfo_showStorageOrders(int param1, int param2)
{
    context.storageShowSpecialOrders = 1;
    UI_Window_requestRefresh();
}

void UI_BuildingInfo_drawEmploymentInfo(BuildingInfoContext *c, int yOffset)
{
    struct Data_Building *b = &Data_Buildings[c->buildingId];
    int textId;
    if (b->numWorkers >= model_get_building(b->type)->laborers)
    {
        textId = 0;
    }
    else if (Data_CityInfo.population <= 0)
    {
        textId = 16; // no people in city
    }
    else if (b->housesCovered <= 0)
    {
        textId = 17; // no employees nearby
    }
    else if (b->housesCovered < 40)
    {
        textId = 20; // poor access to employees
    }
    else if (Data_CityInfo.laborCategory[b->laborCategory].workersAllocated <= 0)
    {
        textId = 18; // no people allocated
    }
    else
    {
        textId = 19; // too few people allocated
    }
    if (!textId && b->housesCovered < 40)
    {
        textId = 20; // poor access to employees
    }
    Graphics_drawImage(image_group(ID_Graphic_ContextIcons) + 14,
                       c->xOffset + 40, yOffset + 6);
    if (textId)
    {
        int width = Widget_GameText_drawNumberWithDescription(8, 12, b->numWorkers,
                    c->xOffset + 60, yOffset + 10, FONT_SMALL_BLACK);
        width += Widget::Text::drawNumber(model_get_building(b->type)->laborers, '(', "",
                                          c->xOffset + 70 + width, yOffset + 10, FONT_SMALL_BLACK);
        Widget_GameText_draw(69, 0, c->xOffset + 70 + width, yOffset + 10, FONT_SMALL_BLACK);
        Widget_GameText_draw(69, textId, c->xOffset + 70, yOffset + 26, FONT_SMALL_BLACK);
    }
    else
    {
        int width = Widget_GameText_drawNumberWithDescription(8, 12, b->numWorkers,
                    c->xOffset + 60, yOffset + 16, FONT_SMALL_BLACK);
        width += Widget::Text::drawNumber(model_get_building(b->type)->laborers, '(', "",
                                          c->xOffset + 70 + width, yOffset + 16, FONT_SMALL_BLACK);
        Widget_GameText_draw(69, 0, c->xOffset + 70 + width, yOffset + 16, FONT_SMALL_BLACK);
    }
}

static void buttonHelp(int param1, int param2)
{
    if (context.helpId > 0)
    {
        UI_MessageDialog_show(context.helpId, 0);
    }
    else
    {
        UI_MessageDialog_show(10, 0);
    }
    UI_Window_requestRefresh();
}

static void buttonExit(int param1, int param2)
{
    if (context.storageShowSpecialOrders)
    {
        context.storageShowSpecialOrders = 0;
        UI_Window_requestRefresh();
    }
    else
    {
        UI_Window_goTo(Window_City);
    }
}

static void buttonAdvisor(int param1, int param2)
{
    UI_Advisors_goToFromMessage(Advisor_Ratings);
}
