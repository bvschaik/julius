#include "water.h"

#include "building/building.h"
#include "building/model.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/image.h"
#include "core/random.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/water.h"
#include "scenario/map.h"

#include "Data/CityInfo.h"

static const int FLOTSAM_RESOURCE_IDS[] = {
    3, 1, 3, 2, 1, 3, 2, 3, 2, 1, 3, 3, 2, 3, 3, 3, 1, 2, 0, 1
};
static const int FLOTSAM_WAIT_TICKS[] = {
    10, 50, 100, 130, 200, 250, 400, 430, 500, 600, 70, 750, 820, 830, 900, 980, 1010, 1030, 1200, 1300
};

static const int FLOTSAM_TYPE_0[] = {0, 1, 2, 3, 4, 4, 4, 3, 2, 1, 0, 0};
static const int FLOTSAM_TYPE_12[] = {
    0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 3, 2, 1, 0, 0, 1, 1, 2, 2, 1, 1, 0, 0, 0
};
static const int FLOTSAM_TYPE_3[] = {
    0, 0, 1, 1, 2, 2, 3, 3, 4, 4, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

void figure_create_flotsam()
{
    if (!scenario_map_has_river_entry() || !scenario_map_has_river_exit() || !scenario_map_has_flotsam()) {
        return;
    }
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (f->state && f->type == FIGURE_FLOTSAM) {
            figure_delete(f);
        }
    }

    map_point river_entry = scenario_map_river_entry();
    for (int i = 0; i < 20; i++) {
        figure *f = figure_create(FIGURE_FLOTSAM, river_entry.x, river_entry.y, DIR_0_TOP);
        f->actionState = FIGURE_ACTION_128_FLOTSAM_CREATED;
        f->resourceId = FLOTSAM_RESOURCE_IDS[i];
        f->waitTicks = FLOTSAM_WAIT_TICKS[i];
    }
}

void figure_flotsam_action(figure *f)
{
    f->isBoat = 2;
    if (!scenario_map_has_river_exit()) {
        return;
    }
    f->isGhost = 0;
    f->cartGraphicId = 0;
    f->terrainUsage = FigureTerrainUsage_Any;
    switch (f->actionState) {
        case FIGURE_ACTION_128_FLOTSAM_CREATED:
            f->isGhost = 1;
            f->waitTicks--;
            if (f->waitTicks <= 0) {
                f->actionState = FIGURE_ACTION_129_FLOTSAM_FLOATING;
                f->waitTicks = 0;
                if (Data_CityInfo.godCurseNeptuneSankShips && !f->resourceId) {
                    f->minMaxSeen = 1;
                    Data_CityInfo.godCurseNeptuneSankShips = 0;
                }
                map_point river_exit = scenario_map_river_exit();
                f->destinationX = river_exit.x;
                f->destinationY = river_exit.y;
            }
            break;
        case FIGURE_ACTION_129_FLOTSAM_FLOATING:
            if (f->flotsamVisible) {
                f->flotsamVisible = 0;
            } else {
                f->flotsamVisible = 1;
                f->waitTicks++;
                figure_movement_move_ticks(f, 1);
                f->isGhost = 0;
                f->heightAdjustedTicks = 0;
                if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                    f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                    f->actionState = FIGURE_ACTION_130_FLOTSAM_OFF_MAP;
                }
            }
            break;
        case FIGURE_ACTION_130_FLOTSAM_OFF_MAP:
            f->isGhost = 1;
            f->minMaxSeen = 0;
            f->actionState = FIGURE_ACTION_128_FLOTSAM_CREATED;
            if (f->waitTicks >= 400) {
                f->waitTicks = random_byte() & 7;
            } else if (f->waitTicks >= 200) {
                f->waitTicks = 50 + (random_byte() & 0xf);
            } else if (f->waitTicks >= 100) {
                f->waitTicks = 100 + (random_byte() & 0x1f);
            } else if (f->waitTicks >= 50) {
                f->waitTicks = 200 + (random_byte() & 0x3f);
            } else {
                f->waitTicks = 300 + random_byte();
            }
            map_figure_delete(f);
            map_point river_entry = scenario_map_river_entry();
            f->x = river_entry.x;
            f->y = river_entry.y;
            f->gridOffset = map_grid_offset(f->x, f->y);
            f->crossCountryX = 15 * f->x;
            f->crossCountryY = 15 * f->y;
            break;
    }
    if (f->resourceId == 0) {
        figure_image_increase_offset(f, 12);
        if (f->minMaxSeen) {
            f->graphicId = image_group(GROUP_FIGURE_FLOTSAM_Sheep) + FLOTSAM_TYPE_0[f->graphicOffset];
        } else {
            f->graphicId = image_group(GROUP_FIGURE_FLOTSAM_0) + FLOTSAM_TYPE_0[f->graphicOffset];
        }
    } else if (f->resourceId == 1) {
        figure_image_increase_offset(f, 24);
        f->graphicId = image_group(GROUP_FIGURE_FLOTSAM_1) + FLOTSAM_TYPE_12[f->graphicOffset];
    } else if (f->resourceId == 2) {
        figure_image_increase_offset(f, 24);
        f->graphicId = image_group(GROUP_FIGURE_FLOTSAM_2) + FLOTSAM_TYPE_12[f->graphicOffset];
    } else if (f->resourceId == 3) {
        figure_image_increase_offset(f, 24);
        if (FLOTSAM_TYPE_3[f->graphicOffset] == -1) {
            f->graphicId = 0;
        } else {
            f->graphicId = image_group(GROUP_FIGURE_FLOTSAM_3) + FLOTSAM_TYPE_3[f->graphicOffset];
        }
    }
}

void figure_shipwreck_action(figure *f)
{
    f->isGhost = 0;
    f->heightAdjustedTicks = 0;
    f->isBoat = 1;
    figure_image_increase_offset(f, 128);
    if (f->waitTicks < 1000) {
        map_figure_delete(f);
        int x_tile, y_tile;
        if (map_water_find_shipwreck_tile(f, &x_tile, &y_tile)) {
            f->x = x_tile;
            f->y = y_tile;
            f->gridOffset = map_grid_offset(f->x, f->y);
            f->crossCountryX = 15 * f->x + 7;
            f->crossCountryY = 15 * f->y + 7;
        }
        map_figure_add(f);
        f->waitTicks = 1000;
    }
    f->waitTicks++;
    if (f->waitTicks > 2000) {
        f->state = FigureState_Dead;
    }
    f->graphicId = image_group(GROUP_FIGURE_SHIPWRECK) + f->graphicOffset / 16;
}

void figure_fishing_boat_action(figure *f)
{
    building *b = building_get(f->buildingId);
    if (b->state != BUILDING_STATE_IN_USE) {
        f->state = FigureState_Dead;
    }
    if (f->actionState != FIGURE_ACTION_190_FISHING_BOAT_CREATED && b->data.other.boatFigureId != f->id) {
        int x_tile, y_tile;
        b = building_get(map_water_get_wharf_for_new_fishing_boat(f, &x_tile, &y_tile));
        if (b->id) {
            f->buildingId = b->id;
            b->data.other.boatFigureId = f->id;
            f->actionState = FIGURE_ACTION_193_FISHING_BOAT_GOING_TO_WHARF;
            f->destinationX = x_tile;
            f->destinationY = y_tile;
            f->sourceX = x_tile;
            f->sourceY = y_tile;
            figure_route_remove(f);
        } else {
            f->state = FigureState_Dead;
        }
    }
    f->isGhost = 0;
    f->isBoat = 1;
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    switch (f->actionState) {
        case FIGURE_ACTION_190_FISHING_BOAT_CREATED:
            f->waitTicks++;
            if (f->waitTicks >= 50) {
                f->waitTicks = 0;
                int x_tile, y_tile;
                int wharf_id = map_water_get_wharf_for_new_fishing_boat(f, &x_tile, &y_tile);
                if (wharf_id) {
                    b->figureId = 0; // remove from original building
                    f->buildingId = wharf_id;
                    building_get(wharf_id)->data.other.boatFigureId = f->id;
                    f->actionState = FIGURE_ACTION_193_FISHING_BOAT_GOING_TO_WHARF;
                    f->destinationX = x_tile;
                    f->destinationY = y_tile;
                    f->sourceX = x_tile;
                    f->sourceY = y_tile;
                    figure_route_remove(f);
                }
            }
            break;
        case FIGURE_ACTION_191_FISHING_BOAT_GOING_TO_FISH:
            figure_movement_move_ticks(f, 1);
            f->heightAdjustedTicks = 0;
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                int x_tile, y_tile;
                if (map_water_find_alternative_fishing_boat_tile(f, &x_tile, &y_tile)) {
                    figure_route_remove(f);
                    f->destinationX = x_tile;
                    f->destinationY = y_tile;
                    f->direction = f->previousTileDirection;
                } else {
                    f->actionState = FIGURE_ACTION_192_FISHING_BOAT_FISHING;
                    f->waitTicks = 0;
                }
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->actionState = FIGURE_ACTION_194_FISHING_BOAT_AT_WHARF;
                f->destinationX = f->sourceX;
                f->destinationY = f->sourceY;
            }
            break;
        case FIGURE_ACTION_192_FISHING_BOAT_FISHING:
            f->waitTicks++;
            if (f->waitTicks >= 200) {
                f->waitTicks = 0;
                f->actionState = FIGURE_ACTION_195_FISHING_BOAT_RETURNING_WITH_FISH;
                f->destinationX = f->sourceX;
                f->destinationY = f->sourceY;
                figure_route_remove(f);
            }
            break;
        case FIGURE_ACTION_193_FISHING_BOAT_GOING_TO_WHARF:
            figure_movement_move_ticks(f, 1);
            f->heightAdjustedTicks = 0;
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_194_FISHING_BOAT_AT_WHARF;
                f->waitTicks = 0;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                // cannot reach grounds
                city_message_post_with_message_delay(MESSAGE_CAT_FISHING_BLOCKED, 1, MESSAGE_FISHING_BOAT_BLOCKED, 12);
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_194_FISHING_BOAT_AT_WHARF:
            {
            int pct_workers = calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
            int max_wait_ticks = 5 * (102 - pct_workers);
            if (b->data.other.fishingBoatHasFish > 0) {
                pct_workers = 0;
            }
            if (pct_workers > 0) {
                f->waitTicks++;
                if (f->waitTicks >= max_wait_ticks) {
                    f->waitTicks = 0;
                    int x_tile, y_tile;
                    if (scenario_map_closest_fishing_point(f->x, f->y, &x_tile, &y_tile)) {
                        f->actionState = FIGURE_ACTION_191_FISHING_BOAT_GOING_TO_FISH;
                        f->destinationX = x_tile;
                        f->destinationY = y_tile;
                        figure_route_remove(f);
                    }
                }
            }
            }
            break;
        case FIGURE_ACTION_195_FISHING_BOAT_RETURNING_WITH_FISH:
            figure_movement_move_ticks(f, 1);
            f->heightAdjustedTicks = 0;
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_194_FISHING_BOAT_AT_WHARF;
                f->waitTicks = 0;
                b->figureSpawnDelay = 1;
                b->data.other.fishingBoatHasFish++;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
    }
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previousTileDirection);
    
    if (f->actionState == FIGURE_ACTION_192_FISHING_BOAT_FISHING) {
        f->graphicId = image_group(GROUP_FIGURE_SHIP) + dir + 16;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_SHIP) + dir + 8;
    }
}

void figure_sink_all_ships()
{
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (f->state != FigureState_Alive) {
            continue;
        }
        if (f->type == FIGURE_TRADE_SHIP) {
            building_get(f->destinationBuildingId)->data.dock.trade_ship_id = 0;
        } else if (f->type == FIGURE_FISHING_BOAT) {
            building_get(f->buildingId)->data.other.boatFigureId = 0;
        } else {
            continue;
        }
        f->buildingId = 0;
        f->type = FIGURE_SHIPWRECK;
        f->waitTicks = 0;
    }
}
