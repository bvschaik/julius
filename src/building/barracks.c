#include "barracks.h"

#include "building/count.h"
#include "building/model.h"
#include "building/monument.h"
#include "core/config.h"
#include "city/buildings.h"
#include "city/military.h"
#include "city/resource.h"
#include "core/calc.h"
#include "figure/action.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "map/grid.h"
#include "map/road_access.h"

#define INFINITE 10000

static int is_valid_destination(building *b, int road_network_id)
{
    return b->state == BUILDING_STATE_IN_USE && map_has_road_access(b->x, b->y, b->size, 0) &&
        b->distance_from_entry > 0 && b->road_network_id == road_network_id &&
        b->loads_stored < MAX_WEAPONS_BARRACKS;
}

int building_get_barracks_for_weapon(int x, int y, int resource, int road_network_id,
    int distance_from_entry, map_point *dst)
{
    if (resource != RESOURCE_WEAPONS) {
        return 0;
    }
    if (city_resource_is_stockpiled(resource)) {
        return 0;
    }
    int min_dist = INFINITE;
    building *min_building = 0;
    for (building *b = building_first_of_type(BUILDING_BARRACKS); b; b = b->next_of_type) {
        if (!is_valid_destination(b, road_network_id)) {
            continue;
        }
        int dist = calc_maximum_distance(b->x, b->y, x, y);
        dist += 8 * b->loads_stored;
        if (dist < min_dist) {
            min_dist = dist;
            min_building = b;
        }
    }
    building *monument = building_first_of_type(BUILDING_GRAND_TEMPLE_MARS);
    if (monument && monument->data.monument.phase == MONUMENT_FINISHED &&
        is_valid_destination(monument, road_network_id)) {
        int dist = calc_maximum_distance(monument->x, monument->y, x, y);
        dist += 8 * monument->loads_stored;
        if (dist < min_dist) {
            min_dist = dist;
            min_building = monument;
        }
    }
    if (min_building && min_dist < INFINITE) {
        if (dst) {
            map_point_store_result(min_building->road_access_x, min_building->road_access_y, dst);
        }
        return min_building->id;
    }
    return 0;
}

void building_barracks_add_weapon(building *barracks)
{
    if (barracks->id > 0) {
        barracks->loads_stored++;
    }
}

static int get_closest_legion_needing_soldiers(const building *barracks)
{
    int recruit_type = LEGION_RECRUIT_NONE;
    int min_formation_id = 0;
    int min_distance = INFINITE;
    for (int i = 1; i < formation_count(); i++) {
        formation *m = formation_get(i);
        if (!m->in_use || !m->is_legion) {
            continue;
        }
        if (m->in_distant_battle || m->legion_recruit_type == LEGION_RECRUIT_NONE) {
            continue;
        }
        if (m->legion_recruit_type == LEGION_RECRUIT_LEGIONARY && barracks->loads_stored <= 0) {
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
    int min_distance = INFINITE;
    for (building *b = building_first_of_type(BUILDING_MILITARY_ACADEMY); b; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE &&
            b->num_workers >= model_get_building(BUILDING_MILITARY_ACADEMY)->laborers) {
            int dist = calc_maximum_distance(fort->x, fort->y, b->x, b->y);
            if (dist < min_distance) {
                min_distance = dist;
                min_building_id = b->id;
            }
        }
    }
    return min_building_id;
}

int building_barracks_create_soldier(building *barracks, int x, int y)
{
    int formation_id = get_closest_legion_needing_soldiers(barracks);
    if (formation_id > 0) {
        formation *m = formation_get(formation_id);
        figure *f = figure_create(m->figure_type, x, y, DIR_0_TOP);
        f->formation_id = formation_id;
        f->formation_at_rest = 1;
        if (m->figure_type == FIGURE_FORT_LEGIONARY) {
            if (barracks->loads_stored > 0) {
                barracks->loads_stored--;
            }
        }
        int academy_id = get_closest_military_academy(building_get(m->building_id));
        if (academy_id) {
            map_point road;
            building *academy = building_get(academy_id);
            if (map_has_road_access(academy->x, academy->y, academy->size, &road)) {
                f->action_state = FIGURE_ACTION_85_SOLDIER_GOING_TO_MILITARY_ACADEMY;
                f->destination_x = road.x;
                f->destination_y = road.y;
                f->destination_grid_offset = map_grid_offset(f->destination_x, f->destination_y);
            } else {
                f->action_state = FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT;
            }
        } else {
            f->action_state = FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT;
        }
        if (m->num_figures == MAX_FORMATION_FIGURES - 1) {
            m->legion_recruit_type = LEGION_RECRUIT_NONE;
        }
    }
    formation_calculate_figures();
    return formation_id ? 1 : 0;
}

static building *get_unmanned_tower_of_type(building_type type, building *barracks, map_point *road)
{
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE && b->num_workers > 0 &&
            !b->figure_id && !b->figure_id4 && (b->road_network_id == barracks->road_network_id || config_get(CONFIG_GP_CH_TOWER_SENTRIES_GO_OFFROAD))) {
            if (map_has_road_access(b->x, b->y, b->size, road)) {
                return b;
            }
        }
    }
    return 0;
}

building *building_barracks_get_unmanned_tower(building *barracks, map_point *road)
{
    building *tower = get_unmanned_tower_of_type(BUILDING_TOWER, barracks, road);
    if (tower) {
        return tower;
    }
    tower = get_unmanned_tower_of_type(BUILDING_WATCHTOWER, barracks, road);
    return tower;
}

int building_barracks_create_tower_sentry(building *barracks, int x, int y)
{
    map_point road;
    building *tower = building_barracks_get_unmanned_tower(barracks, &road);
    if (!tower) {
        return 0;
    }
    figure *f = figure_create(FIGURE_TOWER_SENTRY, x, y, DIR_0_TOP);
    f->action_state = FIGURE_ACTION_174_TOWER_SENTRY_GOING_TO_TOWER;
    if (map_has_road_access(tower->x, tower->y, tower->size, &road)) {
        f->destination_x = road.x;
        f->destination_y = road.y;
    } else {
        f->state = FIGURE_STATE_DEAD;
    }
    tower->figure_id = f->id;
    f->building_id = tower->id;
    return 1;
}

void building_barracks_toggle_priority(building *barracks)
{
    barracks->subtype.barracks_priority = 1 - barracks->subtype.barracks_priority;
}

int building_barracks_get_priority(building *barracks)
{
    return barracks->subtype.barracks_priority;
}
