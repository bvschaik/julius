#include "monument.h"

#include "assets/assets.h"
#include "building/image.h"
#include "building/model.h"
#include "building/properties.h"
#include "city/finance.h"
#include "city/message.h"
#include "city/resource.h"
#include "core/array.h"
#include "core/calc.h"
#include "core/log.h"
#include "empire/city.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/orientation.h"
#include "map/road_access.h"
#include "map/terrain.h"
#include "scenario/property.h"

#define MONUMENTS_SIZE_STEP 100
#define DELIVERY_ARRAY_SIZE_STEP 200
#define ORIGINAL_DELIVERY_BUFFER_SIZE 16
#define MODULES_PER_TEMPLE 2
#define ARCHITECTS RESOURCE_NONE

#define MAX_PHASES 6

#define BUILDING_MONUMENT_FIRST_ID BUILDING_HIPPODROME

#define NOTHING 0
#define INFINITE 10000

typedef struct {
    const int phases;
    const int resources[MAX_PHASES][RESOURCE_MAX];
} monument_type;

static const monument_type grand_temple = {
    .phases    = 6,
    .resources = {
        { [ARCHITECTS] = 1, [RESOURCE_MARBLE] = 20 },
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 8, [RESOURCE_MARBLE] = 20 },
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 8, [RESOURCE_MARBLE] = 16 },
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 8, [RESOURCE_CLAY] = 28, [RESOURCE_MARBLE] = 12 },
        { [ARCHITECTS] = 4 },
        { NOTHING }
    }
};

static const monument_type pantheon = {
    .phases    = 6,
    .resources = {
        { [ARCHITECTS] = 1, [RESOURCE_MARBLE] = 16 },
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 8,  [RESOURCE_MARBLE] = 16 },
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 16, [RESOURCE_MARBLE] = 32 },
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 32, [RESOURCE_CLAY] = 40, [RESOURCE_MARBLE] = 32 },
        { [ARCHITECTS] = 4 },
        { NOTHING }
    }
};

static const monument_type lighthouse = {
    .phases    = 5,
    .resources = {
        { [ARCHITECTS] = 1, [RESOURCE_MARBLE] = 12 },
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 8,  [RESOURCE_MARBLE] = 12 },
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 16, [RESOURCE_MARBLE] = 8 },
        { [ARCHITECTS] = 4, [RESOURCE_TIMBER] = 8,  [RESOURCE_CLAY] = 20, [RESOURCE_MARBLE] = 8 },
        { NOTHING }
    }
};

static const monument_type colosseum = {
    .phases    = 5,
    .resources = {
        { [ARCHITECTS] = 1, [RESOURCE_MARBLE] = 12 },
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 8,  [RESOURCE_MARBLE] = 16 },
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 12, [RESOURCE_MARBLE] = 16 },
        { [ARCHITECTS] = 4, [RESOURCE_TIMBER] = 12, [RESOURCE_CLAY] = 16, [RESOURCE_MARBLE] = 12 },
        { NOTHING }
    }
};

static const monument_type hippodrome = {
    .phases    = 5,
    .resources = {
        { [ARCHITECTS] = 1, [RESOURCE_MARBLE] = 32 },
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 16, [RESOURCE_MARBLE] = 32 },
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 16, [RESOURCE_MARBLE] = 32 },
        { [ARCHITECTS] = 4, [RESOURCE_TIMBER] = 32, [RESOURCE_CLAY] = 46, [RESOURCE_MARBLE] = 32 },
        { NOTHING }
    }
};

static const monument_type oracle_and_small_mausuleum = {
    .phases    = 2,
    .resources = {
        { [ARCHITECTS] = 1, [RESOURCE_MARBLE] = 2 },
        { NOTHING }
    }
};

static const monument_type large_temple_nymphaeum_and_large_mausuleum = {
    .phases    = 2,
    .resources = {
        { [ARCHITECTS] = 1, [RESOURCE_MARBLE] = 4 },
        { NOTHING }
    }
};


static const monument_type caravanserai = {
    .phases    = 2,
    .resources = {
        { [ARCHITECTS] = 1, [RESOURCE_TIMBER] = 6, [RESOURCE_CLAY] = 8, [RESOURCE_MARBLE] = 6 },
        { NOTHING }
    }
};

static const monument_type city_mint = {
    .phases    = 2,
    .resources = {
        { [ARCHITECTS] = 2, [RESOURCE_TIMBER] = 6, [RESOURCE_CLAY] = 8, [RESOURCE_IRON] = 4 },
        { NOTHING }
    }
};

static const monument_type *MONUMENT_TYPES[BUILDING_TYPE_MAX] = {
    [BUILDING_GRAND_TEMPLE_CERES]   = &grand_temple,
    [BUILDING_GRAND_TEMPLE_NEPTUNE] = &grand_temple,
    [BUILDING_GRAND_TEMPLE_MERCURY] = &grand_temple,
    [BUILDING_GRAND_TEMPLE_MARS]    = &grand_temple,
    [BUILDING_GRAND_TEMPLE_VENUS]   = &grand_temple,
    [BUILDING_PANTHEON]             = &pantheon,
    [BUILDING_ORACLE]               = &oracle_and_small_mausuleum,
    [BUILDING_LARGE_TEMPLE_CERES]   = &large_temple_nymphaeum_and_large_mausuleum,
    [BUILDING_LARGE_TEMPLE_NEPTUNE] = &large_temple_nymphaeum_and_large_mausuleum,
    [BUILDING_LARGE_TEMPLE_MERCURY] = &large_temple_nymphaeum_and_large_mausuleum,
    [BUILDING_LARGE_TEMPLE_MARS]    = &large_temple_nymphaeum_and_large_mausuleum,
    [BUILDING_LARGE_TEMPLE_VENUS]   = &large_temple_nymphaeum_and_large_mausuleum,
    [BUILDING_LIGHTHOUSE]           = &lighthouse,
    [BUILDING_COLOSSEUM]            = &colosseum,
    [BUILDING_HIPPODROME]           = &hippodrome,
    [BUILDING_NYMPHAEUM]            = &large_temple_nymphaeum_and_large_mausuleum,
    [BUILDING_LARGE_MAUSOLEUM]      = &large_temple_nymphaeum_and_large_mausuleum,
    [BUILDING_SMALL_MAUSOLEUM]      = &oracle_and_small_mausuleum,
    [BUILDING_CARAVANSERAI]         = &caravanserai,
    [BUILDING_CITY_MINT]            = &city_mint
};

typedef struct {
    int walker_id;
    int destination_id;
    int resource;
    int cartloads;
} monument_delivery;

array(monument_delivery) monument_deliveries;

int building_monument_deliver_resource(building *b, int resource)
{
    if (b->id <= 0 || !building_monument_is_monument(b) ||
        b->resources[resource] <= 0) {
        return 0;
    }

    while (b->prev_part_building_id) {
        b = building_get(b->prev_part_building_id);
    }

    b->resources[resource]--;

    while (b->next_part_building_id) {
        b = building_get(b->next_part_building_id);
        b->resources[resource]--;
    }
    return 1;
}

int building_monument_access_point(building *b, map_point *dst)
{
    if (b->size < 3 || b->type == BUILDING_HIPPODROME) {
        dst->x = b->x;
        dst->y = b->y;
        return 1;
    }
    int dx = b->x - b->road_access_x;
    int dy = b->y - b->road_access_y;
    int half_size = b->size / 2;
    int even_size = b->size % 2;

    if (dx == -half_size && dy == -b->size) {
        dst->x = b->x + half_size;
        dst->y = b->y + b->size - 1;
        return 1;
    } if (dx == 1 && dy == -half_size) {
        dst->x = b->x;
        dst->y = b->y + half_size;
        return 1;
    }
    if (dx == -half_size && dy == 1) {
        dst->x = b->x + half_size;
        dst->y = b->y;
        return 1;
    }
    if (dx == -b->size && dy == -half_size) {
        dst->x = b->x + b->size - 1;
        dst->y = b->y + half_size;
        return 1;
    }
    if (!even_size) {
        return 0;
    }
    if (dx == -1 && dy == -b->size) {
        dst->x = b->x + 1;
        dst->y = b->y + b->size - 1;
        return 1;
    }
    if (dx == 1 && dy == -1) {
        dst->x = b->x;
        dst->y = b->y + 1;
        return 1;
    }
    if (dx == -1 && dy == 1) {
        dst->x = b->x + 1;
        dst->y = b->y;
        return 1;
    }
    if (dx == -b->size && dy == -1) {
        dst->x = b->x + b->size - 1;
        dst->y = b->y + 1;
        return 1;
    }
    return 0;
}

int building_monument_add_module(building *b, int module_type)
{
    if (!building_monument_is_monument(b) ||
        b->data.monument.phase != MONUMENT_FINISHED ||
        (b->data.monument.upgrades && b->type != BUILDING_CARAVANSERAI && b->type != BUILDING_LIGHTHOUSE)) {
        return 0;
    }
    b->data.monument.upgrades = module_type;
    map_building_tiles_add(b->id, b->x, b->y, b->size, building_image_get(b), TERRAIN_BUILDING);
    return 1;
}

int building_monument_get_monument(int x, int y, int resource, int road_network_id,
    int distance_from_entry, map_point *dst)
{
    if (city_resource_is_stockpiled(resource)) {
        return 0;
    }
    int min_dist = INFINITE;
    building *min_building = 0;
    for (building_type type = BUILDING_MONUMENT_FIRST_ID; type < BUILDING_TYPE_MAX; type++) {
        if (!MONUMENT_TYPES[type]) {
            continue;
        }
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->data.monument.phase == MONUMENT_FINISHED ||
                b->data.monument.phase < MONUMENT_START ||
                building_monument_is_construction_halted(b) ||
                (!resource && building_monument_needs_resources(b))) {
                continue;
            }
            short needed = b->resources[resource];
            if ((needed - building_monument_resource_in_delivery(b, resource)) <= 0) {
                continue;
            }
            if (!map_has_road_access(b->x, b->y, b->size, 0) ||
                b->distance_from_entry <= 0 || b->road_network_id != road_network_id) {
                continue;
            }
            int dist = calc_maximum_distance(b->x, b->y, x, y);
            if (dist < min_dist) {
                min_dist = dist;
                min_building = b;
            }
        }
    }
    if (min_building && min_dist < INFINITE) {
        if (dst) {
            map_point_store_result(min_building->road_access_x, min_building->road_access_y, dst);
        }
        return min_building->id;
    }
    return 0;
}

int building_monument_has_unfinished_monuments(void)
{
    for (building_type type = BUILDING_MONUMENT_FIRST_ID; type < BUILDING_TYPE_MAX; type++) {
        if (!MONUMENT_TYPES[type]) {
            continue;
        }
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->data.monument.phase != MONUMENT_FINISHED) {
                return 1;
            }
        }
    }
    return 0;
}

int building_monument_resources_needed_for_monument_type(building_type type, int resource, int phase)
{
    return MONUMENT_TYPES[type] ? MONUMENT_TYPES[type]->resources[phase - 1][resource] : 0;
}

void building_monument_set_phase(building *b, int phase)
{
    if (phase == building_monument_phases(b->type)) {
        phase = MONUMENT_FINISHED;
    }
    if (phase == b->data.monument.phase) {
        return;
    }
    b->data.monument.phase = phase;
    map_building_tiles_add(b->id, b->x, b->y, b->size, building_image_get(b), TERRAIN_BUILDING);
    if (b->data.monument.phase != MONUMENT_FINISHED) {
        for (int resource = 0; resource < RESOURCE_MAX; resource++) {
            b->resources[resource] =
                building_monument_resources_needed_for_monument_type(b->type, resource,
                b->data.monument.phase);
        }
    }
}

int building_monument_is_monument(const building *b)
{
    return building_monument_type_is_monument(b->type);
}

int building_monument_type_is_monument(building_type type)
{
    return type > BUILDING_NONE && type < BUILDING_TYPE_MAX && MONUMENT_TYPES[type] != 0;
}

int building_monument_type_is_mini_monument(building_type type)
{
    return building_monument_type_is_monument(type) && building_properties_for_type(type)->size < 5;
}

int building_monument_is_grand_temple(building_type type)
{
    switch (type) {
        case BUILDING_GRAND_TEMPLE_CERES:
        case BUILDING_GRAND_TEMPLE_NEPTUNE:
        case BUILDING_GRAND_TEMPLE_MERCURY:
        case BUILDING_GRAND_TEMPLE_MARS:
        case BUILDING_GRAND_TEMPLE_VENUS:
            return 1;
        default:
            return 0;
    }
}

int building_monument_needs_resource(building *b, int resource)
{
    if (b->data.monument.phase == MONUMENT_FINISHED) {
        return 0;
    }
    return (b->resources[resource]);
}

void building_monuments_set_construction_phase(int phase)
{
    for (building_type type = BUILDING_MONUMENT_FIRST_ID; type < BUILDING_TYPE_MAX; type++) {
        if (!MONUMENT_TYPES[type]) {
            continue;
        }
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            building_monument_set_phase(b, phase);
        }
    }
}

int building_monument_get_venus_gt(void)
{
    building *monument = building_first_of_type(BUILDING_GRAND_TEMPLE_VENUS);
    return monument ? monument->id : 0;
}

int building_monument_get_neptune_gt(void)
{
    building *monument = building_first_of_type(BUILDING_GRAND_TEMPLE_NEPTUNE);
    return monument ? monument->id : 0;
}

int building_monument_phases(building_type type)
{
    return MONUMENT_TYPES[type] ? MONUMENT_TYPES[type]->phases : 0;
}

void building_monument_finish_monuments(void)
{
    for (building_type type = BUILDING_MONUMENT_FIRST_ID; type < BUILDING_TYPE_MAX; type++) {
        if (!MONUMENT_TYPES[type]) {
            continue;
        }
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->data.monument.phase == MONUMENT_FINISHED) {
                continue;
            }
            building_monument_set_phase(b, MONUMENT_FINISHED);
            for (int resource = 0; resource < RESOURCE_MAX; resource++) {
                b->resources[resource] = 0;
            }
        }
    }
}

int building_monument_needs_resources(building *b)
{
    if (b->data.monument.phase == MONUMENT_FINISHED) {
        return 0;
    }
    for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
        if (b->resources[resource] > 0) {
            return 1;
        }
    }
    return 0;
}

int building_monument_progress(building *b)
{
    if (building_monument_needs_resources(b)) {
        return 0;
    }
    if (b->data.monument.phase == MONUMENT_FINISHED) {
        return 0;
    }
    while (b->prev_part_building_id) {
        b = building_get(b->prev_part_building_id);
    }
    building_monument_set_phase(b, b->data.monument.phase + 1);

    while (b->next_part_building_id) {
        b = building_get(b->next_part_building_id);
        building_monument_set_phase(b, b->data.monument.phase + 1);
    }
    if (b->data.monument.phase == MONUMENT_FINISHED) {
        if (building_monument_is_grand_temple(b->type)) {
            city_message_post(1, MESSAGE_GRAND_TEMPLE_COMPLETE, 0, b->grid_offset);
        } else if (b->type == BUILDING_PANTHEON) {
            city_message_post(1, MESSAGE_PANTHEON_COMPLETE, 0, b->grid_offset);
        } else if (b->type == BUILDING_LIGHTHOUSE) {
            city_message_post(1, MESSAGE_LIGHTHOUSE_COMPLETE, 0, b->grid_offset);
        } else if (b->type == BUILDING_COLOSSEUM) {
            city_message_post(1, MESSAGE_COLOSSEUM_COMPLETE, 0, b->grid_offset);
        } else if (b->type == BUILDING_HIPPODROME) {
            city_message_post(1, MESSAGE_HIPPODROME_COMPLETE, 0, b->grid_offset);
        }
    }
    return 1;
}

static int delivery_in_use(const monument_delivery *delivery)
{
    return delivery->destination_id != 0;
}

void building_monument_initialize_deliveries(void)
{
    if (!array_init(monument_deliveries, DELIVERY_ARRAY_SIZE_STEP, 0, delivery_in_use)) {
        log_error("Failed to create monument array. The game will likely crash.", 0, 0);
    }
}

void building_monument_add_delivery(int monument_id, int figure_id, int resource_id, int loads_no)
{
    monument_delivery *delivery;
    array_new_item(monument_deliveries, 0, delivery);
    if (!delivery) {
        log_error("Failed to create a new monument delivery. The game maybe running out of memory", 0, 0);
        return;
    }
    delivery->destination_id = monument_id;
    delivery->walker_id = figure_id;
    delivery->resource = resource_id;
    delivery->cartloads = loads_no;
}

void building_monument_remove_delivery(int figure_id)
{
    monument_delivery *delivery;
    array_foreach(monument_deliveries, delivery)
    {
        if (delivery->walker_id == figure_id) {
            delivery->destination_id = 0;
        }
    }
    array_trim(monument_deliveries);
}

static int resource_in_delivery(int monument_id, int resource_id)
{
    int resources = 0;
    monument_delivery *delivery;
    array_foreach(monument_deliveries, delivery)
    {
        if (delivery->destination_id == monument_id &&
            delivery->resource == resource_id) {
            resources += delivery->cartloads;
        }
    }
    return resources;
}

static int resource_in_delivery_multipart(building *b, int resource_id)
{
    int resources = 0;

    while (b->prev_part_building_id) {
        b = building_get(b->prev_part_building_id);
    }

    while (b->id) {
        monument_delivery *delivery;
        array_foreach(monument_deliveries, delivery)
        {
            if (delivery->destination_id == b->id &&
                delivery->resource == resource_id) {
                resources += delivery->cartloads;
            }
        }
        b = building_get(b->next_part_building_id);
    }

    return resources;
}

int building_monument_resource_in_delivery(building *b, int resource_id)
{
    if (b->next_part_building_id || b->prev_part_building_id) {
        return resource_in_delivery_multipart(b, resource_id);
    } else {
        return resource_in_delivery(b->id, resource_id);
    }
}

int building_monument_get_id(building_type type)
{
    building *b = building_first_of_type(type);
    if (!building_monument_type_is_monument(type) || !b) {
        return 0;
    }
    return b->id;
}

int building_monument_count_grand_temples(void)
{
    int count = 0;
    static const building_type grand_temples[] = {
        BUILDING_GRAND_TEMPLE_CERES,
        BUILDING_GRAND_TEMPLE_NEPTUNE,
        BUILDING_GRAND_TEMPLE_MERCURY,
        BUILDING_GRAND_TEMPLE_MARS,
        BUILDING_GRAND_TEMPLE_VENUS
    };
    for (int i = 0; i < 5; i++) {
        building_type type = grand_temples[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            count++;
        }
    }
    return count;
}

int building_monument_has_labour_problems(building *b)
{
    const model_building *model = model_get_building(b->type);

    if (b->num_workers < model->laborers) {
        return 1;
    } else {
        return 0;
    }
}

int building_monument_working(building_type type)
{
    int monument_id = building_monument_get_id(type);
    building *b = building_get(monument_id);
    if (!monument_id) {
        return 0;
    }
    if (b->data.monument.phase != MONUMENT_FINISHED || b->state != BUILDING_STATE_IN_USE) {
        return 0;
    }

    if (building_monument_has_labour_problems(b)) {
        return 0;
    }

    return monument_id;
}

int building_monument_has_required_resources_to_build(building_type type)
{
    int phases = building_monument_phases(type);
    if (!phases) {
        return 1;
    }
    for (int phase = 1; phase < phases; phase++) {
        for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (building_monument_resources_needed_for_monument_type(type, r, phase) > 0 &&
                !empire_can_produce_resource_potentially(r) && !empire_can_import_resource_potentially(r)) {
                return 0;
            }
        }
    }
    return 1;
}

int building_monument_upgraded(building_type type)
{
    int monument_id = building_monument_working(type);
    building *b = building_get(monument_id);
    if (!monument_id) {
        return 0;
    }
    if (!b->data.monument.upgrades) {
        return 0;
    }
    return monument_id;
}

int building_monument_module_type(building_type type)
{
    int monument_id = building_monument_working(type);

    if (!monument_id) {
        return 0;
    }

    building *b = building_get(monument_id);
    return b->data.monument.upgrades;
}

int building_monument_gt_module_is_active(int module)
{
    int module_num = module % MODULES_PER_TEMPLE + 1;
    int temple_type = module / MODULES_PER_TEMPLE + BUILDING_GRAND_TEMPLE_CERES;

    return building_monument_module_type(temple_type) == module_num;
}

int building_monument_pantheon_module_is_active(int module)
{
    return building_monument_module_type(BUILDING_PANTHEON) == (module - (PANTHEON_MODULE_1_DESTINATION_PRIESTS - 1));
}

static void delivery_save(buffer *buf, monument_delivery *delivery)
{
    buffer_write_i32(buf, delivery->walker_id);
    buffer_write_i32(buf, delivery->destination_id);
    buffer_write_i32(buf, delivery->resource);
    buffer_write_i32(buf, delivery->cartloads);
}

static void delivery_load(buffer *buf, monument_delivery *delivery, int size)
{
    delivery->walker_id = buffer_read_i32(buf);
    delivery->destination_id = buffer_read_i32(buf);
    delivery->resource = resource_remap(buffer_read_i32(buf));
    delivery->cartloads = buffer_read_i32(buf);

    if (size > ORIGINAL_DELIVERY_BUFFER_SIZE) {
        buffer_skip(buf, size - ORIGINAL_DELIVERY_BUFFER_SIZE);
    }
}

void building_monument_delivery_save_state(buffer *buf)
{
    int buf_size = 4 + monument_deliveries.size * ORIGINAL_DELIVERY_BUFFER_SIZE;
    uint8_t *buf_data = malloc(buf_size);
    buffer_init(buf, buf_data, buf_size);
    buffer_write_i32(buf, ORIGINAL_DELIVERY_BUFFER_SIZE);

    monument_delivery *delivery;
    array_foreach(monument_deliveries, delivery)
    {
        delivery_save(buf, delivery);
    }
}

void building_monument_delivery_load_state(buffer *buf, int includes_delivery_buffer_size)
{
    int delivery_buf_size = ORIGINAL_DELIVERY_BUFFER_SIZE;
    int buf_size = buf->size;

    if (includes_delivery_buffer_size) {
        delivery_buf_size = buffer_read_i32(buf);
        buf_size -= 4;
    }

    int deliveries_to_load = buf_size / delivery_buf_size;

    if (!array_init(monument_deliveries, DELIVERY_ARRAY_SIZE_STEP, 0, delivery_in_use) ||
        !array_expand(monument_deliveries, deliveries_to_load)) {
        log_error("Failed to create the monument deliveries array. The game may crash.", 0, 0);
    }

    for (int i = 0; i < deliveries_to_load; i++) {
        delivery_load(buf, array_next(monument_deliveries), delivery_buf_size);
    }
}

int building_monument_is_construction_halted(building *b)
{
    return building_main(b)->state == BUILDING_STATE_MOTHBALLED;
}

int building_monument_toggle_construction_halted(building *b)
{
    if (b->state == BUILDING_STATE_MOTHBALLED) {
        b->state = BUILDING_STATE_IN_USE;
        return 0;
    } else {
        b->state = BUILDING_STATE_MOTHBALLED;
        return 1;
    }
}

int building_monument_is_unfinished_monument(const building *b)
{
    return building_monument_is_monument(b) && b->data.monument.phase != MONUMENT_FINISHED;
}
