#ifndef GAME_RESOURCE_H
#define GAME_RESOURCE_H

#include "building/type.h"
#include "core/lang.h"

/**
 * @file
 * Type definitions for resources
 */

/**
 * Resource types
 *
 * If you add a new resource, please follow the order: food -> raw materials -> goods, as the game expects resources
 * to be in that order.
 *
 * If you add a resource to the end of the food, raw materials or goods list, please update the "RESOURCE_MAX_*" value
 * Same if you add it to the start of a food, raw materials or goods list, in which case you need to update the
 * "RESOURCE_MIN_*" value.
 */
typedef enum {
    RESOURCE_NONE = 0,
    RESOURCE_WHEAT,
    RESOURCE_VEGETABLES,
    RESOURCE_FRUIT,
    RESOURCE_MEAT,
    RESOURCE_FISH = RESOURCE_MEAT,
    RESOURCE_CLAY,
    RESOURCE_TIMBER,
    RESOURCE_OLIVES,
    RESOURCE_VINES,
    RESOURCE_IRON,
    RESOURCE_MARBLE,
    RESOURCE_POTTERY,
    RESOURCE_FURNITURE,
    RESOURCE_OIL,
    RESOURCE_WINE,
    RESOURCE_WEAPONS,
    RESOURCE_DENARII,
    RESOURCE_TROOPS,
    // helper constants
    RESOURCE_MIN_FOOD = RESOURCE_WHEAT,
    RESOURCE_MAX_FOOD = RESOURCE_FISH + 1,
    RESOURCE_MIN_RAW = RESOURCE_CLAY,
    RESOURCE_MAX_RAW = RESOURCE_MARBLE + 1,
    RESOURCE_MIN_GOOD = RESOURCE_POTTERY,
    RESOURCE_MAX_GOOD = RESOURCE_WEAPONS + 1,
    RESOURCE_MIN = RESOURCE_MIN_FOOD,
    RESOURCE_MAX = RESOURCE_MAX_GOOD,
    RESOURCE_MAX_FOOD_LEGACY = 7,
    RESOURCE_MAX_LEGACY = 16,
    RESOURCE_TOTAL_SPECIAL = 2
} resource_type;

#define LEGACY_INVENTORY_MAX 8

typedef enum {
    RESOURCE_ORIGINAL_VERSION = 0,
    RESOURCE_DYNAMIC_VERSION = 1,
    RESOURCE_REORDERED_VERSION = 2,
    RESOURCE_CURRENT_VERSION = 2
} resource_version;

typedef enum {
    RESOURCE_FLAG_NONE = 0,
    RESOURCE_FLAG_FOOD = 1,
    RESOURCE_FLAG_RAW_MATERIAL = 2,
    RESOURCE_FLAG_GOOD = 4,
    RESOURCE_FLAG_SPECIAL = 8
} resource_flags;

typedef struct {
    resource_type type;
    resource_flags flags;
    const uint8_t *text;
    const char *xml_attr_name;
    int is_inventory;
    building_type industry;
    building_type workshop;
    struct {
        int storage;
        struct {
            int single_load;
            int multiple_loads;
            int eight_loads;
        } cart;
        int icon;
        int empire;
        struct {
            int icon;
            int empire;
        } editor;
    } image;
    struct {
        int buy;
        int sell;
    } default_trade_price;
} resource_data;

void resource_init(void);

int resource_is_food(resource_type resource);

int resource_is_raw_material(resource_type resource);

int resource_is_good(resource_type resource);

const resource_data *resource_get_data(resource_type resource);

void resource_set_mapping(int version);

resource_type resource_map_legacy_inventory(int id);

resource_type resource_remap(int id);

int resource_total_mapped(void);

int resource_total_food_mapped(void);

#endif // GAME_RESOURCE_H
