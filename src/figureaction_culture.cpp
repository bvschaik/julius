#include "figureaction_private.h"

#include "building.h"
#include "figure.h"
#include "terrain.h"

#include <game>

static void FigureAction_cultureCommon(int figureId, int numTicks)
{
    struct Data_Figure *f = &Data_Figures[figureId];
    switch (f->actionState)
    {
    case FigureActionState_150_Attack:
        FigureAction_Common_handleAttack(figureId);
        break;
    case FigureActionState_149_Corpse:
        FigureAction_Common_handleCorpse(figureId);
        break;
    case FigureActionState_125_Roaming:
        f->isGhost = 0;
        f->roamLength++;
        if (f->roamLength >= f->maxRoamLength)
        {
            int x, y;
            if (Terrain_getClosestRoadWithinRadius(
                        Data_Buildings[f->buildingId].x,
                        Data_Buildings[f->buildingId].y,
                        Data_Buildings[f->buildingId].size,
                        2, &x, &y))
            {
                f->actionState = FigureActionState_126_RoamerReturning;
                f->destinationX = x;
                f->destinationY = y;
                FigureRoute_remove(figureId);
                f->roamLength = 0;
            }
            else
            {
                f->state = FigureState_Dead;
            }
        }
        FigureMovement_roamTicks(figureId, numTicks);
        break;
    case FigureActionState_126_RoamerReturning:
        FigureMovement_walkTicks(figureId, numTicks);
        if (f->direction == DirFigure_8_AtDestination ||
                f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost)
        {
            f->state = FigureState_Dead;
        }
        break;
    }
}

static void FigureAction_culture(int figureId, int graphicCategory)
{
    struct Data_Figure *f = &Data_Figures[figureId];
    f->terrainUsage = FigureTerrainUsage_Roads;
    f->useCrossCountry = 0;
    f->maxRoamLength = 384;
    int buildingId = f->buildingId;
    if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].figureId != figureId)
    {
        f->state = FigureState_Dead;
    }
    FigureActionIncreaseGraphicOffset(f, 12);
    FigureAction_cultureCommon(figureId, 1);
    if (f->actionState == FigureActionState_149_Corpse)
    {
        f->graphicId = image_group(graphicCategory) +
                       FigureActionCorpseGraphicOffset(f) + 96;
    }
    else
    {
        f->graphicId = image_group(graphicCategory) +
                       FigureActionDirection(f) + 8 * f->graphicOffset;
    }
}

void FigureAction_priest(int figureId)
{
    FigureAction_culture(figureId, ID_Graphic_Figure_Priest);
}

void FigureAction_schoolChild(int figureId)
{
    struct Data_Figure *f = &Data_Figures[figureId];
    f->terrainUsage = FigureTerrainUsage_Roads;
    f->useCrossCountry = 0;
    f->maxRoamLength = 96;
    int buildingId = f->buildingId;
    if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].type != BUILDING_SCHOOL)
    {
        f->state = FigureState_Dead;
    }
    FigureActionIncreaseGraphicOffset(f, 12);
    switch (f->actionState)
    {
    case FigureActionState_150_Attack:
        FigureAction_Common_handleAttack(figureId);
        break;
    case FigureActionState_149_Corpse:
        FigureAction_Common_handleCorpse(figureId);
        break;
    case FigureActionState_125_Roaming:
        f->isGhost = 0;
        f->roamLength++;
        if (f->roamLength >= f->maxRoamLength)
        {
            f->state = FigureState_Dead;
        }
        FigureMovement_roamTicks(figureId, 2);
        break;
    }
    if (f->actionState == FigureActionState_149_Corpse)
    {
        f->graphicId = image_group(ID_Graphic_Figure_SchoolChild) +
                       FigureActionCorpseGraphicOffset(f) + 96;
    }
    else
    {
        f->graphicId = image_group(ID_Graphic_Figure_SchoolChild) +
                       FigureActionDirection(f) + 8 * f->graphicOffset;
    }
}

void FigureAction_teacher(int figureId)
{
    FigureAction_culture(figureId, ID_Graphic_Figure_TeacherLibrarian);
}

void FigureAction_librarian(int figureId)
{
    FigureAction_culture(figureId, ID_Graphic_Figure_TeacherLibrarian);
}

void FigureAction_barber(int figureId)
{
    FigureAction_culture(figureId, ID_Graphic_Figure_Barber);
}

void FigureAction_bathhouseWorker(int figureId)
{
    FigureAction_culture(figureId, ID_Graphic_Figure_BathhouseWorker);
}

void FigureAction_doctor(int figureId)
{
    FigureAction_culture(figureId, ID_Graphic_Figure_DoctorSurgeon);
}

void FigureAction_surgeon(int figureId)
{
    FigureAction_culture(figureId, ID_Graphic_Figure_DoctorSurgeon);
}

void FigureAction_missionary(int figureId)
{
    struct Data_Figure *f = &Data_Figures[figureId];
    f->terrainUsage = FigureTerrainUsage_Roads;
    f->useCrossCountry = 0;
    f->maxRoamLength = 192;
    int buildingId = f->buildingId;
    if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].figureId != figureId)
    {
        f->state = FigureState_Dead;
    }
    FigureActionIncreaseGraphicOffset(f, 12);
    FigureAction_cultureCommon(figureId, 1);
    FigureActionUpdateGraphic(f, image_group(ID_Graphic_Figure_Missionary));
}

void FigureAction_patrician(int figureId)
{
    struct Data_Figure *f = &Data_Figures[figureId];
    f->terrainUsage = FigureTerrainUsage_Roads;
    f->useCrossCountry = 0;
    f->maxRoamLength = 128;
    if (!BuildingIsInUse(f->buildingId))
    {
        f->state = FigureState_Dead;
    }
    FigureActionIncreaseGraphicOffset(f, 12);
    FigureAction_cultureCommon(figureId, 1);
    FigureActionUpdateGraphic(f, image_group(ID_Graphic_Figure_Patrician));
}

void FigureAction_laborSeeker(int figureId)
{
    struct Data_Figure *f = &Data_Figures[figureId];
    f->terrainUsage = FigureTerrainUsage_Roads;
    f->useCrossCountry = 0;
    f->maxRoamLength = 384;
    int buildingId = f->buildingId;
    if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].figureId2 != figureId)
    {
        f->state = FigureState_Dead;
    }
    FigureActionIncreaseGraphicOffset(f, 12);
    FigureAction_cultureCommon(figureId, 1);
    FigureActionUpdateGraphic(f, image_group(ID_Graphic_Figure_LaborSeeker));
}

void FigureAction_marketTrader(int figureId)
{
    struct Data_Figure *f = &Data_Figures[figureId];
    f->terrainUsage = FigureTerrainUsage_Roads;
    f->useCrossCountry = 0;
    f->maxRoamLength = 384;

    if (!BuildingIsInUse(f->buildingId) || Data_Buildings[f->buildingId].figureId != figureId)
    {
        f->state = FigureState_Dead;
    }
    FigureActionIncreaseGraphicOffset(f, 12);
    if (f->actionState == FigureActionState_125_Roaming)
    {
        // force return on out of stock
        int stock = Building_Market_getMaxGoodsStock(f->buildingId) +
                    Building_Market_getMaxGoodsStock(f->buildingId);
        if (f->roamLength >= 96 && stock <= 0)
        {
            f->roamLength = f->maxRoamLength;
        }
    }
    FigureAction_cultureCommon(figureId, 1);
    FigureActionUpdateGraphic(f, image_group(ID_Graphic_Figure_MarketLady));
}
