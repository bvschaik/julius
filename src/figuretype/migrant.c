#include "migrant.h"

#include "building/house.h"
#include "building/house_population.h"
#include "building/model.h"
#include "city/map.h"
#include "city/population.h"
#include "core/calc.h"
#include "core/image.h"
#include "core/time.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "game/undo.h"
#include "map/road_access.h"

static struct {
    int available;
    time_millis last_check;
} houses_with_room;

void figure_create_immigrant(building *house, int num_people)
{
    const map_tile *entry = city_map_entry_point();
    figure *f = figure_create(FIGURE_IMMIGRANT, entry->x, entry->y, DIR_0_TOP);
    f->action_state = FIGURE_ACTION_1_IMMIGRANT_CREATED;
    f->immigrant_building_id = house->id;
    house->immigrant_figure_id = f->id;
    f->wait_ticks = 10 + (house->house_figure_generation_delay & 0x7f);
    f->migrant_num_people = num_people;
}

void figure_create_emigrant(building *house, int num_people)
{
    city_population_remove(num_people);
    if (num_people < house->house_population) {
        house->house_population -= num_people;
    } else {
        building_house_change_to_vacant_lot(house);
    }
    figure *f = figure_create(FIGURE_EMIGRANT, house->x, house->y, DIR_0_TOP);
    f->action_state = FIGURE_ACTION_4_EMIGRANT_CREATED;
    f->wait_ticks = 0;
    f->migrant_num_people = num_people;
}

figure *figure_create_homeless(building *house, int num_people)
{
    figure *f = figure_create(FIGURE_HOMELESS, house->x, house->y, DIR_0_TOP);
    f->building_id = house->id;
    f->action_state = FIGURE_ACTION_7_HOMELESS_CREATED;
    f->wait_ticks = 0;
    f->migrant_num_people = num_people;
    city_population_remove_homeless(num_people);
    return f;
}

static void update_direction_and_image(figure *f)
{
    figure_image_update(f, image_group(GROUP_FIGURE_MIGRANT));
    if (f->action_state == FIGURE_ACTION_2_IMMIGRANT_ARRIVING ||
        f->action_state == FIGURE_ACTION_6_EMIGRANT_LEAVING) {
        int dir = figure_image_direction(f);
        f->cart_image_id = image_group(GROUP_FIGURE_MIGRANT_CART) + dir;
        figure_image_set_cart_offset(f, (dir + 4) % 8);
    }
}

static int closest_house_with_room(int x, int y)
{
    if (houses_with_room.last_check == time_get_millis() && !houses_with_room.available) {
        return 0;
    }
    int available_houses = 0;
    int min_dist = 1000;
    int min_building_id = 0;
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->house_size && !b->has_plague &&
                b->distance_from_entry > 0 && b->house_population_room > 0) {
                if (!b->immigrant_figure_id) {
                    int dist = calc_maximum_distance(x, y, b->x, b->y);
                    available_houses++;
                    if (dist < min_dist) {
                        min_dist = dist;
                        min_building_id = b->id;
                    }
                }
            }
        }
    }
    available_houses--;
    houses_with_room.last_check = time_get_millis();
    houses_with_room.available = available_houses;
    return min_building_id;
}

void figure_immigrant_action(figure *f)
{
    building *b = building_get(f->immigrant_building_id);

    f->terrain_usage = TERRAIN_USAGE_ANY;
    f->cart_image_id = 0;
    if (b->state != BUILDING_STATE_IN_USE || b->immigrant_figure_id != f->id || !b->house_size || b->has_plague) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }

    figure_image_increase_offset(f, 12);

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_1_IMMIGRANT_CREATED:
            f->is_ghost = 1;
            f->image_offset = 0;
            f->wait_ticks--;
            if (f->wait_ticks <= 0) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->action_state = FIGURE_ACTION_2_IMMIGRANT_ARRIVING;
                    f->destination_x = x_road;
                    f->destination_y = y_road;
                    f->roam_length = 0;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
        case FIGURE_ACTION_2_IMMIGRANT_ARRIVING:
            f->is_ghost = 0;
            figure_movement_move_ticks(f, 1);
            switch (f->direction) {
                case DIR_FIGURE_AT_DESTINATION:
                    f->action_state = FIGURE_ACTION_3_IMMIGRANT_ENTERING_HOUSE;
                    figure_movement_set_cross_country_destination(f, b->x, b->y);
                    f->roam_length = 0;
                    break;
                case DIR_FIGURE_REROUTE:
                    figure_route_remove(f);
                    break;
                case DIR_FIGURE_LOST:
                    b->immigrant_figure_id = 0;
                    b->distance_from_entry = 0;
                    f->state = FIGURE_STATE_DEAD;
                    break;
            }
            break;
        case FIGURE_ACTION_3_IMMIGRANT_ENTERING_HOUSE:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                f->state = FIGURE_STATE_DEAD;
                int max_people = house_population_get_capacity(b);

                int room = max_people - b->house_population;
                if (room < 0) {
                    room = 0;
                }
                if (room < f->migrant_num_people) {
                    f->migrant_num_people = room;
                }
                int is_empty = b->house_population == 0;
                b->house_population += f->migrant_num_people;
                b->house_population_room = max_people - b->house_population;
                city_population_add(f->migrant_num_people);
                if (is_empty) {
                    building_house_change_to(b, BUILDING_HOUSE_SMALL_TENT);
                }
                b->immigrant_figure_id = 0;
            }
            f->is_ghost = f->in_building_wait_ticks ? 1 : 0;
            break;
    }

    update_direction_and_image(f);
}

void figure_emigrant_action(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ANY;
    f->cart_image_id = 0;

    figure_image_increase_offset(f, 12);

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_4_EMIGRANT_CREATED:
            f->is_ghost = 1;
            f->image_offset = 0;
            f->wait_ticks++;
            if (f->wait_ticks >= 5) {
                int x_road, y_road;
                if (!map_closest_road_within_radius(f->x, f->y, 1, 5, &x_road, &y_road)) {
                    f->state = FIGURE_STATE_DEAD;
                }
                f->action_state = FIGURE_ACTION_5_EMIGRANT_EXITING_HOUSE;
                figure_movement_set_cross_country_destination(f, x_road, y_road);
                f->roam_length = 0;
            }
            break;
        case FIGURE_ACTION_5_EMIGRANT_EXITING_HOUSE:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                const map_tile *entry = city_map_entry_point();
                f->action_state = FIGURE_ACTION_6_EMIGRANT_LEAVING;
                f->destination_x = entry->x;
                f->destination_y = entry->y;
                f->roam_length = 0;
                f->progress_on_tile = 15;
            }
            f->is_ghost = f->in_building_wait_ticks ? 1 : 0;
            break;
        case FIGURE_ACTION_6_EMIGRANT_LEAVING:
            f->use_cross_country = 0;
            f->is_ghost = 0;
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                f->direction == DIR_FIGURE_REROUTE ||
                f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
    update_direction_and_image(f);
}

void figure_homeless_action(figure *f)
{
    figure_image_increase_offset(f, 12);
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS_HIGHWAY;

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_7_HOMELESS_CREATED:
            f->image_offset = 0;
            f->wait_ticks++;
            if (f->wait_ticks > 51) {
                int building_id = closest_house_with_room(f->x, f->y);
                if (building_id) {
                    building *b = building_get(building_id);
                    int x_road, y_road;
                    if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                        b->immigrant_figure_id = f->id;
                        f->immigrant_building_id = building_id;
                        f->action_state = FIGURE_ACTION_8_HOMELESS_GOING_TO_HOUSE;
                        f->destination_x = x_road;
                        f->destination_y = y_road;
                        f->roam_length = 0;
                    } else {
                        f->state = FIGURE_STATE_DEAD;
                    }
                } else {
                    const map_tile *exit = city_map_exit_point();
                    f->action_state = FIGURE_ACTION_10_HOMELESS_LEAVING;
                    f->destination_x = exit->x;
                    f->destination_y = exit->y;
                    f->roam_length = 0;
                    f->wait_ticks = 0;
                }
            }
            break;
        case FIGURE_ACTION_8_HOMELESS_GOING_TO_HOUSE:
            f->is_ghost = 0;
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                building_get(f->immigrant_building_id)->immigrant_figure_id = 0;
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                building *b = building_get(f->immigrant_building_id);
                f->action_state = FIGURE_ACTION_9_HOMELESS_ENTERING_HOUSE;
                figure_movement_set_cross_country_destination(f, b->x, b->y);
                f->roam_length = 0;
            }
            break;
        case FIGURE_ACTION_9_HOMELESS_ENTERING_HOUSE:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                f->state = FIGURE_STATE_DEAD;
                building *b = building_get(f->immigrant_building_id);
                if (f->immigrant_building_id && building_is_house(b->type) && !b->has_plague) {
                    int max_people = model_get_house(b->subtype.house_level)->max_people;
                    if (b->house_is_merged) {
                        max_people *= 4;
                    }
                    int room = max_people - b->house_population;
                    if (room < 0) {
                        room = 0;
                    }
                    if (room < f->migrant_num_people) {
                        f->migrant_num_people = room;
                    }
                    int is_empty = b->house_population == 0;
                    b->house_population += f->migrant_num_people;
                    b->house_population_room = max_people - b->house_population;
                    city_population_add_homeless(f->migrant_num_people);
                    if (is_empty) {
                        building_house_change_to(b, BUILDING_HOUSE_SMALL_TENT);
                    }
                    b->immigrant_figure_id = 0;
                    game_undo_disable();
                }
            }
            break;
        case FIGURE_ACTION_10_HOMELESS_LEAVING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            f->wait_ticks++;
            if (f->wait_ticks > FIGURE_REROUTE_DESTINATION_TICKS) {
                f->wait_ticks = 0;
                int building_id = closest_house_with_room(f->x, f->y);
                if (building_id > 0) {
                    building *b = building_get(building_id);
                    int x_road, y_road;
                    if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                        b->immigrant_figure_id = f->id;
                        f->immigrant_building_id = building_id;
                        f->action_state = FIGURE_ACTION_8_HOMELESS_GOING_TO_HOUSE;
                        f->destination_x = x_road;
                        f->destination_y = y_road;
                        f->roam_length = 0;
                        figure_route_remove(f);
                    }
                }
            }
            break;
    }
    figure_image_update(f, image_group(GROUP_FIGURE_HOMELESS));
}
