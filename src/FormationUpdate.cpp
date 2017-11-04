#include "Formation.hpp"

#include "Figure.hpp"
#include "FigureAction.hpp"
#include "Grid.hpp"
#include "PlayerMessage.hpp"
#include "Routing.hpp"
#include "Sound.hpp"
#include "TerrainGraphics.hpp"

#include "Data/Building.hpp"
#include "Data/CityInfo.hpp"
#include "Data/Constants.hpp"
#include "Data/Grid.hpp"
#include "Data/Settings.hpp"
#include "Data/Figure.hpp"

#include "core/calc.hpp"
#include "core/random.hpp"
#include "figure/enemy_army.hpp"
#include "figure/formation.hpp"

static const int enemyAttackBuildingPriority[4][24] =
{
    {
        BUILDING_GRANARY, BUILDING_WAREHOUSE, BUILDING_MARKET,
        BUILDING_WHEAT_FARM, BUILDING_VEGETABLE_FARM, BUILDING_FRUIT_FARM,
        BUILDING_OLIVE_FARM, BUILDING_VINES_FARM, BUILDING_PIG_FARM, 0
    },
    {
        BUILDING_SENATE_UPGRADED, BUILDING_SENATE,
        BUILDING_FORUM_UPGRADED, BUILDING_FORUM, 0
    },
    {
        BUILDING_GOVERNORS_PALACE, BUILDING_GOVERNORS_VILLA, BUILDING_GOVERNORS_HOUSE,
        BUILDING_HOUSE_LUXURY_PALACE, BUILDING_HOUSE_LARGE_PALACE,
        BUILDING_HOUSE_MEDIUM_PALACE, BUILDING_HOUSE_SMALL_PALACE,
        BUILDING_HOUSE_GRAND_VILLA, BUILDING_HOUSE_LARGE_VILLA,
        BUILDING_HOUSE_MEDIUM_VILLA, BUILDING_HOUSE_SMALL_VILLA,
        BUILDING_HOUSE_GRAND_INSULA, BUILDING_HOUSE_LARGE_INSULA,
        BUILDING_HOUSE_MEDIUM_INSULA, BUILDING_HOUSE_SMALL_INSULA,
        BUILDING_HOUSE_LARGE_CASA, BUILDING_HOUSE_SMALL_CASA,
        BUILDING_HOUSE_LARGE_HOVEL, BUILDING_HOUSE_SMALL_HOVEL,
        BUILDING_HOUSE_LARGE_SHACK, BUILDING_HOUSE_SMALL_SHACK,
        BUILDING_HOUSE_LARGE_TENT, BUILDING_HOUSE_SMALL_TENT, 0
    },
    {
        BUILDING_MILITARY_ACADEMY, BUILDING_BARRACKS, 0
    }
};

static const int rioterAttackBuildingPriority[100] =
{
    79, 78, 77, 29, 28, 27, 26, 25, 85, 84, 32, 33, 98, 65, 66, 67,
    68, 69, 87, 86, 30, 31, 47, 52, 46, 48, 53, 51, 24, 23, 22, 21,
    20, 46, 48, 114, 113, 112, 111, 110, 71, 72, 70, 74, 75, 76, 60, 61,
    62, 63, 64, 34, 36, 37, 35, 94, 19, 18, 17, 16, 15, 49, 106, 107,
    109, 108, 90, 100, 101, 102, 103, 104, 105, 55, 81, 91, 92, 14, 13, 12, 11, 10, 0
};

static const int layoutOrientationLegionIndexOffsets[13][4][40] =
{
    {
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, -6, 0, 6, 0, -6, 2, 6, 2, -2, 4, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -6, 0, 6, 2, -6, 2, 6, 4, -2, 6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -6, 0, 6, 0, -6, -2, 6, -2, -4, -6, 4, -6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -6, 0, 6, -2, -6, -2, 6, -6, -4, -6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, -6, 0, 6, 0, -6, 2, 6, 2, -2, 4, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -6, 0, 6, 2, -6, 2, 6, 4, -2, 6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -6, 0, 6, 0, -6, -2, 6, -2, -4, -6, 4, -6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -6, 0, 6, -2, -6, -2, 6, -6, -4, -6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, -6, 0, 6, 0, -6, 2, 6, 2, -2, 4, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -6, 0, 6, 2, -6, 2, 6, 4, -2, 6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -6, 0, 6, 0, -6, -2, 6, -2, -4, -6, 4, -6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -6, 0, 6, -2, -6, -2, 6, -6, -4, -6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, -6, 0, 6, 0, -6, 2, 6, 2, -2, 4, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -6, 0, 6, 2, -6, 2, 6, 4, -2, 6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -6, 0, 6, 0, -6, -2, 6, -2, -4, -6, 4, -6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -6, 0, 6, -2, -6, -2, 6, -6, -4, -6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, -4, 0, 4, 0, -12, 0, 12, 0, -4, 12, 4, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -4, 0, 4, 0, -12, 0, 12, 12, -4, 12, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -4, 0, 4, 0, -12, 0, 12, 0, -4, -12, 4, -12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -4, 0, 4, 0, -12, 0, 12, -12, -4, -12, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    }
};

static void tickDecreaseLegionDamage()
{
    for (int i = 1; i < MAX_FIGURES; i++)
    {
        struct Data_Figure *f = &Data_Figures[i];
        if (f->state == FigureState_Alive && FigureIsLegion(f->type))
        {
            if (f->actionState == FigureActionState_80_SoldierAtRest)
            {
                if (f->damage)
                {
                    f->damage--;
                }
            }
        }
    }
}

static void update_direction(const formation *m, void *data)
{
    formation_update_direction(m->id, Data_Figures[m->figures[0]].direction);
}
static void tickUpdateDirection()
{
    formation_foreach_non_herd(update_direction, 0);
}

static void tickUpdateLegions()
{
    for (int i = 1; i <= MAX_LEGIONS; i++)
    {
        const formation *m = formation_get(i);
        if (m->in_use != 1 || !m->is_legion)
        {
            continue;
        }
        formation_decrease_monthly_counters(m->id);
        if (Data_CityInfo.numEnemiesInCity <= 0)
        {
            formation_clear_monthly_counters(m->id);
        }
        for (int n = 0; n < MAX_FORMATION_FIGURES; n++)
        {
            if (Data_Figures[m->figures[n]].actionState == FigureActionState_150_Attack)
            {
                formation_record_fight(m->id);
            }
        }
        if (formation_has_low_morale(m->id))
        {
            // flee back to fort
            for (int n = 0; n < MAX_FORMATION_FIGURES; n++)
            {
                struct Data_Figure *f = &Data_Figures[m->figures[n]];
                if (f->actionState != FigureActionState_150_Attack &&
                        f->actionState != FigureActionState_149_Corpse &&
                        f->actionState != FigureActionState_148_Fleeing)
                {
                    f->actionState = FigureActionState_148_Fleeing;
                    FigureRoute_remove(m->figures[n]);
                }
            }
        }
        else if (m->layout == FORMATION_MOP_UP)
        {
            if (enemy_army_total_enemy_formations() +
                    Data_CityInfo.numRiotersInCity +
                    Data_CityInfo.numAttackingNativesInCity > 0)
            {
                for (int n = 0; n < MAX_FORMATION_FIGURES; n++)
                {
                    struct Data_Figure *f = &Data_Figures[m->figures[n]];
                    if (m->figures[n] != 0 &&
                            f->actionState != FigureActionState_150_Attack &&
                            f->actionState != FigureActionState_149_Corpse)
                    {
                        f->actionState = FigureActionState_86_SoldierMoppingUp;
                    }
                }
            }
            else
            {
                formation_restore_layout(m->id);
            }
        }
    }
}

static void addRomanSoldierConcentration(int x, int y, int radius, int amount)
{
    int xMin = x - radius;
    int yMin = y - radius;
    int xMax = x + radius;
    int yMax = y + radius;
    Bound2ToMap(xMin, yMin, xMax, yMax);

    for (int yy = yMin; yy <= yMax; yy++)
    {
        for (int xx = xMin; xx <= xMax; xx++)
        {
            int gridOffset = GridOffset(xx, yy);
            Data_Grid_romanSoldierConcentration[gridOffset] += amount;
            if (Data_Grid_figureIds[gridOffset] > 0)
            {
                int type = Data_Figures[Data_Grid_figureIds[gridOffset]].type;
                if (FigureIsLegion(type))
                {
                    Data_Grid_romanSoldierConcentration[gridOffset] += 2;
                }
            }
        }
    }
}

static void calculateRomanSoldierConcentration()
{
    Grid_clearUByteGrid(Data_Grid_romanSoldierConcentration);
    for (int i = 1; i <= MAX_LEGIONS; i++)
    {
        const formation *m = formation_get(i);
        if (m->in_use != 1 || !m->is_legion)
        {
            continue;
        }
        if (m->num_figures > 0)
        {
            addRomanSoldierConcentration(m->x_home, m->y_home, 7, 1);
        }
        if (m->num_figures > 3)
        {
            addRomanSoldierConcentration(m->x_home, m->y_home, 6, 1);
        }
        if (m->num_figures > 6)
        {
            addRomanSoldierConcentration(m->x_home, m->y_home, 5, 1);
        }
        if (m->num_figures > 9)
        {
            addRomanSoldierConcentration(m->x_home, m->y_home, 4, 1);
        }
        if (m->num_figures > 12)
        {
            addRomanSoldierConcentration(m->x_home, m->y_home, 3, 1);
        }
        if (m->num_figures > 15)
        {
            addRomanSoldierConcentration(m->x_home, m->y_home, 2, 1);
        }
    }
}

static int getHighestRomanSoldierConcentration(int x, int y, int radius, int *xTile, int *yTile)
{
    int xMin = x - radius;
    int yMin = y - radius;
    int xMax = x + radius;
    int yMax = y + radius;
    Bound2ToMap(xMin, yMin, xMax, yMax);

    int maxValue = 0;
    int maxX, maxY;
    for (int yy = yMin; yy <= yMax; yy++)
    {
        for (int xx = xMin; xx <= xMax; xx++)
        {
            int gridOffset = GridOffset(xx, yy);
            if (Data_Grid_routingDistance[gridOffset] > 0 &&
                    Data_Grid_romanSoldierConcentration[gridOffset] > maxValue)
            {
                maxValue = Data_Grid_romanSoldierConcentration[gridOffset];
                maxX = xx;
                maxY = yy;
            }
        }
    }
    if (maxValue > 0)
    {
        *xTile = maxX;
        *yTile = maxY;
        return 1;
    }
    return 0;
}

static void setNativeTargetBuilding(int formationId)
{
    int minBuildingId = 0;
    int minDistance = 10000;
    for (int i = 1; i < MAX_BUILDINGS; i++)
    {
        if (!BuildingIsInUse(i))
        {
            continue;
        }
        switch (Data_Buildings[i].type)
        {
        case BUILDING_MISSION_POST:
        case BUILDING_NATIVE_HUT:
        case BUILDING_NATIVE_CROPS:
        case BUILDING_NATIVE_MEETING:
        case BUILDING_WAREHOUSE:
        case BUILDING_FORT:
            break;
        default:
        {
            int distance = calc_maximum_distance(
                               Data_CityInfo.nativeMainMeetingCenterX,
                               Data_CityInfo.nativeMainMeetingCenterY,
                               Data_Buildings[i].x, Data_Buildings[i].y);
            if (distance < minDistance)
            {
                minBuildingId = i;
                minDistance = distance;
            }
        }
        }
    }
    if (minBuildingId > 0)
    {
        formation_set_destination_building(formationId,
                                           Data_Buildings[minBuildingId].x,
                                           Data_Buildings[minBuildingId].y,
                                           minBuildingId);
    }
}

static void setEnemyTargetBuilding(const formation *m)
{
    int attack = m->attack_type;
    if (attack == FORMATION_ATTACK_RANDOM)
    {
        attack = random_byte() & 3;
    }
    int bestTypeIndex = 100;
    int buildingId = 0;
    int minDistance = 10000;
    for (int i = 1; i < MAX_BUILDINGS; i++)
    {
        struct Data_Building *b = &Data_Buildings[i];
        if (!BuildingIsInUse(i) || Data_Grid_romanSoldierConcentration[b->gridOffset])
        {
            continue;
        }
        for (int n = 0; n < 24 && n <= bestTypeIndex && enemyAttackBuildingPriority[attack][n]; n++)
        {
            if (b->type == enemyAttackBuildingPriority[attack][n])
            {
                int distance = calc_maximum_distance(m->x_home, m->y_home, b->x, b->y);
                if (n < bestTypeIndex)
                {
                    bestTypeIndex = n;
                    buildingId = i;
                    minDistance = distance;
                }
                else if (distance < minDistance)
                {
                    buildingId = i;
                    minDistance = distance;
                }
                break;
            }
        }
    }
    if (buildingId <= 0)
    {
        // no target buildings left: take rioter attack priority
        for (int i = 1; i < MAX_BUILDINGS; i++)
        {
            struct Data_Building *b = &Data_Buildings[i];
            if (!BuildingIsInUse(i) || Data_Grid_romanSoldierConcentration[b->gridOffset])
            {
                continue;
            }
            for (int n = 0; n < 100 && n <= bestTypeIndex && rioterAttackBuildingPriority[n]; n++)
            {
                if (b->type == rioterAttackBuildingPriority[n])
                {
                    int distance = calc_maximum_distance(m->x_home, m->y_home, b->x, b->y);
                    if (n < bestTypeIndex)
                    {
                        bestTypeIndex = n;
                        buildingId = i;
                        minDistance = distance;
                    }
                    else if (distance < minDistance)
                    {
                        buildingId = i;
                        minDistance = distance;
                    }
                    break;
                }
            }
        }
    }
    if (buildingId <= 0)
    {
        return;
    }
    struct Data_Building *b = &Data_Buildings[buildingId];
    if (b->type == BUILDING_WAREHOUSE)
    {
        formation_set_destination_building(m->id, b->x + 1, b->y, buildingId + 1);
    }
    else
    {
        formation_set_destination_building(m->id, b->x, b->y, buildingId);
    }
}

static void enemyApproachTarget(const formation *m)
{
    if (Routing_canTravelOverLandNonCitizen(m->x_home, m->y_home,
                                            m->destination_x, m->destination_y, m->destination_building_id, 400) ||
            Routing_canTravelThroughEverythingNonCitizen(m->x_home, m->y_home,
                    m->destination_x, m->destination_y))
    {
        int xTile, yTile;
        if (Routing_getClosestXYWithinRange(8, m->x_home, m->y_home,
                                            m->destination_x, m->destination_y, 20, &xTile, &yTile))
        {
            formation_set_destination(m->id, xTile, yTile);
        }
    }
}

static void marsKillEnemies()
{
    if (Data_CityInfo.godBlessingMarsEnemiesToKill <= 0)
    {
        return;
    }
    int toKill = Data_CityInfo.godBlessingMarsEnemiesToKill;
    int gridOffset = 0;
    for (int i = 1; i < MAX_FIGURES && toKill > 0; i++)
    {
        struct Data_Figure *f = &Data_Figures[i];
        if (f->state != FigureState_Alive)
        {
            continue;
        }
        if (FigureIsEnemy(f->type) && f->type != FIGURE_ENEMY54_GLADIATOR)
        {
            f->actionState = FigureActionState_149_Corpse;
            toKill--;
            if (!gridOffset)
            {
                gridOffset = f->gridOffset;
            }
        }
    }
    Data_CityInfo.godBlessingMarsEnemiesToKill = 0;
    PlayerMessage_post(1, Message_105_SpiritOfMars, 0, gridOffset);
}

static void setFormationFiguresToEnemyInitial(int formationId)
{
    const formation *m = formation_get(formationId);
    for (int i = 0; i < MAX_FORMATION_FIGURES; i++)
    {
        if (m->figures[i] > 0)
        {
            struct Data_Figure *f = &Data_Figures[m->figures[i]];
            if (f->actionState != FigureActionState_149_Corpse &&
                    f->actionState != FigureActionState_150_Attack)
            {
                f->actionState = FigureActionState_151_EnemyInitial;
                f->waitTicks = 0;
            }
        }
    }
}

static void updateEnemyMovement(const formation *m, int romanDistance)
{
    const enemy_army *army = enemy_army_get(m->invasion_id);
    formation_state *state = formation_get_state(m->id);
    int regroup = 0;
    int halt = 0;
    int pursueTarget = 0;
    int advance = 0;
    int targetFormationId = 0;
    if (m->missile_fired)
    {
        halt = 1;
    }
    else if (m->missile_attack_timeout)
    {
        pursueTarget = 1;
        targetFormationId = m->missile_attack_formation_id;
    }
    else if (m->wait_ticks < 32)
    {
        regroup = 1;
        state->duration_advance = 4;
    }
    else if (army->ignore_roman_soldiers)
    {
        halt = 0;
        regroup = 0;
        advance = 1;
    }
    else
    {
        int haltDuration, advanceDuration, regroupDuration;
        if (army->layout == FORMATION_ENEMY8 || army->layout == FORMATION_ENEMY12)
        {
            switch (m->enemy_legion_index)
            {
            case 0:
            case 1:
                regroupDuration = 2;
                advanceDuration = 4;
                haltDuration = 2;
                break;
            case 2:
            case 3:
                regroupDuration = 2;
                advanceDuration = 5;
                haltDuration = 3;
                break;
            default:
                regroupDuration = 2;
                advanceDuration = 6;
                haltDuration = 4;
                break;
            }
            if (!romanDistance)
            {
                advanceDuration += 6;
                haltDuration--;
                regroupDuration--;
            }
        }
        else
        {
            if (romanDistance)
            {
                regroupDuration = 6;
                advanceDuration = 4;
                haltDuration = 2;
            }
            else
            {
                regroupDuration = 1;
                advanceDuration = 12;
                haltDuration = 1;
            }
        }
        if (state->duration_halt)
        {
            state->duration_advance = 0;
            state->duration_regroup = 0;
            halt = 1;
            state->duration_halt--;
            if (state->duration_halt <= 0)
            {
                state->duration_regroup = regroupDuration;
                setFormationFiguresToEnemyInitial(m->id);
                regroup = 0;
                halt = 1;
            }
        }
        else if (state->duration_regroup)
        {
            state->duration_advance = 0;
            state->duration_halt = 0;
            regroup = 1;
            state->duration_regroup--;
            if (state->duration_regroup <= 0)
            {
                state->duration_advance = advanceDuration;
                setFormationFiguresToEnemyInitial(m->id);
                advance = 1;
                regroup = 0;
            }
        }
        else
        {
            state->duration_regroup = 0;
            state->duration_halt = 0;
            advance = 1;
            state->duration_advance--;
            if (state->duration_advance <= 0)
            {
                state->duration_halt = haltDuration;
                setFormationFiguresToEnemyInitial(m->id);
                halt = 1;
                advance = 0;
            }
        }
    }

    if (m->wait_ticks > 32)
    {
        marsKillEnemies();
    }
    if (halt)
    {
        formation_set_destination(m->id, m->x_home, m->y_home);
    }
    else if (pursueTarget)
    {
        if (targetFormationId > 0)
        {
            const formation *target = formation_get(targetFormationId);
            if (target->num_figures > 0)
            {
                formation_set_destination(m->id, target->x_home, target->y_home);
            }
        }
        else
        {
            formation_set_destination(m->id, army->destination_x, army->destination_y);
        }
    }
    else if (regroup)
    {
        int layout = army->layout;
        int xOffset = layoutOrientationLegionIndexOffsets[layout][m->orientation / 2][2 * m->enemy_legion_index] +
                      army->home_x;
        int yOffset = layoutOrientationLegionIndexOffsets[layout][m->orientation / 2][2 * m->enemy_legion_index + 1] +
                      army->home_y;
        int xTile, yTile;
        if (FigureAction_HerdEnemy_moveFormationTo(m->id, xOffset, yOffset, &xTile, &yTile))
        {
            formation_set_destination(m->id, xTile, yTile);
        }
    }
    else if (advance)
    {
        int layout = army->layout;
        int xOffset = layoutOrientationLegionIndexOffsets[layout][m->orientation / 2][2 * m->enemy_legion_index] +
                      army->destination_x;
        int yOffset = layoutOrientationLegionIndexOffsets[layout][m->orientation / 2][2 * m->enemy_legion_index + 1] +
                      army->destination_y;
        int xTile, yTile;
        if (FigureAction_HerdEnemy_moveFormationTo(m->id, xOffset, yOffset, &xTile, &yTile))
        {
            formation_set_destination(m->id, xTile, yTile);
        }
    }
}

static void update_enemy_formation(const formation *m, void *data)
{
    if (m->is_legion)
    {
        return;
    }
    int *romanDistance = (int*) data;
    enemy_army *army = enemy_army_get_editable(m->invasion_id);
    if (enemy_army_is_stronger_than_legions())
    {
        if (m->figure_type != FIGURE_FORT_JAVELIN)
        {
            army->ignore_roman_soldiers = 1;
        }
    }
    formation_decrease_monthly_counters(m->id);
    if (Data_CityInfo.numSoldiersInCity <= 0)
    {
        formation_clear_monthly_counters(m->id);
    }
    for (int n = 0; n < MAX_FORMATION_FIGURES; n++)
    {
        int figureId = m->figures[n];
        if (Data_Figures[figureId].actionState == FigureActionState_150_Attack)
        {
            int opponentId = Data_Figures[figureId].opponentId;
            if (!FigureIsDead(opponentId) && FigureIsLegion(Data_Figures[opponentId].type))
            {
                formation_record_fight(m->id);
            }
        }
    }
    if (formation_has_low_morale(m->id))
    {
        for (int n = 0; n < MAX_FORMATION_FIGURES; n++)
        {
            struct Data_Figure *f = &Data_Figures[m->figures[n]];
            if (f->actionState != FigureActionState_150_Attack &&
                    f->actionState != FigureActionState_149_Corpse &&
                    f->actionState != FigureActionState_148_Fleeing)
            {
                f->actionState = FigureActionState_148_Fleeing;
                FigureRoute_remove(m->figures[n]);
            }
        }
        return;
    }
    if (m->figures[0] && Data_Figures[m->figures[0]].state == FigureState_Alive)
    {
        formation_set_home(m->id, Data_Figures[m->figures[0]].x, Data_Figures[m->figures[0]].y);
    }
    if (!army->formation_id)
    {
        army->formation_id = m->id;
        army->home_x = m->x_home;
        army->home_y = m->y_home;
        army->layout = m->layout;
        *romanDistance = 0;
        Routing_canTravelOverLandNonCitizen(m->x_home, m->y_home, -2, -2, 100000, 300);
        int xTile, yTile;
        if (getHighestRomanSoldierConcentration(m->x_home, m->y_home, 16, &xTile, &yTile))
        {
            *romanDistance = 1;
        }
        else if (getHighestRomanSoldierConcentration(m->x_home, m->y_home, 32, &xTile, &yTile))
        {
            *romanDistance = 2;
        }
        if (army->ignore_roman_soldiers)
        {
            *romanDistance = 0;
        }
        if (*romanDistance == 1)
        {
            // attack roman legion
            army->destination_x = xTile;
            army->destination_y = yTile;
            army->destination_building_id = 0;
        }
        else
        {
            setEnemyTargetBuilding(m);
            enemyApproachTarget(m);
            army->destination_x = m->destination_x;
            army->destination_y = m->destination_y;
            army->destination_building_id = m->destination_building_id;
        }
    }
    formation_set_enemy_legion(m->id, army->num_legions++);
    formation_increase_wait_ticks(m->id);
    formation_set_destination_building(m->id,
                                       army->destination_x, army->destination_y, army->destination_building_id
                                      );

    updateEnemyMovement(m, *romanDistance);
}
static void tickUpdateEnemies()
{
    if (enemy_army_total_enemy_formations() <= 0)
    {
        enemy_armies_clear_ignore_roman_soldiers();
    }
    else
    {
        if (enemy_army_totals_should_calculate_roman_influence())
        {
            calculateRomanSoldierConcentration();
        }
        enemy_armies_clear_formations();
        int romanDistance = 0;
        formation_foreach_non_herd(update_enemy_formation, &romanDistance);
    }
    setNativeTargetBuilding(0);
}

static int getHerdRoamingDestination(int formationId, int allowNegativeDesirability,
                                     int x, int y, int distance, int direction, int *xTile, int *yTile)
{
    int targetDirection = (formationId + random_byte()) & 6;
    if (direction)
    {
        targetDirection = direction;
        allowNegativeDesirability = 1;
    }
    for (int i = 0; i < 4; i++)
    {
        int xTarget, yTarget;
        switch (targetDirection)
        {
        case Dir_0_Top:
            xTarget = x;
            yTarget = y - distance;
            break;
        case Dir_1_TopRight:
            xTarget = x + distance;
            yTarget = y - distance;
            break;
        case Dir_2_Right:
            xTarget = x + distance;
            yTarget = y;
            break;
        case Dir_3_BottomRight:
            xTarget = x + distance;
            yTarget = y + distance;
            break;
        case Dir_4_Bottom:
            xTarget = x;
            yTarget = y + distance;
            break;
        case Dir_5_BottomLeft:
            xTarget = x - distance;
            yTarget = y + distance;
            break;
        case Dir_6_Left:
            xTarget = x - distance;
            yTarget = y;
            break;
        case Dir_7_TopLeft:
            xTarget = x - distance;
            yTarget = y - distance;
            break;
        default:
            continue;
        }
        if (xTarget <= 0)
        {
            xTarget = 1;
        }
        else if (yTarget <= 0)
        {
            yTarget = 1;
        }
        else if (xTarget >= Data_Settings_Map.width - 1)
        {
            xTarget = Data_Settings_Map.width - 2;
        }
        else if (yTarget >= Data_Settings_Map.height - 1)
        {
            yTarget = Data_Settings_Map.height - 2;
        }
        if (TerrainGraphics_getFreeTileForHerd(xTarget, yTarget, allowNegativeDesirability, xTile, yTile))
        {
            return 1;
        }
        targetDirection += 2;
        if (targetDirection > 6)
        {
            targetDirection = 0;
        }
    }
    return 0;
}

static void moveAnimals(const formation *m, int attackingAnimals)
{
    for (int i = 0; i < MAX_FORMATION_FIGURES; i++)
    {
        if (m->figures[i] <= 0) continue;
        int figureId = m->figures[i];
        struct Data_Figure *f = &Data_Figures[figureId];
        if (f->actionState == FigureActionState_149_Corpse ||
                f->actionState == FigureActionState_150_Attack)
        {
            continue;
        }
        f->waitTicks = 401;
        if (attackingAnimals)
        {
            int targetId = FigureAction_CombatWolf_getTarget(f->x, f->y, 6);
            if (targetId)
            {
                f->actionState = FigureActionState_199_WolfAttacking;
                f->destinationX = Data_Figures[targetId].x;
                f->destinationY = Data_Figures[targetId].y;
                f->targetFigureId = targetId;
                Data_Figures[targetId].targetedByFigureId = figureId;
                f->targetFigureCreatedSequence = Data_Figures[targetId].createdSequence;
                FigureRoute_remove(figureId);
            }
            else
            {
                f->actionState = FigureActionState_196_HerdAnimalAtRest;
            }
        }
        else
        {
            f->actionState = FigureActionState_196_HerdAnimalAtRest;
        }
    }
}

static void update_herd_formation(const formation *m)
{
    if (formation_can_spawn_wolf(m->id))
    {
        // spawn new wolf
        if (!(Data_Grid_terrain[GridOffset(m->x, m->y)] & Terrain_d73f))
        {
            int wolfId = Figure_create(m->figure_type, m->x, m->y, Dir_0_Top);
            Data_Figures[wolfId].actionState = FigureActionState_196_HerdAnimalAtRest;
            Data_Figures[wolfId].formationId = m->id;
            Data_Figures[wolfId].waitTicks = wolfId & 0x1f;
        }
    }
    int attackingAnimals = 0;
    for (int fig = 0; fig < MAX_FORMATION_FIGURES; fig++)
    {
        int figureId = m->figures[fig];
        if (figureId > 0 && Data_Figures[figureId].actionState == FigureActionState_150_Attack)
        {
            attackingAnimals++;
        }
    }
    if (m->missile_attack_timeout)
    {
        attackingAnimals = 1;
    }
    if (m->figures[0] && Data_Figures[m->figures[0]].state == FigureState_Alive)
    {
        formation_set_home(m->id, Data_Figures[m->figures[0]].x, Data_Figures[m->figures[0]].y);
    }
    int roamDistance;
    int roamDelay;
    int allowNegativeDesirability;
    switch (m->figure_type)
    {
    case FIGURE_SHEEP:
        roamDistance = 8;
        roamDelay = 20;
        allowNegativeDesirability = 0;
        attackingAnimals = 0;
        break;
    case FIGURE_ZEBRA:
        roamDistance = 20;
        roamDelay = 4;
        allowNegativeDesirability = 0;
        attackingAnimals = 0;
        break;
    case FIGURE_WOLF:
        roamDistance = 16;
        roamDelay = 6;
        allowNegativeDesirability = 1;
        break;
    default:
        return;
    }
    formation_increase_wait_ticks(m->id);
    if (m->wait_ticks > roamDelay || attackingAnimals)
    {
        formation_reset_wait_ticks(m->id);
        if (attackingAnimals)
        {
            formation_set_destination(m->id, m->x_home, m->y_home);
            moveAnimals(m, attackingAnimals);
        }
        else
        {
            int xTile, yTile;
            if (getHerdRoamingDestination(m->id, allowNegativeDesirability, m->x_home, m->y_home, roamDistance, m->herd_direction, &xTile, &yTile))
            {
                formation_herd_clear_direction(m->id);
                if (FigureAction_HerdEnemy_moveFormationTo(m->id, xTile, yTile, &xTile, &yTile))
                {
                    formation_set_destination(m->id, xTile, yTile);
                    if (m->figure_type == FIGURE_WOLF)
                    {
                        Data_CityInfo.soundMarchWolf--;
                        if (Data_CityInfo.soundMarchWolf <= 0)
                        {
                            Data_CityInfo.soundMarchWolf = 12;
                            Sound_Effects_playChannel(SoundChannel_WolfHowl);
                        }
                    }
                    moveAnimals(m, attackingAnimals);
                }
            }
        }
    }
}
static void tickUpdateHerds()
{
    if (Data_CityInfo.numAnimalsInCity <= 0)
    {
        return;
    }
    formation_foreach_herd(update_herd_formation);
}

void Formation_Tick_updateAll(int secondTime)
{
    Formation_calculateLegionTotals();
    Formation_calculateFigures();
    tickUpdateDirection();
    tickDecreaseLegionDamage();
    if (!secondTime)
    {
        formation_update_monthly_morale_deployed();
    }
    formation_legion_set_max_figures();
    tickUpdateLegions();
    tickUpdateEnemies();
    tickUpdateHerds();
}

int Formation_Rioter_getTargetBuilding(int *xTile, int *yTile)
{
    int bestTypeIndex = 100;
    int buildingId = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++)
    {
        if (!BuildingIsInUse(i))
        {
            continue;
        }
        int type = Data_Buildings[i].type;
        for (int b = 0; b < 100 && b <= bestTypeIndex && rioterAttackBuildingPriority[b]; b++)
        {
            if (type == rioterAttackBuildingPriority[b])
            {
                if (b < bestTypeIndex)
                {
                    bestTypeIndex = b;
                    buildingId = i;
                }
                break;
            }
        }
    }
    if (buildingId <= 0)
    {
        return 0;
    }
    struct Data_Building *b = &Data_Buildings[buildingId];
    if (b->type == BUILDING_WAREHOUSE)
    {
        *xTile = b->x + 1;
        *yTile = b->y;
        return buildingId + 1;
    }
    else
    {
        *xTile = b->x;
        *yTile = b->y;
        return buildingId;
    }
}
