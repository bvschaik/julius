#ifndef GAME_RESOURCE_H
#define GAME_RESOURCE_H

#include "building/type.h"
#include "city/warning.h"
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
    RESOURCE_FISH,
    RESOURCE_CLAY,
    RESOURCE_TIMBER,
    RESOURCE_OLIVES,
    RESOURCE_VINES,
    RESOURCE_IRON,
    RESOURCE_MARBLE,
    RESOURCE_GOLD,
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
    RESOURCE_MIN_NON_FOOD = RESOURCE_MAX_FOOD,
    RESOURCE_MAX_NON_FOOD = RESOURCE_WEAPONS + 1,
    RESOURCE_MIN = RESOURCE_MIN_FOOD,
    RESOURCE_MAX = RESOURCE_MAX_NON_FOOD,
    RESOURCE_TOTAL_SPECIAL = 2,

    // Values for old versions
    RESOURCE_MAX_FOOD_LEGACY = 7,
    RESOURCE_MAX_FOOD_REORDERED = 5,
    RESOURCE_MAX_FOOD_WITH_FISH = 6,

    RESOURCE_MAX_LEGACY = 16,
    RESOURCE_MAX_WITH_FISH = 17,
    RESOURCE_MAX_WITH_GOLD = 18
} resource_type;

#define LEGACY_INVENTORY_MAX 8

typedef enum {
    RESOURCE_ORIGINAL_VERSION = 0,
    RESOURCE_DYNAMIC_VERSION = 1,
    RESOURCE_REORDERED_VERSION = 2,
    RESOURCE_SEPARATE_FISH_AND_MEAT_VERSION = 3,
    RESOURCE_HAS_GOLD_VERSION = 4,
    RESOURCE_CURRENT_VERSION = RESOURCE_HAS_GOLD_VERSION
} resource_version;

typedef enum {
    RESOURCE_FLAG_NONE = 0,
    RESOURCE_FLAG_FOOD = 1,
    RESOURCE_FLAG_STORABLE = 2,
    RESOURCE_FLAG_INVENTORY = 4 | RESOURCE_FLAG_STORABLE // Inventory goods are always storable
} resource_flags;

typedef struct {
    resource_type type;
    resource_flags flags;
    const uint8_t *text;
    const char *xml_attr_name;
    building_type industry;
    int production_per_month;
    struct {
        warning_type needed;
        warning_type create_industry;
    } warning;
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

int resource_is_inventory(resource_type resource);

resource_type resource_get_from_industry(building_type industry);

const resource_type *resource_get_raw_materials_for_good(resource_type good);

const resource_type *resource_get_goods_from_raw_material(resource_type raw_material);

const resource_data *resource_get_data(resource_type resource);

void resource_set_mapping(int version);

int resource_mapping_get_version(void);

resource_type resource_map_legacy_inventory(int id);

resource_type resource_produced_by_building_type(int building_type);

int resource_production_per_month(resource_type);

resource_type resource_remap(int id);

int resource_total_mapped(void);

int resource_total_food_mapped(void);

#endif // GAME_RESOURCE_H
