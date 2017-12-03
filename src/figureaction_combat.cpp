#include "figureaction_private.h"

#include "core/calc.h"
#include "figuremovement.h"


#include "figure/properties.h"
#include "figure/type.h"

int FigureAction_CombatSoldier_getTarget(int x, int y, int maxDistance)
{
    int minFigureId = 0;
    int minDistance = 10000;
    for (int i = 1; i < MAX_FIGURES; i++)
    {
        if (FigureIsDead(i))
        {
            continue;
        }
        struct Data_Figure *f = &Data_Figures[i];
        if (FigureIsEnemy(f->type) || f->type == FIGURE_RIOTER ||
                (f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FigureActionState_159_NativeAttacking))
        {
            int distance = calc_maximum_distance(x, y, f->x, f->y);
            if (distance <= maxDistance)
            {
                if (f->targetedByFigureId)
                {
                    distance *= 2; // penalty
                }
                if (distance < minDistance)
                {
                    minDistance = distance;
                    minFigureId = i;
                }
            }
        }
    }
    if (minFigureId)
    {
        return minFigureId;
    }
    for (int i = 1; i < MAX_FIGURES; i++)
    {
        if (FigureIsDead(i))
        {
            continue;
        }
        struct Data_Figure *f = &Data_Figures[i];
        if (FigureIsEnemy(f->type) || f->type == FIGURE_RIOTER ||
                (f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FigureActionState_159_NativeAttacking))
        {
            return i;
        }
    }
    return 0;
}

int FigureAction_CombatSoldier_getMissileTarget(int soldierId, int maxDistance, int *xTile, int *yTile)
{
    int x = Data_Figures[soldierId].x;
    int y = Data_Figures[soldierId].y;

    int minFigureId = 0;
    int minDistance = maxDistance;
    for (int i = 1; i < MAX_FIGURES; i++)
    {
        if (FigureIsDead(i))
        {
            continue;
        }
        struct Data_Figure *f = &Data_Figures[i];
        if (FigureIsEnemy(f->type) || FigureIsHerd(f->type) ||
                (f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FigureActionState_159_NativeAttacking))
        {
            int distance = calc_maximum_distance(x, y, f->x, f->y);
            if (distance < minDistance && FigureMovement_canLaunchCrossCountryMissile(x, y, f->x, f->y))
            {
                minDistance = distance;
                minFigureId = i;
            }
        }
    }
    if (minFigureId)
    {
        *xTile = Data_Figures[minFigureId].x;
        *yTile = Data_Figures[minFigureId].y;
        return minFigureId;
    }
    return 0;
}

int FigureAction_CombatWolf_getTarget(int x, int y, int maxDistance)
{
    int minFigureId = 0;
    int minDistance = 10000;
    for (int i = 1; i < MAX_FIGURES; i++)
    {
        struct Data_Figure *f = &Data_Figures[i];
        if (FigureIsDead(i) || !f->type)
        {
            continue;
        }
        switch (f->type)
        {
        case FIGURE_EXPLOSION:
        case FIGURE_FORT_STANDARD:
        case FIGURE_TRADE_SHIP:
        case FIGURE_FISHING_BOAT:
        case FIGURE_MAP_FLAG:
        case FIGURE_FLOTSAM:
        case FIGURE_SHIPWRECK:
        case FIGURE_INDIGENOUS_NATIVE:
        case FIGURE_TOWER_SENTRY:
        case FIGURE_NATIVE_TRADER:
        case FIGURE_ARROW:
        case FIGURE_JAVELIN:
        case FIGURE_BOLT:
        case FIGURE_BALLISTA:
        case FIGURE_CREATURE:
            continue;
        }
        if (FigureIsEnemy(f->type) || FigureIsHerd(f->type))
        {
            continue;
        }
        if (FigureIsLegion(f->type) && f->actionState == FigureActionState_80_SoldierAtRest)
        {
            continue;
        }
        int distance = calc_maximum_distance(x, y, f->x, f->y);
        if (f->targetedByFigureId)
        {
            distance *= 2;
        }
        if (distance < minDistance)
        {
            minDistance = distance;
            minFigureId = i;
        }
    }
    if (minDistance <= maxDistance && minFigureId)
    {
        return minFigureId;
    }
    return 0;
}

int FigureAction_CombatEnemy_getTarget(int x, int y)
{
    int minFigureId = 0;
    int minDistance = 10000;
    for (int i = 1; i < MAX_FIGURES; i++)
    {
        if (FigureIsDead(i))
        {
            continue;
        }
        struct Data_Figure *f = &Data_Figures[i];
        if (!f->targetedByFigureId && FigureIsLegion(f->type))
        {
            int distance = calc_maximum_distance(x, y, f->x, f->y);
            if (distance < minDistance)
            {
                minDistance = distance;
                minFigureId = i;
            }
        }
    }
    if (minFigureId)
    {
        return minFigureId;
    }
    // no 'free' soldier found, take first one
    for (int i = 1; i < MAX_FIGURES; i++)
    {
        if (FigureIsDead(i))
        {
            continue;
        }
        if (FigureIsLegion(Data_Figures[i].type))
        {
            return i;
        }
    }
    return 0;
}

int FigureAction_CombatEnemy_getMissileTarget(int enemyId, int maxDistance, int attackCitizens, int *xTile, int *yTile)
{
    int x = Data_Figures[enemyId].x;
    int y = Data_Figures[enemyId].y;

    int minFigureId = 0;
    int minDistance = maxDistance;
    for (int i = 1; i < MAX_FIGURES; i++)
    {
        struct Data_Figure *f = &Data_Figures[i];
        if (FigureIsDead(i) || !f->type)
        {
            continue;
        }
        switch (f->type)
        {
        case FIGURE_EXPLOSION:
        case FIGURE_FORT_STANDARD:
        case FIGURE_MAP_FLAG:
        case FIGURE_FLOTSAM:
        case FIGURE_INDIGENOUS_NATIVE:
        case FIGURE_NATIVE_TRADER:
        case FIGURE_ARROW:
        case FIGURE_JAVELIN:
        case FIGURE_BOLT:
        case FIGURE_BALLISTA:
        case FIGURE_CREATURE:
        case FIGURE_FISH_GULLS:
        case FIGURE_SHIPWRECK:
        case FIGURE_SHEEP:
        case FIGURE_WOLF:
        case FIGURE_ZEBRA:
        case FIGURE_SPEAR:
            continue;
        }
        int distance;
        if (FigureIsLegion(f->type))
        {
            distance = calc_maximum_distance(x, y, f->x, f->y);
        }
        else if (attackCitizens && f->isFriendly)
        {
            distance = calc_maximum_distance(x, y, f->x, f->y) + 5;
        }
        else
        {
            continue;
        }
        if (distance < minDistance && FigureMovement_canLaunchCrossCountryMissile(x, y, f->x, f->y))
        {
            minDistance = distance;
            minFigureId = i;
        }
    }
    if (minFigureId)
    {
        *xTile = Data_Figures[minFigureId].x;
        *yTile = Data_Figures[minFigureId].y;
        return minFigureId;
    }
    return 0;
}


void FigureAction_Combat_attackFigure(int figureId, int opponentId)
{
    struct Data_Figure *f = &Data_Figures[figureId];
    int figureCategory = figure_properties_for_type((figure_type)f->type)->category;
    if (figureCategory <= FIGURE_CATEGORY_INACTIVE || figureCategory >= FIGURE_CATEGORY_CRIMINAL ||
            f->actionState == FigureActionState_150_Attack)
    {
        return;
    }
    int guard = 0;
    while (1)
    {
        if (++guard >= MAX_FIGURES || opponentId <= 0)
        {
            break;
        }
        if (opponentId == figureId)
        {
            opponentId = Data_Figures[opponentId].nextFigureIdOnSameTile;
            continue;
        }
        struct Data_Figure *opponent = &Data_Figures[opponentId];
        int opponentCategory = figure_properties_for_type((figure_type)opponent->type)->category;
        int attack = 0;
        if (opponent->state != FigureState_Alive)
        {
            attack = 0;
        }
        else if (opponent->actionState == FigureActionState_149_Corpse)
        {
            attack = 0;
        }
        else if (figureCategory == FIGURE_CATEGORY_ARMED && opponentCategory == FIGURE_CATEGORY_NATIVE)
        {
            if (opponent->actionState == FigureActionState_159_NativeAttacking)
            {
                attack = 1;
            }
        }
        else if (figureCategory == FIGURE_CATEGORY_ARMED && opponentCategory == FIGURE_CATEGORY_CRIMINAL)
        {
            attack = 1;
        }
        else if (figureCategory == FIGURE_CATEGORY_ARMED && opponentCategory == FIGURE_CATEGORY_HOSTILE)
        {
            attack = 1;
        }
        else if (figureCategory == FIGURE_CATEGORY_HOSTILE && opponentCategory == FIGURE_CATEGORY_CITIZEN)
        {
            attack = 1;
        }
        else if (figureCategory == FIGURE_CATEGORY_HOSTILE && opponentCategory == FIGURE_CATEGORY_ARMED)
        {
            attack = 1;
        }
        else if (figureCategory == FIGURE_CATEGORY_HOSTILE && opponentCategory == FIGURE_CATEGORY_CRIMINAL)
        {
            attack = 1;
        }
        else if (figureCategory == FIGURE_CATEGORY_ARMED && opponentCategory == FIGURE_CATEGORY_ANIMAL)
        {
            attack = 1;
        }
        else if (figureCategory == FIGURE_CATEGORY_HOSTILE && opponentCategory == FIGURE_CATEGORY_ANIMAL)
        {
            attack = 1;
        }
        if (attack && opponent->actionState == FigureActionState_150_Attack && opponent->numAttackers >= 2)
        {
            attack = 0;
        }
        if (attack)
        {
            f->actionStateBeforeAttack = f->actionState;
            f->actionState = FigureActionState_150_Attack;
            f->opponentId = opponentId;
            f->attackerId1 = opponentId;
            f->numAttackers = 1;
            f->attackGraphicOffset = 12;
            if (opponent->x != opponent->destinationX || opponent->y != opponent->destinationY)
            {
                f->attackDirection = calc_general_direction(f->previousTileX, f->previousTileY,
                                     opponent->previousTileX, opponent->previousTileY);
            }
            else
            {
                f->attackDirection = calc_general_direction(f->previousTileX, f->previousTileY,
                                     opponent->x, opponent->y);
            }
            if (f->attackDirection >= 8)
            {
                f->attackDirection = 0;
            }
            if (opponent->actionState != FigureActionState_150_Attack)
            {
                opponent->actionStateBeforeAttack = opponent->actionState;
                opponent->actionState = FigureActionState_150_Attack;
                opponent->attackGraphicOffset = 0;
                opponent->attackDirection = (f->attackDirection + 4) % 8;
            }
            if (opponent->numAttackers == 0)
            {
                opponent->attackerId1 = figureId;
                opponent->opponentId = figureId;
                opponent->numAttackers = 1;
            }
            else if (opponent->numAttackers == 1)
            {
                opponent->attackerId2 = figureId;
                opponent->numAttackers = 2;
            }
            return;
        }
        opponentId = opponent->nextFigureIdOnSameTile;
    }
}
