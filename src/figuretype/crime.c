#include "crime.h"

#include "building/building.h"
#include "building/destruction.h"
#include "city/figures.h"
#include "city/finance.h"
#include "city/message.h"
#include "city/population.h"
#include "city/ratings.h"
#include "city/sentiment.h"
#include "core/image.h"
#include "core/random.h"
#include "figure/combat.h"
#include "figure/formation_enemy.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "game/tutorial.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "scenario/property.h"

static const int CRIMINAL_OFFSETS[] = {
    0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1
};

static void generate_rioter(building *b)
{
    int x_road, y_road;
    if (!map_closest_road_within_radius(b->x, b->y, b->size, 4, &x_road, &y_road)) {
        return;
    }
    city_sentiment_add_criminal();
    int people_in_mob;
    int population = city_population();
    if (population <= 150) {
        people_in_mob = 1;
    } else if (population <= 300) {
        people_in_mob = 2;
    } else if (population <= 800) {
        people_in_mob = 3;
    } else if (population <= 1200) {
        people_in_mob = 4;
    } else if (population <= 2000) {
        people_in_mob = 5;
    } else {
        people_in_mob = 6;
    }
    int x_target, y_target;
    int target_building_id = formation_rioter_get_target_building(&x_target, &y_target);
    for (int i = 0; i < people_in_mob; i++) {
        figure *f = figure_create(FIGURE_RIOTER, x_road, y_road, DIR_4_BOTTOM);
        f->action_state = FIGURE_ACTION_120_RIOTER_CREATED;
        f->roam_length = 0;
        f->wait_ticks = 10 + 4 * i;
        if (target_building_id) {
            f->destination_x = x_target;
            f->destination_y = y_target;
            f->destination_building_id = target_building_id;
        } else {
            f->state = FIGURE_STATE_DEAD;
        }
    }
    building_destroy_by_rioter(b);
    city_ratings_peace_record_rioter();
    city_sentiment_change_happiness(20);
    tutorial_on_crime();
    city_message_apply_sound_interval(MESSAGE_CAT_RIOT);
    city_message_post_with_popup_delay(MESSAGE_CAT_RIOT, MESSAGE_RIOT, b->type, map_grid_offset(x_road, y_road));
}

static void generate_mugger(building *b)
{
    city_sentiment_add_criminal();
    if (b->house_criminal_active < 2) {
        b->house_criminal_active = 2;
        int x_road, y_road;
        if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
            figure *f = figure_create(FIGURE_CRIMINAL, x_road, y_road, DIR_4_BOTTOM);
            f->wait_ticks = 10 + (b->house_figure_generation_delay & 0xf);
            city_ratings_peace_record_criminal();
            int taxes_this_year = city_finance_overview_this_year()->income.taxes;
            if (taxes_this_year > 20) {
                int money_stolen = taxes_this_year / 4;
                if (money_stolen > 400) {
                    money_stolen = 400 - random_byte() / 2;
                }
                city_message_post(1, MESSAGE_THEFT, money_stolen, f->grid_offset);
                city_finance_process_stolen(money_stolen);
            }
        }
    }
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
}

void figure_generate_criminals(void)
{
    building *min_building = 0;
    int min_happiness = 50;
    int max_id = building_get_highest_id();
    for (int i = 1; i <= max_id; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->house_size) {
            if (b->sentiment.house_happiness >= 50) {
                b->house_criminal_active = 0;
            } else if (b->sentiment.house_happiness < min_happiness) {
                min_happiness = b->sentiment.house_happiness;
                min_building = b;
            }
        }
    }
    if (min_building) {
        if (scenario_is_tutorial_1() || scenario_is_tutorial_2()) {
            return;
        }
        int sentiment = city_sentiment();
        if (sentiment < 30) {
            if (random_byte() >= sentiment + 50) {
                if (min_happiness <= 10) {
                    generate_rioter(min_building);
                } else if (min_happiness < 30) {
                    generate_mugger(min_building);
                } else if (min_happiness < 50) {
                    generate_protestor(min_building);
                }
            }
        } else if (sentiment < 60) {
            if (random_byte() >= sentiment + 40) {
                if (min_happiness < 30) {
                    generate_mugger(min_building);
                } else if (min_happiness < 50) {
                    generate_protestor(min_building);
                }
            }
        } else {
            if (random_byte() >= sentiment + 20) {
                if (min_happiness < 50) {
                    generate_protestor(min_building);
                }
            }
        }
    }
}

void figure_protestor_action(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    figure_image_increase_offset(f, 64);
    f->cart_image_id = 0;
    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->state = FIGURE_STATE_DEAD;
    }
    f->wait_ticks++;
    if (f->wait_ticks > 200) {
        f->state = FIGURE_STATE_DEAD;
        f->image_offset = 0;
    }
    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_group(GROUP_FIGURE_CRIMINAL) + figure_image_corpse_offset(f) + 96;
    } else {
        f->image_id = image_group(GROUP_FIGURE_CRIMINAL) + CRIMINAL_OFFSETS[f->image_offset / 4] + 104;
    }
}

void figure_criminal_action(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    figure_image_increase_offset(f, 32);
    f->cart_image_id = 0;
    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->state = FIGURE_STATE_DEAD;
    }
    f->wait_ticks++;
    if (f->wait_ticks > 200) {
        f->state = FIGURE_STATE_DEAD;
        f->image_offset = 0;
    }
    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_group(GROUP_FIGURE_CRIMINAL) + figure_image_corpse_offset(f) + 96;
    } else {
        f->image_id = image_group(GROUP_FIGURE_CRIMINAL) + CRIMINAL_OFFSETS[f->image_offset / 2] + 104;
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
                int x_tile, y_tile;
                int building_id = formation_rioter_get_target_building(&x_tile, &y_tile);
                if (building_id) {
                    f->destination_x = x_tile;
                    f->destination_y = y_tile;
                    f->destination_building_id = building_id;
                    figure_route_remove(f);
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
        case FIGURE_ACTION_121_RIOTER_MOVING:
            figure_image_increase_offset(f, 12);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                int x_tile, y_tile;
                int building_id = formation_rioter_get_target_building(&x_tile, &y_tile);
                if (building_id) {
                    f->destination_x = x_tile;
                    f->destination_y = y_tile;
                    f->destination_building_id = building_id;
                    figure_route_remove(f);
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->action_state = FIGURE_ACTION_120_RIOTER_CREATED;
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_ATTACK) {
                if (f->image_offset > 12) {
                    f->image_offset = 0;
                }
            }
            break;
    }
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
    } else if (f->action_state == FIGURE_ACTION_121_RIOTER_MOVING) {
        f->image_id = image_group(GROUP_FIGURE_CRIMINAL) + dir + 8 * f->image_offset;
    } else {
        f->image_id = image_group(GROUP_FIGURE_CRIMINAL) + 104 + CRIMINAL_OFFSETS[f->image_offset / 2];
    }
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
                continue;
        }
        if (b->house_size && b->subtype.house_level < HOUSE_SMALL_CASA) {
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
