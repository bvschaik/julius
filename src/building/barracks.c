#include "barracks.h"

#include "building/count.h"
#include "core/calc.h"
#include "figure/action.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "game/resource.h"
#include "map/grid.h"
#include "map/road_access.h"

#include "Data/CityInfo.h"
#include "../Formation.h"

int building_get_barracks_for_weapon(int resource, int road_network_id, int *x_dst, int *y_dst)
{
    if (resource != RESOURCE_WEAPONS) {
        return 0;
    }
    if (Data_CityInfo.resourceStockpiled[RESOURCE_WEAPONS]) {
        return 0;
    }
    if (building_count_active(BUILDING_BARRACKS) <= 0) {
        return 0;
    }
    building *b = building_get(Data_CityInfo.buildingBarracksBuildingId);
    if (b->loadsStored < 5 && Data_CityInfo.militaryLegionaryLegions > 0) {
        if (map_has_road_access(b->x, b->y, b->size, x_dst, y_dst) && b->roadNetworkId == road_network_id) {
            return Data_CityInfo.buildingBarracksBuildingId;
        }
    }
    return 0;
}

void building_barracks_add_weapon(building *barracks)
{
    if (barracks->id > 0) {
        barracks->loadsStored++;
    }
}

struct fort_state {
    const int building_id;
    const int has_weapons;
    int recruit_type;
    int formation_id;
    int min_distance;
};

static void get_closest_fort_needing_soldiers(const formation *formation, void *data)
{
    struct fort_state *state = (struct fort_state*) data;
    if (formation->in_distant_battle || !formation->legion_recruit_type) {
        return;
    }
    if (formation->legion_recruit_type == LEGION_RECRUIT_LEGIONARY && !state->has_weapons) {
        return;
    }
    building *barracks = building_get(state->building_id);
    building *fort = building_get(formation->building_id);
    int dist = calc_maximum_distance(barracks->x, barracks->y, fort->x, fort->y);
    if (formation->legion_recruit_type > state->recruit_type ||
        (formation->legion_recruit_type == state->recruit_type && dist < state->min_distance)) {
        state->recruit_type = formation->legion_recruit_type;
        state->formation_id = formation->id;
        state->min_distance = dist;
    }
}

int building_barracks_create_soldier(building *barracks, int x, int y)
{
    struct fort_state state = {barracks->id, barracks->loadsStored > 0, 0, 0, 10000};
    formation_foreach_legion(get_closest_fort_needing_soldiers, &state);
    if (state.formation_id > 0) {
        const formation *m = formation_get(state.formation_id);
        figure *f = figure_create(m->figure_type, x, y, DIR_0_TOP);
        f->formationId = state.formation_id;
        f->formationAtRest = 1;
        if (m->figure_type == FIGURE_FORT_LEGIONARY) {
            if (barracks->loadsStored > 0) {
                barracks->loadsStored--;
            }
        }
        int academy_id = Formation_getClosestMilitaryAcademy(state.formation_id);
        if (academy_id) {
            int x_road, y_road;
            building *academy = building_get(academy_id);
            if (map_has_road_access(academy->x, academy->y, academy->size, &x_road, &y_road)) {
                f->actionState = FIGURE_ACTION_85_SOLDIER_GOING_TO_MILITARY_ACADEMY;
                f->destinationX = x_road;
                f->destinationY = y_road;
                f->destinationGridOffsetSoldier = map_grid_offset(f->destinationX, f->destinationY);
            } else {
                f->actionState = FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT;
            }
        } else {
            f->actionState = FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT;
        }
    }
    Formation_calculateFigures();
    return state.formation_id ? 1 : 0;
}

int building_barracks_create_tower_sentry(building *barracks, int x, int y)
{
    if (Data_Buildings_Extra.barracksTowerSentryRequested <= 0) {
        return 0;
    }
    building *tower = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (BuildingIsInUse(b) && b->type == BUILDING_TOWER && b->numWorkers > 0 &&
            !b->figureId && b->roadNetworkId == barracks->roadNetworkId) {
            tower = b;
            break;
        }
    }
    if (!tower) {
        return 0;
    }
    figure *f = figure_create(FIGURE_TOWER_SENTRY, x, y, DIR_0_TOP);
    f->actionState = FIGURE_ACTION_174_TOWER_SENTRY_GOING_TO_TOWER;
    int x_road, y_road;
    if (map_has_road_access(tower->x, tower->y, tower->size, &x_road, &y_road)) {
        f->destinationX = x_road;
        f->destinationY = y_road;
    } else {
        f->state = FigureState_Dead;
    }
    tower->figureId = f->id;
    f->buildingId = tower->id;
    return 1;
}
