#include "destruction.h"

#include "city/message.h"
#include "city/population.h"
#include "city/ratings.h"
#include "core/image.h"
#include "figuretype/missile.h"
#include "figuretype/wall.h"
#include "game/undo.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "sound/effect.h"

#include "Data/CityInfo.h"

#include <string.h>

static void destroy_on_fire(building *b, int plagued)
{
    game_undo_disable();
    b->fireRisk = 0;
    b->damageRisk = 0;
    if (b->houseSize && b->housePopulation) {
        city_population_remove_home_removed(b->housePopulation);
    }
    int was_tent = b->houseSize && b->subtype.houseLevel <= HOUSE_LARGE_TENT;
    b->housePopulation = 0;
    b->houseSize = 0;
    b->outputResourceId = 0;
    b->distanceFromEntry = 0;
    building_clear_related_data(b);

    int waterside_building = 0;
    if (b->type == BUILDING_DOCK || b->type == BUILDING_WHARF || b->type == BUILDING_SHIPYARD) {
        waterside_building = 1;
    }
    int num_tiles;
    if (b->size >= 2 && b->size <= 5) {
        num_tiles = b->size * b->size;
    } else {
        num_tiles = 0;
    }
    map_building_tiles_remove(b->id, b->x, b->y);
    if (map_terrain_is(b->gridOffset, TERRAIN_WATER)) {
        b->state = BUILDING_STATE_DELETED_BY_GAME;
    } else {
        b->type = BUILDING_BURNING_RUIN;
        b->figureId4 = 0;
        b->taxIncomeOrStorage = 0;
        b->fireDuration = (b->houseGenerationDelay & 7) + 1;
        b->fireProof = 1;
        b->size = 1;
        b->ruinHasPlague = plagued;
        memset(&b->data, 0, 42);
        int image_id;
        if (was_tent) {
            image_id = image_group(GROUP_TERRAIN_RUBBLE_TENT);
        } else {
            int random = map_random_get(b->gridOffset) & 3;
            image_id = image_group(GROUP_TERRAIN_RUBBLE_GENERAL) + 9 * random;
        }
        map_building_tiles_add(b->id, b->x, b->y, 1, image_id, TERRAIN_BUILDING);
    }
    static const int x_tiles[] = {0, 1, 1, 0, 2, 2, 2, 1, 0, 3, 3, 3, 3, 2, 1, 0, 4, 4, 4, 4, 4, 3, 2, 1, 0, 5, 5, 5, 5, 5, 5, 4, 3, 2, 1, 0};
    static const int y_tiles[] = {0, 0, 1, 1, 0, 1, 2, 2, 2, 0, 1, 2, 3, 3, 3, 3, 0, 1, 2, 3, 4, 4, 4, 4, 4, 0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 5};
    for (int tile = 1; tile < num_tiles; tile++) {
        int x = x_tiles[tile] + b->x;
        int y = y_tiles[tile] + b->y;
        if (map_terrain_is(map_grid_offset(x, y), TERRAIN_WATER)) {
            continue;
        }
        building *ruin = building_create(BUILDING_BURNING_RUIN, x, y);
        int image_id;
        if (was_tent) {
            image_id = image_group(GROUP_TERRAIN_RUBBLE_TENT);
        } else {
            int random = map_random_get(ruin->gridOffset) & 3;
            image_id = image_group(GROUP_TERRAIN_RUBBLE_GENERAL) + 9 * random;
        }
        map_building_tiles_add(ruin->id, ruin->x, ruin->y, 1, image_id, TERRAIN_BUILDING);
        ruin->fireDuration = (ruin->houseGenerationDelay & 7) + 1;
        ruin->figureId4 = 0;
        ruin->fireProof = 1;
        ruin->ruinHasPlague = plagued;
    }
    if (waterside_building) {
        map_routing_update_water();
    }
}

static void destroy_linked_parts(building *b, int on_fire)
{
    building *part = b;
    for (int i = 0; i < 9; i++) {
        if (part->prevPartBuildingId <= 0) {
            break;
        }
        int part_id = part->prevPartBuildingId;
        part = building_get(part_id);
        if (on_fire) {
            destroy_on_fire(part, 0);
        } else {
            map_building_tiles_set_rubble(part_id, part->x, part->y, part->size);
            part->state = BUILDING_STATE_RUBBLE;
        }
    }

    part = b;
    for (int i = 0; i < 9; i++) {
        part = building_next(part);
        if (part->id <= 0) {
            break;
        }
        if (on_fire) {
            destroy_on_fire(part, 0);
        } else {
            map_building_tiles_set_rubble(part->id, part->x, part->y, part->size);
            part->state = BUILDING_STATE_RUBBLE;
        }
    }
}

void building_destroy_by_collapse(building *b)
{
    b->state = BUILDING_STATE_RUBBLE;
    map_building_tiles_set_rubble(b->id, b->x, b->y, b->size);
    figure_create_explosion_cloud(b->x, b->y, b->size);
    destroy_linked_parts(b, 0);
}

void building_destroy_by_fire(building *b)
{
    destroy_on_fire(b, 0);
    destroy_linked_parts(b, 1);
}

void building_destroy_by_plague(building *b)
{
    destroy_on_fire(b, 1);
}

void building_destroy_by_rioter(building *b)
{
    destroy_on_fire(b, 0);
}

int building_destroy_first_of_type(building_type type)
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->type == type) {
            int grid_offset = b->gridOffset;
            game_undo_disable();
            b->state = BUILDING_STATE_RUBBLE;
            map_building_tiles_set_rubble(i, b->x, b->y, b->size);
            sound_effect_play(SOUND_EFFECT_EXPLOSION);
            map_routing_update_land();
            return grid_offset;
        }
    }
    return 0;
}

void building_destroy_last_placed()
{
    int highest_sequence = 0;
    building *last_building = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_CREATED || b->state == BUILDING_STATE_IN_USE) {
            if (b->createdSequence > highest_sequence) {
                highest_sequence = b->createdSequence;
                last_building = b;
            }
        }
    }
    if (last_building) {
        city_message_post(1, MESSAGE_ROAD_TO_ROME_BLOCKED, 0, last_building->gridOffset);
        game_undo_disable();
        building_destroy_by_collapse(last_building);
        map_routing_update_land();
    }
}

void building_destroy_increase_enemy_damage(int grid_offset, int max_damage)
{
    if (map_building_damage_increase(grid_offset) > max_damage) {
        building_destroy_by_enemy(map_grid_offset_to_x(grid_offset),
            map_grid_offset_to_y(grid_offset), grid_offset);
    }
}

void building_destroy_by_enemy(int x, int y, int grid_offset)
{
    int building_id = map_building_at(grid_offset);
    if (building_id > 0) {
        building *b = building_get(building_id);
        if (b->state == BUILDING_STATE_IN_USE) {
            city_ratings_peace_building_destroyed(b->type);
            building_destroy_by_collapse(b);
        }
    } else {
        if (map_terrain_is(grid_offset, TERRAIN_WALL)) {
            figure_kill_tower_sentries_at(x, y);
        }
        map_building_tiles_set_rubble(0, x, y, 1);
    }
    figure_tower_sentry_reroute();
    map_tiles_update_area_walls(x, y, 3);
    map_tiles_update_region_aqueducts(x - 3, y - 3, x + 3, y + 3);
    map_routing_update_land();
    map_routing_update_walls();
}
