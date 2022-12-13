#include "crime.h"

#include "building/building.h"
#include "building/destruction.h"
#include "building/granary.h"
#include "building/distribution.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/data_private.h"
#include "city/figures.h"
#include "city/finance.h"
#include "city/games.h"
#include "city/message.h"
#include "city/population.h"
#include "city/ratings.h"
#include "city/sentiment.h"
#include "city/warning.h"
#include "core/calc.h"
#include "core/image.h"
#include "core/random.h"
#include "figure/combat.h"
#include "figure/formation_enemy.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "game/tutorial.h"
#include "game/resource.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "scenario/property.h"

#define MAX_LOOTING_DISTANCE 120

static const int CRIMINAL_OFFSETS[] = {
    0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1
};

typedef struct {
    int building_id;
    int resource;
} looter_destination;

int get_looter_destination(figure *f)
{
    inventory_storage_info info[INVENTORY_MAX];
    looter_destination possible_destinations[INVENTORY_MAX];
    if (!building_distribution_get_inventory_storages_for_figure(info, 0, 0, f, MAX_LOOTING_DISTANCE)) {
        return 0;
    }

    int resource = 0;
    int building_id = 0;
    int options = 0;

    for (int i = 0; i < INVENTORY_MAX; ++i) {
        if (info[i].building_id > 0) {
            possible_destinations[options].building_id = info[i].building_id;
            possible_destinations[options].resource = i;
            options += 1;
        }
    }

    if (options) {
        int random_index = random_from_stdlib() % options;
        building_id = possible_destinations[random_index].building_id;

        building *storage = building_get(building_id);
        resource = resource_from_inventory(possible_destinations[random_index].resource);

        f->destination_x = storage->road_access_x;
        f->destination_y = storage->road_access_y;
        f->destination_building_id = storage->id;
        f->collecting_item_id = resource;

        return storage->id;
    } else {
        return 0;
    }
}

void figure_crime_loot_storage(figure *f, int resource, int building_id)
{
    building *storage = building_get(building_id);

    if (storage->type == BUILDING_GRANARY) {
        building_granary_remove_resource(storage, resource, 100);
        city_warning_show(WARNING_GRANARY_BREAKIN, NEW_WARNING_SLOT);
    } else {
        building_warehouse_remove_resource(storage, resource, 1);
        city_warning_show(WARNING_WAREHOUSE_BREAKIN, NEW_WARNING_SLOT);
    }

    city_message_apply_sound_interval(MESSAGE_CAT_THEFT);
    city_message_post_with_popup_delay(MESSAGE_CAT_THEFT, MESSAGE_LOOTING, storage->type, f->grid_offset);
}

static void figure_crime_steal_money(figure *f)
{
    int treasury = city_finance_treasury();
    int money_stolen = treasury / 40;
    if (money_stolen > 400) {
        money_stolen = 400 - random_byte() / 2;
    }

    if (money_stolen < 10) {
        return;
    }
    city_message_apply_sound_interval(MESSAGE_CAT_THEFT);
    city_message_post_with_popup_delay(MESSAGE_CAT_THEFT, MESSAGE_THEFT, money_stolen, f->grid_offset);
    city_warning_show(WARNING_THEFT, NEW_WARNING_SLOT);
    city_finance_process_stolen(money_stolen);
}

static void generate_striker(building *b)
{
    int x_road, y_road;
    if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road) && b->figure_id4 == 0) {
        figure *f = figure_create(FIGURE_PROTESTER, x_road, y_road, DIR_4_BOTTOM);
        f->building_id = b->id;
        b->figure_id4 = f->id;
    }
}

static void generate_rioter(building *b, int amount)
{
    int x_road, y_road;
    if (city_ratings_peace_num_rioters() > city_population() / 500) {
        return;
    }

    if (!map_closest_road_within_radius(b->x, b->y, b->size, 4, &x_road, &y_road)) {
        return;
    }

    b->house_criminal_active = 1;

    city_sentiment_add_criminal();

    for (int i = 0; i < amount; i++) {
        figure *f = figure_create(FIGURE_RIOTER, x_road, y_road, DIR_4_BOTTOM);
        f->action_state = FIGURE_ACTION_120_RIOTER_CREATED;
        f->roam_length = 0;
        f->wait_ticks = 10 + 4 * i;
        f->terrain_usage = TERRAIN_USAGE_ENEMY;
    }
    city_ratings_peace_record_rioter();

    tutorial_on_crime();
    city_message_apply_sound_interval(MESSAGE_CAT_RIOT);
    city_message_post_with_popup_delay(MESSAGE_CAT_RIOT, MESSAGE_RIOT, b->type, map_grid_offset(x_road, y_road));
    city_sentiment_set_crime_cooldown();

}

static void generate_looter(building *b, int amount)
{
    int x_road, y_road;
    if (!map_closest_road_within_radius(b->x, b->y, b->size, 4, &x_road, &y_road)) {
        return;
    }
    city_sentiment_add_criminal();

    for (int i = 0; i < amount; i++) {
        figure *f = figure_create(FIGURE_CRIMINAL_LOOTER, x_road, y_road, DIR_4_BOTTOM);
        f->terrain_usage = TERRAIN_USAGE_ANY;
        f->action_state = FIGURE_ACTION_226_CRIMINAL_LOOTER_CREATED;
        f->roam_length = 0;
        f->wait_ticks = 10 + 4 * i;
    }
    b->house_criminal_active = 1;
    city_sentiment_set_crime_cooldown();
}

static void generate_robber(building *b, int amount)
{
    if (city_finance_treasury() < 400) {
        return;
    }

    int x_road, y_road;
    if (!map_closest_road_within_radius(b->x, b->y, b->size, 4, &x_road, &y_road)) {
        return;
    }
    city_sentiment_add_criminal();
    b->house_criminal_active = 1;

    for (int i = 0; i < amount; i++) {
        figure *f = figure_create(FIGURE_CRIMINAL_ROBBER, x_road, y_road, DIR_4_BOTTOM);
        f->action_state = FIGURE_ACTION_227_CRIMINAL_ROBBER_CREATED;
        f->roam_length = 0;
        f->wait_ticks = 10 + 4 * i;
    }
    city_sentiment_set_crime_cooldown();
}

static void generate_protestor(building *b)
{
    city_sentiment_add_protester();
    if (b->house_criminal_active < 1) {
        b->house_criminal_active = 1;
        int x_road, y_road;
        if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
            figure *f = figure_create(FIGURE_PROTESTER, x_road, y_road, DIR_4_BOTTOM);
            f->wait_ticks = 10 + (b->house_figure_generation_delay & 0xf);
            city_ratings_peace_record_criminal();
        }
    }
    city_sentiment_set_crime_cooldown();
}

void figure_generate_criminals(void)
{
    if (city_games_executions_active()) {
        return;
    }

    for (building_type type = BUILDING_MARBLE_QUARRY; type <= BUILDING_POTTERY_WORKSHOP; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->strike_duration_days > 0) {
                generate_striker(b);
            }
        }
    }

    building *min_building = 0;
    int min_happiness = 50;
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
                continue;
            }
            if (b->sentiment.house_happiness >= 50) {
                b->house_criminal_active = 0;
            } else if (b->sentiment.house_happiness < min_happiness) {
                min_happiness = b->sentiment.house_happiness;
                min_building = b;
            }
        }
    }
    if (city_sentiment_crime_cooldown() > 0) {
        city_sentiment_reduce_crime_cooldown();
        return;
    }
    if (min_building) {
        if (scenario_is_tutorial_1() || scenario_is_tutorial_2()) {
            return;
        }
        int sentiment = city_sentiment();
        if (random_byte() >= sentiment + 20) {
            min_building->sentiment.house_happiness += 5;
            if (min_happiness <= 15) {
                int unhappy_population = city_sentiment_get_population_below_happiness(25);
                if (calc_percentage(unhappy_population, city_population()) >= 5) {
                    int amount = calc_bound(unhappy_population / 100, 1, 20);
                    generate_rioter(min_building, amount);
                    generate_looter(min_building, amount);
                    generate_robber(min_building, amount);
                    city_sentiment_set_min_happiness(30);
                } else {
                    generate_looter(min_building, 3);
                    generate_robber(min_building, 3);
                }
            } else if (min_happiness < 30) {
                generate_looter(min_building, min_happiness < 20 ? 2 : 1);
            } else if (min_happiness < 45) {
                generate_robber(min_building, min_happiness < 30 ? 2 : 1);
            } else if (min_happiness < 50) {
                generate_protestor(min_building);
            }
        }
    }
}

void figure_protestor_action(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS_HIGHWAY;
    figure_image_increase_offset(f, 64);
    city_figures_add_protester();
    f->cart_image_id = 0;

    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        figure_combat_handle_corpse(f);
    }
    f->wait_ticks++;
    if (f->wait_ticks > 200 && f->building_id == 0) {
        f->state = FIGURE_STATE_DEAD;
        f->image_offset = 0;
    }
    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_group(GROUP_FIGURE_CRIMINAL) + figure_image_corpse_offset(f) + 96;
    } else {
        f->image_id = image_group(GROUP_FIGURE_CRIMINAL) + CRIMINAL_OFFSETS[f->image_offset / 4] + 104;
    }
}

static void set_criminal_image(figure *f)
{
    int dir;
    if (f->direction == DIR_FIGURE_ATTACK) {
        dir = f->attack_direction;
    } else if (f->direction < 8) {
        dir = f->direction;
    } else {
        dir = f->previous_tile_direction;
    }
    dir = figure_image_normalize_direction(dir);
    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_group(GROUP_FIGURE_CRIMINAL) + 96 + figure_image_corpse_offset(f);
    } else if (f->direction == DIR_FIGURE_ATTACK) {
        f->image_id = image_group(GROUP_FIGURE_CRIMINAL) + 104 + CRIMINAL_OFFSETS[f->image_offset % 16];
    } else if (f->action_state == FIGURE_ACTION_121_RIOTER_MOVING || f->action_state == FIGURE_ACTION_228_CRIMINAL_GOING_TO_LOOT || f->action_state == FIGURE_ACTION_229_CRIMINAL_GOING_TO_ROB) {
        f->image_id = image_group(GROUP_FIGURE_CRIMINAL) + dir + 8 * f->image_offset;
    } else {
        f->image_id = image_group(GROUP_FIGURE_CRIMINAL) + 104 + CRIMINAL_OFFSETS[f->image_offset / 2];
    }
}


void figure_rioter_action(figure *f)
{
    city_figures_add_rioter(!f->targeted_by_figure_id);
    f->terrain_usage = TERRAIN_USAGE_ENEMY;
    f->max_roam_length = 480;
    f->cart_image_id = 0;
    f->is_ghost = 0;

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_120_RIOTER_CREATED:
            figure_image_increase_offset(f, 32);
            f->wait_ticks++;
            if (f->wait_ticks >= 160) {
                f->action_state = FIGURE_ACTION_121_RIOTER_MOVING;
                int x_tile, y_tile, resource = 0, target_building_id;
                target_building_id = formation_rioter_get_target_building(&x_tile, &y_tile);

                if (target_building_id) {
                    f->destination_x = x_tile;
                    f->destination_y = y_tile;
                    f->destination_building_id = target_building_id;
                    f->collecting_item_id = resource;
                    figure_route_remove(f);
                } else {
                    f->state = FIGURE_STATE_DEAD;
                    figure_route_remove(f);
                }
            }
            break;
        case FIGURE_ACTION_121_RIOTER_MOVING:
            figure_image_increase_offset(f, 12);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                figure_rioter_collapse_building(f);

                int x_tile, y_tile;
                int building_id = formation_rioter_get_target_building(&x_tile, &y_tile);
                if (building_id) {
                    f->destination_x = x_tile;
                    f->destination_y = y_tile;
                    f->destination_building_id = building_id;
                    figure_route_remove(f);
                } else {
                    f->type = FIGURE_CRIMINAL;
                    f->action_state = FIGURE_ACTION_120_RIOTER_CREATED;
                    figure_route_remove(f);
                }
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_ATTACK) {
                if (f->image_offset > 12) {
                    f->image_offset = 0;
                }
            }
            break;
    }

    set_criminal_image(f);
}

void figure_robber_action(figure *f)
{
    city_figures_add_robber(!f->targeted_by_figure_id);
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS_HIGHWAY;
    f->max_roam_length = 480;
    f->cart_image_id = 0;
    f->is_ghost = 0;

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_227_CRIMINAL_ROBBER_CREATED:
            figure_image_increase_offset(f, 32);
            f->wait_ticks++;
            if (f->wait_ticks >= 160) {
                int x_tile, y_tile, resource = 0, target_building_id;
                target_building_id = formation_rioter_get_target_building_for_robbery(f->x, f->y, &x_tile, &y_tile);

                if (target_building_id) {
                    f->destination_x = x_tile;
                    f->destination_y = y_tile;
                    f->destination_building_id = target_building_id;
                    f->collecting_item_id = resource;
                    f->action_state = FIGURE_ACTION_229_CRIMINAL_GOING_TO_ROB;
                    figure_route_remove(f);
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
        case FIGURE_ACTION_229_CRIMINAL_GOING_TO_ROB:
            figure_image_increase_offset(f, 12);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                figure_crime_steal_money(f);
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }

    set_criminal_image(f);
}

void figure_looter_action(figure *f)
{
    city_figures_add_looter(!f->targeted_by_figure_id);
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS_HIGHWAY;
    f->max_roam_length = 480;
    f->cart_image_id = 0;
    f->is_ghost = 0;

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_226_CRIMINAL_LOOTER_CREATED:
            figure_image_increase_offset(f, 32);
            f->wait_ticks++;
            if (f->wait_ticks >= 160) {
                int target_building_id = get_looter_destination(f);

                if (target_building_id) {
                    f->action_state = FIGURE_ACTION_228_CRIMINAL_GOING_TO_LOOT;
                    figure_route_remove(f);
                } else {
                    f->state = FIGURE_STATE_DEAD;
                    figure_route_remove(f);
                }
            }
            break;
        case FIGURE_ACTION_228_CRIMINAL_GOING_TO_LOOT:
            figure_image_increase_offset(f, 12);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                figure_crime_loot_storage(f, f->collecting_item_id, f->destination_building_id);
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }

    set_criminal_image(f);
}


int figure_rioter_collapse_building(figure *f)
{
    for (int dir = 0; dir < 8; dir += 2) {
        int grid_offset = f->grid_offset + map_grid_direction_delta(dir);
        if (!map_building_at(grid_offset)) {
            continue;
        }
        building *b = building_get(map_building_at(grid_offset));
        switch (b->type) {
            case BUILDING_WAREHOUSE_SPACE:
            case BUILDING_WAREHOUSE:
            case BUILDING_FORT_GROUND:
            case BUILDING_FORT:
            case BUILDING_BURNING_RUIN:
            case BUILDING_NATIVE_CROPS:
            case BUILDING_NATIVE_HUT:
            case BUILDING_NATIVE_MEETING:
            case BUILDING_RESERVOIR:
            case BUILDING_FOUNTAIN:
            case BUILDING_WELL:
            case BUILDING_MARKET:
            case BUILDING_GRANARY:
            case BUILDING_FORUM:
            case BUILDING_SENATE:
                continue;
            default:
                break;
        }
        if (b->house_size && b->subtype.house_level < HOUSE_SMALL_CASA) {
            continue;
        }
        if (b->fire_proof) {
            continue;
        }
        city_message_apply_sound_interval(MESSAGE_CAT_RIOT_COLLAPSE);
        city_message_post(0, MESSAGE_DESTROYED_BUILDING, b->type, f->grid_offset);
        city_message_increase_category_count(MESSAGE_CAT_RIOT_COLLAPSE);
        building_destroy_by_rioter(b);
        f->action_state = FIGURE_ACTION_120_RIOTER_CREATED;
        f->wait_ticks = 0;
        f->direction = dir;
        return 1;
    }
    return 0;
}


