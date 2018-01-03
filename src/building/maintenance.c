#include "maintenance.h"

#include "building/building.h"
#include "building/destruction.h"
#include "building/list.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/random.h"
#include "figuretype/missile.h"
#include "game/tutorial.h"
#include "game/undo.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/routing_terrain.h"
#include "scenario/property.h"
#include "sound/effect.h"

#include "Data/CityInfo.h"

static int fire_spread_direction = 0;

void building_maintenance_update_fire_direction()
{
    fire_spread_direction = random_byte() & 7;
}

void building_maintenance_update_burning_ruins()
{
    scenario_climate climate = scenario_property_climate();
    int recalculate_terrain = 0;
    building_list_burning_clear();
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || b->type != BUILDING_BURNING_RUIN) {
            continue;
        }
        if (b->fireDuration < 0) {
            b->fireDuration = 0;
        }
        b->fireDuration++;
        if (b->fireDuration > 32) {
            game_undo_disable();
            b->state = BUILDING_STATE_RUBBLE;
            map_building_tiles_set_rubble(i, b->x, b->y, b->size);
            recalculate_terrain = 1;
            continue;
        }
        if (b->ruinHasPlague) {
            continue;
        }
        building_list_burning_add(i);
        if (climate == CLIMATE_DESERT) {
            if (b->fireDuration & 3) { // check spread every 4 ticks
                continue;
            }
        } else {
            if (b->fireDuration & 7) { // check spread every 8 ticks
                continue;
            }
        }
        if ((b->houseGenerationDelay & 3) != (random_byte() & 3)) {
            continue;
        }
        int dir1 = fire_spread_direction - 1;
        if (dir1 < 0) dir1 = 7;
        int dir2 = fire_spread_direction + 1;
        if (dir2 > 7) dir2 = 0;
        
        int grid_offset = b->gridOffset;
        int next_building_id = map_building_at(grid_offset + map_grid_direction_delta(fire_spread_direction));
        if (next_building_id && !building_get(next_building_id)->fireProof) {
            building_destroy_by_fire(building_get(next_building_id));
            sound_effect_play(SOUND_EFFECT_EXPLOSION);
            recalculate_terrain = 1;
        } else {
            next_building_id = map_building_at(grid_offset + map_grid_direction_delta(dir1));
            if (next_building_id && !building_get(next_building_id)->fireProof) {
                building_destroy_by_fire(building_get(next_building_id));
                sound_effect_play(SOUND_EFFECT_EXPLOSION);
                recalculate_terrain = 1;
            } else {
                next_building_id = map_building_at(grid_offset + map_grid_direction_delta(dir2));
                if (next_building_id && !building_get(next_building_id)->fireProof) {
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

    const int *burning = building_list_burning_items();
    int burning_size = building_list_burning_size();
    for (int i = 0; i < burning_size; i++) {
        int building_id = burning[i];
        building *b = building_get(building_id);
        if (BuildingIsInUse(b) && b->type == BUILDING_BURNING_RUIN && !b->ruinHasPlague && b->distanceFromEntry) {
            int dist = calc_maximum_distance(x, y, b->x, b->y);
            if (b->figureId4) {
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
        city_message_post_with_popup_delay(MESSAGE_CAT_COLLAPSE, MESSAGE_COLLAPSED_BUILDING, b->type, b->gridOffset);
    }
    
    game_undo_disable();
    building_destroy_by_collapse(b);
}

static void fire_building(building *b)
{
    city_message_apply_sound_interval(MESSAGE_CAT_FIRE);
    if (!tutorial_handle_fire()) {
        city_message_post_with_popup_delay(MESSAGE_CAT_FIRE, MESSAGE_FIRE, b->type, b->gridOffset);
    }
    
    building_destroy_by_fire(b);
    sound_effect_play(SOUND_EFFECT_EXPLOSION);
}

void building_maintenance_check_fire_collapse()
{
    Data_CityInfo.numProtestersThisMonth = 0;
    Data_CityInfo.numCriminalsThisMonth = 0;

    scenario_climate climate = scenario_property_climate();
    int recalculate_terrain = 0;
    int random_global = random_byte() & 7;
    for (int i = 1; i <= Data_Buildings_Extra.highestBuildingIdInUse; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || b->fireProof) {
            continue;
        }
        if (b->type == BUILDING_HIPPODROME && b->prevPartBuildingId) {
            continue;
        }
        int random_building = (i + map_random_get(b->gridOffset)) & 7;
        // damage
        b->damageRisk += (random_building == random_global) ? 3 : 1;
        if (tutorial_extra_damage_risk()) {
            b->damageRisk += 5;
        }
        if (b->houseSize && b->subtype.houseLevel <= HOUSE_LARGE_TENT) {
            b->damageRisk = 0;
        }
        if (b->damageRisk > 200) {
            collapse_building(b);
            recalculate_terrain = 1;
            continue;
        }
        // fire
        if (random_building == random_global) {
            if (!b->houseSize) {
                b->fireRisk += 5;
            } else if (b->housePopulation <= 0) {
                b->fireRisk = 0;
            } else if (b->subtype.houseLevel <= HOUSE_LARGE_SHACK) {
                b->fireRisk += 10;
            } else if (b->subtype.houseLevel <= HOUSE_GRAND_INSULA) {
                b->fireRisk += 5;
            } else {
                b->fireRisk += 2;
            }
            if (tutorial_extra_fire_risk()) {
                b->fireRisk += 5;
            }
            if (climate == CLIMATE_NORTHERN) {
                b->fireRisk = 0;
            } else if (climate == CLIMATE_DESERT) {
                b->fireRisk += 3;
            }
        }
        if (b->fireRisk > 100) {
            fire_building(b);
            recalculate_terrain = 1;
        }
    }

    if (recalculate_terrain) {
        map_routing_update_land();
    }
}
