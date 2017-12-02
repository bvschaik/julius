#include "core/calc.h"
#include "graphics.h"
#include "terrain.h"

#include <data>
#include <ui>
#include <scenario>
#include <game>

#include "building/count.h"
#include "building/model.h"
#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "empire/city.h"
#include "graphics/image.h"

#include <string.h>

static void checkRoadAccess(int buildingType, int x, int y, int size);
static void checkWorkers(int buildingType);
static void checkMarket(int buildingType);
static void checkBarracks(int buildingType);
static void checkWall(int buildingType, int x, int y, int size);
static void checkWater(int buildingType, int x, int y);

static void checkActorAccess(int buildingType);
static void checkGladiatorAccess(int buildingType);
static void checkLionAccess(int buildingType);
static void checkCharioteerAccess(int buildingType);

static void checkIronAccess(int buildingType);
static void checkVinesAccess(int buildingType);
static void checkOlivesAccess(int buildingType);
static void checkTimberAccess(int buildingType);
static void checkClayAccess(int buildingType);
static void checkWeaponsAccess(int buildingType);

static struct
{
    int center;
    int top;
} offsets[] =
{
    {300, 30}, {300, 55}, {300, 80}, {300, 105}, {300, 130}
};

static struct
{
    short warningId;
    short center;
    short topOffset;
    short inUse;
    short textLength;
    short boxWidth;
    time_millis time;
    char text[100];
} warnings[5];

static int numWarnings;

static int hasWarningAlready;

void UI_Warning_draw()
{
    if (UI_Window_getId() != Window_City)
    {
        UI_Warning_clearAll();
        numWarnings = 0;
        return;
    }

    numWarnings = 0;
    for (int i = 0; i < 5; i++)
    {
        if (!warnings[i].inUse)
        {
            continue;
        }
        numWarnings++;
        int topOffset = warnings[i].topOffset;
        if (Data_State.gamePaused)
        {
            topOffset += 70;
        }
        int center = warnings[i].center;
        Widget::Panel::drawSmallLabelButton(
            center - warnings[i].boxWidth / 2 + 1, topOffset,
            warnings[i].boxWidth / 16 + 1, 1);
        if (warnings[i].boxWidth < 460)
        {
            // ornaments at the side
            Graphics_drawImage(image_group(GROUP_CONTEXT_ICONS) + 15,
                               center - warnings[i].boxWidth / 2 + 2, topOffset + 2);
            Graphics_drawImage(image_group(GROUP_CONTEXT_ICONS) + 15,
                               center + warnings[i].boxWidth / 2 - 30, topOffset + 2);
        }
        Widget::Text::drawCentered(warnings[i].text,
                                   center - warnings[i].boxWidth / 2 + 1, topOffset + 4,
                                   warnings[i].boxWidth, FONT_NORMAL_WHITE, 0);

        // clear after 15 seconds
        if (time_get_millis() > warnings[i].time + 15000)
        {
            warnings[i].inUse = 0;
        }
    }
}

void UI_Warning_show(int warningId)
{
    if (!setting_warnings())
    {
        return;
    }
    for (int i = 0; i < 5; i++)
    {
        if (warnings[i].inUse)
        {
            continue;
        }
        warnings[i].inUse = 1;
        const char *text;
        if (warningId == Warning_Orientation)
        {
            text = lang_get_string(17, Data_State.map.orientation);
        }
        else
        {
            text = lang_get_string(19, warningId - 2);
        }
        hasWarningAlready = 1;
        warnings[i].warningId = warningId;
        int width = Widget::Text::getWidth(text, FONT_NORMAL_BLACK);
        if (width <= 100)
        {
            warnings[i].boxWidth = 200;
        }
        else if (width <= 200)
        {
            warnings[i].boxWidth = 300;
        }
        else if (width <= 300)
        {
            warnings[i].boxWidth = 400;
        }
        else
        {
            warnings[i].boxWidth = 460;
        }
        warnings[i].time = time_get_millis();
        warnings[i].textLength = strlen(text);
        memset(warnings[i].text, 0, 100);
        string_copy(text, warnings[i].text, warnings[i].textLength);
        return;
    }
}

int UI_Warning_hasWarnings()
{
    return numWarnings > 0;
}

void UI_Warning_clearAll()
{
    for (int i = 0; i < 5; i++)
    {
        warnings[i].center = offsets[i].center = (Data_Screen.width - 180) / 2;
        warnings[i].topOffset = offsets[i].top;
        warnings[i].inUse = 0;
    }
}

void UI_Warning_clearHasWarningFlag()
{
    hasWarningAlready = 0;
}

//reviewed
void UI_Warning_checkNewBuilding(int buildingType, int x, int y, int size)
{
    UI_Warning_checkFoodStocks(buildingType);
    checkWorkers(buildingType);
    checkMarket(buildingType);
    checkActorAccess(buildingType);
    checkGladiatorAccess(buildingType);
    checkLionAccess(buildingType);
    checkCharioteerAccess(buildingType);

    checkBarracks(buildingType);
    checkWeaponsAccess(buildingType);

    checkWall(buildingType, x, y, size);
    checkWater(buildingType, x, y);

    checkIronAccess(buildingType);
    checkVinesAccess(buildingType);
    checkOlivesAccess(buildingType);
    checkTimberAccess(buildingType);
    checkClayAccess(buildingType);

    checkRoadAccess(buildingType, x, y, size);
}

void UI_Warning_checkFoodStocks(int buildingType)
{
    if (!hasWarningAlready && buildingType == BUILDING_HOUSE_VACANT_LOT)
    {
        if (Data_CityInfo.population >= 200 && !scenario_property_climate())
        {
            if (calc_percentage(Data_CityInfo.foodInfoFoodStoredLastMonth,
                                Data_CityInfo.foodInfoFoodConsumedLastMonth) <= 95)
            {
                UI_Warning_show(Warning_MoreFoodNeeded);
            }
        }
    }
}

void UI_Warning_checkReservoirWater(int buildingType)
{
    if (!hasWarningAlready && buildingType == BUILDING_RESERVOIR)
    {
        if (building_count_active(BUILDING_RESERVOIR))
        {
            UI_Warning_show(Warning_ConnectToReservoir);
        }
        else
        {
            UI_Warning_show(Warning_PlaceReservoirNextToWater);
        }
    }
}

static void checkRoadAccess(int buildingType, int x, int y, int size)
{
    switch (buildingType)
    {
    case BUILDING_SMALL_STATUE:
    case BUILDING_MEDIUM_STATUE:
    case BUILDING_LARGE_STATUE:
    case BUILDING_FOUNTAIN:
    case BUILDING_WELL:
    case BUILDING_RESERVOIR:
    case BUILDING_GATEHOUSE:
    case BUILDING_TRIUMPHAL_ARCH:
    case BUILDING_HOUSE_VACANT_LOT:
    case BUILDING_FORT:
    case BUILDING_FORT_LEGIONARIES:
    case BUILDING_FORT_JAVELIN:
    case BUILDING_FORT_MOUNTED:
        return;
    }

    int hasRoad = 0;
    if (Terrain_hasRoadAccess(x, y, size, 0, 0))
    {
        hasRoad = 1;
    }
    else if (buildingType == BUILDING_WAREHOUSE && Terrain_hasRoadAccess(x, y, size, 0, 0))
    {
        hasRoad = 1;
    }
    else if (buildingType == BUILDING_HIPPODROME && Terrain_hasRoadAccessHippodrome(x, y, 0, 0))
    {
        hasRoad = 1;
    }
    if (!hasRoad)
    {
        UI_Warning_show(Warning_RoadAccessNeeded);
    }
}

static void checkWater(int buildingType, int x, int y)
{
    if (!hasWarningAlready)
    {
        if (buildingType == BUILDING_FOUNTAIN || buildingType == BUILDING_BATHHOUSE)
        {
            int gridOffset = Data_State.map.gridStartOffset + GRID_SIZE * y + x;
            int hasWater = 0;
            if (Data_Grid_terrain[gridOffset] & Terrain_ReservoirRange)
            {
                hasWater = 1;
            }
            else if (buildingType == BUILDING_BATHHOUSE)
            {
                if (Data_Grid_terrain[gridOffset + 1] & Terrain_ReservoirRange)
                {
                    hasWater = 1;
                }
                else if (Data_Grid_terrain[gridOffset + GRID_SIZE] & Terrain_ReservoirRange)
                {
                    hasWater = 1;
                }
                else if (Data_Grid_terrain[gridOffset + GRID_SIZE + 1] & Terrain_ReservoirRange)
                {
                    hasWater = 1;
                }
            }
            if (!hasWater)
            {
                UI_Warning_show(Warning_WaterPipeAccessNeeded);
            }
        }
    }
}

static void checkWorkers(int buildingType)
{
    if (!hasWarningAlready && buildingType != BUILDING_WELL)
    {
        if (model_get_building((building_type)buildingType)->laborers > 0 &&
                Data_CityInfo.workersNeeded >= 10)
        {
            UI_Warning_show(Warning_WorkersNeeded);
        }
    }
}

static void checkMarket(int buildingType)
{
    if (!hasWarningAlready && buildingType == BUILDING_GRANARY)
    {
        if (building_count_active(BUILDING_MARKET) <= 0)
        {
            UI_Warning_show(Warning_BuildMarket);
        }
    }
}

static void checkBarracks(int buildingType)
{
    if (!hasWarningAlready)
    {
        if (buildingType == BUILDING_FORT_JAVELIN ||
                buildingType == BUILDING_FORT_LEGIONARIES ||
                buildingType == BUILDING_FORT_MOUNTED)
        {
            if (building_count_active(BUILDING_BARRACKS) <= 0)
            {
                UI_Warning_show(Warning_BuildBarracks);
            }
        }
    }
}

static void checkWeaponsAccess(int buildingType)
{
    if (!hasWarningAlready && buildingType == BUILDING_BARRACKS)
    {
        if (Data_CityInfo.resourceStored[RESOURCE_WEAPONS] <= 0)
        {
            UI_Warning_show(Warning_WeaponsNeeded);
        }
    }
}

static void checkWall(int buildingType, int x, int y, int size)
{
    if (!hasWarningAlready && buildingType == BUILDING_TOWER)
    {
        if (!Terrain_isAdjacentToWall(x, y, size))
        {
            UI_Warning_show(Warning_SentriesNeedWall);
        }
    }
}

static void checkActorAccess(int buildingType)
{
    if (!hasWarningAlready && buildingType == BUILDING_THEATER)
    {
        if (building_count_active(BUILDING_ACTOR_COLONY) <= 0)
        {
            UI_Warning_show(Warning_BuildActorColony);
        }
    }
}

static void checkGladiatorAccess(int buildingType)
{
    if (!hasWarningAlready && buildingType == BUILDING_AMPHITHEATER)
    {
        if (building_count_active(BUILDING_GLADIATOR_SCHOOL) <= 0)
        {
            UI_Warning_show(Warning_BuildGladiatorSchool);
        }
    }
}

static void checkLionAccess(int buildingType)
{
    if (!hasWarningAlready && buildingType == BUILDING_COLOSSEUM)
    {
        if (building_count_active(BUILDING_LION_HOUSE) <= 0)
        {
            UI_Warning_show(Warning_BuildLionHouse);
        }
    }
}

static void checkCharioteerAccess(int buildingType)
{
    if (!hasWarningAlready && buildingType == BUILDING_HIPPODROME)
    {
        if (building_count_active(BUILDING_CHARIOT_MAKER) <= 0)
        {
            UI_Warning_show(Warning_BuildChariotMaker);
        }
    }
}

static void checkIronAccess(int buildingType)
{
    if (buildingType == BUILDING_WEAPONS_WORKSHOP &&
            building_count_industry_active(RESOURCE_IRON) <= 0)
    {
        if (Data_CityInfo.resourceStored[RESOURCE_WEAPONS] <= 0 &&
                Data_CityInfo.resourceStored[RESOURCE_IRON] <= 0)
        {
            UI_Warning_show(Warning_IronNeeded);
            if (empire_can_produce_resource(RESOURCE_IRON))
            {
                UI_Warning_show(Warning_BuildIronMine);
            }
            else if (!empire_can_import_resource(RESOURCE_IRON))
            {
                UI_Warning_show(Warning_OpenTradeToImport);
            }
            else if (Data_CityInfo.resourceTradeStatus[RESOURCE_IRON] != TradeStatus_Import)
            {
                UI_Warning_show(Warning_TradeImportResource);
            }
        }
    }
}

static void checkVinesAccess(int buildingType)
{
    if (buildingType == BUILDING_WINE_WORKSHOP &&
            building_count_industry_active(RESOURCE_VINES) <= 0)
    {
        if (Data_CityInfo.resourceStored[RESOURCE_WINE] <= 0 &&
                Data_CityInfo.resourceStored[RESOURCE_VINES] <= 0)
        {
            UI_Warning_show(Warning_VinesNeeded);
            if (empire_can_produce_resource(RESOURCE_VINES))
            {
                UI_Warning_show(Warning_BuildVinesFarm);
            }
            else if (!empire_can_import_resource(RESOURCE_VINES))
            {
                UI_Warning_show(Warning_OpenTradeToImport);
            }
            else if (Data_CityInfo.resourceTradeStatus[RESOURCE_VINES] != TradeStatus_Import)
            {
                UI_Warning_show(Warning_TradeImportResource);
            }
        }
    }
}

static void checkOlivesAccess(int buildingType)
{
    if (buildingType == BUILDING_OIL_WORKSHOP &&
            building_count_industry_active(RESOURCE_OLIVES) <= 0)
    {
        if (Data_CityInfo.resourceStored[RESOURCE_OIL] <= 0 &&
                Data_CityInfo.resourceStored[RESOURCE_OLIVES] <= 0)
        {
            UI_Warning_show(Warning_OlivesNeeded);
            if (empire_can_produce_resource(RESOURCE_OLIVES))
            {
                UI_Warning_show(Warning_BuildOliveFarm);
            }
            else if (!empire_can_import_resource(RESOURCE_OLIVES))
            {
                UI_Warning_show(Warning_OpenTradeToImport);
            }
            else if (Data_CityInfo.resourceTradeStatus[RESOURCE_OLIVES] != TradeStatus_Import)
            {
                UI_Warning_show(Warning_TradeImportResource);
            }
        }
    }
}

static void checkTimberAccess(int buildingType)
{
    if (buildingType == BUILDING_FURNITURE_WORKSHOP &&
            building_count_industry_active(RESOURCE_TIMBER) <= 0)
    {
        if (Data_CityInfo.resourceStored[RESOURCE_FURNITURE] <= 0 &&
                Data_CityInfo.resourceStored[RESOURCE_TIMBER] <= 0)
        {
            UI_Warning_show(Warning_TimberNeeded);
            if (empire_can_produce_resource(RESOURCE_TIMBER))
            {
                UI_Warning_show(Warning_BuildTimberYard);
            }
            else if (!empire_can_import_resource(RESOURCE_TIMBER))
            {
                UI_Warning_show(Warning_OpenTradeToImport);
            }
            else if (Data_CityInfo.resourceTradeStatus[RESOURCE_TIMBER] != TradeStatus_Import)
            {
                UI_Warning_show(Warning_TradeImportResource);
            }
        }
    }
}

static void checkClayAccess(int buildingType)
{
    if (buildingType == BUILDING_POTTERY_WORKSHOP &&
            building_count_industry_active(RESOURCE_CLAY) <= 0)
    {
        if (Data_CityInfo.resourceStored[RESOURCE_POTTERY] <= 0 &&
                Data_CityInfo.resourceStored[RESOURCE_CLAY] <= 0)
        {
            UI_Warning_show(Warning_ClayNeeded);
            if (empire_can_produce_resource(RESOURCE_CLAY))
            {
                UI_Warning_show(Warning_BuildClayPit);
            }
            else if (!empire_can_import_resource(RESOURCE_CLAY))
            {
                UI_Warning_show(Warning_OpenTradeToImport);
            }
            else if (Data_CityInfo.resourceTradeStatus[RESOURCE_CLAY] != TradeStatus_Import)
            {
                UI_Warning_show(Warning_TradeImportResource);
            }
        }
    }
}
