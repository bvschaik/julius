#include "resource.h"

#include "building/type.h"
#include "core/image.h"
#include "core/image_group_editor.h"
#include "game/save_version.h"
#include "scenario/building.h"
#include "translation/translation.h"

#define RESOURCE_ALL (RESOURCE_MAX + RESOURCE_TOTAL_SPECIAL)

static const resource_type resource_mappings[][RESOURCE_ALL] = {
    {
        RESOURCE_NONE, RESOURCE_WHEAT, RESOURCE_VEGETABLES, RESOURCE_FRUIT, RESOURCE_OLIVES, RESOURCE_VINES,
        RESOURCE_MEAT, RESOURCE_WINE, RESOURCE_OIL, RESOURCE_IRON, RESOURCE_TIMBER, RESOURCE_CLAY, RESOURCE_MARBLE,
        RESOURCE_WEAPONS, RESOURCE_FURNITURE, RESOURCE_POTTERY, RESOURCE_DENARII, RESOURCE_TROOPS
    },
    {
        RESOURCE_NONE, RESOURCE_WHEAT, RESOURCE_VEGETABLES, RESOURCE_FRUIT, RESOURCE_MEAT,
        RESOURCE_CLAY, RESOURCE_TIMBER, RESOURCE_OLIVES, RESOURCE_VINES, RESOURCE_IRON, RESOURCE_MARBLE,
        RESOURCE_POTTERY, RESOURCE_FURNITURE, RESOURCE_OIL, RESOURCE_WINE, RESOURCE_WEAPONS,
        RESOURCE_DENARII, RESOURCE_TROOPS
    }
};

static const resource_type legacy_inventory_mapping[LEGACY_INVENTORY_MAX] = {
    RESOURCE_WHEAT, RESOURCE_VEGETABLES, RESOURCE_FRUIT, RESOURCE_MEAT,
    RESOURCE_WINE, RESOURCE_OIL, RESOURCE_FURNITURE, RESOURCE_POTTERY
};

static struct {
    const resource_type *resources;
    const resource_type *inventory;
    int total_resources;
    int total_food_resources;
    int special_resources;
} mapping;

static resource_data resource_info[RESOURCE_ALL] = {
    [RESOURCE_NONE]       = { .type = RESOURCE_NONE },
    [RESOURCE_WHEAT]      = { .type = RESOURCE_WHEAT,      .flags = RESOURCE_FLAG_FOOD,         .xml_attr_name = "wheat",       .industry = BUILDING_WHEAT_FARM,         .default_trade_price = {  28,  22 }, .is_inventory = 1 },
    [RESOURCE_VEGETABLES] = { .type = RESOURCE_VEGETABLES, .flags = RESOURCE_FLAG_FOOD,         .xml_attr_name = "vegetables",  .industry = BUILDING_VEGETABLE_FARM,     .default_trade_price = {  38,  30 }, .is_inventory = 1 },
    [RESOURCE_FRUIT]      = { .type = RESOURCE_FRUIT,      .flags = RESOURCE_FLAG_FOOD,         .xml_attr_name = "fruit",       .industry = BUILDING_FRUIT_FARM,         .default_trade_price = {  38,  30 }, .is_inventory = 1 },
    [RESOURCE_OLIVES]     = { .type = RESOURCE_OLIVES,     .flags = RESOURCE_FLAG_RAW_MATERIAL, .xml_attr_name = "olives",      .industry = BUILDING_OLIVE_FARM,         .default_trade_price = {  42,  34 }, .workshop = BUILDING_OIL_WORKSHOP },
    [RESOURCE_VINES]      = { .type = RESOURCE_VINES,      .flags = RESOURCE_FLAG_RAW_MATERIAL, .xml_attr_name = "vines",       .industry = BUILDING_VINES_FARM,         .default_trade_price = {  44,  36 }, .workshop = BUILDING_WINE_WORKSHOP },
    [RESOURCE_MEAT]       = { .type = RESOURCE_MEAT,       .flags = RESOURCE_FLAG_FOOD,         .xml_attr_name = "meat|fish",   .industry = BUILDING_PIG_FARM,           .default_trade_price = {  44,  36 }, .is_inventory = 1 },
    [RESOURCE_WINE]       = { .type = RESOURCE_WINE,       .flags = RESOURCE_FLAG_GOOD,         .xml_attr_name = "wine",        .industry = BUILDING_WINE_WORKSHOP,      .default_trade_price = { 215, 160 }, .is_inventory = 1 },
    [RESOURCE_OIL]        = { .type = RESOURCE_OIL,        .flags = RESOURCE_FLAG_GOOD,         .xml_attr_name = "oil",         .industry = BUILDING_OIL_WORKSHOP,       .default_trade_price = { 180, 140 }, .is_inventory = 1 },
    [RESOURCE_IRON]       = { .type = RESOURCE_IRON,       .flags = RESOURCE_FLAG_RAW_MATERIAL, .xml_attr_name = "iron",        .industry = BUILDING_IRON_MINE,          .default_trade_price = {  60,  40 }, .workshop = BUILDING_WEAPONS_WORKSHOP },
    [RESOURCE_TIMBER]     = { .type = RESOURCE_TIMBER,     .flags = RESOURCE_FLAG_RAW_MATERIAL, .xml_attr_name = "timber|wood", .industry = BUILDING_TIMBER_YARD,        .default_trade_price = {  50,  35 }, .workshop = BUILDING_FURNITURE_WORKSHOP },
    [RESOURCE_CLAY]       = { .type = RESOURCE_CLAY,       .flags = RESOURCE_FLAG_RAW_MATERIAL, .xml_attr_name = "clay",        .industry = BUILDING_CLAY_PIT,           .default_trade_price = {  40,  30 }, .workshop = BUILDING_POTTERY_WORKSHOP },
    [RESOURCE_MARBLE]     = { .type = RESOURCE_MARBLE,     .flags = RESOURCE_FLAG_RAW_MATERIAL, .xml_attr_name = "marble",      .industry = BUILDING_MARBLE_QUARRY,      .default_trade_price = { 200, 140 } },
    [RESOURCE_WEAPONS]    = { .type = RESOURCE_WEAPONS,    .flags = RESOURCE_FLAG_GOOD,         .xml_attr_name = "weapons",     .industry = BUILDING_WEAPONS_WORKSHOP,   .default_trade_price = { 250, 180 } },
    [RESOURCE_FURNITURE]  = { .type = RESOURCE_FURNITURE,  .flags = RESOURCE_FLAG_GOOD,         .xml_attr_name = "furniture",   .industry = BUILDING_FURNITURE_WORKSHOP, .default_trade_price = { 200, 150 }, .is_inventory = 1 },
    [RESOURCE_POTTERY]    = { .type = RESOURCE_POTTERY,    .flags = RESOURCE_FLAG_GOOD,         .xml_attr_name = "pottery",     .industry = BUILDING_POTTERY_WORKSHOP,   .default_trade_price = { 180, 140 }, .is_inventory = 1 },
    [RESOURCE_DENARII]    = { .type = RESOURCE_DENARII,    .flags = RESOURCE_FLAG_SPECIAL },
    [RESOURCE_TROOPS]     = { .type = RESOURCE_TROOPS,     .flags = RESOURCE_FLAG_SPECIAL }
};

// TODO when separating fish from meat this goes to the main resource array
static resource_data fish_resource = {
    .type = RESOURCE_FISH, .flags = RESOURCE_FLAG_FOOD, .xml_attr_name = "meat|fish", .industry = BUILDING_WHARF, .default_trade_price = { 44, 36 }, .is_inventory = 1
};

int resource_is_food(resource_type resource)
{
    return (resource_info[resource].flags & RESOURCE_FLAG_FOOD) != 0;
}

int resource_is_raw_material(resource_type resource)
{
    return (resource_info[resource].flags & RESOURCE_FLAG_RAW_MATERIAL) != 0;
}

int resource_is_good(resource_type resource)
{
    return (resource_info[resource].flags & RESOURCE_FLAG_GOOD) != 0;
}

void resource_init(void)
{
    int food_index = 0;
    int good_index = 0;

    for (int i = RESOURCE_MIN; i < RESOURCE_MAX_LEGACY; i++) {
        resource_data *info = &resource_info[resource_mappings[0][i]];
        info->text = lang_get_string(23, i);
        info->image.cart.single_load = image_group(GROUP_FIGURE_CARTPUSHER_CART) + 8 * i;
        if (resource_is_food(info->type)) {
            info->image.cart.multiple_loads = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) + 8 * food_index;
            info->image.cart.eight_loads = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) + 32 + 8 * (food_index + 1);
            food_index++;
        } else {
            info->image.cart.multiple_loads = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_RESOURCE) + 8 * good_index;
            info->image.cart.eight_loads = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_RESOURCE) + 8 * good_index;
            good_index++;
        }
        info->image.empire = image_group(GROUP_EMPIRE_RESOURCES) + i;
        info->image.icon = image_group(GROUP_RESOURCE_ICONS) + i;
        info->image.storage = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_FILLED) + 4 * (i - 1);
        info->image.editor.icon = image_group(GROUP_EDITOR_RESOURCE_ICONS) + i;
        info->image.editor.empire = image_group(GROUP_EDITOR_EMPIRE_RESOURCES) + i;
    }

    fish_resource.text = lang_get_string(CUSTOM_TRANSLATION, TR_RESOURCE_FISH);
    fish_resource.image.cart.single_load = image_group(GROUP_FIGURE_CARTPUSHER_CART) + 696;
    fish_resource.image.cart.multiple_loads = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) + 56;
    fish_resource.image.cart.eight_loads = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) + 88;
    fish_resource.image.storage = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_FILLED) + 60;
    fish_resource.image.icon = image_group(GROUP_RESOURCE_ICONS) + 17;
    fish_resource.image.empire = image_group(GROUP_EMPIRE_RESOURCES) + 17;
    fish_resource.image.editor.icon = image_group(GROUP_EDITOR_RESOURCE_ICONS) + 17;
    fish_resource.image.editor.empire = image_group(GROUP_EDITOR_EMPIRE_RESOURCES) + 17;

    resource_info[RESOURCE_NONE].text = lang_get_string(23, 0);

    resource_info[RESOURCE_DENARII].image.icon = image_group(GROUP_RESOURCE_ICONS) + 16;
    resource_info[RESOURCE_DENARII].image.editor.icon = image_group(GROUP_EDITOR_RESOURCE_ICONS) + 16;
    resource_info[RESOURCE_DENARII].text = lang_get_string(23, 16);

    resource_info[RESOURCE_TROOPS].text = lang_get_string(23, 17);
}

const resource_data *resource_get_data(resource_type resource)
{
    if (resource == RESOURCE_MEAT && scenario_building_allowed(BUILDING_WHARF)) {
        return &fish_resource;
    }
    return &resource_info[resource];
}

void resource_set_mapping(int version)
{
    switch (version) {    
        case RESOURCE_ORIGINAL_VERSION:
            mapping.resources = resource_mappings[RESOURCE_ORIGINAL_VERSION];
            mapping.inventory = legacy_inventory_mapping;
            mapping.total_resources = RESOURCE_MAX_LEGACY;
            mapping.total_food_resources = RESOURCE_MAX_FOOD_LEGACY;
            break;
        case RESOURCE_DYNAMIC_VERSION:
            mapping.resources = resource_mappings[RESOURCE_ORIGINAL_VERSION];
            mapping.inventory = 0;
            mapping.total_resources = RESOURCE_MAX_LEGACY;
            mapping.total_food_resources = RESOURCE_MAX_FOOD_LEGACY;
            break;
        case RESOURCE_REORDERED_VERSION:
        default:
            mapping.resources = 0;
            mapping.inventory = 0;
            mapping.total_resources = RESOURCE_MAX;
            mapping.total_food_resources = RESOURCE_MAX_FOOD;
            break;
    }
}

resource_type resource_map_legacy_inventory(int id)
{
    return mapping.inventory ? mapping.inventory[id] : id;
}

resource_type resource_remap(int id)
{
    return mapping.resources ? mapping.resources[id] : id;
}

int resource_total_mapped(void)
{
    return mapping.total_resources;
}

int resource_total_food_mapped(void)
{
    return mapping.total_food_resources;
}
