#include "maintenance.h"

#include "building/building.h"
#include "building/destruction.h"
#include "building/list.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/map.h"
#include "city/message.h"
#include "city/population.h"
#include "city/sentiment.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/calc.h"
#include "core/random.h"
#include "figuretype/migrant.h"
#include "game/state.h"
#include "game/tutorial.h"
#include "game/undo.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/road_access.h"
#include "map/road_network.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "scenario/property.h"
#include "sound/effect.h"

static struct {
    int fire_spread_direction;
    int obstruction_message_displayed;
} data;

void building_maintenance_update_fire_direction(void)
{
    data.fire_spread_direction = random_byte() & 7;
}

void building_maintenance_update_burning_ruins(void)
{
    scenario_climate climate = scenario_property_climate();
    int recalculate_terrain = 0;
    building_list_burning_clear();
    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if ((b->state != BUILDING_STATE_IN_USE && b->state != BUILDING_STATE_MOTHBALLED) ||
            b->type != BUILDING_BURNING_RUIN) {
            continue;
        }
        if (b->fire_duration < 0) {
            b->fire_duration = 0;
        }
        b->fire_duration++;
        if (b->fire_duration > 32) {
            game_undo_disable();
            b->state = BUILDING_STATE_RUBBLE;
            map_building_tiles_set_rubble(i, b->x, b->y, b->size);
            recalculate_terrain = 1;
            continue;
        }
        if (b->has_plague) {
            continue;
        }
        building_list_burning_add(i);
        if (climate == CLIMATE_DESERT) {
            if (b->fire_duration & 3) { // check spread every 4 ticks
                continue;
            }
        } else {
            if (b->fire_duration & 7) { // check spread every 8 ticks
                continue;
            }
        }
        if ((b->house_figure_generation_delay & 3) != (random_byte() & 3)) {
            continue;
        }
        int dir1 = data.fire_spread_direction - 1;
        if (dir1 < 0) {
            dir1 = 7;
        }
        int dir2 = data.fire_spread_direction + 1;
        if (dir2 > 7) {
            dir2 = 0;
        }

        int grid_offset = b->grid_offset;
        int next_building_id = map_building_at(grid_offset + map_grid_direction_delta(data.fire_spread_direction));
        if (next_building_id && !building_get(next_building_id)->fire_proof) {
            building_destroy_by_fire(building_get(next_building_id));
            sound_effect_play(SOUND_EFFECT_EXPLOSION);
            recalculate_terrain = 1;
        } else {
            next_building_id = map_building_at(grid_offset + map_grid_direction_delta(dir1));
            if (next_building_id && !building_get(next_building_id)->fire_proof) {
                building_destroy_by_fire(building_get(next_building_id));
                sound_effect_play(SOUND_EFFECT_EXPLOSION);
                recalculate_terrain = 1;
            } else {
                next_building_id = map_building_at(grid_offset + map_grid_direction_delta(dir2));
                if (next_building_id && !building_get(next_building_id)->fire_proof) {
                    building_destroy_by_fire(building_get(next_building_id));
                    sound_effect_play(SOUND_EFFECT_EXPLOSION);
                    recalculate_terrain = 1;
                }
            }
        }
    }
    if (recalculate_terrain) {
        map_routing_update_land();
    }
}

int building_maintenance_get_closest_burning_ruin(int x, int y, int *distance)
{
    int min_free_building_id = 0;
    int min_occupied_building_id = 0;
    int min_occupied_dist = *distance = 10000;

    int burning_size = building_list_burning_size();
    for (int i = 0; i < burning_size; i++) {
        int building_id = building_list_burning_item(i);
        building *b = building_get(building_id);
        if ((b->state == BUILDING_STATE_IN_USE || b->state == BUILDING_STATE_MOTHBALLED) &&
            b->type == BUILDING_BURNING_RUIN && !b->has_plague && b->distance_from_entry) {
            int dist = calc_maximum_distance(x, y, b->x, b->y);
            if (b->figure_id4) {
                if (dist < min_occupied_dist) {
                    min_occupied_dist = dist;
                    min_occupied_building_id = building_id;
                }
            } else if (dist < *distance) {
                *distance = dist;
                min_free_building_id = building_id;
            }
        }
    }
    if (!min_free_building_id && min_occupied_dist <= 2) {
        min_free_building_id = min_occupied_building_id;
        *distance = 2;
    }
    return min_free_building_id;
}

static void collapse_building(building *b)
{
    city_message_apply_sound_interval(MESSAGE_CAT_COLLAPSE);
    if (!tutorial_handle_collapse()) {
        city_message_post_with_popup_delay(MESSAGE_CAT_COLLAPSE, MESSAGE_COLLAPSED_BUILDING, b->type, b->grid_offset);
    }

    game_undo_disable();
    building_destroy_by_collapse(b);
}

static void fire_building(building *b)
{
    city_message_apply_sound_interval(MESSAGE_CAT_FIRE);
    if (!tutorial_handle_fire()) {
        city_message_post_with_popup_delay(MESSAGE_CAT_FIRE, MESSAGE_FIRE, b->type, b->grid_offset);
    }

    building_destroy_by_fire(b);
    sound_effect_play(SOUND_EFFECT_EXPLOSION);
}

void building_maintenance_check_fire_collapse(void)
{
    city_sentiment_reset_protesters_criminals();

    scenario_climate climate = scenario_property_climate();
    int recalculate_terrain = 0;
    int random_global = random_byte() & 7;

    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || b->fire_proof) {
            continue;
        }
        if (b->type == BUILDING_HIPPODROME && b->prev_part_building_id) {
            continue;
        }
        int random_building = (i + map_random_get(b->grid_offset)) & 7;
        // damage
        b->damage_risk += random_building == random_global ? 3 : 1;
        if (tutorial_extra_damage_risk()) {
            b->damage_risk += 5;
        }
        if (b->house_size && b->subtype.house_level <= HOUSE_LARGE_TENT) {
            b->damage_risk = 0;
        }
        if (b->damage_risk > 200) {
            collapse_building(b);
            recalculate_terrain = 1;
            continue;
        }
        // fire
        if (random_building == random_global) {
            int fire_increase = 0;
            if (!b->house_size) {
                fire_increase += 5;
            } else if (b->house_population <= 0) {
                fire_increase = 0;
            } else if (b->subtype.house_level <= HOUSE_LARGE_SHACK) {
                fire_increase += 10;
            } else if (b->subtype.house_level <= HOUSE_GRAND_INSULA) {
                fire_increase += 5;
            } else {
                fire_increase += 2;
            }
            if (tutorial_extra_fire_risk()) {
                fire_increase += 5;
            }
            if (climate == CLIMATE_NORTHERN) {
                fire_increase = 0;
            } else if (climate == CLIMATE_DESERT) {
                fire_increase += 3;
            }

            b->fire_risk += fire_increase;
        }
        if (b->fire_risk > 100) {
            fire_building(b);
            recalculate_terrain = 1;
        }
    }

    if (recalculate_terrain) {
        map_routing_update_land();
    }
}

void building_maintenance_check_rome_access(void)
{
    const map_tile *entry_point = city_map_entry_point();
    map_routing_calculate_distances(entry_point->x, entry_point->y);
    int problem_grid_offset = 0;
    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->house_size) {
            int x_road = 0;
            int y_road = 0;
            if (!map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                // no road: eject people
                b->distance_from_entry = 0;
                b->house_unreachable_ticks++;
                if (b->house_unreachable_ticks > 4) {
                    if (b->house_population) {
                        figure_create_homeless(b, b->house_population);
                        b->house_population = 0;
                        b->house_unreachable_ticks = 0;
                    }
                    b->state = BUILDING_STATE_UNDO;
                }
            } else if (map_routing_distance(map_grid_offset(x_road, y_road))) {
                // reachable from rome
                b->distance_from_entry = map_routing_distance(map_grid_offset(x_road, y_road));
                b->house_unreachable_ticks = 0;
            } else if (map_closest_reachable_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                b->distance_from_entry = map_routing_distance(map_grid_offset(x_road, y_road));
                b->house_unreachable_ticks = 0;
            } else {
                // no reachable road in radius
                if (!b->house_unreachable_ticks) {
                    problem_grid_offset = b->grid_offset;
                }
                b->house_unreachable_ticks++;
                if (b->house_unreachable_ticks > 8) {
                    b->distance_from_entry = 0;
                    b->house_unreachable_ticks = 0;
                    b->state = BUILDING_STATE_UNDO;
                }
            }
            b->road_access_x = x_road;
            b->road_access_y = y_road;
        } else if (b->type == BUILDING_WAREHOUSE) {
            b->distance_from_entry = 0;
            int x_road, y_road;
            // Try to match the road network/access point to the loading bay first
            int road_grid_offset = map_road_to_largest_network_rotation(b->subtype.orientation, b->x, b->y, 1, &x_road, &y_road);
            // If there's no road access to the loading bay, use any tile touching the warehouse
            if (road_grid_offset < 0 || !map_terrain_is(road_grid_offset, TERRAIN_ROAD)) {
                road_grid_offset = map_road_to_largest_network_rotation(b->subtype.orientation, b->x, b->y, 3, &x_road, &y_road);
            }
            if (road_grid_offset >= 0) {
                b->road_network_id = map_road_network_get(road_grid_offset);
                b->distance_from_entry = map_routing_distance(road_grid_offset);
                b->road_access_x = x_road;
                b->road_access_y = y_road;
            }
        } else if (b->type == BUILDING_WAREHOUSE_SPACE) {
            b->distance_from_entry = 0;
            building *main_building = building_main(b);
            b->road_network_id = main_building->road_network_id;
            b->distance_from_entry = main_building->distance_from_entry;
            b->road_access_x = main_building->road_access_x;
            b->road_access_y = main_building->road_access_y;
        } else if (b->type == BUILDING_HIPPODROME) {
            b->distance_from_entry = 0;
            int x_road, y_road;
            int rotated = b->subtype.orientation;
            int road_grid_offset = map_road_to_largest_network_hippodrome(b->x, b->y, &x_road, &y_road, rotated);
            if (road_grid_offset >= 0) {
                b->road_network_id = map_road_network_get(road_grid_offset);
                b->distance_from_entry = map_routing_distance(road_grid_offset);
                b->road_access_x = x_road;
                b->road_access_y = y_road;
            }
        } else if ((b->type >= BUILDING_GRAND_TEMPLE_CERES && b->type <= BUILDING_GRAND_TEMPLE_VENUS) ||
            (b->type == BUILDING_PANTHEON)) {
            b->distance_from_entry = 0;
            int x_road, y_road;
            int road_grid_offset = map_road_to_largest_network_grand_temple(b->x, b->y, &x_road, &y_road);
            if (road_grid_offset >= 0) {
                b->road_network_id = map_road_network_get(road_grid_offset);
                b->distance_from_entry = map_routing_distance(road_grid_offset);
                b->road_access_x = x_road;
                b->road_access_y = y_road;
            }
        } else if (b->type == BUILDING_COLOSSEUM) {
            b->distance_from_entry = 0;
            int x_road, y_road;
            int road_grid_offset = map_road_to_largest_network_colosseum(b->x, b->y, &x_road, &y_road);
            if (road_grid_offset >= 0) {
                b->road_network_id = map_road_network_get(road_grid_offset);
                b->distance_from_entry = map_routing_distance(road_grid_offset);
                b->road_access_x = x_road;
                b->road_access_y = y_road;
            }
        } else if (b->type == BUILDING_LIGHTHOUSE || b->type == BUILDING_LARGE_MAUSOLEUM ||
            b->type == BUILDING_NYMPHAEUM ||
            (b->type >= BUILDING_LARGE_TEMPLE_CERES && b->type <= BUILDING_LARGE_TEMPLE_VENUS)) {
            b->distance_from_entry = 0;
            int x_road, y_road;
            int road_grid_offset = map_road_to_largest_network_lighthouse(b->x, b->y, &x_road, &y_road);
            if (road_grid_offset >= 0) {
                b->road_network_id = map_road_network_get(road_grid_offset);
                b->distance_from_entry = map_routing_distance(road_grid_offset);
                b->road_access_x = x_road;
                b->road_access_y = y_road;
            }
        } else if (b->type == BUILDING_CARAVANSERAI) {
            b->distance_from_entry = 0;
            int x_road, y_road;
            int road_grid_offset = map_road_to_largest_network_caravanserai(b->x, b->y, &x_road, &y_road);
            if (road_grid_offset >= 0) {
                b->road_network_id = map_road_network_get(road_grid_offset);
                b->distance_from_entry = map_routing_distance(road_grid_offset);
                b->road_access_x = x_road;
                b->road_access_y = y_road;
            }
        } else if (b->type == BUILDING_FORT) {
            b->distance_from_entry = 0;
            int x_road, y_road;
            int road_grid_offset = map_road_to_largest_network(b->x, b->y, b->size, &x_road, &y_road);
            if (road_grid_offset < 0) {
                int reachable = map_closest_reachable_spot_within_radius(b->x, b->y, b->size, 1, &x_road, &y_road);
                if (reachable) {
                    road_grid_offset = map_grid_offset(x_road, y_road);
                }
            }
            if (road_grid_offset >= 0) {
                b->road_network_id = map_road_network_get(road_grid_offset);
                b->distance_from_entry = map_routing_distance(road_grid_offset);
                b->road_access_x = x_road;
                b->road_access_y = y_road;
            }
        } else { // other building
            b->distance_from_entry = 0;
            int x_road, y_road;
            int road_grid_offset = map_road_to_largest_network(b->x, b->y, b->size, &x_road, &y_road);
            if (road_grid_offset >= 0) {
                b->road_network_id = map_road_network_get(road_grid_offset);
                b->distance_from_entry = map_routing_distance(road_grid_offset);
                b->road_access_x = x_road;
                b->road_access_y = y_road;
            }
        }
    }
    const map_tile *exit_point = city_map_exit_point();

    if (!map_routing_distance(exit_point->grid_offset)) {
        // no route through city
        if (city_population() <= 0) {
            return;
        }
        if (!data.obstruction_message_displayed) {
            data.obstruction_message_displayed = 1;
            city_message_post(1, MESSAGE_ROAD_TO_ROME_WARNING, 0, 0);
            game_state_pause();
            return;
        }
        for (int i = 0; i < 15; i++) {
            map_routing_delete_first_wall_or_aqueduct(entry_point->x, entry_point->y);
            map_routing_delete_first_wall_or_aqueduct(exit_point->x, exit_point->y);
            map_routing_calculate_distances(entry_point->x, entry_point->y);

            map_tiles_update_all_walls();
            map_tiles_update_all_aqueducts(0);
            map_tiles_update_all_empty_land();
            map_tiles_update_all_meadow();

            map_routing_update_land();
            map_routing_update_walls();

            if (map_routing_distance(exit_point->grid_offset)) {
                city_message_post(1, MESSAGE_ROAD_TO_ROME_OBSTRUCTED, 0, 0);
                game_undo_disable();
                return;
            }
        }
        building_destroy_last_placed();
    } else if (problem_grid_offset) {
        // parts of city disconnected
        city_warning_show(WARNING_CITY_BOXED_IN, NEW_WARNING_SLOT);
        city_warning_show(WARNING_CITY_BOXED_IN_PEOPLE_WILL_PERISH, NEW_WARNING_SLOT);
        city_view_go_to_grid_offset(problem_grid_offset);
    } else {
        data.obstruction_message_displayed = 0;
    }
}
