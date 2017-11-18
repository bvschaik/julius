#include "formation.h"

#include "core/calc.h"
#include "figure.h"
#include "figuremovement.h"
#include "routing.h"
#include "ui/warning.h"

#include <sound>
#include <data>

#include "building/model.h"
#include "figure/enemy_army.h"
#include "figure/formation.h"
#include "figure/properties.h"

#include <string.h>

void Formation_clearInvasionInfo()
{
    enemy_armies_clear();
}

int Formation_createLegion(int buildingId)
{
    Formation_calculateLegionTotals();

    struct Data_Building *b = &Data_Buildings[buildingId];
    int formation_id = formation_create_legion(buildingId, b->x, b->y, b->subtype.fortFigureType);
    if (!formation_id)
    {
        return 0;
    }
    int standardId = Figure_create(FIGURE_FORT_STANDARD, 0, 0, 0);
    Data_Figures[standardId].buildingId = buildingId;
    Data_Figures[standardId].formationId = formation_id;
    formation_set_standard(formation_id, standardId);

    return formation_id;
}

void Formation_deleteFortAndBanner(int formationId)
{
    if (formationId > 0)
    {
        const formation *m = formation_get(formationId);
        if (m->in_use)
        {
            if (m->standard_figure_id)
            {
                Figure_delete(m->standard_figure_id);
            }
            formation_clear(formationId);
            Formation_calculateLegionTotals();
        }
    }
}

int Formation_getLegionFormationAtGridOffset(int gridOffset)
{
    for (int figureId = Data_Grid_figureIds[gridOffset];
            figureId && figureId != Data_Figures[figureId].nextFigureIdOnSameTile;
            figureId = Data_Figures[figureId].nextFigureIdOnSameTile)
    {
        if (FigureIsLegion(Data_Figures[figureId].type) ||
                Data_Figures[figureId].type == FIGURE_FORT_STANDARD)
        {
            return Data_Figures[figureId].formationId;
        }
    }
    return 0;
}

int Formation_getFormationForBuilding(int gridOffset)
{
    int buildingId = Data_Grid_buildingIds[gridOffset];
    if (buildingId > 0)
    {
        struct Data_Building *b = &Data_Buildings[buildingId];
        if (BuildingIsInUse(buildingId) && (b->type == BUILDING_FORT || b->type == BUILDING_FORT_GROUND))
        {
            return b->formationId;
        }
    }
    return 0;
}

void Formation_legionMoveTo(int formationId, int x, int y)
{
    const formation *m = formation_get(formationId);
    Routing_getDistance(m->x_home, m->y_home);
    if (Data_Grid_routingDistance[GridOffset(x, y)] <= 0)
    {
        return; // unable to route there
    }
    if (x == m->x_home && y == m->y_home)
    {
        return; // use legionReturnHome
    }
    if (m->cursed_by_mars)
    {
        return;
    }
    formation_move_standard(m->id, x, y);
    if (m->morale <= 20)
    {
        UI_Warning_show(Warning_LegionMoraleTooLow);
    }
    for (int i = 0; i < MAX_FORMATION_FIGURES && m->figures[i]; i++)
    {
        int figureId = m->figures[i];
        struct Data_Figure *f = &Data_Figures[figureId];
        if (f->actionState == FigureActionState_149_Corpse ||
                f->actionState == FigureActionState_150_Attack)
        {
            continue;
        }
        if (formation_legion_prepare_to_move(m->id))
        {
            f->alternativeLocationIndex = 0;
            f->actionState = FigureActionState_83_SoldierGoingToStandard;
            FigureRoute_remove(figureId);
        }
    }
}

void Formation_legionReturnHome(int formationId)
{
    const formation *m = formation_get(formationId);
    Routing_getDistance(m->x_home, m->y_home);
    if (Data_Grid_routingDistance[GridOffset(m->x, m->y)] <= 0)
    {
        return; // unable to route home
    }
    if (m->cursed_by_mars)
    {
        return;
    }
    formation_set_at_fort(m->id, 1);
    formation_restore_layout(m->id);
    for (int i = 0; i < MAX_FORMATION_FIGURES && m->figures[i]; i++)
    {
        int figureId = m->figures[i];
        struct Data_Figure *f = &Data_Figures[figureId];
        if (f->actionState == FigureActionState_149_Corpse ||
                f->actionState == FigureActionState_150_Attack)
        {
            continue;
        }
        if (formation_legion_prepare_to_move(m->id))
        {
            f->actionState = FigureActionState_81_SoldierGoingToFort;
            FigureRoute_remove(figureId);
        }
    }
}

static void update_totals(const formation *m)
{
    if (m->is_legion)
    {
        formation_totals_add_legion(m->id);
        if (m->figure_type == FIGURE_FORT_LEGIONARY)
        {
            Data_CityInfo.militaryLegionaryLegions++;
        }
    }
    if (m->missile_attack_timeout <= 0 && m->figures[0])
    {
        int figureId = m->figures[0];
        if (Data_Figures[figureId].state == FigureState_Alive)
        {
            formation_set_home(m->id, Data_Figures[figureId].x, Data_Figures[figureId].y);
        }
    }
}
void Formation_calculateLegionTotals()
{
    formation_totals_clear_legions();
    Data_CityInfo.militaryLegionaryLegions = 0;
    formation_foreach(update_totals);
}

static void are_legions_needing_recruits(const formation *m, void *data)
{
    if (m->legion_recruit_type != LEGION_RECRUIT_NONE)
    {
        *(int*) data = 1;
    }
}
int Formation_anyLegionNeedsSoldiers()
{
    int yes = 0;
    formation_foreach_legion(are_legions_needing_recruits, &yes);
    return yes;
}

int Formation_getClosestMilitaryAcademy(int formationId)
{
    int fortId = formation_get(formationId)->building_id;
    int fortX = Data_Buildings[fortId].x;
    int fortY = Data_Buildings[fortId].y;
    int minBuildingId = 0;
    int minDistance = 10000;
    for (int i = 1; i < MAX_BUILDINGS; i++)
    {
        if (BuildingIsInUse(i) &&
                Data_Buildings[i].type == BUILDING_MILITARY_ACADEMY &&
                Data_Buildings[i].numWorkers >= model_get_building(BUILDING_MILITARY_ACADEMY)->laborers)
        {
            int dist = calc_maximum_distance(fortX, fortY, Data_Buildings[i].x, Data_Buildings[i].y);
            if (dist < minDistance)
            {
                minDistance = dist;
                minBuildingId = i;
            }
        }
    }
    return minBuildingId;
}

void Formation_setNewSoldierRequest(int buildingId)
{
    const formation *m = formation_get(Data_Buildings[buildingId].formationId);
    formation_set_recruit_type(m->id, LEGION_RECRUIT_NONE);
    if (!m->is_at_fort || m->cursed_by_mars || m->num_figures == m->max_figures)
    {
        return;
    }
    if (m->num_figures < m->max_figures)
    {
        int type = Data_Buildings[buildingId].subtype.fortFigureType;
        if (type == FIGURE_FORT_LEGIONARY)
        {
            formation_set_recruit_type(m->id, LEGION_RECRUIT_LEGIONARY);
        }
        else if (type == FIGURE_FORT_JAVELIN)
        {
            formation_set_recruit_type(m->id, LEGION_RECRUIT_JAVELIN);
        }
        else if (type == FIGURE_FORT_MOUNTED)
        {
            formation_set_recruit_type(m->id, LEGION_RECRUIT_MOUNTED);
        }
    }
    else     // too many figures
    {
        int tooMany = m->num_figures - m->max_figures;
        for (int i = MAX_FORMATION_FIGURES - 1; i >= 0 && tooMany > 0; i--)
        {
            if (m->figures[i])
            {
                Data_Figures[m->figures[i]].actionState = FigureActionState_82_SoldierReturningToBarracks;
                tooMany--;
            }
        }
        Formation_calculateFigures();
    }
}

static void update_legion_enemy_totals(const formation *m, void *data)
{
    if (m->is_legion)
    {
        if (m->num_figures > 0)
        {
            int wasHalted = m->is_halted;
            formation_set_halted(m->id, 1);
            for (int fig = 0; fig < m->num_figures; fig++)
            {
                int figureId = m->figures[fig];
                if (figureId && Data_Figures[figureId].direction != Dir_8_None)
                {
                    formation_set_halted(m->id, 0);
                }
            }
            int total_strength = m->num_figures;
            if (m->figure_type == FIGURE_FORT_LEGIONARY)
            {
                total_strength += m->num_figures / 2;
            }
            enemy_army_totals_add_legion_formation(total_strength);
            if (m->figure_type == FIGURE_FORT_LEGIONARY)
            {
                if (!wasHalted && m->is_halted)
                {
                    sound_effect_play(SOUND_EFFECT_FORMATION_SHIELD);
                }
            }
        }
    }
    else
    {
        // enemy
        if (m->num_figures <= 0)
        {
            formation_clear(m->id);
        }
        else
        {
            enemy_army_totals_add_enemy_formation(m->num_figures);
        }
    }
}

static void update_soldier_totals(const formation *m, void *data)
{
    Data_CityInfo.militaryTotalLegions++;
    Data_CityInfo.militaryTotalSoldiers += m->num_figures;
    if (m->empire_service && m->num_figures > 0)
    {
        Data_CityInfo.militaryTotalLegionsEmpireService++;
    }
}

void Formation_calculateFigures()
{
    formation_clear_figures();
    for (int i = 1; i < MAX_FIGURES; i++)
    {
        if (Data_Figures[i].state != FigureState_Alive)
        {
            continue;
        }
        int figtype = Data_Figures[i].type;
        if (!FigureIsLegion(figtype) && !FigureIsEnemy(figtype) && !FigureIsHerd(figtype))
        {
            continue;
        }
        if (figtype == FIGURE_ENEMY54_GLADIATOR)
        {
            continue;
        }
        int formationId = Data_Figures[i].formationId;
        int index = formation_add_figure(formationId, i,
                                         Data_Figures[i].formationAtRest != 1,
                                         Data_Figures[i].damage,
                                         figure_properties_for_type(figtype)->max_damage
                                        );
        Data_Figures[i].indexInFormation = index;
    }

    enemy_army_totals_clear();
    formation_foreach_non_herd(update_legion_enemy_totals, 0);

    Data_CityInfo.militaryTotalLegionsEmpireService = 0;
    Data_CityInfo.militaryTotalSoldiers = 0;
    Data_CityInfo.militaryTotalLegions = 0;
    formation_foreach_legion(update_soldier_totals, 0);
}

void Formation_updateAfterDeath(int formationId)
{
    Formation_calculateFigures();
    int pctDead = calc_percentage(1, formation_get(formationId)->num_figures);
    int morale;
    if (pctDead < 8)
    {
        morale = -5;
    }
    else if (pctDead < 10)
    {
        morale = -7;
    }
    else if (pctDead < 14)
    {
        morale = -10;
    }
    else if (pctDead < 20)
    {
        morale = -12;
    }
    else if (pctDead < 30)
    {
        morale = -15;
    }
    else
    {
        morale = -20;
    }
    formation_change_morale(formationId, morale);
}

int Formation_getInvasionGridOffset(int invasionSeq)
{
    int formationId = formation_for_invasion(invasionSeq);
    if (formationId)
    {
        const formation *m = formation_get(formationId);
        if (m->x_home > 0 || m->y_home > 0)
        {
            return GridOffset(m->x_home, m->y_home);
        }
    }
    return 0;
}

static void dispatch_soldiers(const formation *m, void *data)
{
    if (!m->empire_service || m->num_figures <= 0)
    {
        return;
    }
    formation_set_distant_battle(m->id, 1);
    formation_set_at_fort(m->id, 0);
    int *num_legions = (int*) data;
    *num_legions = *num_legions + 1;
    int strengthFactor;
    if (m->has_military_training)
    {
        strengthFactor = m->figure_type == FIGURE_FORT_LEGIONARY ? 3 : 2;
    }
    else
    {
        strengthFactor = m->figure_type == FIGURE_FORT_LEGIONARY ? 2 : 1;
    }
    Data_CityInfo.distantBattleRomanStrength += strengthFactor * m->num_figures;
    for (int fig = 0; fig < m->num_figures; fig++)
    {
        int figureId = m->figures[fig];
        if (figureId > 0 &&
                Data_Figures[figureId].state == FigureState_Alive &&
                Data_Figures[figureId].actionState != FigureActionState_149_Corpse)
        {
            Data_Figures[figureId].actionState = FigureActionState_87_SoldierGoingToDistantBattle;
        }
    }
}

void Formation_dispatchLegionsToDistantBattle()
{
    Data_CityInfo.distantBattleRomanStrength = 0;
    int numLegions = 0;
    formation_foreach_legion(dispatch_soldiers, &numLegions);
    if (numLegions > 0)
    {
        Data_CityInfo.distantBattleRomanMonthsToTravel = Data_Scenario.distantBattleTravelMonthsRoman;
    }
}

static void return_soldiers(const formation *m, void *data)
{
    if (!m->in_distant_battle)
    {
        return;
    }
    formation_set_distant_battle(m->id, 0);
    for (int fig = 0; fig < m->num_figures; fig++)
    {
        int figureId = m->figures[fig];
        if (figureId > 0 &&
                Data_Figures[figureId].state == FigureState_Alive &&
                Data_Figures[figureId].actionState != FigureActionState_149_Corpse)
        {
            Data_Figures[figureId].actionState = FigureActionState_88_SoldierReturningFromDistantBattle;
            Data_Figures[figureId].formationAtRest = 1;
        }
    }
}

void Formation_legionsReturnFromDistantBattle()
{
    formation_foreach_legion(return_soldiers, NULL);
}

static void kill_soldiers(const formation *m, void *data)
{
    int killPercentage = *(int*) data;
    if (!m->in_distant_battle)
    {
        return;
    }
    formation_change_morale(m->id, -75);
    int numSoldiersTotal = 0;
    for (int fig = 0; fig < m->num_figures; fig++)
    {
        int figureId = m->figures[fig];
        if (figureId > 0 && Data_Figures[figureId].state == FigureState_Alive &&
                Data_Figures[figureId].actionState != FigureActionState_149_Corpse)
        {
            numSoldiersTotal++;
        }
    }
    int numSoldiersToKill = calc_adjust_with_percentage(numSoldiersTotal, killPercentage);
    if (numSoldiersToKill >= numSoldiersTotal)
    {
        formation_set_at_fort(m->id, 1);
        formation_set_distant_battle(m->id, 0);
    }
    for (int fig = 0; fig < m->num_figures; fig++)
    {
        int figureId = m->figures[fig];
        if (figureId > 0 && Data_Figures[figureId].state == FigureState_Alive &&
                Data_Figures[figureId].actionState != FigureActionState_149_Corpse)
        {
            if (numSoldiersToKill)
            {
                numSoldiersToKill--;
                Data_Figures[figureId].state = FigureState_Dead;
            }
        }
    }
}

void Formation_legionKillSoldiersInDistantBattle(int killPercentage)
{
    formation_foreach_legion(kill_soldiers, &killPercentage);
}

int Formation_marsCurseFort()
{
    int bestLegionId = 0;
    int bestLegionWeight = 0;
    for (int i = 1; i <= 6; i++)   // BUG assumes no legions beyond index 6
    {
        const formation *m = formation_get(i);
        if (m->in_use == 1 && m->is_legion)
        {
            int weight = m->num_figures;
            if (m->figure_type == FIGURE_FORT_LEGIONARY)
            {
                weight *= 2;
            }
            if (weight > bestLegionWeight)
            {
                bestLegionWeight = weight;
                bestLegionId = i;
            }
        }
    }
    if (bestLegionId <= 0)
    {
        return 0;
    }
    const formation *m = formation_get(bestLegionId);
    for (int i = 0; i < MAX_FORMATION_FIGURES - 1; i++)   // BUG: last figure not cursed
    {
        if (m->figures[i] > 0)
        {
            Data_Figures[m->figures[i]].actionState = FigureActionState_82_SoldierReturningToBarracks;
        }
    }
    formation_set_cursed(bestLegionId);
    Formation_calculateFigures();
    return 1;
}

void Formation_Tick_updateRestMorale()
{
    formation_update_monthly_morale_at_rest();
}
