#include "building.h"

#include "building/building_state.h"
#include "building/building_variant.h"
#include "building/industry.h"
#include "building/granary.h"
#include "building/menu.h"
#include "building/model.h"
#include "building/monument.h"
#include "building/properties.h"
#include "building/rotation.h"
#include "building/storage.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/population.h"
#include "city/warning.h"
#include "core/array.h"
#include "core/calc.h"
#include "core/log.h"
#include "figure/figure.h"
#include "figure/formation_legion.h"
#include "game/difficulty.h"
#include "game/save_version.h"
#include "game/undo.h"
#include "map/building_tiles.h"
#include "map/desirability.h"
#include "map/elevation.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"

#define BUILDING_ARRAY_SIZE_STEP 2000

static struct {
    array(building) buildings;
    building *first_of_type[BUILDING_TYPE_MAX];
    building *last_of_type[BUILDING_TYPE_MAX];
} data;

static struct {
    int created_sequence;
    int incorrect_houses;
    int unfixable_houses;
} extra;

building *building_get(int id)
{
    return array_item(data.buildings, id);
}

int building_dist(int x, int y, int w, int h, building *b)
{
    int size = building_properties_for_type(b->type)->size;
    int dist = calc_box_distance(x, y, w, h, b->x, b->y, size, size);
    return dist;
}

void building_get_from_buffer(buffer *buf, int id, building *b, int includes_building_size, int save_version,
    int buffer_offset)
{
    buffer_set(buf, 0);
    int building_buf_size = BUILDING_STATE_ORIGINAL_BUFFER_SIZE;
    int buf_skip = 0;

    if (includes_building_size) {
        building_buf_size = buffer_read_i32(buf);
        buf_skip = 4;
    }
    buf_skip += buffer_offset;
    buffer_set(buf, building_buf_size * id + buf_skip);
    building_state_load_from_buffer(buf, b, building_buf_size, save_version, 1);
}

int building_count(void)
{
    return data.buildings.size;
}

int building_find(building_type type)
{
    for (building *b = data.first_of_type[type]; b; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE) {
            return b->id;
        }
    }
    return 0;
}

building *building_first_of_type(building_type type)
{
    return data.first_of_type[type];
}

building *building_main(building *b)
{
    for (int guard = 0; guard < 9; guard++) {
        if (b->prev_part_building_id <= 0) {
            return b;
        }
        b = array_item(data.buildings, b->prev_part_building_id);
    }
    return array_first(data.buildings);
}

building *building_next(building *b)
{
    return array_item(data.buildings, b->next_part_building_id);
}

static void fill_adjacent_types(building *b)
{
    building *first = data.first_of_type[b->type];
    building *last = data.last_of_type[b->type];
    if (!first || !last) {
        b->prev_of_type = 0;
        b->next_of_type = 0;
        data.first_of_type[b->type] = b;
        data.last_of_type[b->type] = b;
    } else if (b->id < first->id) {
        first->prev_of_type = b;
        b->next_of_type = first;
        b->prev_of_type = 0;
        data.first_of_type[b->type] = b;
    } else if (b->id > last->id) {
        last->next_of_type = b;
        b->prev_of_type = last;
        b->next_of_type = 0;
        data.last_of_type[b->type] = b;
    } else if (b != first && b != last) {
        int id = b->id - 1;
        while (id) {
            building *prev = building_get(id);
            if (prev->state != BUILDING_STATE_UNUSED &&
                prev->type == b->type) {
                b->prev_of_type = prev;
                b->next_of_type = prev->next_of_type;
                b->next_of_type->prev_of_type = b;
                prev->next_of_type = b;
                break;
            }
            id--;
        }
    }
}

static void remove_adjacent_types(building *b)
{
    building *first = data.first_of_type[b->type];
    building *last = data.last_of_type[b->type];
    if (b == first && b == last) {
        data.first_of_type[b->type] = 0;
        data.last_of_type[b->type] = 0;
    } else if (b == first) {
        data.first_of_type[b->type] = b->next_of_type;
        if (b->next_of_type) {
            b->next_of_type->prev_of_type = 0;
        }
    } else if (b == last) {
        data.last_of_type[b->type] = b->prev_of_type;
        if (b->prev_of_type) {
            b->prev_of_type->next_of_type = 0;
        }
    } else {
        b->prev_of_type->next_of_type = b->next_of_type;
        b->next_of_type->prev_of_type = b->prev_of_type;
    }
    b->prev_of_type = 0;
    b->next_of_type = 0;
}

building *building_create(building_type type, int x, int y)
{
    building *b;
    array_new_item(data.buildings, 1, b);
    if (!b) {
        city_warning_show(WARNING_DATA_LIMIT_REACHED, NEW_WARNING_SLOT);
        return array_first(data.buildings);
    }

    const building_properties *props = building_properties_for_type(type);

    memset(&(b->data), 0, sizeof(b->data));

    b->state = BUILDING_STATE_CREATED;
    b->faction_id = 1;
    b->type = type;
    b->size = props->size;
    b->created_sequence = extra.created_sequence++;
    b->sentiment.house_happiness = 100;
    b->distance_from_entry = 0;

    fill_adjacent_types(b);

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
            break;
        case BUILDING_OIL_WORKSHOP:
            b->output_resource_id = RESOURCE_OIL;
            break;
        case BUILDING_WEAPONS_WORKSHOP:
            b->output_resource_id = RESOURCE_WEAPONS;
            break;
        case BUILDING_FURNITURE_WORKSHOP:
            b->output_resource_id = RESOURCE_FURNITURE;
            break;
        case BUILDING_POTTERY_WORKSHOP:
            b->output_resource_id = RESOURCE_POTTERY;
            break;
        case BUILDING_GRAND_TEMPLE_VENUS:
            b->output_resource_id = RESOURCE_WINE;
            break;
        case BUILDING_WHARF:
            b->output_resource_id = RESOURCE_FISH;
            break;
        default:
            b->output_resource_id = RESOURCE_NONE;
            break;
    }

    if (type == BUILDING_GRANARY) {
        b->resources[RESOURCE_NONE] = FULL_GRANARY;
    }

    if (type == BUILDING_MARKET || type == BUILDING_DOCK) {
        // Set it as accepting all goods
        for (int i = 0; i < RESOURCE_MAX; i++) {
            b->accepted_goods[i] = 1;
        }
    }

    if (type == BUILDING_WAREHOUSE || type == BUILDING_HIPPODROME) {
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
    b->sickness_level = 0;
    b->sickness_duration = 0;
    b->sickness_doctor_cure = 0;
    b->fumigation_frame = 0;
    b->fumigation_direction = 0;
    return b;
}

void building_change_type(building *b, building_type type)
{
    if (b->type == type) {
        return;
    }
    remove_adjacent_types(b);
    b->type = type;
    fill_adjacent_types(b);
}

static void building_delete(building *b)
{
    building_clear_related_data(b);
    remove_adjacent_types(b);
    int id = b->id;
    memset(b, 0, sizeof(building));
    b->id = id;

    array_trim(data.buildings);
}

void building_clear_related_data(building *b)
{
    if (b->storage_id) {
        building_storage_delete(b->storage_id);
        b->storage_id = 0;
    }
    if (b->type == BUILDING_FORT) {
        formation_legion_delete_for_fort(b);
    }
    if (b->type == BUILDING_TRIUMPHAL_ARCH) {
        city_buildings_remove_triumphal_arch();
        building_menu_update();
    }
}

building *building_restore_from_undo(building *to_restore)
{
    building *b = array_item(data.buildings, to_restore->id);
    memcpy(b, to_restore, sizeof(building));
    if (b->id >= data.buildings.size) {
        data.buildings.size = b->id + 1;
    }
    fill_adjacent_types(b);
    return b;
}

void building_trim(void)
{
    array_trim(data.buildings);
}

void building_update_state(void)
{
    int land_recalc = 0;
    int wall_recalc = 0;
    int road_recalc = 0;
    int aqueduct_recalc = 0;
    building *b;
    array_foreach(data.buildings, b) {
        if (b->state == BUILDING_STATE_CREATED) {
            b->state = BUILDING_STATE_IN_USE;
        }
        if (b->state == BUILDING_STATE_IN_USE && b->house_size) {
            continue;
        }
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
            if (building_type_is_roadblock(b->type)) {
                // Leave the road behind the deleted roadblock
                map_terrain_add(b->grid_offset, TERRAIN_ROAD);
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
        } else if (b->immigrant_figure_id) {
            const figure *f = figure_get(b->immigrant_figure_id);
            if (f->state != FIGURE_STATE_ALIVE || f->destination_building_id != i) {
                b->immigrant_figure_id = 0;
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
        map_tiles_update_all_highways();
    }
}

void building_update_desirability(void)
{
    building *b;
    array_foreach(data.buildings, b)
    {
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

int building_is_active(const building *b)
{
    if (b->state != BUILDING_STATE_IN_USE) {
        return 0;
    }
    if (b->house_size) {
        return 1;
    }
    switch (b->type) {
        case BUILDING_RESERVOIR:
        case BUILDING_FOUNTAIN:
            return b->has_water_access;
        case BUILDING_ORACLE:
        case BUILDING_NYMPHAEUM:
        case BUILDING_SMALL_MAUSOLEUM:
        case BUILDING_LARGE_MAUSOLEUM:
            return b->data.monument.phase == MONUMENT_FINISHED;
        case BUILDING_WHARF:
            return b->num_workers > 0 && b->data.industry.fishing_boat_id;
        case BUILDING_DOCK:
            return b->num_workers > 0 && b->has_water_access;
        default:
            return b->num_workers > 0;
    }
}

int building_is_primary_product_producer(building_type type)
{
    return building_is_raw_resource_producer(type) || building_is_farm(type) || type == BUILDING_WHARF;
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
        (type == BUILDING_GARDENS) || (type == BUILDING_GARDEN_PATH) ||
        (type >= BUILDING_HORSE_STATUE && type <= BUILDING_LARGE_MAUSOLEUM) ||
        type == BUILDING_GARDEN_WALL || type == BUILDING_GLADIATOR_STATUE
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

// All buildings capable of collecting and storing goods as a market
int building_has_supplier_inventory(building_type type)
{
    return (type == BUILDING_MARKET ||
        type == BUILDING_MESS_HALL ||
        type == BUILDING_CARAVANSERAI ||
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

unsigned char building_stockpiling_toggle(building *b)
{
    b->data.industry.is_stockpiling = !b->data.industry.is_stockpiling;
    return b->data.industry.is_stockpiling;
}

int building_get_levy(const building *b)
{
    int levy = b->monthly_levy;
    if (levy <= 0) {
        return 0;
    }
    if (building_monument_type_is_monument(b->type) && b->data.monument.phase != MONUMENT_FINISHED) {
        return 0;
    }
    if (b->state != BUILDING_STATE_IN_USE && levy && !b->prev_part_building_id) {
        return 0;
    }
    if (b->prev_part_building_id) {
        return 0;
    }


    // Pantheon base bonus
    if (building_monument_working(BUILDING_PANTHEON) &&
        ((b->type >= BUILDING_SMALL_TEMPLE_CERES && b->type <= BUILDING_LARGE_TEMPLE_VENUS) ||
        (b->type >= BUILDING_GRAND_TEMPLE_CERES && b->type <= BUILDING_GRAND_TEMPLE_VENUS) ||
        b->type == BUILDING_ORACLE || b->type == BUILDING_NYMPHAEUM || b->type == BUILDING_SMALL_MAUSOLEUM ||
        b->type == BUILDING_LARGE_MAUSOLEUM)) {
        levy = (levy / 4) * 3;
    }

    // Mars module 1 bonus
    if (building_monument_gt_module_is_active(MARS_MODULE_1_MESS_HALL)) {
        switch (b->type) {
            case BUILDING_FORT:
                levy = (levy / 4) * 3;
                break;
            default:
                break;
        }
    }

    return difficulty_adjust_levies(levy);
}

int building_get_tourism(const building *b)
{
    return b->is_tourism_venue;
}

int building_get_laborers(building_type type)
{
    const model_building *model = model_get_building(type);
    int workers = model->laborers;
    // Neptune GT bonus
    if (type == BUILDING_FOUNTAIN && building_monument_working(BUILDING_GRAND_TEMPLE_NEPTUNE)) {
        workers /= 2;
    }
    return workers;
}

void building_totals_add_corrupted_house(int unfixable)
{
    extra.incorrect_houses++;
    if (unfixable) {
        extra.unfixable_houses++;
    }
}

static void initialize_new_building(building *b, int position)
{
    b->id = position;
}

static int building_in_use(const building *b)
{
    return b->state != BUILDING_STATE_UNUSED || game_undo_contains_building(b->id);
}

void building_clear_all(void)
{
    memset(data.first_of_type, 0, sizeof(data.first_of_type));
    memset(data.last_of_type, 0, sizeof(data.last_of_type));

    if (!array_init(data.buildings, BUILDING_ARRAY_SIZE_STEP, initialize_new_building, building_in_use) ||
        !array_next(data.buildings)) { // Ignore first building
        log_error("Unable to allocate enough memory for the building array. The game will now crash.", 0, 0);
    }

    extra.created_sequence = 0;
    extra.incorrect_houses = 0;
    extra.unfixable_houses = 0;
}

void building_save_state(buffer *buf, buffer *highest_id, buffer *highest_id_ever,
    buffer *sequence, buffer *corrupt_houses)
{
    int buf_size = 4 + data.buildings.size * BUILDING_STATE_CURRENT_BUFFER_SIZE;
    uint8_t *buf_data = malloc(buf_size);
    buffer_init(buf, buf_data, buf_size);
    buffer_write_i32(buf, BUILDING_STATE_CURRENT_BUFFER_SIZE);
    building *b;
    array_foreach(data.buildings, b)
    {
        building_state_save_to_buffer(buf, b);
    }
    buffer_write_i32(highest_id, data.buildings.size);
    buffer_write_i32(highest_id_ever, data.buildings.size);
    buffer_skip(highest_id_ever, 4);
    buffer_write_i32(sequence, extra.created_sequence);

    buffer_write_i32(corrupt_houses, extra.incorrect_houses);
    buffer_write_i32(corrupt_houses, extra.unfixable_houses);
}

void building_load_state(buffer *buf, buffer *sequence, buffer *corrupt_houses, int save_version)
{
    int building_buf_size = BUILDING_STATE_ORIGINAL_BUFFER_SIZE;
    int buf_size = buf->size;

    if (save_version > SAVE_GAME_LAST_STATIC_VERSION) {
        building_buf_size = buffer_read_i32(buf);
        buf_size -= 4;
    }

    int buildings_to_load = buf_size / building_buf_size;

    if (!array_init(data.buildings, BUILDING_ARRAY_SIZE_STEP, initialize_new_building, building_in_use) ||
        !array_expand(data.buildings, buildings_to_load)) {
        log_error("Unable to allocate enought memory for the building array. The game will now crash.", 0, 0);
    }

    memset(data.first_of_type, 0, sizeof(data.first_of_type));
    memset(data.last_of_type, 0, sizeof(data.last_of_type));

    int highest_id_in_use = 0;

    for (int i = 0; i < buildings_to_load; i++) {
        building *b = array_next(data.buildings);
        building_state_load_from_buffer(buf, b, building_buf_size, save_version, 0);
        if (b->state != BUILDING_STATE_UNUSED) {
            highest_id_in_use = i;
            fill_adjacent_types(b);
        }
    }

    // Fix messy old hack that assigned type BUILDING_GARDENS to building 0
    building *b = array_first(data.buildings);
    if (b->state == BUILDING_STATE_UNUSED && b->type == BUILDING_GARDENS) {
        b->type = BUILDING_NONE;
    }

    data.buildings.size = highest_id_in_use + 1;

    extra.created_sequence = buffer_read_i32(sequence);

    extra.incorrect_houses = buffer_read_i32(corrupt_houses);
    extra.unfixable_houses = buffer_read_i32(corrupt_houses);
}
