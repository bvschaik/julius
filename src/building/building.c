#include "building.h"

#include "building/building_state.h"
#include "building/monument.h"
#include "building/properties.h"
#include "building/rotation.h"
#include "building/storage.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/population.h"
#include "city/warning.h"
#include "figure/formation_legion.h"
#include "game/difficulty.h"
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
#include "menu.h"

#include <string.h>
#include <stdlib.h>

#define BUILDING_ARRAY_SIZE_STEP 2000

static struct {
    building *all_buildings;
    int building_array_size;
} data;

static struct {
    int highest_id_in_use;
    int highest_id_ever;
    int created_sequence;
    int incorrect_houses;
    int unfixable_houses;
} extra = {0, 0, 0, 0};

building *building_get(int id)
{
    return &data.all_buildings[id];
}

int building_count(void)
{
    return data.building_array_size;
}

int building_find(building_type type)
{
    for (int i = 1; i < data.building_array_size; ++i) {
        building *b = &data.all_buildings[i];
        if (b->state == BUILDING_STATE_IN_USE && b->type == type) {
            return i;
        }
    }
    return 0;
}

building *building_main(building *b)
{
    for (int guard = 0; guard < 9; guard++) {
        if (b->prev_part_building_id <= 0) {
            return b;
        }
        b = &data.all_buildings[b->prev_part_building_id];
    }
    return &data.all_buildings[0];
}

building *building_next(building *b)
{
    return &data.all_buildings[b->next_part_building_id];
}

static void create_building_array(int size)
{
    free(data.all_buildings);
    data.building_array_size = size;
    data.all_buildings = malloc(size * sizeof(building));
    for (int i = 0; i < size; i++) {
        memset(&data.all_buildings[i], 0, sizeof(building));
        data.all_buildings[i].id = i;
    }
}

static int expand_building_array(void)
{
    building *b = realloc(data.all_buildings, (data.building_array_size + BUILDING_ARRAY_SIZE_STEP) * sizeof(building));
    if (!b) {
        return 0;
    }
    data.all_buildings = b;
    data.building_array_size += BUILDING_ARRAY_SIZE_STEP;
    for (int i = data.building_array_size - BUILDING_ARRAY_SIZE_STEP; i < data.building_array_size; i++) {
        memset(&data.all_buildings[i], 0, sizeof(building));
        data.all_buildings[i].id = i;
    }
    return 1;
}

building *building_create(building_type type, int x, int y)
{
    building *b = 0;
    for (int i = 1; i < data.building_array_size; i++) {
        if (data.all_buildings[i].state == BUILDING_STATE_UNUSED && !game_undo_contains_building(i)) {
            b = &data.all_buildings[i];
            break;
        }
    }
    if (!b) {
        if (!expand_building_array()) {
            city_warning_show(WARNING_DATA_LIMIT_REACHED);
            return &data.all_buildings[0];
        }
        b = &data.all_buildings[data.building_array_size - BUILDING_ARRAY_SIZE_STEP];
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
        case BUILDING_GRAND_TEMPLE_VENUS:
            b->output_resource_id = RESOURCE_WINE;
            break;
        case BUILDING_WHARF:
            b->output_resource_id = RESOURCE_MEAT;
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

    if(type == BUILDING_WAREHOUSE || type == BUILDING_HIPPODROME) {
        b->subtype.orientation = building_rotation_get_rotation();
    }
    
    b->x = x;
    b->y = y;
    b->grid_offset = map_grid_offset(x, y);
    b->house_figure_generation_delay = map_random_get(b->grid_offset) & 0x7f;
    b->figure_roam_direction = b->house_figure_generation_delay & 6;
    b->fire_proof = props->fire_proof;
    b->is_adjacent_to_water = map_terrain_is_adjacent_to_water(x, y, b->size);

    // init expanded data
    b->house_tavern_wine_access = 0;
    b->house_tavern_meat_access = 0;
    b->house_arena_gladiator = 0;
    b->house_arena_lion = 0;
    b->is_tourism_venue = 0;
    b->tourism_disabled = 0;
    b->tourism_income = 0;
    b->tourism_income_this_year = 0;
    b->upgrade_level = 0;
    b->variant = 0;
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
    if (b->type == BUILDING_TRIUMPHAL_ARCH) {
        city_buildings_remove_triumphal_arch();
        building_menu_update();
    }
    if (b->type == BUILDING_MESS_HALL) {
        city_buildings_remove_mess_hall();
    }
}

void building_update_state(void)
{
    int land_recalc = 0;
    int wall_recalc = 0;
    int road_recalc = 0;
    int aqueduct_recalc = 0;
    for (int i = 1; i < data.building_array_size; i++) {
        building *b = &data.all_buildings[i];
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
                } else if ((b->type >= BUILDING_GRAND_TEMPLE_CERES && b->type <= BUILDING_GRAND_TEMPLE_VENUS) || b->type == BUILDING_PANTHEON || b->type == BUILDING_LIGHTHOUSE) {
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
    for (int i = 1; i < data.building_array_size; i++) {
        building *b = &data.all_buildings[i];
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

// For Venus GT base bonus
int building_is_statue_garden_temple(building_type type)
{
    return ((type >= BUILDING_SMALL_TEMPLE_CERES && type <= BUILDING_LARGE_TEMPLE_VENUS) ||
        (type >= BUILDING_GRAND_TEMPLE_CERES && type <= BUILDING_GRAND_TEMPLE_VENUS) ||
        (type >= BUILDING_SMALL_STATUE && type <= BUILDING_LARGE_STATUE) ||
        (type >= BUILDING_SMALL_POND && type <= BUILDING_PANTHEON) ||
        (type == BUILDING_GARDENS)
     );
}

int building_is_ceres_temple(building_type type)
{
    return (type == BUILDING_SMALL_TEMPLE_CERES || type == BUILDING_LARGE_TEMPLE_CERES);
}

int building_is_neptune_temple(building_type type)
{
    return (type == BUILDING_SMALL_TEMPLE_NEPTUNE || type == BUILDING_LARGE_TEMPLE_NEPTUNE);
}

int building_is_mercury_temple(building_type type)
{
    return (type == BUILDING_SMALL_TEMPLE_MERCURY || type == BUILDING_LARGE_TEMPLE_MERCURY);
}

int building_is_mars_temple(building_type type)
{
    return (type == BUILDING_SMALL_TEMPLE_MARS || type == BUILDING_LARGE_TEMPLE_MARS);
}

int building_is_venus_temple(building_type type)
{
    return (type == BUILDING_SMALL_TEMPLE_VENUS || type == BUILDING_LARGE_TEMPLE_VENUS);
}

// all buildings capable of collecting and storing goods as a market
int building_has_supplier_inventory(building_type type) {
    return (type == BUILDING_MARKET ||
        type == BUILDING_MESS_HALL ||
        type == BUILDING_SMALL_TEMPLE_CERES ||
        type == BUILDING_LARGE_TEMPLE_CERES ||
        type == BUILDING_SMALL_TEMPLE_VENUS ||
        type == BUILDING_LARGE_TEMPLE_VENUS ||
        type == BUILDING_TAVERN);
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
    for (int i = 1; i < data.building_array_size; i++) {
        if (data.all_buildings[i].state != BUILDING_STATE_UNUSED) {
            extra.highest_id_in_use = i;
        }
    }
    if (extra.highest_id_in_use > extra.highest_id_ever) {
        extra.highest_id_ever = extra.highest_id_in_use;
    }
}

int building_mothball_toggle(building *b)
{
    if (b->state == BUILDING_STATE_IN_USE) {
        b->state = BUILDING_STATE_MOTHBALLED;
        b->num_workers = 0;
    } else if (b->state == BUILDING_STATE_MOTHBALLED) {
        b->state = BUILDING_STATE_IN_USE;
    }
    return b->state;
}

int building_mothball_set(building *b, int mothball)
{
    if (mothball) {
        if (b->state == BUILDING_STATE_IN_USE) {
            b->state = BUILDING_STATE_MOTHBALLED;
            b->num_workers = 0;
        }
    } else if (b->state == BUILDING_STATE_MOTHBALLED) {
        b->state = BUILDING_STATE_IN_USE;
    }
    return b->state;

}

int building_get_levy(const building* b)
{
    int levy = b->monthly_levy;
    if (levy <= 0) {
        return 0;
    }
    //Pantheon base bonus
    if (building_monument_working(BUILDING_PANTHEON) && 
        ((b->type >= BUILDING_SMALL_TEMPLE_CERES && b->type <= BUILDING_LARGE_TEMPLE_VENUS) ||
        (b->type >= BUILDING_GRAND_TEMPLE_CERES && b->type <= BUILDING_GRAND_TEMPLE_VENUS) || b->type == BUILDING_ORACLE)) {
        levy = (levy / 4) * 3;
    }
        
    return difficulty_adjust_levies(levy);
    
}

int building_get_tourism(const building* b)
{
    return b->is_tourism_venue;
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
    create_building_array(BUILDING_ARRAY_SIZE_STEP);

    extra.highest_id_in_use = 0;
    extra.highest_id_ever = 0;
    extra.created_sequence = 0;
    extra.incorrect_houses = 0;
    extra.unfixable_houses = 0;
}

void building_save_state(buffer *buf, buffer *highest_id, buffer *highest_id_ever,
                         buffer *sequence, buffer *corrupt_houses)
{
    int buf_size = 4 + data.building_array_size * BUILDING_STATE_CURRENT_BUFFER_SIZE;
    uint8_t *buf_data = malloc(buf_size);
    buffer_init(buf, buf_data, buf_size);
    buffer_write_i32(buf, BUILDING_STATE_CURRENT_BUFFER_SIZE);
    for (int i = 0; i < data.building_array_size; i++) {
        building_state_save_to_buffer(buf, &data.all_buildings[i]);
    }
    buffer_write_i32(highest_id, extra.highest_id_in_use);
    buffer_write_i32(highest_id_ever, extra.highest_id_ever);
    buffer_skip(highest_id_ever, 4);
    buffer_write_i32(sequence, extra.created_sequence);

    buffer_write_i32(corrupt_houses, extra.incorrect_houses);
    buffer_write_i32(corrupt_houses, extra.unfixable_houses);
}

void building_load_state(buffer *buf, buffer *highest_id, buffer *highest_id_ever,
                         buffer *sequence, buffer *corrupt_houses, int includes_building_size)
{
    int building_buf_size = BUILDING_STATE_ORIGINAL_BUFFER_SIZE;
    int buf_size = buf->size;

    if (includes_building_size) {
        building_buf_size = buffer_read_i32(buf);
        buf_size -= 4;
    }      
    
    int buildings_to_load = buf_size / building_buf_size;

    create_building_array(buildings_to_load);

    // Reduce number of used buildings on old Augustus savefiles that were hardcoded to load 10000. Improves performance
    int highest_id_in_use = 0;
    int reduce_building_array_size = !includes_building_size && buildings_to_load == 10000;

    for (int i = 0; i < data.building_array_size; i++) {
        building_state_load_from_buffer(buf, &data.all_buildings[i], building_buf_size);
        data.all_buildings[i].id = i;
        if (reduce_building_array_size && data.all_buildings[i].state != BUILDING_STATE_UNUSED) {
            highest_id_in_use = i;
        }
    }
    if (reduce_building_array_size) {
        data.building_array_size = BUILDING_ARRAY_SIZE_STEP;
        while (highest_id_in_use > data.building_array_size) {
            data.building_array_size += BUILDING_ARRAY_SIZE_STEP;
        }
    }

    extra.highest_id_in_use = buffer_read_i32(highest_id);
    extra.highest_id_ever = buffer_read_i32(highest_id_ever);
    buffer_skip(highest_id_ever, 4);
    extra.created_sequence = buffer_read_i32(sequence);

    extra.incorrect_houses = buffer_read_i32(corrupt_houses);
    extra.unfixable_houses = buffer_read_i32(corrupt_houses);
}
