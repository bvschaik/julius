#include "monument.h"

#include "assets/assets.h"
#include "building/image.h"
#include "building/model.h"
#include "city/finance.h"
#include "city/message.h"
#include "city/resource.h"
#include "core/array.h"
#include "core/calc.h"
#include "core/log.h"
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
#define INFINITE 10000

static int grand_temple_resources[6][RESOURCE_MAX] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 20, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 16, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 28, 12, 0, 0, 0 },
    { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static int pantheon_resources[6][RESOURCE_MAX] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 16, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 32, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 40, 32, 0, 0, 0 },
    { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static int lighthouse_resources[5][RESOURCE_MAX] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 12, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 8, 0, 0, 0 },
    { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 20, 8, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static int colosseum_resources[5][RESOURCE_MAX] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 16, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 16, 0, 0, 0 },
    { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 16, 12, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static int hippodrome_resources[5][RESOURCE_MAX] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 32, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 32, 0, 0, 0 },
    { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 46, 32, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static int oracle_lt_resources[2][RESOURCE_MAX] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static int large_temple_resources[2][RESOURCE_MAX] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static int nymphaeum_resources[2][RESOURCE_MAX] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static int small_mausoleum_resources[2][RESOURCE_MAX] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static int large_mausoleum_resources[2][RESOURCE_MAX] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static int caravanserai_resources[2][RESOURCE_MAX] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 8, 6, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static const building_type MONUMENT_BUILDING_TYPES[] = {
    BUILDING_ORACLE,
    BUILDING_LARGE_TEMPLE_CERES,
    BUILDING_LARGE_TEMPLE_NEPTUNE,
    BUILDING_LARGE_TEMPLE_MERCURY,
    BUILDING_LARGE_TEMPLE_MARS,
    BUILDING_LARGE_TEMPLE_VENUS,
    BUILDING_GRAND_TEMPLE_CERES,
    BUILDING_GRAND_TEMPLE_NEPTUNE,
    BUILDING_GRAND_TEMPLE_MERCURY,
    BUILDING_GRAND_TEMPLE_MARS,
    BUILDING_GRAND_TEMPLE_VENUS,
    BUILDING_PANTHEON,
    BUILDING_LIGHTHOUSE,
    BUILDING_COLOSSEUM,
    BUILDING_HIPPODROME,
    BUILDING_NYMPHAEUM,
    BUILDING_LARGE_MAUSOLEUM,
    BUILDING_SMALL_MAUSOLEUM,
    BUILDING_CARAVANSERAI
};

#define MAX_MONUMENT_TYPES (sizeof(MONUMENT_BUILDING_TYPES) / sizeof(building_type))

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
        b->data.monument.resources_needed[resource] <= 0) {
        return 0;
    }

    while (b->prev_part_building_id) {
        b = building_get(b->prev_part_building_id);
    }

    b->data.monument.resources_needed[resource]--;

    while (b->next_part_building_id) {
        b = building_get(b->next_part_building_id);
        b->data.monument.resources_needed[resource]--;
    }
    return 1;
}

int building_monument_access_point(building *b, map_point *dst)
{
    int dx = b->x - b->road_access_x;
    int dy = b->y - b->road_access_y;
    switch (b->type) {
        default:
            return 0;
        case BUILDING_LARGE_TEMPLE_CERES:
        case BUILDING_LARGE_TEMPLE_NEPTUNE:
        case BUILDING_LARGE_TEMPLE_MERCURY:
        case BUILDING_LARGE_TEMPLE_MARS:
        case BUILDING_LARGE_TEMPLE_VENUS:
        case BUILDING_LIGHTHOUSE:
        case BUILDING_NYMPHAEUM:
        case BUILDING_LARGE_MAUSOLEUM:
            if (dx == -1 && dy == -3) {
                dst->x = b->x + 1;
                dst->y = b->y + 2;
            } else if (dx == 1 && dy == -1) {
                dst->x = b->x;
                dst->y = b->y + 1;
            } else if (dx == -3 && dy == -1) {
                dst->x = b->x + 2;
                dst->y = b->y + 1;
            } else if (dx == -1 && dy == 1) {
                dst->x = b->x + 1;
                dst->y = b->y;
            } else {
                return 0;
            }
            return 1;
        case BUILDING_GRAND_TEMPLE_CERES:
        case BUILDING_GRAND_TEMPLE_NEPTUNE:
        case BUILDING_GRAND_TEMPLE_MERCURY:
        case BUILDING_GRAND_TEMPLE_MARS:
        case BUILDING_GRAND_TEMPLE_VENUS:
        case BUILDING_PANTHEON:
            if (dx == -3 && dy == -7) {
                dst->x = b->x + 3;
                dst->y = b->y + 6;
            } else if (dx == 1 && dy == -3) {
                dst->x = b->x;
                dst->y = b->y + 3;
            } else if (dx == -3 && dy == 1) {
                dst->x = b->x + 3;
                dst->y = b->y;
            } else if (dx == -7 && dy == -3) {
                dst->x = b->x + 6;
                dst->y = b->y + 3;
            } else {
                return 0;
            }
            return 1;
        case BUILDING_COLOSSEUM:
            if (dx == -2 && dy == -5) {
                dst->x = b->x + 2;
                dst->y = b->y + 4;
            } else if (dx == 1 && dy == -2) {
                dst->x = b->x;
                dst->y = b->y + 2;
            } else if (dx == -2 && dy == 1) {
                dst->x = b->x + 2;
                dst->y = b->y;
            } else if (dx == -5 && dy == -2) {
                dst->x = b->x + 4;
                dst->y = b->y + 2;
            } else {
                return 0;
            }
            return 1;
        case BUILDING_ORACLE:
        case BUILDING_SMALL_MAUSOLEUM:
        case BUILDING_HIPPODROME:
            dst->x = b->x;
            dst->y = b->y;
            return 1;
        case BUILDING_CARAVANSERAI:
            if (dx == -2 && dy == -4) {
                dst->x = b->x + 2;
                dst->y = b->y + 3;
            } else if (dx == -1 && dy == -4) {
                dst->x = b->x + 1;
                dst->y = b->y + 3;
            } else if (dx == 1 && dy == -1) {
                dst->x = b->x;
                dst->y = b->y + 1;
            } else if (dx == 1 && dy == -2) {
                dst->x = b->x;
                dst->y = b->y + 2;
            } else if (dx == -2 && dy == 1) {
                dst->x = b->x + 2;
                dst->y = b->y;
            } else if (dx == -1 && dy == 1) {
                dst->x = b->x + 1;
                dst->y = b->y;
            } else if (dx == -4 && dy == -1) {
                dst->x = b->x + 3;
                dst->y = b->y + 1;
            } else if (dx == -4 && dy == -2) {
                dst->x = b->x + 3;
                dst->y = b->y + 2;
            } else {
                return 0;
            }
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
    for (int i = 0; i < MAX_MONUMENT_TYPES; i++) {
        building_type type = MONUMENT_BUILDING_TYPES[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->data.monument.phase == MONUMENT_FINISHED ||
                b->data.monument.phase < MONUMENT_START ||
                building_monument_is_construction_halted(b) ||
                (!resource && building_monument_needs_resources(b))) {
                continue;
            }
            short needed = b->data.monument.resources_needed[resource];
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
    for (int i = 0; i < MAX_MONUMENT_TYPES; i++) {
        building_type type = MONUMENT_BUILDING_TYPES[i];
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
    switch (type) {
        case BUILDING_LARGE_TEMPLE_CERES:
        case BUILDING_LARGE_TEMPLE_MERCURY:
        case BUILDING_LARGE_TEMPLE_NEPTUNE:
        case BUILDING_LARGE_TEMPLE_MARS:
        case BUILDING_LARGE_TEMPLE_VENUS:
            return large_temple_resources[phase - 1][resource];
        case BUILDING_ORACLE:
            return oracle_lt_resources[phase - 1][resource];
        case BUILDING_GRAND_TEMPLE_CERES:
        case BUILDING_GRAND_TEMPLE_MERCURY:
        case BUILDING_GRAND_TEMPLE_NEPTUNE:
        case BUILDING_GRAND_TEMPLE_MARS:
        case BUILDING_GRAND_TEMPLE_VENUS:
            return grand_temple_resources[phase - 1][resource];
        case BUILDING_PANTHEON:
            return pantheon_resources[phase - 1][resource];
        case BUILDING_LIGHTHOUSE:
            return lighthouse_resources[phase - 1][resource];
        case BUILDING_COLOSSEUM:
            return colosseum_resources[phase - 1][resource];
        case BUILDING_HIPPODROME:
            return hippodrome_resources[phase - 1][resource];
        case BUILDING_NYMPHAEUM:
            return nymphaeum_resources[phase - 1][resource];
        case BUILDING_LARGE_MAUSOLEUM:
            return large_mausoleum_resources[phase - 1][resource];
        case BUILDING_SMALL_MAUSOLEUM:
            return small_mausoleum_resources[phase - 1][resource];
        case BUILDING_CARAVANSERAI:
            return caravanserai_resources[phase - 1][resource];
        default:
            return 0;
    }
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
            b->data.monument.resources_needed[resource] =
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
    for (int i = 0; i < MAX_MONUMENT_TYPES; i++) {
        if (type == MONUMENT_BUILDING_TYPES[i]) {
            return 1;
        }
    }
    return 0;
}

int building_monument_type_is_mini_monument(building_type type)
{
    switch (type) {
        case BUILDING_ORACLE:
        case BUILDING_LARGE_TEMPLE_CERES:
        case BUILDING_LARGE_TEMPLE_NEPTUNE:
        case BUILDING_LARGE_TEMPLE_MERCURY:
        case BUILDING_LARGE_TEMPLE_MARS:
        case BUILDING_LARGE_TEMPLE_VENUS:
        case BUILDING_SMALL_MAUSOLEUM:
        case BUILDING_LARGE_MAUSOLEUM:
        case BUILDING_NYMPHAEUM:
        case BUILDING_CARAVANSERAI:
            return 1;
        default:
            return 0;
    }
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
    return (b->data.monument.resources_needed[resource]);
}

void building_monuments_set_construction_phase(int phase)
{
    for (int i = 0; i < MAX_MONUMENT_TYPES; i++) {
        building_type type = MONUMENT_BUILDING_TYPES[i];
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
    switch (type) {
        case BUILDING_PANTHEON:
        case BUILDING_GRAND_TEMPLE_CERES:
        case BUILDING_GRAND_TEMPLE_MERCURY:
        case BUILDING_GRAND_TEMPLE_NEPTUNE:
        case BUILDING_GRAND_TEMPLE_MARS:
        case BUILDING_GRAND_TEMPLE_VENUS:
            return 6;
        case BUILDING_LIGHTHOUSE:
        case BUILDING_COLOSSEUM:
        case BUILDING_HIPPODROME:
            return 5;
        case BUILDING_ORACLE:
        case BUILDING_LARGE_TEMPLE_CERES:
        case BUILDING_LARGE_TEMPLE_NEPTUNE:
        case BUILDING_LARGE_TEMPLE_MERCURY:
        case BUILDING_LARGE_TEMPLE_MARS:
        case BUILDING_LARGE_TEMPLE_VENUS:
        case BUILDING_LARGE_MAUSOLEUM:
        case BUILDING_SMALL_MAUSOLEUM:
        case BUILDING_NYMPHAEUM:
        case BUILDING_CARAVANSERAI:
            return 2;
        default:
            return 0;
    }
}

void building_monument_finish_monuments(void)
{
    for (int i = 0; i < MAX_MONUMENT_TYPES; i++) {
        building_type type = MONUMENT_BUILDING_TYPES[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->data.monument.phase == MONUMENT_FINISHED) {
                continue;
            }
            building_monument_set_phase(b, MONUMENT_FINISHED);
            for (int resource = 0; resource < RESOURCE_MAX; resource++) {
                b->data.monument.resources_needed[resource] = 0;
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
        if (b->data.monument.resources_needed[resource] > 0) {
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
    delivery->resource = buffer_read_i32(buf);
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
