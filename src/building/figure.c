#include "building/figure.h"

#include "building/barracks.h"
#include "building/granary.h"
#include "building/industry.h"
#include "building/market.h"
#include "building/model.h"
#include "building/warehouse.h"
#include "city/buildings.h"
#include "city/entertainment.h"
#include "city/message.h"
#include "city/population.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/image.h"
#include "figure/figure.h"
#include "figure/formation_legion.h"
#include "figure/movement.h"
#include "game/resource.h"
#include "map/building_tiles.h"
#include "map/desirability.h"
#include "map/image.h"
#include "map/random.h"
#include "map/road_access.h"
#include "map/terrain.h"
#include "map/water.h"

static int worker_percentage(const building *b)
{
    return calc_percentage(b->num_workers, model_get_building(b->type)->laborers);
}

static void check_labor_problem(building *b)
{
    if (b->houses_covered <= 0) {
        b->show_on_problem_overlay = 2;
    }
}

static void generate_labor_seeker(building *b, int x, int y)
{
    if (city_population() <= 0) {
        return;
    }
    if (config_get(CONFIG_GP_CH_GLOBAL_LABOUR)) {
	// If it can access Rome
        if (b->distance_from_entry) {
            b->houses_covered = 100;
	} else {
            b->houses_covered = 0;
	}
	return;
    }
    if (b->figure_id2) {
        figure *f = figure_get(b->figure_id2);
        if (!f->state || f->type != FIGURE_LABOR_SEEKER || f->building_id != b->id) {
            b->figure_id2 = 0;
        }
    } else {
        figure *f = figure_create(FIGURE_LABOR_SEEKER, x, y, DIR_0_TOP);
        f->action_state = FIGURE_ACTION_125_ROAMING;
        f->building_id = b->id;
        b->figure_id2 = f->id;
        figure_movement_init_roaming(f);
    }
}

static void spawn_labor_seeker(building *b, int x, int y, int min_houses)
{
    if (config_get(CONFIG_GP_CH_GLOBAL_LABOUR)) {
	// If it can access Rome
        if (b->distance_from_entry) {
            b->houses_covered = 2 * min_houses;
	}
	else {
            b->houses_covered = 0;
	}
    } else if (b->houses_covered <= min_houses) {
        generate_labor_seeker(b, x, y);
    }
}

static int has_figure_of_types(building *b, figure_type type1, figure_type type2)
{
    if (b->figure_id <= 0) {
        return 0;
    }
    figure *f = figure_get(b->figure_id);
    if (f->state && f->building_id == b->id && (f->type == type1 || f->type == type2)) {
        return 1;
    } else {
        b->figure_id = 0;
        return 0;
    }
}

static int has_figure_of_type(building *b, figure_type type)
{
    return has_figure_of_types(b, type, 0);
}

static int default_spawn_delay(building *b)
{
    int pct_workers = worker_percentage(b);
    if (pct_workers >= 100) {
        return 3;
    } else if (pct_workers >= 75) {
        return 7;
    } else if (pct_workers >= 50) {
        return 15;
    } else if (pct_workers >= 25) {
        return 29;
    } else if (pct_workers >= 1) {
        return 44;
    } else {
        return 0;
    }
}

static void create_roaming_figure(building *b, int x, int y, figure_type type)
{
    figure *f = figure_create(type, x, y, DIR_0_TOP);
    f->action_state = FIGURE_ACTION_125_ROAMING;
    f->building_id = b->id;
    b->figure_id = f->id;
    figure_movement_init_roaming(f);
}

static int spawn_patrician(building *b, int spawned)
{
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > 40 && !spawned) {
            b->figure_spawn_delay = 0;
            figure *f = figure_create(FIGURE_PATRICIAN, road.x, road.y, DIR_4_BOTTOM);
            f->action_state = FIGURE_ACTION_125_ROAMING;
            f->building_id = b->id;
            figure_movement_init_roaming(f);
            return 1;
        }
    }
    return spawned;
}

static void spawn_figure_warehouse(building *b)
{
    check_labor_problem(b);
    building *space = b;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id) {
            space->show_on_problem_overlay = b->show_on_problem_overlay;
        }
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road) ||
        map_has_road_access(b->x, b->y, 3, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 100);
        if (has_figure_of_type(b, FIGURE_WAREHOUSEMAN)) {
            return;
        }
        int resource;
        int task = building_warehouse_determine_worker_task(b, &resource);
        if (task != WAREHOUSE_TASK_NONE) {
            figure *f = figure_create(FIGURE_WAREHOUSEMAN, road.x, road.y, DIR_4_BOTTOM);
            f->action_state = FIGURE_ACTION_50_WAREHOUSEMAN_CREATED;
            if (task == WAREHOUSE_TASK_GETTING) {
                f->resource_id = RESOURCE_NONE;
                f->collecting_item_id = resource;
            } else {
                f->resource_id = resource;
            }
            b->figure_id = f->id;
            f->building_id = b->id;
        }
    }
}

static void spawn_figure_granary(building *b)
{
    check_labor_problem(b);
    map_point road;
    if (map_has_road_access_granary(b->x, b->y, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 100);
        if (has_figure_of_type(b, FIGURE_WAREHOUSEMAN)) {
            return;
        }
        int task = building_granary_determine_worker_task(b);
        if (task != GRANARY_TASK_NONE) {
            figure *f = figure_create(FIGURE_WAREHOUSEMAN, road.x, road.y, DIR_4_BOTTOM);
            f->action_state = FIGURE_ACTION_50_WAREHOUSEMAN_CREATED;
            f->resource_id = task;
            b->figure_id = f->id;
            f->building_id = b->id;
        }
    }
}

static void spawn_figure_tower(building *b)
{
    check_labor_problem(b);
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        if (b->num_workers <= 0) {
            return;
        }
        if (!b->figure_id4 && b->figure_id) { // has sentry but no ballista -> create
            figure *f = figure_create(FIGURE_BALLISTA, b->x, b->y, DIR_0_TOP);
            b->figure_id4 = f->id;
            f->building_id = b->id;
            f->action_state = FIGURE_ACTION_180_BALLISTA_CREATED;
        }
        has_figure_of_type(b, FIGURE_TOWER_SENTRY);
        if (b->figure_id <= 0) {
            building_barracks_request_tower_sentry();
        }
    }
}

static void spawn_figure_engineers_post(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_ENGINEER)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 100);
        int pct_workers = worker_percentage(b);
        int spawn_delay;
        if (pct_workers >= 100) {
            spawn_delay = 0;
        } else if (pct_workers >= 75) {
            spawn_delay = 1;
        } else if (pct_workers >= 50) {
            spawn_delay = 3;
        } else if (pct_workers >= 25) {
            spawn_delay = 7;
        } else if (pct_workers >= 1) {
            spawn_delay = 15;
        } else {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            figure *f = figure_create(FIGURE_ENGINEER, road.x, road.y, DIR_0_TOP);
            f->action_state = FIGURE_ACTION_60_ENGINEER_CREATED;
            f->building_id = b->id;
            b->figure_id = f->id;
        }
    }
}

static void spawn_figure_prefecture(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_PREFECT)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 100);
        int pct_workers = worker_percentage(b);
        int spawn_delay;
        if (pct_workers >= 100) {
            spawn_delay = 0;
        } else if (pct_workers >= 75) {
            spawn_delay = 1;
        } else if (pct_workers >= 50) {
            spawn_delay = 3;
        } else if (pct_workers >= 25) {
            spawn_delay = 7;
        } else if (pct_workers >= 1) {
            spawn_delay = 15;
        } else {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            figure *f = figure_create(FIGURE_PREFECT, road.x, road.y, DIR_0_TOP);
            f->action_state = FIGURE_ACTION_70_PREFECT_CREATED;
            f->building_id = b->id;
            b->figure_id = f->id;
        }
    }
}

static void spawn_figure_actor_colony(building *b)
{
    check_labor_problem(b);
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            figure *f = figure_create(FIGURE_ACTOR, road.x, road.y, DIR_0_TOP);
            f->action_state = FIGURE_ACTION_90_ENTERTAINER_AT_SCHOOL_CREATED;
            f->building_id = b->id;
            b->figure_id = f->id;
        }
    }
}

static void spawn_figure_gladiator_school(building *b)
{
    check_labor_problem(b);
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            figure *f = figure_create(FIGURE_GLADIATOR, road.x, road.y, DIR_0_TOP);
            f->action_state = FIGURE_ACTION_90_ENTERTAINER_AT_SCHOOL_CREATED;
            f->building_id = b->id;
            b->figure_id = f->id;
        }
    }
}

static void spawn_figure_lion_house(building *b)
{
    check_labor_problem(b);
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int pct_workers = worker_percentage(b);
        int spawn_delay;
        if (pct_workers >= 100) {
            spawn_delay = 5;
        } else if (pct_workers >= 75) {
            spawn_delay = 10;
        } else if (pct_workers >= 50) {
            spawn_delay = 20;
        } else if (pct_workers >= 25) {
            spawn_delay = 35;
        } else if (pct_workers >= 1) {
            spawn_delay = 60;
        } else {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            figure *f = figure_create(FIGURE_LION_TAMER, road.x, road.y, DIR_0_TOP);
            f->action_state = FIGURE_ACTION_90_ENTERTAINER_AT_SCHOOL_CREATED;
            f->building_id = b->id;
            b->figure_id = f->id;
        }
    }
}

static void spawn_figure_chariot_maker(building *b)
{
    check_labor_problem(b);
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int pct_workers = worker_percentage(b);
        int spawn_delay;
        if (pct_workers >= 100) {
            spawn_delay = 7;
        } else if (pct_workers >= 75) {
            spawn_delay = 15;
        } else if (pct_workers >= 50) {
            spawn_delay = 30;
        } else if (pct_workers >= 25) {
            spawn_delay = 60;
        } else if (pct_workers >= 1) {
            spawn_delay = 90;
        } else {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            figure *f = figure_create(FIGURE_CHARIOTEER, road.x, road.y, DIR_0_TOP);
            f->action_state = FIGURE_ACTION_90_ENTERTAINER_AT_SCHOOL_CREATED;
            f->building_id = b->id;
            b->figure_id = f->id;
        }
    }
}

static void spawn_figure_amphitheater(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_types(b, FIGURE_ACTOR, FIGURE_GLADIATOR)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        if (b->houses_covered <= 50 ||
            (b->data.entertainment.days1 <= 0 && b->data.entertainment.days2 <= 0)) {
            generate_labor_seeker(b, road.x, road.y);
        }
        int pct_workers = worker_percentage(b);
        int spawn_delay;
        if (pct_workers >= 100) {
            spawn_delay = 3;
        } else if (pct_workers >= 75) {
            spawn_delay = 7;
        } else if (pct_workers >= 50) {
            spawn_delay = 15;
        } else if (pct_workers >= 25) {
            spawn_delay = 29;
        } else if (pct_workers >= 1) {
            spawn_delay = 44;
        } else {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            figure *f;
            if (b->data.entertainment.days1 > 0) {
                f = figure_create(FIGURE_GLADIATOR, road.x, road.y, DIR_0_TOP);
            } else {
                f = figure_create(FIGURE_ACTOR, road.x, road.y, DIR_0_TOP);
            }
            f->action_state = FIGURE_ACTION_94_ENTERTAINER_ROAMING;
            f->building_id = b->id;
            b->figure_id = f->id;
            figure_movement_init_roaming(f);
        }
    }
}

static void spawn_figure_theater(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_ACTOR)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        if (b->houses_covered <= 50 || b->data.entertainment.days1 <= 0) {
            generate_labor_seeker(b, road.x, road.y);
        }
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            figure *f = figure_create(FIGURE_ACTOR, road.x, road.y, DIR_0_TOP);
            f->action_state = FIGURE_ACTION_94_ENTERTAINER_ROAMING;
            f->building_id = b->id;
            b->figure_id = f->id;
            figure_movement_init_roaming(f);
        }
    }
}

static void spawn_figure_hippodrome(building *b)
{
    check_labor_problem(b);
    if (b->prev_part_building_id) {
        return;
    }
    building *part = b;
    for (int i = 0; i < 2; i++) {
        part = building_next(part);
        if (part->id) {
            part->show_on_problem_overlay = b->show_on_problem_overlay;
        }
    }
    if (has_figure_of_type(b, FIGURE_CHARIOTEER)) {
        return;
    }
    map_point road;
    if (map_has_road_access_hippodrome(b->x, b->y, &road)) {
        if (b->houses_covered <= 50 || b->data.entertainment.days1 <= 0) {
            generate_labor_seeker(b, road.x, road.y);
        }
        int pct_workers = worker_percentage(b);
        int spawn_delay;
        if (pct_workers >= 100) {
            spawn_delay = 7;
        } else if (pct_workers >= 75) {
            spawn_delay = 15;
        } else if (pct_workers >= 50) {
            spawn_delay = 30;
        } else if (pct_workers >= 25) {
            spawn_delay = 50;
        } else if (pct_workers >= 1) {
            spawn_delay = 80;
        } else {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            figure *f = figure_create(FIGURE_CHARIOTEER, road.x, road.y, DIR_0_TOP);
            f->action_state = FIGURE_ACTION_94_ENTERTAINER_ROAMING;
            f->building_id = b->id;
            b->figure_id = f->id;
            figure_movement_init_roaming(f);

            if (!city_entertainment_hippodrome_has_race()) {
                // create mini-horses
                figure *horse1 = figure_create(FIGURE_HIPPODROME_HORSES, b->x + 2, b->y + 1, DIR_2_RIGHT);
                horse1->action_state = FIGURE_ACTION_200_HIPPODROME_HORSE_CREATED;
                horse1->building_id = b->id;
                horse1->resource_id = 0;
                horse1->speed_multiplier = 3;

                figure *horse2 = figure_create(FIGURE_HIPPODROME_HORSES, b->x + 2, b->y + 2, DIR_2_RIGHT);
                horse2->action_state = FIGURE_ACTION_200_HIPPODROME_HORSE_CREATED;
                horse2->building_id = b->id;
                horse2->resource_id = 1;
                horse2->speed_multiplier = 2;

                if (b->data.entertainment.days1 > 0) {
                    if (city_entertainment_show_message_hippodrome()) {
                        city_message_post(1, MESSAGE_WORKING_HIPPODROME, 0, 0);
                    }
                }
            }
        }
    }
}

static void spawn_figure_colosseum(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_types(b, FIGURE_GLADIATOR, FIGURE_LION_TAMER)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        if (b->houses_covered <= 50 ||
            (b->data.entertainment.days1 <= 0 && b->data.entertainment.days2 <= 0)) {
            generate_labor_seeker(b, road.x, road.y);
        }
        int pct_workers = worker_percentage(b);
        int spawn_delay;
        if (pct_workers >= 100) {
            spawn_delay = 6;
        } else if (pct_workers >= 75) {
            spawn_delay = 12;
        } else if (pct_workers >= 50) {
            spawn_delay = 20;
        } else if (pct_workers >= 25) {
            spawn_delay = 40;
        } else if (pct_workers >= 1) {
            spawn_delay = 70;
        } else {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            figure *f;
            if (b->data.entertainment.days1 > 0) {
                f = figure_create(FIGURE_LION_TAMER, road.x, road.y, DIR_0_TOP);
            } else {
                f = figure_create(FIGURE_GLADIATOR, road.x, road.y, DIR_0_TOP);
            }
            f->action_state = FIGURE_ACTION_94_ENTERTAINER_ROAMING;
            f->building_id = b->id;
            b->figure_id = f->id;
            figure_movement_init_roaming(f);
            if (b->data.entertainment.days1 > 0 || b->data.entertainment.days2 > 0) {
                if (city_entertainment_show_message_colosseum()) {
                    city_message_post(1, MESSAGE_WORKING_COLOSSEUM, 0, 0);
                }
            }
        }
    }
}

static void set_market_graphic(building *b)
{
    if (b->state != BUILDING_STATE_IN_USE) {
        return;
    }
    if (map_desirability_get(b->grid_offset) <= 30) {
        map_building_tiles_add(b->id, b->x, b->y, b->size,
            image_group(GROUP_BUILDING_MARKET), TERRAIN_BUILDING);
    } else {
        map_building_tiles_add(b->id, b->x, b->y, b->size,
            image_group(GROUP_BUILDING_MARKET_FANCY), TERRAIN_BUILDING);
    }
}

static void spawn_figure_market(building *b)
{
    set_market_graphic(b);
    check_labor_problem(b);
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int pct_workers = worker_percentage(b);
        int spawn_delay;
        if (pct_workers >= 100) {
            spawn_delay = 2;
        } else if (pct_workers >= 75) {
            spawn_delay = 5;
        } else if (pct_workers >= 50) {
            spawn_delay = 10;
        } else if (pct_workers >= 25) {
            spawn_delay = 20;
        } else if (pct_workers >= 1) {
            spawn_delay = 30;
        } else {
            return;
        }
        // market trader
        if (!has_figure_of_type(b, FIGURE_MARKET_TRADER)) {
            b->figure_spawn_delay++;
            if (b->figure_spawn_delay <= spawn_delay) {
                return;
            }
            b->figure_spawn_delay = 0;
            create_roaming_figure(b, road.x, road.y, FIGURE_MARKET_TRADER);
        }
        // market buyer or labor seeker
        if (b->figure_id2) {
            figure *f = figure_get(b->figure_id2);
            if (f->state != FIGURE_STATE_ALIVE || (f->type != FIGURE_MARKET_BUYER && f->type != FIGURE_LABOR_SEEKER)) {
                b->figure_id2 = 0;
            }
        } else {
            map_has_road_access(b->x, b->y, b->size, &road);
            int dst_building_id = building_market_get_storage_destination(b);
            if (dst_building_id > 0) {
                figure *f = figure_create(FIGURE_MARKET_BUYER, road.x, road.y, DIR_0_TOP);
                f->action_state = FIGURE_ACTION_145_MARKET_BUYER_GOING_TO_STORAGE;
                f->building_id = b->id;
                b->figure_id2 = f->id;
                f->destination_building_id = dst_building_id;
                f->collecting_item_id = b->data.market.fetch_inventory_id;
                building *b_dst = building_get(dst_building_id);
                if (map_has_road_access(b_dst->x, b_dst->y, b_dst->size, &road) ||
                    map_has_road_access(b_dst->x, b_dst->y, 3, &road)) {
                    f->destination_x = road.x;
                    f->destination_y = road.y;
                } else {
                    f->action_state = FIGURE_ACTION_146_MARKET_BUYER_RETURNING;
                    f->destination_x = f->x;
                    f->destination_y = f->y;
                }
            }
        }
    }
}

static void set_bathhouse_graphic(building *b)
{
    if (b->state != BUILDING_STATE_IN_USE) {
        return;
    }
    if (map_terrain_exists_tile_in_area_with_type(b->x, b->y, b->size, TERRAIN_RESERVOIR_RANGE)) {
        b->has_water_access = 1;
    } else {
        b->has_water_access = 0;
    }
    if (b->has_water_access && b->num_workers) {
        if (map_desirability_get(b->grid_offset) <= 30) {
            map_building_tiles_add(b->id, b->x, b->y, b->size,
                image_group(GROUP_BUILDING_BATHHOUSE_WATER), TERRAIN_BUILDING);
        } else {
            map_building_tiles_add(b->id, b->x, b->y, b->size,
                image_group(GROUP_BUILDING_BATHHOUSE_FANCY_WATER), TERRAIN_BUILDING);
        }
    } else {
        if (map_desirability_get(b->grid_offset) <= 30) {
            map_building_tiles_add(b->id, b->x, b->y, b->size,
                image_group(GROUP_BUILDING_BATHHOUSE_NO_WATER), TERRAIN_BUILDING);
        } else {
            map_building_tiles_add(b->id, b->x, b->y, b->size,
                image_group(GROUP_BUILDING_BATHHOUSE_FANCY_NO_WATER), TERRAIN_BUILDING);
        }
    }
}

static void spawn_figure_bathhouse(building *b)
{
    set_bathhouse_graphic(b);
    check_labor_problem(b);
    if (!b->has_water_access) {
        b->show_on_problem_overlay = 2;
    }
    if (has_figure_of_type(b, FIGURE_BATHHOUSE_WORKER)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road) && b->has_water_access) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            create_roaming_figure(b, road.x, road.y, FIGURE_BATHHOUSE_WORKER);
        }
    }
}

static void spawn_figure_school(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_SCHOOL_CHILD)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;

            figure *child1 = figure_create(FIGURE_SCHOOL_CHILD, road.x, road.y, DIR_0_TOP);
            child1->action_state = FIGURE_ACTION_125_ROAMING;
            child1->building_id = b->id;
            b->figure_id = child1->id;
            figure_movement_init_roaming(child1);

            figure *child2 = figure_create(FIGURE_SCHOOL_CHILD, road.x, road.y, DIR_0_TOP);
            child2->action_state = FIGURE_ACTION_125_ROAMING;
            child2->building_id = b->id;
            figure_movement_init_roaming(child2);

            figure *child3 = figure_create(FIGURE_SCHOOL_CHILD, road.x, road.y, DIR_0_TOP);
            child3->action_state = FIGURE_ACTION_125_ROAMING;
            child3->building_id = b->id;
            figure_movement_init_roaming(child3);

            figure *child4 = figure_create(FIGURE_SCHOOL_CHILD, road.x, road.y, DIR_0_TOP);
            child4->action_state = FIGURE_ACTION_125_ROAMING;
            child4->building_id = b->id;
            figure_movement_init_roaming(child4);
        }
    }
}

static void spawn_figure_library(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_LIBRARIAN)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            create_roaming_figure(b, road.x, road.y, FIGURE_LIBRARIAN);
        }
    }
}

static void spawn_figure_academy(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_TEACHER)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            create_roaming_figure(b, road.x, road.y, FIGURE_TEACHER);
        }
    }
}

static void spawn_figure_barber(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_BARBER)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            create_roaming_figure(b, road.x, road.y, FIGURE_BARBER);
        }
    }
}

static void spawn_figure_doctor(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_DOCTOR)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            create_roaming_figure(b, road.x, road.y, FIGURE_DOCTOR);
        }
    }
}

static void spawn_figure_hospital(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_SURGEON)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            create_roaming_figure(b, road.x, road.y, FIGURE_SURGEON);
        }
    }
}

static void spawn_figure_temple(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_PRIEST)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int pct_workers = worker_percentage(b);
        int spawn_delay;
        if (model_get_building(b->type)->laborers <= 0) {
            spawn_delay = 7;
        } else if (pct_workers >= 100) {
            spawn_delay = 3;
        } else if (pct_workers >= 75) {
            spawn_delay = 7;
        } else if (pct_workers >= 50) {
            spawn_delay = 10;
        } else if (pct_workers >= 25) {
            spawn_delay = 15;
        } else if (pct_workers >= 1) {
            spawn_delay = 20;
        } else {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            create_roaming_figure(b, road.x, road.y, FIGURE_PRIEST);
        }
    }
}

static void set_senate_graphic(building *b)
{
    if (b->state != BUILDING_STATE_IN_USE) {
        return;
    }
    if (map_desirability_get(b->grid_offset) <= 30) {
        map_building_tiles_add(b->id, b->x, b->y, b->size,
            image_group(GROUP_BUILDING_SENATE), TERRAIN_BUILDING);
    } else {
        map_building_tiles_add(b->id, b->x, b->y, b->size,
            image_group(GROUP_BUILDING_SENATE_FANCY), TERRAIN_BUILDING);
    }
}

static void spawn_figure_senate_forum(building *b)
{
    if (b->type == BUILDING_SENATE_UPGRADED) {
        set_senate_graphic(b);
    }
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_TAX_COLLECTOR)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int pct_workers = worker_percentage(b);
        int spawn_delay;
        if (pct_workers >= 100) {
            spawn_delay = 0;
        } else if (pct_workers >= 75) {
            spawn_delay = 1;
        } else if (pct_workers >= 50) {
            spawn_delay = 3;
        } else if (pct_workers >= 25) {
            spawn_delay = 7;
        } else if (pct_workers >= 1) {
            spawn_delay = 15;
        } else {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            figure *f = figure_create(FIGURE_TAX_COLLECTOR, road.x, road.y, DIR_0_TOP);
            f->action_state = FIGURE_ACTION_40_TAX_COLLECTOR_CREATED;
            f->building_id = b->id;
            b->figure_id = f->id;
        }
    }
}

static void spawn_figure_mission_post(building *b)
{
    if (has_figure_of_type(b, FIGURE_MISSIONARY)) {
        return;
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        if (city_population() > 0) {
            city_buildings_set_mission_post_operational();
            b->figure_spawn_delay++;
            if (b->figure_spawn_delay > 1) {
                b->figure_spawn_delay = 0;
                create_roaming_figure(b, road.x, road.y, FIGURE_MISSIONARY);
            }
        }
    }
}

static void spawn_figure_industry(building *b)
{
    check_labor_problem(b);
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        if (has_figure_of_type(b, FIGURE_CART_PUSHER)) {
            return;
        }
        if (building_industry_has_produced_resource(b)) {
            building_industry_start_new_production(b);
            figure *f = figure_create(FIGURE_CART_PUSHER, road.x, road.y, DIR_4_BOTTOM);
            f->action_state = FIGURE_ACTION_20_CARTPUSHER_INITIAL;
            f->resource_id = b->output_resource_id;
            f->building_id = b->id;
            b->figure_id = f->id;
            f->wait_ticks = 30;
        }
    }
}

static void spawn_figure_wharf(building *b)
{
    check_labor_problem(b);
    if (b->data.industry.fishing_boat_id) {
        figure *f = figure_get(b->data.industry.fishing_boat_id);
        if (f->state != FIGURE_STATE_ALIVE || f->type != FIGURE_FISHING_BOAT) {
            b->data.industry.fishing_boat_id = 0;
        }
    }
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        if (has_figure_of_type(b, FIGURE_CART_PUSHER)) {
            return;
        }
        if (b->figure_spawn_delay) {
            b->figure_spawn_delay = 0;
            b->data.industry.has_fish = 0;
            b->output_resource_id = RESOURCE_MEAT;
            figure *f = figure_create(FIGURE_CART_PUSHER, road.x, road.y, DIR_4_BOTTOM);
            f->action_state = FIGURE_ACTION_20_CARTPUSHER_INITIAL;
            f->resource_id = RESOURCE_MEAT;
            f->building_id = b->id;
            b->figure_id = f->id;
            f->wait_ticks = 30;
        }
    }
}

static void spawn_figure_shipyard(building *b)
{
    check_labor_problem(b);
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        if (has_figure_of_type(b, FIGURE_FISHING_BOAT)) {
            return;
        }
        int pct_workers = worker_percentage(b);
        if (pct_workers >= 100) {
            b->data.industry.progress += 10;
        } else if (pct_workers >= 75) {
            b->data.industry.progress += 8;
        } else if (pct_workers >= 50) {
            b->data.industry.progress += 6;
        } else if (pct_workers >= 25) {
            b->data.industry.progress += 4;
        } else if (pct_workers >= 1) {
            b->data.industry.progress += 2;
        }
        if (b->data.industry.progress >= 160) {
            b->data.industry.progress = 0;
            map_point boat;
            if (map_water_can_spawn_fishing_boat(b->x, b->y, b->size, &boat)) {
                figure *f = figure_create(FIGURE_FISHING_BOAT, boat.x, boat.y, DIR_0_TOP);
                f->action_state = FIGURE_ACTION_190_FISHING_BOAT_CREATED;
                f->building_id = b->id;
                b->figure_id = f->id;
            }
        }
    }
}

static void spawn_figure_dock(building *b)
{
    check_labor_problem(b);
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 50);
        int pct_workers = worker_percentage(b);
        int max_dockers;
        if (pct_workers >= 75) {
            max_dockers = 3;
        } else if (pct_workers >= 50) {
            max_dockers = 2;
        } else if (pct_workers > 0) {
            max_dockers = 1;
        } else {
            max_dockers = 0;
        }
        // count existing dockers
        int existing_dockers = 0;
        for (int i = 0; i < 3; i++) {
            if (b->data.dock.docker_ids[i]) {
                if (figure_get(b->data.dock.docker_ids[i])->type == FIGURE_DOCKER) {
                    existing_dockers++;
                } else {
                    b->data.dock.docker_ids[i] = 0;
                }
            }
        }
        if (existing_dockers > max_dockers) {
            // too many dockers, kill one of them
            for (int i = 2; i >= 0; i--) {
                if (b->data.dock.docker_ids[i]) {
                    figure_get(b->data.dock.docker_ids[i])->state = FIGURE_STATE_DEAD;
                    break;
                }
            }
        } else if (existing_dockers < max_dockers) {
            figure *f = figure_create(FIGURE_DOCKER, road.x, road.y, DIR_4_BOTTOM);
            f->action_state = FIGURE_ACTION_132_DOCKER_IDLING;
            f->building_id = b->id;
            for (int i = 0; i < 3; i++) {
                if (!b->data.dock.docker_ids[i]) {
                    b->data.dock.docker_ids[i] = f->id;
                    break;
                }
            }
        }
    }
}

static void spawn_figure_native_hut(building *b)
{
    map_image_set(b->grid_offset, image_group(GROUP_BUILDING_NATIVE) + (map_random_get(b->grid_offset) & 1));
    if (has_figure_of_type(b, FIGURE_INDIGENOUS_NATIVE)) {
        return;
    }
    int x_out, y_out;
    if (b->subtype.native_meeting_center_id > 0 && map_terrain_get_adjacent_road_or_clear_land(b->x, b->y, b->size, &x_out, &y_out)) {
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > 4) {
            b->figure_spawn_delay = 0;
            figure *f = figure_create(FIGURE_INDIGENOUS_NATIVE, x_out, y_out, DIR_0_TOP);
            f->action_state = FIGURE_ACTION_158_NATIVE_CREATED;
            f->building_id = b->id;
            b->figure_id = f->id;
        }
    }
}

static void spawn_figure_native_meeting(building *b)
{
    map_building_tiles_add(b->id, b->x, b->y, 2,
        image_group(GROUP_BUILDING_NATIVE) + 2, TERRAIN_BUILDING);
    if (city_buildings_is_mission_post_operational() && !has_figure_of_type(b, FIGURE_NATIVE_TRADER)) {
        int x_out, y_out;
        if (map_terrain_get_adjacent_road_or_clear_land(b->x, b->y, b->size, &x_out, &y_out)) {
            b->figure_spawn_delay++;
            if (b->figure_spawn_delay > 8) {
                b->figure_spawn_delay = 0;
                figure *f = figure_create(FIGURE_NATIVE_TRADER, x_out, y_out, DIR_0_TOP);
                f->action_state = FIGURE_ACTION_162_NATIVE_TRADER_CREATED;
                f->building_id = b->id;
                b->figure_id = f->id;
            }
        }
    }
}

static void spawn_figure_barracks(building *b)
{
    check_labor_problem(b);
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 100);
        int pct_workers = worker_percentage(b);
        int spawn_delay;
        if (pct_workers >= 100) {
            spawn_delay = 8;
        } else if (pct_workers >= 75) {
            spawn_delay = 12;
        } else if (pct_workers >= 50) {
            spawn_delay = 16;
        } else if (pct_workers >= 25) {
            spawn_delay = 32;
        } else if (pct_workers >= 1) {
            spawn_delay = 48;
        } else {
            return;
        }
        b->figure_spawn_delay++;
        if (b->figure_spawn_delay > spawn_delay) {
            b->figure_spawn_delay = 0;
            map_has_road_access(b->x, b->y, b->size, &road);
            switch (b->subtype.barracks_priority)
            {
            case PRIORITY_FORT:
                if (!building_barracks_create_soldier(b, road.x, road.y)) {
                    building_barracks_create_tower_sentry(b, road.x, road.y);
                }
                break;
            default:
                if (!building_barracks_create_tower_sentry(b, road.x, road.y)) {
                    building_barracks_create_soldier(b, road.x, road.y);
                }
            }

        }
    }
}

static void spawn_figure_military_academy(building *b)
{
    check_labor_problem(b);
    map_point road;
    if (map_has_road_access(b->x, b->y, b->size, &road)) {
        spawn_labor_seeker(b, road.x, road.y, 100);
    }
}

static void update_native_crop_progress(building *b)
{
    b->data.industry.progress++;
    if (b->data.industry.progress >= 5) {
        b->data.industry.progress = 0;
    }
    map_image_set(b->grid_offset, image_group(GROUP_BUILDING_FARM_CROPS) + b->data.industry.progress);
}


void building_figure_generate(void)
{
    int patrician_generated = 0;
    building_barracks_decay_tower_sentry_request();
    int max_id = building_get_highest_id();
    for (int i = 1; i <= max_id; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->type == BUILDING_WAREHOUSE_SPACE || (b->type == BUILDING_HIPPODROME && b->prev_part_building_id)) {
            continue;
        }
        b->show_on_problem_overlay = 0;
        // range of building types
        if (b->type >= BUILDING_HOUSE_SMALL_VILLA && b->type <= BUILDING_HOUSE_LUXURY_PALACE) {
            patrician_generated = spawn_patrician(b, patrician_generated);
        } else if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_POTTERY_WORKSHOP) {
            spawn_figure_industry(b);
        } else if (b->type >= BUILDING_SENATE && b->type <= BUILDING_FORUM_UPGRADED) {
            spawn_figure_senate_forum(b);
        } else if (b->type >= BUILDING_SMALL_TEMPLE_CERES && b->type <= BUILDING_LARGE_TEMPLE_VENUS) {
            spawn_figure_temple(b);
        } else {
            // single building type
            switch (b->type) {
                case BUILDING_WAREHOUSE:
                    spawn_figure_warehouse(b);
                    break;
                case BUILDING_GRANARY:
                    spawn_figure_granary(b);
                    break;
                case BUILDING_TOWER:
                    spawn_figure_tower(b);
                    break;
                case BUILDING_ENGINEERS_POST:
                    spawn_figure_engineers_post(b);
                    break;
                case BUILDING_PREFECTURE:
                    spawn_figure_prefecture(b);
                    break;
                case BUILDING_ACTOR_COLONY:
                    spawn_figure_actor_colony(b);
                    break;
                case BUILDING_GLADIATOR_SCHOOL:
                    spawn_figure_gladiator_school(b);
                    break;
                case BUILDING_LION_HOUSE:
                    spawn_figure_lion_house(b);
                    break;
                case BUILDING_CHARIOT_MAKER:
                    spawn_figure_chariot_maker(b);
                    break;
                case BUILDING_AMPHITHEATER:
                    spawn_figure_amphitheater(b);
                    break;
                case BUILDING_THEATER:
                    spawn_figure_theater(b);
                    break;
                case BUILDING_HIPPODROME:
                    spawn_figure_hippodrome(b);
                    break;
                case BUILDING_COLOSSEUM:
                    spawn_figure_colosseum(b);
                    break;
                case BUILDING_MARKET:
                    spawn_figure_market(b);
                    break;
                case BUILDING_BATHHOUSE:
                    spawn_figure_bathhouse(b);
                    break;
                case BUILDING_SCHOOL:
                    spawn_figure_school(b);
                    break;
                case BUILDING_LIBRARY:
                    spawn_figure_library(b);
                    break;
                case BUILDING_ACADEMY:
                    spawn_figure_academy(b);
                    break;
                case BUILDING_BARBER:
                    spawn_figure_barber(b);
                    break;
                case BUILDING_DOCTOR:
                    spawn_figure_doctor(b);
                    break;
                case BUILDING_HOSPITAL:
                    spawn_figure_hospital(b);
                    break;
                case BUILDING_MISSION_POST:
                    spawn_figure_mission_post(b);
                    break;
                case BUILDING_DOCK:
                    spawn_figure_dock(b);
                    break;
                case BUILDING_WHARF:
                    spawn_figure_wharf(b);
                    break;
                case BUILDING_SHIPYARD:
                    spawn_figure_shipyard(b);
                    break;
                case BUILDING_NATIVE_HUT:
                    spawn_figure_native_hut(b);
                    break;
                case BUILDING_NATIVE_MEETING:
                    spawn_figure_native_meeting(b);
                    break;
                case BUILDING_NATIVE_CROPS:
                    update_native_crop_progress(b);
                    break;
                case BUILDING_FORT:
                    formation_legion_update_recruit_status(b);
                    break;
                case BUILDING_BARRACKS:
                    spawn_figure_barracks(b);
                    break;
                case BUILDING_MILITARY_ACADEMY:
                    spawn_figure_military_academy(b);
                    break;
            }
        }
    }
}
