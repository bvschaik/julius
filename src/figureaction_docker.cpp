#include "figureaction_private.h"

#include "figure.h"
#include "resource.h"
#include "trader.h"

#include <data>
#include <game>
#include <core>

#include "figure/trader.h"
#include "figure/type.h"

static int dockerDeliverImportResource(int figureId, int buildingId)
{
    int shipId = Data_Buildings[buildingId].data.other.boatFigureId;
    if (!shipId)
    {
        return 0;
    }
    struct Data_Figure *ship = &Data_Figures[shipId];
    if (ship->actionState != FigureActionState_112_TradeShipMoored || ship->loadsSoldOrCarrying <= 0)
    {
        return 0;
    }
    struct Data_Figure *f = &Data_Figures[figureId];
    int x, y;
    if (Data_CityInfo.buildingTradeCenterBuildingId)
    {
        int tcId = Data_CityInfo.buildingTradeCenterBuildingId;
        x = Data_Buildings[tcId].x;
        y = Data_Buildings[tcId].y;
    }
    else
    {
        x = f->x;
        y = f->y;
    }
    int xTile, yTile;
    int warehouseId = Trader_getClosestWarehouseForImportDocker(x, y, ship->empireCityId,
                      Data_Buildings[buildingId].distanceFromEntry, Data_Buildings[buildingId].roadNetworkId, &xTile, &yTile);
    if (!warehouseId)
    {
        return 0;
    }
    ship->loadsSoldOrCarrying--;
    f->destinationBuildingId = warehouseId;
    f->waitTicks = 0;
    f->actionState = FigureActionState_133_DockerImportQueue;
    f->destinationX = xTile;
    f->destinationY = yTile;
    f->resourceId = Data_CityInfo.tradeNextImportResourceDocker;
    return 1;
}

static int dockerGetExportResource(int figureId, int buildingId)
{
    int shipId = Data_Buildings[buildingId].data.other.boatFigureId;
    if (!shipId)
    {
        return 0;
    }
    struct Data_Figure *ship = &Data_Figures[shipId];
    if (ship->actionState != FigureActionState_112_TradeShipMoored || ship->traderAmountBought >= 12)
    {
        return 0;
    }
    struct Data_Figure *f = &Data_Figures[figureId];
    int x, y;
    if (Data_CityInfo.buildingTradeCenterBuildingId)
    {
        int tcId = Data_CityInfo.buildingTradeCenterBuildingId;
        x = Data_Buildings[tcId].x;
        y = Data_Buildings[tcId].y;
    }
    else
    {
        x = f->x;
        y = f->y;
    }
    int xTile, yTile;
    int warehouseId = Trader_getClosestWarehouseForExportDocker(x, y, ship->empireCityId,
                      Data_Buildings[buildingId].distanceFromEntry, Data_Buildings[buildingId].roadNetworkId, &xTile, &yTile);
    if (!warehouseId)
    {
        return 0;
    }
    ship->traderAmountBought++;
    f->destinationBuildingId = warehouseId;
    f->actionState = FigureActionState_136_DockerExportGoingToWarehouse;
    f->waitTicks = 0;
    f->destinationX = xTile;
    f->destinationY = yTile;
    f->resourceId = Data_CityInfo.tradeNextExportResourceDocker;
    return 1;
}

static void setCartGraphic(struct Data_Figure *f)
{
    f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART) + 8 * f->resourceId;
    f->cartGraphicId += Resource_getGraphicIdOffset(f->resourceId, 1);
}

void FigureAction_docker(int figureId)
{
    struct Data_Figure *f = &Data_Figures[figureId];
    struct Data_Building *b = &Data_Buildings[f->buildingId];
    FigureActionIncreaseGraphicOffset(f, 12);
    f->cartGraphicId = 0;
    if (!BuildingIsInUse(f->buildingId))
    {
        f->state = FigureState_Dead;
    }
    if (b->type != BUILDING_DOCK && b->type != BUILDING_WHARF)
    {
        f->state = FigureState_Dead;
    }
    if (b->data.other.dockNumShips)
    {
        b->data.other.dockNumShips--;
    }
    if (b->data.other.boatFigureId)
    {
        struct Data_Figure *ship = &Data_Figures[b->data.other.boatFigureId];
        if (ship->state != FigureState_Alive || ship->type != FIGURE_TRADE_SHIP)
        {
            b->data.other.boatFigureId = 0;
        }
        else if (trader_has_traded_max(ship->traderId))
        {
            b->data.other.boatFigureId = 0;
        }
        else if (ship->actionState == FigureActionState_115_TradeShipLeaving)
        {
            b->data.other.boatFigureId = 0;
        }
    }
    f->terrainUsage = FigureTerrainUsage_Roads;
    switch (f->actionState)
    {
    case FigureActionState_150_Attack:
        FigureAction_Common_handleAttack(figureId);
        break;
    case FigureActionState_149_Corpse:
        FigureAction_Common_handleCorpse(figureId);
        break;
    case FigureActionState_132_DockerIdling:
        f->resourceId = 0;
        f->cartGraphicId = 0;
        if (!dockerDeliverImportResource(figureId, f->buildingId))
        {
            dockerGetExportResource(figureId, f->buildingId);
        }
        f->graphicOffset = 0;
        break;
    case FigureActionState_133_DockerImportQueue:
        f->cartGraphicId = 0;
        f->graphicOffset = 0;
        if (b->data.other.dockQueuedDockerId <= 0)
        {
            b->data.other.dockQueuedDockerId = figureId;
            f->waitTicks = 0;
        }
        if (b->data.other.dockQueuedDockerId == figureId)
        {
            b->data.other.dockNumShips = 120;
            f->waitTicks++;
            if (f->waitTicks >= 80)
            {
                f->actionState = FigureActionState_135_DockerImportGoingToWarehouse;
                f->waitTicks = 0;
                setCartGraphic(f);
                b->data.other.dockQueuedDockerId = 0;
            }
        }
        else
        {
            int hasQueuedDocker = 0;
            for (int i = 0; i < 3; i++)
            {
                int dockerId = b->data.other.dockFigureIds[i];
                if (dockerId && b->data.other.dockQueuedDockerId == dockerId &&
                        Data_Figures[dockerId].state == FigureState_Alive)
                {
                    if (Data_Figures[dockerId].actionState == FigureActionState_133_DockerImportQueue ||
                            Data_Figures[dockerId].actionState == FigureActionState_134_DockerExportQueue)
                    {
                        hasQueuedDocker = 1;
                    }
                }
            }
            if (!hasQueuedDocker)
            {
                b->data.other.dockQueuedDockerId = 0;
            }
        }
        break;
    case FigureActionState_134_DockerExportQueue:
        setCartGraphic(f);
        if (b->data.other.dockQueuedDockerId <= 0)
        {
            b->data.other.dockQueuedDockerId = figureId;
            f->waitTicks = 0;
        }
        if (b->data.other.dockQueuedDockerId == figureId)
        {
            b->data.other.dockNumShips = 120;
            f->waitTicks++;
            if (f->waitTicks >= 80)
            {
                f->actionState = FigureActionState_132_DockerIdling;
                f->waitTicks = 0;
                f->graphicId = 0;
                f->cartGraphicId = 0;
                b->data.other.dockQueuedDockerId = 0;
            }
        }
        f->waitTicks++;
        if (f->waitTicks >= 20)
        {
            f->actionState = FigureActionState_132_DockerIdling;
            f->waitTicks = 0;
        }
        f->graphicOffset = 0;
        break;
    case FigureActionState_135_DockerImportGoingToWarehouse:
        setCartGraphic(f);
        FigureMovement_walkTicks(figureId, 1);
        if (f->direction == DIR_FIGURE_AT_DESTINATION)
        {
            f->actionState = FigureActionState_139_DockerImportAtWarehouse;
        }
        else if (f->direction == DIR_FIGURE_REROUTE)
        {
            figure_route_remove(figureId);
        }
        else if (f->direction == DIR_FIGURE_LOST)
        {
            f->state = FigureState_Dead;
        }
        if (!BuildingIsInUse(f->destinationBuildingId))
        {
            f->state = FigureState_Dead;
        }
        break;
    case FigureActionState_136_DockerExportGoingToWarehouse:
        f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
        FigureMovement_walkTicks(figureId, 1);
        if (f->direction == DIR_FIGURE_AT_DESTINATION)
        {
            f->actionState = FigureActionState_140_DockerExportAtWarehouse;
        }
        else if (f->direction == DIR_FIGURE_REROUTE)
        {
            figure_route_remove(figureId);
        }
        else if (f->direction == DIR_FIGURE_LOST)
        {
            f->state = FigureState_Dead;
        }
        if (!BuildingIsInUse(f->destinationBuildingId))
        {
            f->state = FigureState_Dead;
        }
        break;
    case FigureActionState_137_DockerExportReturning:
        setCartGraphic(f);
        FigureMovement_walkTicks(figureId, 1);
        if (f->direction == DIR_FIGURE_AT_DESTINATION)
        {
            f->actionState = FigureActionState_134_DockerExportQueue;
            f->waitTicks = 0;
        }
        else if (f->direction == DIR_FIGURE_REROUTE)
        {
            figure_route_remove(figureId);
        }
        else if (f->direction == DIR_FIGURE_LOST)
        {
            f->state = FigureState_Dead;
        }
        if (!BuildingIsInUse(f->destinationBuildingId))
        {
            f->state = FigureState_Dead;
        }
        break;
    case FigureActionState_138_DockerImportReturning:
        setCartGraphic(f);
        FigureMovement_walkTicks(figureId, 1);
        if (f->direction == DIR_FIGURE_AT_DESTINATION)
        {
            f->actionState = FigureActionState_132_DockerIdling;
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
    case FigureActionState_139_DockerImportAtWarehouse:
        setCartGraphic(f);
        f->waitTicks++;
        if (f->waitTicks > 10)
        {
            int tradeCityId;
            if (b->data.other.boatFigureId)
            {
                tradeCityId = Data_Figures[b->data.other.boatFigureId].empireCityId;
            }
            else
            {
                tradeCityId = 0;
            }
            if (Trader_tryImportResource(f->destinationBuildingId, f->resourceId, tradeCityId))
            {
                int traderId = Data_Figures[b->data.other.boatFigureId].traderId;
                trader_record_sold_resource(traderId, (resource_type)f->resourceId);
                f->actionState = FigureActionState_138_DockerImportReturning;
                f->waitTicks = 0;
                f->destinationX = f->sourceX;
                f->destinationY = f->sourceY;
                f->resourceId = 0;
                dockerGetExportResource(figureId, f->buildingId);
            }
            else
            {
                f->actionState = FigureActionState_138_DockerImportReturning;
                f->destinationX = f->sourceX;
                f->destinationY = f->sourceY;
            }
            f->waitTicks = 0;
        }
        f->graphicOffset = 0;
        break;
    case FigureActionState_140_DockerExportAtWarehouse:
        f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
        f->waitTicks++;
        if (f->waitTicks > 10)
        {
            int tradeCityId;
            if (b->data.other.boatFigureId)
            {
                tradeCityId = Data_Figures[b->data.other.boatFigureId].empireCityId;
            }
            else
            {
                tradeCityId = 0;
            }
            f->actionState = FigureActionState_138_DockerImportReturning;
            f->destinationX = f->sourceX;
            f->destinationY = f->sourceY;
            f->waitTicks = 0;
            if (Trader_tryExportResource(f->destinationBuildingId, f->resourceId, tradeCityId))
            {
                int traderId = Data_Figures[b->data.other.boatFigureId].traderId;
                trader_record_bought_resource(traderId, (resource_type)f->resourceId);
                f->actionState = FigureActionState_137_DockerExportReturning;
            }
            else
            {
                dockerGetExportResource(figureId, f->buildingId);
            }
        }
        f->graphicOffset = 0;
        break;
    }

    int dir = f->direction < 8 ? f->direction : f->previousTileDirection;
    FigureActionNormalizeDirection(dir);

    if (f->actionState == FigureActionState_149_Corpse)
    {
        f->graphicId = image_group(GROUP_FIGURE_CARTPUSHER) +
                       FigureActionCorpseGraphicOffset(f) + 96;
        f->cartGraphicId = 0;
    }
    else
    {
        f->graphicId = image_group(GROUP_FIGURE_CARTPUSHER) + dir + 8 * f->graphicOffset;
    }
    if (f->cartGraphicId)
    {
        f->cartGraphicId += dir;
        FigureAction_Common_setCartOffset(figureId, dir);
    }
    else
    {
        f->graphicId = 0;
    }
}
