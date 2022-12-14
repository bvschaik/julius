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
 */
typedef enum {
    RESOURCE_NONE = 0,
    RESOURCE_WHEAT = 1,
    RESOURCE_VEGETABLES = 2,
    RESOURCE_FRUIT = 3,
    RESOURCE_OLIVES = 4,
    RESOURCE_VINES = 5,
    RESOURCE_MEAT = 6,
    RESOURCE_FISH = 6,
    RESOURCE_WINE = 7,
    RESOURCE_OIL = 8,
    RESOURCE_IRON = 9,
    RESOURCE_TIMBER = 10,
    RESOURCE_CLAY = 11,
    RESOURCE_MARBLE = 12,
    RESOURCE_WEAPONS = 13,
    RESOURCE_FURNITURE = 14,
    RESOURCE_POTTERY = 15,
    RESOURCE_DENARII = 16,
    RESOURCE_TROOPS = 17,
    // helper constants
    RESOURCE_MIN = 1,
    RESOURCE_MAX = 16,
    RESOURCE_MAX_LEGACY = 16,
    RESOURCE_MIN_FOOD = 1,
    RESOURCE_MAX_FOOD = 7,
    RESOURCE_MAX_FOOD_LEGACY = 7,
    RESOURCE_MIN_RAW = 9,
    RESOURCE_MAX_RAW = 13,
    RESOURCE_TOTAL_SPECIAL = 2
} resource_type;

#define LEGACY_INVENTORY_MAX 8

typedef enum {
    RESOURCE_ORIGINAL_VERSION = 0,
    RESOURCE_CURRENT_VERSION = 1
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
