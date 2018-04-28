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
    return calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
}

static void check_labor_problem(building *b)
{
    if (b->housesCovered <= 0) {
        b->showOnProblemOverlay = 2;
    }
}

static void generate_labor_seeker(building *b, int x, int y)
{
    if (city_population() <= 0) {
        return;
    }
    if (b->figureId2) {
        figure *f = figure_get(b->figureId2);
        if (!f->state || f->type != FIGURE_LABOR_SEEKER || f->buildingId != b->id) {
            b->figureId2 = 0;
        }
    } else {
        figure *f = figure_create(FIGURE_LABOR_SEEKER, x, y, DIR_0_TOP);
        f->actionState = FIGURE_ACTION_125_ROAMING;
        f->buildingId = b->id;
        b->figureId2 = f->id;
        figure_movement_init_roaming(f);
    }
}

static void spawn_labor_seeker(building *b, int x, int y, int min_houses)
{
    if (b->housesCovered <= min_houses) {
        generate_labor_seeker(b, x, y);
    }
}

static int has_figure_of_types(building *b, figure_type type1, figure_type type2)
{
    if (b->figureId <= 0) {
        return 0;
    }
    figure *f = figure_get(b->figureId);
    if (f->state && f->buildingId == b->id && (f->type == type1 || f->type == type2)) {
        return 1;
    } else {
        b->figureId = 0;
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
    f->actionState = FIGURE_ACTION_125_ROAMING;
    f->buildingId = b->id;
    b->figureId = f->id;
    figure_movement_init_roaming(f);
}

static int spawn_patrician(building *b, int spawned)
{
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > 40 && !spawned) {
            b->figureSpawnDelay = 0;
            figure *f = figure_create(FIGURE_PATRICIAN, x_road, y_road, DIR_4_BOTTOM);
            f->actionState = FIGURE_ACTION_125_ROAMING;
            f->buildingId = b->id;
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
            space->showOnProblemOverlay = b->showOnProblemOverlay;
        }
    }
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road) ||
        map_has_road_access(b->x, b->y, 3, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 100);
        if (has_figure_of_type(b, FIGURE_WAREHOUSEMAN)) {
            return;
        }
        int resource;
        int task = building_warehouse_determine_worker_task(b, &resource);
        if (task != WAREHOUSE_TASK_NONE) {
            figure *f = figure_create(FIGURE_WAREHOUSEMAN, x_road, y_road, DIR_4_BOTTOM);
            f->actionState = FIGURE_ACTION_50_WAREHOUSEMAN_CREATED;
            if (task == WAREHOUSE_TASK_GETTING) {
                f->resourceId = RESOURCE_NONE;
                f->collectingItemId = resource;
            } else {
                f->resourceId = resource;
            }
            b->figureId = f->id;
            f->buildingId = b->id;
        }
    }
}

static void spawn_figure_granary(building *b)
{
    check_labor_problem(b);
    int x_road, y_road;
    if (map_has_road_access_granary(b->x, b->y, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 100);
        if (has_figure_of_type(b, FIGURE_WAREHOUSEMAN)) {
            return;
        }
        int task = building_granary_determine_worker_task(b);
        if (task != GRANARY_TASK_NONE) {
            figure *f = figure_create(FIGURE_WAREHOUSEMAN, x_road, y_road, DIR_4_BOTTOM);
            f->actionState = FIGURE_ACTION_50_WAREHOUSEMAN_CREATED;
            f->resourceId = task;
            b->figureId = f->id;
            f->buildingId = b->id;
        }
    }
}

static void spawn_figure_tower(building *b)
{
    check_labor_problem(b);
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        if (b->numWorkers <= 0) {
            return;
        }
        if (!b->figureId4 && b->figureId) { // has sentry but no ballista -> create
            figure *f = figure_create(FIGURE_BALLISTA, b->x, b->y, DIR_0_TOP);
            b->figureId4 = f->id;
            f->buildingId = b->id;
            f->actionState = FIGURE_ACTION_180_BALLISTA_CREATED;
        }
        has_figure_of_type(b, FIGURE_TOWER_SENTRY);
        if (b->figureId <= 0) {
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
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 100);
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
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            figure *f = figure_create(FIGURE_ENGINEER, x_road, y_road, DIR_0_TOP);
            f->actionState = FIGURE_ACTION_60_ENGINEER_CREATED;
            f->buildingId = b->id;
            b->figureId = f->id;
        }
    }
}

static void spawn_figure_prefecture(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_PREFECT)) {
        return;
    }
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 100);
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
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            figure *f = figure_create(FIGURE_PREFECT, x_road, y_road, DIR_0_TOP);
            f->actionState = FIGURE_ACTION_70_PREFECT_CREATED;
            f->buildingId = b->id;
            b->figureId = f->id;
        }
    }
}

static void spawn_figure_actor_colony(building *b)
{
    check_labor_problem(b);
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            figure *f = figure_create(FIGURE_ACTOR, x_road, y_road, DIR_0_TOP);
            f->actionState = FIGURE_ACTION_90_ENTERTAINER_AT_SCHOOL_CREATED;
            f->buildingId = b->id;
            b->figureId = f->id;
        }
    }
}

static void spawn_figure_gladiator_school(building *b)
{
    check_labor_problem(b);
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            figure *f = figure_create(FIGURE_GLADIATOR, x_road, y_road, DIR_0_TOP);
            f->actionState = FIGURE_ACTION_90_ENTERTAINER_AT_SCHOOL_CREATED;
            f->buildingId = b->id;
            b->figureId = f->id;
        }
    }
}

static void spawn_figure_lion_house(building *b)
{
    check_labor_problem(b);
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
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
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            figure *f = figure_create(FIGURE_LION_TAMER, x_road, y_road, DIR_0_TOP);
            f->actionState = FIGURE_ACTION_90_ENTERTAINER_AT_SCHOOL_CREATED;
            f->buildingId = b->id;
            b->figureId = f->id;
        }
    }
}

static void spawn_figure_chariot_maker(building *b)
{
    check_labor_problem(b);
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
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
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            figure *f = figure_create(FIGURE_CHARIOTEER, x_road, y_road, DIR_0_TOP);
            f->actionState = FIGURE_ACTION_90_ENTERTAINER_AT_SCHOOL_CREATED;
            f->buildingId = b->id;
            b->figureId = f->id;
        }
    }
}

static void spawn_figure_amphitheater(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_types(b, FIGURE_ACTOR, FIGURE_GLADIATOR)) {
        return;
    }
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        if (b->housesCovered <= 50 ||
            (b->data.entertainment.days1 <= 0 && b->data.entertainment.days2 <= 0)) {
            generate_labor_seeker(b, x_road, y_road);
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
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            figure *f;
            if (b->data.entertainment.days1 > 0) {
                f = figure_create(FIGURE_GLADIATOR, x_road, y_road, DIR_0_TOP);
            } else {
                f = figure_create(FIGURE_ACTOR, x_road, y_road, DIR_0_TOP);
            }
            f->actionState = FIGURE_ACTION_94_ENTERTAINER_ROAMING;
            f->buildingId = b->id;
            b->figureId = f->id;
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
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        if (b->housesCovered <= 50 || b->data.entertainment.days1 <= 0) {
            generate_labor_seeker(b, x_road, y_road);
        }
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            figure *f = figure_create(FIGURE_ACTOR, x_road, y_road, DIR_0_TOP);
            f->actionState = FIGURE_ACTION_94_ENTERTAINER_ROAMING;
            f->buildingId = b->id;
            b->figureId = f->id;
            figure_movement_init_roaming(f);
        }
    }
}

static void spawn_figure_hippodrome(building *b)
{
    check_labor_problem(b);
    if (b->prevPartBuildingId) {
        return;
    }
    building *part = b;
    for (int i = 0; i < 2; i++) {
        part = building_next(part);
        if (part->id) {
            part->showOnProblemOverlay = b->showOnProblemOverlay;
        }
    }
    if (has_figure_of_type(b, FIGURE_CHARIOTEER)) {
        return;
    }
    int x_road, y_road;
    if (map_has_road_access_hippodrome(b->x, b->y, &x_road, &y_road)) {
        if (b->housesCovered <= 50 || b->data.entertainment.days1 <= 0) {
            generate_labor_seeker(b, x_road, y_road);
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
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            figure *f = figure_create(FIGURE_CHARIOTEER, x_road, y_road, DIR_0_TOP);
            f->actionState = FIGURE_ACTION_94_ENTERTAINER_ROAMING;
            f->buildingId = b->id;
            b->figureId = f->id;
            figure_movement_init_roaming(f);

            if (!city_entertainment_hippodrome_has_race()) {
                // create mini-horses
                figure *horse1 = figure_create(FIGURE_HIPPODROME_HORSES, b->x + 2, b->y + 1, DIR_2_RIGHT);
                horse1->actionState = FIGURE_ACTION_200_HIPPODROME_HORSE_CREATED;
                horse1->buildingId = b->id;
                horse1->resourceId = 0;
                horse1->speedMultiplier = 3;

                figure *horse2 = figure_create(FIGURE_HIPPODROME_HORSES, b->x + 2, b->y + 2, DIR_2_RIGHT);
                horse2->actionState = FIGURE_ACTION_200_HIPPODROME_HORSE_CREATED;
                horse2->buildingId = b->id;
                horse2->resourceId = 1;
                horse2->speedMultiplier = 2;

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
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        if (b->housesCovered <= 50 ||
            (b->data.entertainment.days1 <= 0 && b->data.entertainment.days2 <= 0)) {
            generate_labor_seeker(b, x_road, y_road);
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
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            figure *f;
            if (b->data.entertainment.days1 > 0) {
                f = figure_create(FIGURE_LION_TAMER, x_road, y_road, DIR_0_TOP);
            } else {
                f = figure_create(FIGURE_GLADIATOR, x_road, y_road, DIR_0_TOP);
            }
            f->actionState = FIGURE_ACTION_94_ENTERTAINER_ROAMING;
            f->buildingId = b->id;
            b->figureId = f->id;
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
    if (map_desirability_get(b->gridOffset) <= 30) {
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
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
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
            b->figureSpawnDelay++;
            if (b->figureSpawnDelay <= spawn_delay) {
                return;
            }
            b->figureSpawnDelay = 0;
            create_roaming_figure(b, x_road, y_road, FIGURE_MARKET_TRADER);
        }
        // market buyer or labor seeker
        if (b->figureId2) {
            figure *f = figure_get(b->figureId2);
            if (f->state != FIGURE_STATE_ALIVE || (f->type != FIGURE_MARKET_BUYER && f->type != FIGURE_LABOR_SEEKER)) {
                b->figureId2 = 0;
            }
        } else {
            map_has_road_access(b->x, b->y, b->size, &x_road, &y_road);
            int dstBuildingId = building_market_get_storage_destination(b);
            if (dstBuildingId > 0) {
                figure *f = figure_create(FIGURE_MARKET_BUYER, x_road, y_road, DIR_0_TOP);
                f->actionState = FIGURE_ACTION_145_MARKET_BUYER_GOING_TO_STORAGE;
                f->buildingId = b->id;
                b->figureId2 = f->id;
                f->destinationBuildingId = dstBuildingId;
                f->collectingItemId = b->data.market.fetch_inventory_id;
                building *bDst = building_get(dstBuildingId);
                if (map_has_road_access(bDst->x, bDst->y, bDst->size, &x_road, &y_road) ||
                    map_has_road_access(bDst->x, bDst->y, 3, &x_road, &y_road)) {
                    f->destinationX = x_road;
                    f->destinationY = y_road;
                } else {
                    f->actionState = FIGURE_ACTION_146_MARKET_BUYER_RETURNING;
                    f->destinationX = f->x;
                    f->destinationY = f->y;
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
        b->hasWaterAccess = 1;
    } else {
        b->hasWaterAccess = 0;
    }
    if (b->hasWaterAccess && b->numWorkers) {
        if (map_desirability_get(b->gridOffset) <= 30) {
            map_building_tiles_add(b->id, b->x, b->y, b->size,
                image_group(GROUP_BUILDING_BATHHOUSE_WATER), TERRAIN_BUILDING);
        } else {
            map_building_tiles_add(b->id, b->x, b->y, b->size,
                image_group(GROUP_BUILDING_BATHHOUSE_FANCY_WATER), TERRAIN_BUILDING);
        }
    } else {
        if (map_desirability_get(b->gridOffset) <= 30) {
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
    if (!b->hasWaterAccess) {
        b->showOnProblemOverlay = 2;
    }
    if (has_figure_of_type(b, FIGURE_BATHHOUSE_WORKER)) {
        return;
    }
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road) && b->hasWaterAccess) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            create_roaming_figure(b, x_road, y_road, FIGURE_BATHHOUSE_WORKER);
        }
    }
}

static void spawn_figure_school(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_SCHOOL_CHILD)) {
        return;
    }
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;

            figure *child1 = figure_create(FIGURE_SCHOOL_CHILD, x_road, y_road, DIR_0_TOP);
            child1->actionState = FIGURE_ACTION_125_ROAMING;
            child1->buildingId = b->id;
            b->figureId = child1->id;
            figure_movement_init_roaming(child1);

            figure *child2 = figure_create(FIGURE_SCHOOL_CHILD, x_road, y_road, DIR_0_TOP);
            child2->actionState = FIGURE_ACTION_125_ROAMING;
            child2->buildingId = b->id;
            figure_movement_init_roaming(child2);

            figure *child3 = figure_create(FIGURE_SCHOOL_CHILD, x_road, y_road, DIR_0_TOP);
            child3->actionState = FIGURE_ACTION_125_ROAMING;
            child3->buildingId = b->id;
            figure_movement_init_roaming(child3);

            figure *child4 = figure_create(FIGURE_SCHOOL_CHILD, x_road, y_road, DIR_0_TOP);
            child4->actionState = FIGURE_ACTION_125_ROAMING;
            child4->buildingId = b->id;
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
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            create_roaming_figure(b, x_road, y_road, FIGURE_LIBRARIAN);
        }
    }
}

static void spawn_figure_academy(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_TEACHER)) {
        return;
    }
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            create_roaming_figure(b, x_road, y_road, FIGURE_TEACHER);
        }
    }
}

static void spawn_figure_barber(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_BARBER)) {
        return;
    }
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            create_roaming_figure(b, x_road, y_road, FIGURE_BARBER);
        }
    }
}

static void spawn_figure_doctor(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_DOCTOR)) {
        return;
    }
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            create_roaming_figure(b, x_road, y_road, FIGURE_DOCTOR);
        }
    }
}

static void spawn_figure_hospital(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_SURGEON)) {
        return;
    }
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        int spawn_delay = default_spawn_delay(b);
        if (!spawn_delay) {
            return;
        }
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            create_roaming_figure(b, x_road, y_road, FIGURE_SURGEON);
        }
    }
}

static void spawn_figure_temple(building *b)
{
    check_labor_problem(b);
    if (has_figure_of_type(b, FIGURE_PRIEST)) {
        return;
    }
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
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
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            create_roaming_figure(b, x_road, y_road, FIGURE_PRIEST);
        }
    }
}

static void set_senate_graphic(building *b)
{
    if (b->state != BUILDING_STATE_IN_USE) {
        return;
    }
    if (map_desirability_get(b->gridOffset) <= 30) {
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
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
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
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            figure *f = figure_create(FIGURE_TAX_COLLECTOR, x_road, y_road, DIR_0_TOP);
            f->actionState = FIGURE_ACTION_40_TAX_COLLECTOR_CREATED;
            f->buildingId = b->id;
            b->figureId = f->id;
        }
    }
}

static void spawn_figure_mission_post(building *b)
{
    if (has_figure_of_type(b, FIGURE_MISSIONARY)) {
        return;
    }
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        if (city_population() > 0) {
            city_buildings_set_mission_post_operational();
            b->figureSpawnDelay++;
            if (b->figureSpawnDelay > 1) {
                b->figureSpawnDelay = 0;
                create_roaming_figure(b, x_road, y_road, FIGURE_MISSIONARY);
            }
        }
    }
}

static void spawn_figure_industry(building *b)
{
    check_labor_problem(b);
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        if (has_figure_of_type(b, FIGURE_CART_PUSHER)) {
            return;
        }
        if (building_industry_has_produced_resource(b)) {
            building_industry_start_new_production(b);
            figure *f = figure_create(FIGURE_CART_PUSHER, x_road, y_road, DIR_4_BOTTOM);
            f->actionState = FIGURE_ACTION_20_CARTPUSHER_INITIAL;
            f->resourceId = b->outputResourceId;
            f->buildingId = b->id;
            b->figureId = f->id;
            f->waitTicks = 30;
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
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        if (has_figure_of_type(b, FIGURE_CART_PUSHER)) {
            return;
        }
        if (b->figureSpawnDelay) {
            b->figureSpawnDelay = 0;
            b->data.industry.has_fish = 0;
            b->outputResourceId = RESOURCE_MEAT;
            figure *f = figure_create(FIGURE_CART_PUSHER, x_road, y_road, DIR_4_BOTTOM);
            f->actionState = FIGURE_ACTION_20_CARTPUSHER_INITIAL;
            f->resourceId = RESOURCE_MEAT;
            f->buildingId = b->id;
            b->figureId = f->id;
            f->waitTicks = 30;
        }
    }
}

static void spawn_figure_shipyard(building *b)
{
    check_labor_problem(b);
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
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
            int xBoat, yBoat;
            if (map_water_can_spawn_fishing_boat(b->x, b->y, b->size, &xBoat, &yBoat)) {
                figure *f = figure_create(FIGURE_FISHING_BOAT, xBoat, yBoat, DIR_0_TOP);
                f->actionState = FIGURE_ACTION_190_FISHING_BOAT_CREATED;
                f->buildingId = b->id;
                b->figureId = f->id;
            }
        }
    }
}

static void spawn_figure_dock(building *b)
{
    check_labor_problem(b);
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 50);
        int pct_workers = worker_percentage(b);
        int maxDockers;
        if (pct_workers >= 75) {
            maxDockers = 3;
        } else if (pct_workers >= 50) {
            maxDockers = 2;
        } else if (pct_workers > 0) {
            maxDockers = 1;
        } else {
            maxDockers = 0;
        }
        // count existing dockers
        int existingDockers = 0;
        for (int i = 0; i < 3; i++) {
            if (b->data.dock.docker_ids[i]) {
                if (figure_get(b->data.dock.docker_ids[i])->type == FIGURE_DOCKER) {
                    existingDockers++;
                } else {
                    b->data.dock.docker_ids[i] = 0;
                }
            }
        }
        if (existingDockers > maxDockers) {
            // too many dockers, kill one of them
            for (int i = 2; i >= 0; i--) {
                if (b->data.dock.docker_ids[i]) {
                    figure_get(b->data.dock.docker_ids[i])->state = FIGURE_STATE_DEAD;
                    break;
                }
            }
        } else if (existingDockers < maxDockers) {
            figure *f = figure_create(FIGURE_DOCKER, x_road, y_road, DIR_4_BOTTOM);
            f->actionState = FIGURE_ACTION_132_DOCKER_IDLING;
            f->buildingId = b->id;
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
    map_image_set(b->gridOffset, image_group(GROUP_BUILDING_NATIVE) + (map_random_get(b->gridOffset) & 1));
    if (has_figure_of_type(b, FIGURE_INDIGENOUS_NATIVE)) {
        return;
    }
    int x_out, y_out;
    if (b->subtype.nativeMeetingCenterId > 0 && map_terrain_get_adjacent_road_or_clear_land(b->x, b->y, b->size, &x_out, &y_out)) {
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > 4) {
            b->figureSpawnDelay = 0;
            figure *f = figure_create(FIGURE_INDIGENOUS_NATIVE, x_out, y_out, DIR_0_TOP);
            f->actionState = FIGURE_ACTION_158_NATIVE_CREATED;
            f->buildingId = b->id;
            b->figureId = f->id;
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
            b->figureSpawnDelay++;
            if (b->figureSpawnDelay > 8) {
                b->figureSpawnDelay = 0;
                figure *f = figure_create(FIGURE_NATIVE_TRADER, x_out, y_out, DIR_0_TOP);
                f->actionState = FIGURE_ACTION_162_NATIVE_TRADER_CREATED;
                f->buildingId = b->id;
                b->figureId = f->id;
            }
        }
    }
}

static void spawn_figure_barracks(building *b)
{
    check_labor_problem(b);
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 100);
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
        b->figureSpawnDelay++;
        if (b->figureSpawnDelay > spawn_delay) {
            b->figureSpawnDelay = 0;
            map_has_road_access(b->x, b->y, b->size, &x_road, &y_road);
            if (!building_barracks_create_tower_sentry(b, x_road, y_road)) {
                building_barracks_create_soldier(b, x_road, y_road);
            }
        }
    }
}

static void spawn_figure_military_academy(building *b)
{
    check_labor_problem(b);
    int x_road, y_road;
    if (map_has_road_access(b->x, b->y, b->size, &x_road, &y_road)) {
        spawn_labor_seeker(b, x_road, y_road, 100);
    }
}

static void update_native_crop_progress(building *b)
{
    b->data.industry.progress++;
    if (b->data.industry.progress >= 5) {
        b->data.industry.progress = 0;
    }
    map_image_set(b->gridOffset, image_group(GROUP_BUILDING_FARM_CROPS) + b->data.industry.progress);
}


void building_figure_generate()
{
    int patrician_generated = 0;
    building_barracks_decay_tower_sentry_request();
    int max_id = building_get_highest_id();
    for (int i = 1; i <= max_id; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->type == BUILDING_WAREHOUSE_SPACE || (b->type == BUILDING_HIPPODROME && b->prevPartBuildingId)) {
            continue;
        }
        b->showOnProblemOverlay = 0;
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
