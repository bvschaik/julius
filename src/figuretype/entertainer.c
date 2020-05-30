#include "entertainer.h"

#include "building/building.h"
#include "building/list.h"
#include "core/calc.h"
#include "core/image.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "map/road_network.h"
#include "scenario/gladiator_revolt.h"

static int determine_destination(int x, int y, building_type type1, building_type type2)
{
    int road_network = map_road_network_get(map_grid_offset(x,y));

    building_list_small_clear();

    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->type != type1 && b->type != type2) {
            continue;
        }
        if (b->distance_from_entry && b->road_network_id == road_network) {
            if (b->type == BUILDING_HIPPODROME && b->prev_part_building_id) {
                continue;
            }
            building_list_small_add(i);
        }
    }
    int total_venues = building_list_small_size();
    if (total_venues <= 0) {
        return 0;
    }
    const int *venues = building_list_small_items();
    int min_building_id = 0;
    int min_distance = 10000;
    for (int i = 0; i < total_venues; i++) {
        building *b = building_get(venues[i]);
        int days_left;
        if (b->type == type1) {
            days_left = b->data.entertainment.days1;
        } else if (b->type == type2) {
            days_left = b->data.entertainment.days2;
        } else {
            days_left = 0;
        }
        int dist = days_left + calc_maximum_distance(x, y, b->x, b->y);
        if (dist < min_distance) {
            min_distance = dist;
            min_building_id = venues[i];
        }
    }
    return min_building_id;
}

static void update_shows(figure *f)
{
    building *b = building_get(f->destination_building_id);
    switch (f->type) {
        case FIGURE_ACTOR:
            b->data.entertainment.play++;
            if (b->data.entertainment.play >= 5) {
                b->data.entertainment.play = 0;
            }
            if (b->type == BUILDING_THEATER) {
                b->data.entertainment.days1 = 32;
            } else {
                b->data.entertainment.days2 = 32;
            }
            break;
        case FIGURE_GLADIATOR:
            if (b->type == BUILDING_AMPHITHEATER) {
                b->data.entertainment.days1 = 32;
            } else {
                b->data.entertainment.days2 = 32;
            }
            break;
        case FIGURE_LION_TAMER:
        case FIGURE_CHARIOTEER:
            b->data.entertainment.days1 = 32;
            break;
    }
}

static void update_image(figure *f)
{
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previous_tile_direction);

    if (f->type == FIGURE_CHARIOTEER) {
        f->cart_image_id = 0;
        if (f->action_state == FIGURE_ACTION_150_ATTACK ||
            f->action_state == FIGURE_ACTION_149_CORPSE) {
            f->image_id = image_group(GROUP_FIGURE_CHARIOTEER) + dir;
        } else {
            f->image_id = image_group(GROUP_FIGURE_CHARIOTEER) +
                dir + 8 * f->image_offset;
        }
        return;
    }
    int image_id;
    if (f->type == FIGURE_ACTOR) {
        image_id = image_group(GROUP_FIGURE_ACTOR);
    } else if (f->type == FIGURE_GLADIATOR) {
        image_id = image_group(GROUP_FIGURE_GLADIATOR);
    } else if (f->type == FIGURE_LION_TAMER) {
        image_id = image_group(GROUP_FIGURE_LION_TAMER);
        if (f->wait_ticks_missile >= 96) {
            image_id = image_group(GROUP_FIGURE_LION_TAMER_WHIP);
        }
        f->cart_image_id = image_group(GROUP_FIGURE_LION);
    } else {
        return;
    }
    if (f->action_state == FIGURE_ACTION_150_ATTACK) {
        if (f->type == FIGURE_GLADIATOR) {
            f->image_id = image_id + 104 + dir + 8 * (f->image_offset / 2);
        } else {
            f->image_id = image_id + dir;
        }
    } else if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_id + 96 + figure_image_corpse_offset(f);
        f->cart_image_id = 0;
    } else {
        f->image_id = image_id + dir + 8 * f->image_offset;
    }
    if (f->cart_image_id) {
        f->cart_image_id += dir + 8 * f->image_offset;
        figure_image_set_cart_offset(f, dir);
    }
}

void figure_entertainer_action(figure *f)
{
    building *b = building_get(f->building_id);
    f->cart_image_id = image_group(GROUP_FIGURE_CARTPUSHER_CART);
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = 512;
    figure_image_increase_offset(f, 12);
    f->wait_ticks_missile++;
    if (f->wait_ticks_missile >= 120) {
        f->wait_ticks_missile = 0;
    }
    if (scenario_gladiator_revolt_is_in_progress() && f->type == FIGURE_GLADIATOR) {
        if (f->action_state == FIGURE_ACTION_92_ENTERTAINER_GOING_TO_VENUE ||
            f->action_state == FIGURE_ACTION_94_ENTERTAINER_ROAMING ||
            f->action_state == FIGURE_ACTION_95_ENTERTAINER_RETURNING) {
            f->type = FIGURE_ENEMY54_GLADIATOR;
            figure_route_remove(f);
            f->roam_length = 0;
            f->action_state = FIGURE_ACTION_158_NATIVE_CREATED;
            return;
        }
    }
    int speed_factor = f->type == FIGURE_CHARIOTEER ? 2 : 1;
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            figure_image_increase_offset(f, 32);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_90_ENTERTAINER_AT_SCHOOL_CREATED:
            f->is_ghost = 1;
            f->image_offset = 0;
            f->wait_ticks_missile = 0;
            f->wait_ticks--;
            if (f->wait_ticks <= 0) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->action_state = FIGURE_ACTION_91_ENTERTAINER_EXITING_SCHOOL;
                    figure_movement_set_cross_country_destination(f, x_road, y_road);
                    f->roam_length = 0;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
        case FIGURE_ACTION_91_ENTERTAINER_EXITING_SCHOOL:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                int dst_building_id = 0;
                switch (f->type) {
                    case FIGURE_ACTOR:
                        dst_building_id = determine_destination(f->x, f->y, BUILDING_THEATER, BUILDING_AMPHITHEATER);
                        break;
                    case FIGURE_GLADIATOR:
                        dst_building_id = determine_destination(f->x, f->y, BUILDING_AMPHITHEATER, BUILDING_COLOSSEUM);
                        break;
                    case FIGURE_LION_TAMER:
                        dst_building_id = determine_destination(f->x, f->y, BUILDING_COLOSSEUM, 0);
                        break;
                    case FIGURE_CHARIOTEER:
                        dst_building_id = determine_destination(f->x, f->y, BUILDING_HIPPODROME, 0);
                        break;
                }
                if (dst_building_id) {
                    building *b_dst = building_get(dst_building_id);
                    int x_road, y_road;
                    if (map_closest_road_within_radius(b_dst->x, b_dst->y, b_dst->size, 2, &x_road, &y_road)) {
                        f->destination_building_id = dst_building_id;
                        f->action_state = FIGURE_ACTION_92_ENTERTAINER_GOING_TO_VENUE;
                        f->destination_x = x_road;
                        f->destination_y = y_road;
                        f->roam_length = 0;
                    } else {
                        f->state = FIGURE_STATE_DEAD;
                    }
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            f->is_ghost = 1;
            break;
        case FIGURE_ACTION_92_ENTERTAINER_GOING_TO_VENUE:
            f->is_ghost = 0;
            f->roam_length++;
            if (f->roam_length >= 3200) {
                f->state = FIGURE_STATE_DEAD;
            }
            figure_movement_move_ticks(f, speed_factor);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                update_shows(f);
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_94_ENTERTAINER_ROAMING:
            f->is_ghost = 0;
            f->roam_length++;
            if (f->roam_length >= f->max_roam_length) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->action_state = FIGURE_ACTION_95_ENTERTAINER_RETURNING;
                    f->destination_x = x_road;
                    f->destination_y = y_road;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            figure_movement_roam_ticks(f, speed_factor);
            break;
        case FIGURE_ACTION_95_ENTERTAINER_RETURNING:
            figure_movement_move_ticks(f, speed_factor);
            if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
    update_image(f);
}
