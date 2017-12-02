#include "figureaction_private.h"

#include "figure.h"
#include "city/message.h"
#include "terrain.h"

#include <data>
#include <scenario>
#include <game>

#include "city/message.h"
#include "building/model.h"
#include "core/calc.h"
#include "core/random.h"

static const int flotsamType0[] = {0, 1, 2, 3, 4, 4, 4, 3, 2, 1, 0, 0};
static const int flotsamType12[] =
{
    0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 3, 2, 1, 0, 0, 1, 1, 2, 2, 1, 1, 0, 0, 0
};
static const int flotsamType3[] =
{
    0, 0, 1, 1, 2, 2, 3, 3, 4, 4, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

void FigureAction_fishingBoat(int figureId)
{
    struct Data_Figure *f = &Data_Figures[figureId];
    struct Data_Building *b = &Data_Buildings[f->buildingId];
    if (!BuildingIsInUse(f->buildingId))
    {
        f->state = FigureState_Dead;
    }
    if (f->actionState != FigureActionState_190_FishingBoatCreated && b->data.other.boatFigureId != figureId)
    {
        int xTile, yTile;
        int buildingId = Terrain_Water_getWharfTileForNewFishingBoat(figureId, &xTile, &yTile);
        b = &Data_Buildings[buildingId];
        if (buildingId)
        {
            f->buildingId = buildingId;
            b->data.other.boatFigureId = figureId;
            f->actionState = FigureActionState_193_FishingBoatSailingToWharf;
            f->destinationX = xTile;
            f->destinationY = yTile;
            f->sourceX = xTile;
            f->sourceY = yTile;
            figure_route_remove(figureId);
        }
        else
        {
            f->state = FigureState_Dead;
        }
    }
    f->isGhost = 0;
    f->isBoat = 1;
    FigureActionIncreaseGraphicOffset(f, 12);
    f->cartGraphicId = 0;
    switch (f->actionState)
    {
    case FigureActionState_190_FishingBoatCreated:
        f->waitTicks++;
        if (f->waitTicks >= 50)
        {
            f->waitTicks = 0;
            int xTile, yTile;
            int buildingId = Terrain_Water_getWharfTileForNewFishingBoat(figureId, &xTile, &yTile);
            if (buildingId)
            {
                b->figureId = 0; // remove from original building
                f->buildingId = buildingId;
                Data_Buildings[buildingId].data.other.boatFigureId = figureId;
                f->actionState = FigureActionState_193_FishingBoatSailingToWharf;
                f->destinationX = xTile;
                f->destinationY = yTile;
                f->sourceX = xTile;
                f->sourceY = yTile;
                figure_route_remove(figureId);
            }
        }
        break;
    case FigureActionState_191_FishingBoatGoingToFish:
        FigureMovement_walkTicks(figureId, 1);
        f->heightAdjustedTicks = 0;
        if (f->direction == DIR_FIGURE_AT_DESTINATION)
        {
            int xTile, yTile;
            if (Terrain_Water_findAlternativeTileForFishingBoat(figureId, &xTile, &yTile))
            {
                figure_route_remove(figureId);
                f->destinationX = xTile;
                f->destinationY = yTile;
                f->direction = f->previousTileDirection;
            }
            else
            {
                f->actionState = FigureActionState_192_FishingBoatFishing;
                f->waitTicks = 0;
            }
        }
        else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST)
        {
            f->actionState = FigureActionState_194_FishingBoatAtWharf;
            f->destinationX = f->sourceX;
            f->destinationY = f->sourceY;
        }
        break;
    case FigureActionState_192_FishingBoatFishing:
        f->waitTicks++;
        if (f->waitTicks >= 200)
        {
            f->waitTicks = 0;
            f->actionState = FigureActionState_195_FishingBoatReturningWithFish;
            f->destinationX = f->sourceX;
            f->destinationY = f->sourceY;
            figure_route_remove(figureId);
        }
        break;
    case FigureActionState_193_FishingBoatSailingToWharf:
        FigureMovement_walkTicks(figureId, 1);
        f->heightAdjustedTicks = 0;
        if (f->direction == DIR_FIGURE_AT_DESTINATION)
        {
            f->actionState = FigureActionState_194_FishingBoatAtWharf;
            f->waitTicks = 0;
        }
        else if (f->direction == DIR_FIGURE_REROUTE)
        {
            figure_route_remove(figureId);
        }
        else if (f->direction == DIR_FIGURE_LOST)
        {
            // cannot reach grounds
            city_message_post_with_message_delay(MESSAGE_CAT_FISHING_BLOCKED, 1, MESSAGE_FISHING_BOAT_BLOCKED, 12);
            f->state = FigureState_Dead;
        }
        break;
    case FigureActionState_194_FishingBoatAtWharf:
    {
        int pctWorkers = calc_percentage(b->numWorkers, model_get_building((building_type)b->type)->laborers);
        int maxWaitTicks = 5 * (102 - pctWorkers);
        if (b->data.other.fishingBoatHasFish > 0)
        {
            pctWorkers = 0;
        }
        if (pctWorkers > 0)
        {
            f->waitTicks++;
            if (f->waitTicks >= maxWaitTicks)
            {
                f->waitTicks = 0;
                int xTile, yTile;
                if (Terrain_Water_getNearestFishTile(figureId, &xTile, &yTile))
                {
                    f->actionState = FigureActionState_191_FishingBoatGoingToFish;
                    f->destinationX = xTile;
                    f->destinationY = yTile;
                    figure_route_remove(figureId);
                }
            }
        }
    }
    break;
    case FigureActionState_195_FishingBoatReturningWithFish:
        FigureMovement_walkTicks(figureId, 1);
        f->heightAdjustedTicks = 0;
        if (f->direction == DIR_FIGURE_AT_DESTINATION)
        {
            f->actionState = FigureActionState_194_FishingBoatAtWharf;
            f->waitTicks = 0;
            b->figureSpawnDelay = 1;
            b->data.other.fishingBoatHasFish++;
        }
        else if (f->direction == DIR_FIGURE_REROUTE)
        {
            figure_route_remove(figureId);
        }
        else if (f->direction == DIR_FIGURE_LOST)
        {
            f->state = FigureState_Dead;
        }
        break;
    }
    int dir = (f->direction < 8) ? f->direction : f->previousTileDirection;
    FigureActionNormalizeDirection(dir);

    if (f->actionState == FigureActionState_192_FishingBoatFishing)
    {
        f->graphicId = image_group(GROUP_FIGURE_SHIP) + dir + 16;
    }
    else
    {
        f->graphicId = image_group(GROUP_FIGURE_SHIP) + dir + 8;
    }
}

void FigureAction_flotsam(int figureId)
{
    struct Data_Figure *f = &Data_Figures[figureId];
    f->isBoat = 2;
    if (!scenario_map_has_river_exit())
    {
        return;
    }
    f->isGhost = 0;
    f->cartGraphicId = 0;
    f->terrainUsage = FigureTerrainUsage_Any;
    switch (f->actionState)
    {
    case FigureActionState_128_FlotsamCreated:
        f->isGhost = 1;
        f->waitTicks--;
        if (f->waitTicks <= 0)
        {
            f->actionState = FigureActionState_129_FlotsamFloating;
            f->waitTicks = 0;
            if (Data_CityInfo.godCurseNeptuneSankShips && !f->resourceId)
            {
                f->minMaxSeen = 1;
                Data_CityInfo.godCurseNeptuneSankShips = 0;
            }
            map_point river_exit = scenario_map_river_exit();
            f->destinationX = river_exit.x;
            f->destinationY = river_exit.y;
        }
        break;
    case FigureActionState_129_FlotsamFloating:
        if (f->flotsamVisible)
        {
            f->flotsamVisible = 0;
        }
        else
        {
            f->flotsamVisible = 1;
            f->waitTicks++;
            FigureMovement_walkTicks(figureId, 1);
            f->isGhost = 0;
            f->heightAdjustedTicks = 0;
            if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                    f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST)
            {
                f->actionState = FigureActionState_130_FlotsamLeftMap;
            }
        }
        break;
    case FigureActionState_130_FlotsamLeftMap:
        f->isGhost = 1;
        f->minMaxSeen = 0;
        f->actionState = FigureActionState_128_FlotsamCreated;
        if (f->waitTicks >= 400)
        {
            f->waitTicks = random_byte() & 7;
        }
        else if (f->waitTicks >= 200)
        {
            f->waitTicks = 50 + (random_byte() & 0xf);
        }
        else if (f->waitTicks >= 100)
        {
            f->waitTicks = 100 + (random_byte() & 0x1f);
        }
        else if (f->waitTicks >= 50)
        {
            f->waitTicks = 200 + (random_byte() & 0x3f);
        }
        else
        {
            f->waitTicks = 300 + random_byte();
        }
        Figure_removeFromTileList(figureId);
        map_point river_entry = scenario_map_river_entry();
        f->x = river_entry.x;
        f->y = river_entry.y;
        f->gridOffset = GridOffset(f->x, f->y);
        f->crossCountryX = 15 * f->x;
        f->crossCountryY = 15 * f->y;
        break;
    }
    if (f->resourceId == 0)
    {
        FigureActionIncreaseGraphicOffset(f, 12);
        if (f->minMaxSeen)
        {
            f->graphicId = image_group(GROUP_FIGURE_FLOTSAM_SHEEP) +
                           flotsamType0[f->graphicOffset];
        }
        else
        {
            f->graphicId = image_group(GROUP_FIGURE_FLOTSAM_0) +
                           flotsamType0[f->graphicOffset];
        }
    }
    else if (f->resourceId == 1)
    {
        FigureActionIncreaseGraphicOffset(f, 24);
        f->graphicId = image_group(GROUP_FIGURE_FLOTSAM_1) +
                       flotsamType12[f->graphicOffset];
    }
    else if (f->resourceId == 2)
    {
        FigureActionIncreaseGraphicOffset(f, 24);
        f->graphicId = image_group(GROUP_FIGURE_FLOTSAM_2) +
                       flotsamType12[f->graphicOffset];
    }
    else if (f->resourceId == 3)
    {
        FigureActionIncreaseGraphicOffset(f, 24);
        if (flotsamType3[f->graphicOffset] == -1)
        {
            f->graphicId = 0;
        }
        else
        {
            f->graphicId = image_group(GROUP_FIGURE_FLOTSAM_3) +
                           flotsamType3[f->graphicOffset];
        }
    }
}

void FigureAction_shipwreck(int figureId)
{
    struct Data_Figure *f = &Data_Figures[figureId];
    f->isGhost = 0;
    f->heightAdjustedTicks = 0;
    f->isBoat = 1;
    FigureActionIncreaseGraphicOffset(f, 128);
    if (f->waitTicks < 1000)
    {
        Figure_removeFromTileList(figureId);
        int xTile, yTile;
        if (Terrain_Water_findOpenWaterForShipwreck(figureId, &xTile, &yTile))
        {
            f->x = xTile;
            f->y = yTile;
            f->gridOffset = GridOffset(f->x, f->y);
            f->crossCountryX = 15 * f->x + 7;
            f->crossCountryY = 15 * f->y + 7;
        }
        Figure_addToTileList(figureId);
        f->waitTicks = 1000;
    }
    f->waitTicks++;
    if (f->waitTicks > 2000)
    {
        f->state = FigureState_Dead;
    }
    f->graphicId = image_group(GROUP_FIGURE_SHIPWRECK) + f->graphicOffset / 16;
}
