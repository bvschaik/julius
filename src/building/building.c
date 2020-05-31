#include "building.h"

#include "building/building_state.h"
#include "building/properties.h"
#include "building/storage.h"
#include "city/buildings.h"
#include "city/population.h"
#include "city/warning.h"
#include "figure/formation_legion.h"
#include "game/resource.h"
#include "game/undo.h"
#include "map/building_tiles.h"
#include "map/desirability.h"
#include "map/elevation.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"

#include <string.h>

static building all_buildings[MAX_BUILDINGS];

static struct {
    int highest_id_in_use;
    int highest_id_ever;
    int created_sequence;
    int incorrect_houses;
    int unfixable_houses;
} extra = {0, 0, 0, 0};

building *building_get(int id)
{
    return &all_buildings[id];
}

building *building_main(building *b)
{
    for (int guard = 0; guard < 9; guard++) {
        if (b->prev_part_building_id <= 0) {
            return b;
        }
        b = &all_buildings[b->prev_part_building_id];
    }
    return &all_buildings[0];
}

building *building_next(building *b)
{
    return &all_buildings[b->next_part_building_id];
}

building *building_create(building_type type, int x, int y)
{
    building *b = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        if (all_buildings[i].state == BUILDING_STATE_UNUSED && !game_undo_contains_building(i)) {
            b = &all_buildings[i];
            break;
        }
    }
    if (!b) {
        city_warning_show(WARNING_DATA_LIMIT_REACHED);
        return &all_buildings[0];
    }

    const building_properties *props = building_properties_for_type(type);

    memset(&(b->data), 0, sizeof(b->data));

    b->state = BUILDING_STATE_CREATED;
    b->faction_id = 1;
    b->unknown_value = city_buildings_unknown_value();
    b->type = type;
    b->size = props->size;
    b->created_sequence = extra.created_sequence++;
    b->sentiment.house_happiness = 50;
    b->distance_from_entry = 0;

    // house size
    b->house_size = 0;
    if (type >= BUILDING_HOUSE_SMALL_TENT && type <= BUILDING_HOUSE_MEDIUM_INSULA) {
        b->house_size = 1;
    } else if (type >= BUILDING_HOUSE_LARGE_INSULA && type <= BUILDING_HOUSE_MEDIUM_VILLA) {
        b->house_size = 2;
    } else if (type >= BUILDING_HOUSE_LARGE_VILLA && type <= BUILDING_HOUSE_MEDIUM_PALACE) {
        b->house_size = 3;
    } else if (type >= BUILDING_HOUSE_LARGE_PALACE && type <= BUILDING_HOUSE_LUXURY_PALACE) {
        b->house_size = 4;
    }

    // subtype
    if (building_is_house(type)) {
        b->subtype.house_level = type - BUILDING_HOUSE_VACANT_LOT;
    } else {
        b->subtype.house_level = 0;
    }

    // input/output resources
    switch (type) {
        case BUILDING_WHEAT_FARM:
            b->output_resource_id = RESOURCE_WHEAT;
            break;
        case BUILDING_VEGETABLE_FARM:
            b->output_resource_id = RESOURCE_VEGETABLES;
            break;
        case BUILDING_FRUIT_FARM:
            b->output_resource_id = RESOURCE_FRUIT;
            break;
        case BUILDING_OLIVE_FARM:
            b->output_resource_id = RESOURCE_OLIVES;
            break;
        case BUILDING_VINES_FARM:
            b->output_resource_id = RESOURCE_VINES;
            break;
        case BUILDING_PIG_FARM:
            b->output_resource_id = RESOURCE_MEAT;
            break;
        case BUILDING_MARBLE_QUARRY:
            b->output_resource_id = RESOURCE_MARBLE;
            break;
        case BUILDING_IRON_MINE:
            b->output_resource_id = RESOURCE_IRON;
            break;
        case BUILDING_TIMBER_YARD:
            b->output_resource_id = RESOURCE_TIMBER;
            break;
        case BUILDING_CLAY_PIT:
            b->output_resource_id = RESOURCE_CLAY;
            break;
        case BUILDING_WINE_WORKSHOP:
            b->output_resource_id = RESOURCE_WINE;
            b->subtype.workshop_type = WORKSHOP_VINES_TO_WINE;
            break;
        case BUILDING_OIL_WORKSHOP:
            b->output_resource_id = RESOURCE_OIL;
            b->subtype.workshop_type = WORKSHOP_OLIVES_TO_OIL;
            break;
        case BUILDING_WEAPONS_WORKSHOP:
            b->output_resource_id = RESOURCE_WEAPONS;
            b->subtype.workshop_type = WORKSHOP_IRON_TO_WEAPONS;
            break;
        case BUILDING_FURNITURE_WORKSHOP:
            b->output_resource_id = RESOURCE_FURNITURE;
            b->subtype.workshop_type = WORKSHOP_TIMBER_TO_FURNITURE;
            break;
        case BUILDING_POTTERY_WORKSHOP:
            b->output_resource_id = RESOURCE_POTTERY;
            b->subtype.workshop_type = WORKSHOP_CLAY_TO_POTTERY;
            break;
        default:
            b->output_resource_id = RESOURCE_NONE;
            break;
    }

    if (type == BUILDING_GRANARY) {
        b->data.granary.resource_stored[RESOURCE_NONE] = 2400;
    }
    
    if (type == BUILDING_MARKET) {
	// Set it as accepting all goods
        b->subtype.market_goods = 0x0000;
    }
    
    b->x = x;
    b->y = y;
    b->grid_offset = map_grid_offset(x, y);
    b->house_figure_generation_delay = map_random_get(b->grid_offset) & 0x7f;
    b->figure_roam_direction = b->house_figure_generation_delay & 6;
    b->fire_proof = props->fire_proof;
    b->is_adjacent_to_water = map_terrain_is_adjacent_to_water(x, y, b->size);

    return b;
}

static void building_delete(building *b)
{
    building_clear_related_data(b);
    int id = b->id;
    memset(b, 0, sizeof(building));
    b->id = id;
}

void building_clear_related_data(building *b)
{
    if (b->storage_id) {
        building_storage_delete(b->storage_id);
    }
    if (b->type == BUILDING_SENATE_UPGRADED) {
        city_buildings_remove_senate(b);
    }
    if (b->type == BUILDING_DOCK) {
        city_buildings_remove_dock();
    }
    if (b->type == BUILDING_BARRACKS) {
        city_buildings_remove_barracks(b);
    }
    if (b->type == BUILDING_DISTRIBUTION_CENTER_UNUSED) {
        city_buildings_remove_distribution_center(b);
    }
    if (b->type == BUILDING_FORT) {
        formation_legion_delete_for_fort(b);
    }
    if (b->type == BUILDING_HIPPODROME) {
        city_buildings_remove_hippodrome();
    }
}

void building_update_state(void)
{
    int land_recalc = 0;
    int wall_recalc = 0;
    int road_recalc = 0;
    int aqueduct_recalc = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = &all_buildings[i];
        if (b->state == BUILDING_STATE_CREATED) {
            b->state = BUILDING_STATE_IN_USE;
        }
        if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
            if (b->state == BUILDING_STATE_UNDO || b->state == BUILDING_STATE_DELETED_BY_PLAYER) {
                if (b->type == BUILDING_TOWER || b->type == BUILDING_GATEHOUSE) {
                    wall_recalc = 1;
                    road_recalc = 1;
                } else if (b->type == BUILDING_RESERVOIR) {
                    aqueduct_recalc = 1;
                } else if (b->type == BUILDING_GRANARY) {
                    road_recalc = 1;
                }
                map_building_tiles_remove(i, b->x, b->y);
		if (b->type == BUILDING_ROADBLOCK) {
		    // Leave the road behind the deleted roadblock
		    map_terrain_add_roadblock_road(b->x,b->y,0);
                    road_recalc = 1;
		}
                land_recalc = 1;
                building_delete(b);
            } else if (b->state == BUILDING_STATE_RUBBLE) {
                if (b->house_size) {
                    city_population_remove_home_removed(b->house_population);
                }
                building_delete(b);
            } else if (b->state == BUILDING_STATE_DELETED_BY_GAME) {
                building_delete(b);
            }
        }
    }
    if (wall_recalc) {
        map_tiles_update_all_walls();
    }
    if (aqueduct_recalc) {
        map_tiles_update_all_aqueducts(0);
    }
    if (land_recalc) {
        map_routing_update_land();
    }
    if (road_recalc) {
        map_tiles_update_all_roads();
    }
}

void building_update_desirability(void)
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = &all_buildings[i];
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        b->desirability = map_desirability_get_max(b->x, b->y, b->size);
        if (b->is_adjacent_to_water) {
            b->desirability += 10;
        }
        switch (map_elevation_at(b->grid_offset)) {
            case 0: break;
            case 1: b->desirability += 10; break;
            case 2: b->desirability += 12; break;
            case 3: b->desirability += 14; break;
            case 4: b->desirability += 16; break;
            default: b->desirability += 18; break;
        }
    }
}

int building_is_house(building_type type)
{
    return type >= BUILDING_HOUSE_VACANT_LOT && type <= BUILDING_HOUSE_LUXURY_PALACE;
}

int building_is_fort(building_type type)
{
    return type == BUILDING_FORT_LEGIONARIES ||
        type == BUILDING_FORT_JAVELIN ||
        type == BUILDING_FORT_MOUNTED;
}

int building_get_highest_id(void)
{
    return extra.highest_id_in_use;
}

void building_update_highest_id(void)
{
    extra.highest_id_in_use = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        if (all_buildings[i].state != BUILDING_STATE_UNUSED) {
            extra.highest_id_in_use = i;
        }
    }
    if (extra.highest_id_in_use > extra.highest_id_ever) {
        extra.highest_id_ever = extra.highest_id_in_use;
    }
}

int building_mothball(building *b)
{
    if (b->state == BUILDING_STATE_IN_USE ) {
        b->state = BUILDING_STATE_MOTHBALLED;
        b->num_workers = 0;
    } else if (b->state == BUILDING_STATE_MOTHBALLED) {
        b->state = BUILDING_STATE_IN_USE;
    }
    return b->state;

}

void building_totals_add_corrupted_house(int unfixable)
{
    extra.incorrect_houses++;
    if (unfixable) {
        extra.unfixable_houses++;
    }
}

void building_clear_all(void)
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        memset(&all_buildings[i], 0, sizeof(building));
        all_buildings[i].id = i;
    }
    extra.highest_id_in_use = 0;
    extra.highest_id_ever = 0;
    extra.created_sequence = 0;
    extra.incorrect_houses = 0;
    extra.unfixable_houses = 0;
}

void building_save_state(buffer *buf, buffer *highest_id, buffer *highest_id_ever,
                         buffer *sequence, buffer *corrupt_houses)
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        building_state_save_to_buffer(buf, &all_buildings[i]);
    }
    buffer_write_i32(highest_id, extra.highest_id_in_use);
    buffer_write_i32(highest_id_ever, extra.highest_id_ever);
    buffer_skip(highest_id_ever, 4);
    buffer_write_i32(sequence, extra.created_sequence);

    buffer_write_i32(corrupt_houses, extra.incorrect_houses);
    buffer_write_i32(corrupt_houses, extra.unfixable_houses);
}

void building_load_state(buffer *buf, buffer *highest_id, buffer *highest_id_ever,
                         buffer *sequence, buffer *corrupt_houses)
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        building_state_load_from_buffer(buf, &all_buildings[i]);
        all_buildings[i].id = i;
    }
    extra.highest_id_in_use = buffer_read_i32(highest_id);
    extra.highest_id_ever = buffer_read_i32(highest_id_ever);
    buffer_skip(highest_id_ever, 4);
    extra.created_sequence = buffer_read_i32(sequence);

    extra.incorrect_houses = buffer_read_i32(corrupt_houses);
    extra.unfixable_houses = buffer_read_i32(corrupt_houses);
}
