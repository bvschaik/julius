#include "entertainer.h"
#include "building/building.h"
#include "building/list.h"
#include "building/monument.h"
#include "city/festival.h"
#include "city/figures.h"
#include "city/map.h"
#include "core/calc.h"
#include "core/image.h"
#include "core/random.h"
#include "figure/combat.h"
#include "figure/enemy_army.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "map/road_network.h"
#include "scenario/gladiator_revolt.h"

#define INFINITE 10000

void figure_spawn_tourist(void)
{
    const map_tile *entry = city_map_entry_point();
    const map_tile *exit = city_map_exit_point();
    if (random_byte() % 2) {
        figure *tourist = figure_create(FIGURE_TOURIST, entry->x, entry->y, DIR_0_TOP);
        tourist->action_state = FIGURE_ACTION_217_TOURIST_CREATED;
    } else {
        figure *tourist = figure_create(FIGURE_TOURIST, exit->x, exit->y, DIR_0_TOP);
        tourist->action_state = FIGURE_ACTION_217_TOURIST_CREATED;
    }
}

static int determine_tourist_destination(int x, int y)
{
    int road_network = map_road_network_get(map_grid_offset(x, y));

    building_list_large_clear();

    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (city_festival_games_active()) {
            if (b->type != city_festival_games_active_venue_type()) {
                continue;
            }
        }
        if (b->is_tourism_venue && !b->tourism_disabled && b->distance_from_entry
            && b->road_network_id == road_network) {
            if (b->type == BUILDING_HIPPODROME && b->prev_part_building_id) {
                continue;
            }
            building_list_large_add(i);
        }
    }
    int total_venues = building_list_large_size();
    if (total_venues <= 0) {
        return 0;
    }

    int index;

    index = random_from_stdlib() % total_venues;
    building *b = building_get(building_list_large_item(index));

    return b->id;
}

static int is_venue(building *b)
{
    switch (b->type) {
        case BUILDING_THEATER:
        case BUILDING_AMPHITHEATER:
        case BUILDING_ARENA:
            return 1;
        case BUILDING_COLOSSEUM:
        case BUILDING_HIPPODROME:
            return b->data.monument.phase == MONUMENT_FINISHED;
        default:
            return 0;
    }
}

static building *determine_destination(figure *f)
{
    int road_network = map_road_network_get(map_grid_offset(f->x, f->y));

    static const building_type destinations_per_entertainer_type[4][3] =
    {
        { BUILDING_THEATER, BUILDING_AMPHITHEATER },
        { BUILDING_AMPHITHEATER, BUILDING_COLOSSEUM, BUILDING_ARENA },
        { BUILDING_COLOSSEUM, BUILDING_ARENA },
        { BUILDING_HIPPODROME }
    };

    const building_type *destinations;
    switch (f->type) {
        case FIGURE_ACTOR:
            destinations = destinations_per_entertainer_type[0];
            break;
        case FIGURE_GLADIATOR:
            destinations = destinations_per_entertainer_type[1];
            break;
        case FIGURE_LION_TAMER:
            destinations = destinations_per_entertainer_type[2];
            break;
        case FIGURE_CHARIOTEER:
            destinations = destinations_per_entertainer_type[3];
            break;
        default:
            return 0;
    }

    building *closest = 0;
    int min_distance = 10000;

    for (int i = 0; i < 3; i++) {
        building_type type = destinations[i];
        int use_secondary_entertainment = (f->type == FIGURE_ACTOR && type == BUILDING_AMPHITHEATER) ||
            (f->type == FIGURE_GLADIATOR && (type == BUILDING_ARENA || type == BUILDING_COLOSSEUM));
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE) {
                continue;
            }
            if ((type == BUILDING_HIPPODROME || type == BUILDING_COLOSSEUM) && b->data.monument.phase != -1) {
                continue;
            }
            if (!b->distance_from_entry || b->road_network_id != road_network) {
                continue;
            }
            if (type == BUILDING_HIPPODROME && b->prev_part_building_id) {
                continue;
            }

            int days_left = use_secondary_entertainment ? b->data.entertainment.days2 : b->data.entertainment.days1;
            int dist = 2 * days_left + calc_maximum_distance(f->x, f->y, b->x, b->y);
            if (dist < min_distance) {
                min_distance = dist;
                closest = b;
            }
        }
    }
    return closest;
}

static void update_shows(figure *f)
{
    building *b = building_main(building_get(f->destination_building_id));
    if (!is_venue(b)) {
        return;
    }
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
    } else if (f->type == FIGURE_TOURIST) {
        image_id = image_group(GROUP_FIGURE_PATRICIAN);
    } else if (f->type == FIGURE_LION_TAMER) {
        image_id = image_group(GROUP_FIGURE_LION_TAMER);
        if (f->wait_ticks_missile >= 96 && f->action_state != FIGURE_ACTION_149_CORPSE) {
            image_id = image_group(GROUP_FIGURE_LION_TAMER_WHIP);
        }
        f->cart_image_id = image_group(GROUP_FIGURE_LION);
    } else {
        return;
    }
    if (f->action_state == FIGURE_ACTION_150_ATTACK) {
        if (f->type == FIGURE_GLADIATOR) {
            f->image_id = image_id + 104 + dir + 8 * (f->image_offset / 2);
            // Correct for two missing frames, animation is glitchy otherwise
            if (f->image_id >= 5705 && f->image_id <= 5706) {
                f->image_id -= 8;
            } else if (f->image_id > 5705) {
                f->image_id -= 2;
            }
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

static int get_enemy_distance(figure *f, int x, int y)
{
    if (f->type == FIGURE_RIOTER || f->type == FIGURE_ENEMY54_GLADIATOR) {
        return calc_maximum_distance(x, y, f->x, f->y);
    } else if (f->type == FIGURE_CRIMINAL_LOOTER || f->type == FIGURE_CRIMINAL_ROBBER) {
        return 3 * calc_maximum_distance(x, y, f->x, f->y);
    } else if (f->type == FIGURE_INDIGENOUS_NATIVE && f->action_state == FIGURE_ACTION_159_NATIVE_ATTACKING) {
        return calc_maximum_distance(x, y, f->x, f->y);
    } else if (figure_is_enemy(f)) {
        return calc_maximum_distance(x, y, f->x, f->y);
    } else if (f->type == FIGURE_WOLF) {
        return 2 * calc_maximum_distance(x, y, f->x, f->y);
    }
    return INFINITE;
}

static int get_nearest_enemy(int x, int y, int *distance)
{
    int min_enemy_id = 0;
    int min_dist = INFINITE;
    for (int i = 1; i < figure_count(); i++) {
        figure *f = figure_get(i);
        if (figure_is_dead(f)) {
            continue;
        }
        int dist = get_enemy_distance(f, x, y);
        if (dist != INFINITE && f->targeted_by_figure_id) {
            figure *pursuiter = figure_get(f->targeted_by_figure_id);
            if (get_enemy_distance(f, pursuiter->x, pursuiter->y) < dist) {
                continue;
            }
        }
        if (dist < min_dist) {
            min_dist = dist;
            min_enemy_id = i;
        }
    }
    *distance = min_dist;
    return min_enemy_id;
}

static int fight_enemy(figure *f)
{
    if (!city_figures_has_security_breach() && enemy_army_total_enemy_formations() <= 0) {
        return 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
        case FIGURE_ACTION_149_CORPSE:
            return 0;
    }

    int distance;
    int enemy_id = get_nearest_enemy(f->x, f->y, &distance);
    if (enemy_id > 0 && distance <= 50) {
        figure *enemy = figure_get(enemy_id);
        if (enemy->targeted_by_figure_id) {
            figure_get(enemy->targeted_by_figure_id)->target_figure_id = 0;
        }
        f->destination_x = enemy->x;
        f->destination_y = enemy->y;
        f->target_figure_id = enemy_id;
        enemy->targeted_by_figure_id = f->id;
        f->target_figure_created_sequence = enemy->created_sequence;
        figure_route_remove(f);
        return 1;
    }
    f->wait_ticks_next_target = 0;
    return 0;
}

void figure_entertainer_action(figure *f)
{
    building *b = building_get(f->building_id);
    f->cart_image_id = image_group(GROUP_FIGURE_CARTPUSHER_CART);
    f->terrain_usage = TERRAIN_USAGE_ROADS_HIGHWAY;
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
                building *b_dst = determine_destination(f);
                if (b_dst) {
                    int x_road, y_road;
                    int found_road = 0;
                    do {
                        if (map_closest_road_within_radius(b_dst->x, b_dst->y, b_dst->size, 2, &x_road, &y_road)) {
                            f->destination_building_id = b_dst->id;
                            f->action_state = FIGURE_ACTION_92_ENTERTAINER_GOING_TO_VENUE;
                            f->destination_x = x_road;
                            f->destination_y = y_road;
                            f->roam_length = 0;
                            found_road = 1;
                            break;
                        }
                        b_dst = building_get(b_dst->next_part_building_id);
                    } while (b_dst->id != 0);
                    if (!found_road) {
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
            f->terrain_usage = TERRAIN_USAGE_ROADS;
            f->roam_length++;
            if (f->roam_length >= f->max_roam_length) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->action_state = FIGURE_ACTION_95_ENTERTAINER_RETURNING;
                    f->destination_x = x_road;
                    f->destination_y = y_road;
                    figure_route_remove(f);
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            figure_movement_roam_ticks(f, speed_factor);
            break;
        case FIGURE_ACTION_95_ENTERTAINER_RETURNING:
            f->terrain_usage = TERRAIN_USAGE_ROADS;
            figure_movement_move_ticks(f, speed_factor);
            if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_230_LION_TAMERS_HUNTING_ENEMIES:
            f->terrain_usage = TERRAIN_USAGE_ANY;
            if (!figure_target_is_alive(f) &&
                !fight_enemy(f)) {
                f->state = FIGURE_STATE_DEAD;
            }
            figure_movement_move_ticks_with_percentage(f, 1, 50);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                figure *target = figure_get(f->target_figure_id);
                f->destination_x = target->x;
                f->destination_y = target->y;
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
    update_image(f);
}

void figure_tourist_action(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    figure_image_increase_offset(f, 12);

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            figure_image_increase_offset(f, 32);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_217_TOURIST_CREATED:
            f->is_ghost = 1;
            f->image_offset = 0;
            f->action_state = FIGURE_ACTION_218_TOURIST_CHOOSING_DESTINATION;
            break;
        case FIGURE_ACTION_218_TOURIST_CHOOSING_DESTINATION:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            int dst_building_id = 0;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                dst_building_id = determine_tourist_destination(f->x, f->y);
                if (dst_building_id) {
                    building *b_dst = building_get(dst_building_id);
                    int x_road, y_road;
                    if (map_closest_road_within_radius(b_dst->x, b_dst->y, b_dst->size, 2, &x_road, &y_road)) {
                        f->destination_building_id = dst_building_id;
                        f->action_state = FIGURE_ACTION_219_TOURIST_GOING_TO_VENUE;
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

        case FIGURE_ACTION_219_TOURIST_GOING_TO_VENUE:
            f->is_ghost = 0;
            figure_movement_move_ticks(f, 1);
            for (int i = 0; i < 12; ++i) {
                if (f->tourist.visited_building_type_ids[i]) {
                    f->tourist.ticks_since_last_visited_id[i]++;
                }
            }
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
    update_image(f);
}
