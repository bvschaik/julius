#include "barracks.h"

#include "building/count.h"
#include "building/model.h"
#include "city/buildings.h"
#include "city/military.h"
#include "city/resource.h"
#include "core/calc.h"
#include "figure/action.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "map/grid.h"
#include "map/road_access.h"

static int tower_sentry_request = 0;

int building_get_barracks_for_weapon(int resource, int road_network_id, int *x_dst, int *y_dst)
{
    if (resource != RESOURCE_WEAPONS) {
        return 0;
    }
    if (city_resource_is_stockpiled(RESOURCE_WEAPONS)) {
        return 0;
    }
    if (building_count_active(BUILDING_BARRACKS) <= 0) {
        return 0;
    }
    building *b = building_get(city_buildings_get_barracks());
    if (b->loadsStored < 5 && city_military_has_legionary_legions()) {
        if (map_has_road_access(b->x, b->y, b->size, x_dst, y_dst) && b->roadNetworkId == road_network_id) {
            return b->id;
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

static int get_closest_legion_needing_soldiers(const building *barracks)
{
    int recruit_type = LEGION_RECRUIT_NONE;
    int min_formation_id = 0;
    int min_distance = 10000;
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = formation_get(i);
        if (!m->in_use || !m->is_legion) {
            continue;
        }
        if (m->in_distant_battle || m->legion_recruit_type == LEGION_RECRUIT_NONE) {
            continue;
        }
        if (m->legion_recruit_type == LEGION_RECRUIT_LEGIONARY && barracks->loadsStored <= 0) {
            continue;
        }
        building *fort = building_get(m->building_id);
        int dist = calc_maximum_distance(barracks->x, barracks->y, fort->x, fort->y);
        if (m->legion_recruit_type > recruit_type ||
            (m->legion_recruit_type == recruit_type && dist < min_distance)) {
            recruit_type = m->legion_recruit_type;
            min_formation_id = m->id;
            min_distance = dist;
        }
    }
    return min_formation_id;
}

static int get_closest_military_academy(const building *fort)
{
    int min_building_id = 0;
    int min_distance = 10000;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_MILITARY_ACADEMY &&
            b->numWorkers >= model_get_building(BUILDING_MILITARY_ACADEMY)->laborers) {
            int dist = calc_maximum_distance(fort->x, fort->y, b->x, b->y);
            if (dist < min_distance) {
                min_distance = dist;
                min_building_id = i;
            }
        }
    }
    return min_building_id;
}

int building_barracks_create_soldier(building *barracks, int x, int y)
{
    int formation_id = get_closest_legion_needing_soldiers(barracks);
    if (formation_id > 0) {
        const formation *m = formation_get(formation_id);
        figure *f = figure_create(m->figure_type, x, y, DIR_0_TOP);
        f->formationId = formation_id;
        f->formationAtRest = 1;
        if (m->figure_type == FIGURE_FORT_LEGIONARY) {
            if (barracks->loadsStored > 0) {
                barracks->loadsStored--;
            }
        }
        int academy_id = get_closest_military_academy(building_get(m->building_id));
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
    formation_calculate_figures();
    return formation_id ? 1 : 0;
}

int building_barracks_create_tower_sentry(building *barracks, int x, int y)
{
    if (tower_sentry_request <= 0) {
        return 0;
    }
    building *tower = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_TOWER && b->numWorkers > 0 &&
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
        f->state = FIGURE_STATE_DEAD;
    }
    tower->figureId = f->id;
    f->buildingId = tower->id;
    return 1;
}

void building_barracks_request_tower_sentry()
{
    tower_sentry_request = 2;
}

void building_barracks_decay_tower_sentry_request()
{
    if (tower_sentry_request > 0) {
        tower_sentry_request--;
    }
}

int building_barracks_has_tower_sentry_request()
{
    return tower_sentry_request;
}

void building_barracks_save_state(buffer *buf)
{
    buffer_write_i32(buf, tower_sentry_request);
}

void building_barracks_load_state(buffer *buf)
{
    tower_sentry_request = buffer_read_i32(buf);
}
