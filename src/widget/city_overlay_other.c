#include "city_overlay_other.h"

#include "building/model.h"
#include "core/calc.h"
#include "game/resource.h"
#include "game/state.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/desirability.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"

#include "Data/CityInfo.h"

static int show_building_religion(const building *b)
{
    return
        b->type == BUILDING_ORACLE || b->type == BUILDING_SMALL_TEMPLE_CERES ||
        b->type == BUILDING_SMALL_TEMPLE_NEPTUNE || b->type == BUILDING_SMALL_TEMPLE_MERCURY ||
        b->type == BUILDING_SMALL_TEMPLE_MARS || b->type == BUILDING_SMALL_TEMPLE_VENUS ||
        b->type == BUILDING_LARGE_TEMPLE_CERES || b->type == BUILDING_LARGE_TEMPLE_NEPTUNE ||
        b->type == BUILDING_LARGE_TEMPLE_MERCURY || b->type == BUILDING_LARGE_TEMPLE_MARS ||
        b->type == BUILDING_LARGE_TEMPLE_VENUS;
}

static int show_building_food_stocks(const building *b)
{
    return b->type == BUILDING_MARKET || b->type == BUILDING_WHARF || b->type == BUILDING_GRANARY;
}

static int show_building_tax_income(const building *b)
{
    return b->type == BUILDING_FORUM || b->type == BUILDING_SENATE_UPGRADED;
}

static int show_building_water(const building *b)
{
    return b->type == BUILDING_WELL || b->type == BUILDING_FOUNTAIN || b->type == BUILDING_RESERVOIR;
}

static int show_building_desirability(const building *b)
{
    return 0;
}


static int show_figure_religion(const figure *f)
{
    return f->type == FIGURE_PRIEST;
}

static int show_figure_food_stocks(const figure *f)
{
    if (f->type == FIGURE_MARKET_BUYER || f->type == FIGURE_MARKET_TRADER ||
        f->type == FIGURE_DELIVERY_BOY || f->type == FIGURE_FISHING_BOAT) {
        return 1;
    } else if (f->type == FIGURE_CART_PUSHER) {
        return resource_is_food(f->resourceId);
    }
    return 0;
}

static int show_figure_tax_income(const figure *f)
{
    return f->type == FIGURE_TAX_COLLECTOR;
}

static int show_figure_none(const figure *f)
{
    return 0;
}


static int get_column_height_religion(const building *b)
{
    return b->houseSize && b->data.house.numGods ? b->data.house.numGods * 17 / 10 : NO_COLUMN;
}

static int get_column_height_food_stocks(const building *b)
{
    if (b->houseSize && model_get_house(b->subtype.houseLevel)->food_types) {
        int pop = b->housePopulation;
        int stocks = 0;
        for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
            stocks += b->data.house.inventory[i];
        }
        int pct_stocks = calc_percentage(stocks, pop);
        if (pct_stocks <= 0) {
            return 10;
        } else if (pct_stocks < 100) {
            return 5;
        } else if (pct_stocks <= 200) {
            return 1;
        }
    }
    return NO_COLUMN;
}

static int get_column_height_tax_income(const building *b)
{
    if (b->houseSize) {
        int pct = calc_adjust_with_percentage(b->taxIncomeOrStorage / 2, Data_CityInfo.taxPercentage);
        if (pct > 0) {
            return pct / 25;
        }
    }
    return NO_COLUMN;
}

static int get_column_height_none(const building *b)
{
    return NO_COLUMN;
}


static int get_tooltip_religion(tooltip_context *c, const building *b)
{
    if (b->data.house.numGods <= 0) {
        return 12;
    } else if (b->data.house.numGods == 1) {
        return 13;
    } else if (b->data.house.numGods == 2) {
        return 14;
    } else if (b->data.house.numGods == 3) {
        return 15;
    } else if (b->data.house.numGods == 4) {
        return 16;
    } else if (b->data.house.numGods == 5) {
        return 17;
    } else {
        return 18; // >5 gods, shouldn't happen...
    }
}

static int get_tooltip_food_stocks(tooltip_context *c, const building *b)
{
    if (b->housePopulation <= 0) {
        return 0;
    }
    if (!model_get_house(b->subtype.houseLevel)->food_types) {
        return 104;
    } else {
        int stocks_present = 0;
        for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
            stocks_present += b->data.house.inventory[i];
        }
        int stocks_per_pop = calc_percentage(stocks_present, b->housePopulation);
        if (stocks_per_pop <= 0) {
            return 4;
        } else if (stocks_per_pop < 100) {
            return 5;
        } else if (stocks_per_pop <= 200) {
            return 6;
        } else {
            return 7;
        }
    }
}

static int get_tooltip_tax_income(tooltip_context *c, const building *b)
{
    int denarii = calc_adjust_with_percentage(b->taxIncomeOrStorage / 2, Data_CityInfo.taxPercentage);
    if (denarii > 0) {
        c->has_numeric_prefix = 1;
        c->numeric_prefix = denarii;
        return 45;
    } else if (b->houseTaxCoverage > 0) {
        return 44;
    } else {
        return 43;
    }
}

static int get_tooltip_water(tooltip_context *c, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_RESERVOIR_RANGE)) {
        if (map_terrain_is(grid_offset, TERRAIN_FOUNTAIN_RANGE)) {
            return 2;
        } else {
            return 1;
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_FOUNTAIN_RANGE)) {
        return 3;
    }
    return 0;
}

static int get_tooltip_desirability(tooltip_context *c, int grid_offset)
{
    int desirability = map_desirability_get(grid_offset);
    if (desirability < 0) {
        return 91;
    } else if (desirability == 0) {
        return 92;
    } else {
        return 93;
    }
}

const city_overlay *city_overlay_for_religion()
{
    static city_overlay overlay = {
        OVERLAY_RELIGION,
        COLUMN_TYPE_ACCESS,
        show_building_religion,
        show_figure_religion,
        get_column_height_religion,
        0,
        get_tooltip_religion,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_food_stocks()
{
    static city_overlay overlay = {
        OVERLAY_FOOD_STOCKS,
        COLUMN_TYPE_RISK,
        show_building_food_stocks,
        show_figure_food_stocks,
        get_column_height_food_stocks,
        0,
        get_tooltip_food_stocks,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_tax_income()
{
    static city_overlay overlay = {
        OVERLAY_TAX_INCOME,
        COLUMN_TYPE_ACCESS,
        show_building_tax_income,
        show_figure_tax_income,
        get_column_height_tax_income,
        0,
        get_tooltip_tax_income,
        0,
        0
    };
    return &overlay;
}

static int terrain_on_water_overlay()
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SCRUB |
        TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_AQUEDUCT | TERRAIN_ELEVATION |
        TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE;
}

static void draw_footprint_water(int x, int y, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }
    if (map_terrain_is(grid_offset, terrain_on_water_overlay())) {
        if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
            city_with_overlay_draw_building_footprint(x, y, grid_offset, 0);
        } else {
            image_draw_isometric_footprint_from_draw_tile(map_image_at(grid_offset), x, y, 0);
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_WALL)) {
        // display grass
        int image_id = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7);
        image_draw_isometric_footprint_from_draw_tile(image_id, x, y, 0);
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        building *b = building_get(map_building_at(grid_offset));
        int terrain = map_terrain_get(grid_offset);
        if (b->id && b->hasWellAccess == 1) {
            terrain |= TERRAIN_FOUNTAIN_RANGE;
        }
        int image_offset;
        switch (terrain & (TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE)) {
            case TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE:
                image_offset = 24;
                break;
            case TERRAIN_RESERVOIR_RANGE:
                image_offset = 8;
                break;
            case TERRAIN_FOUNTAIN_RANGE:
                image_offset = 16;
                break;
            default:
                image_offset = 0;
                break;
        }
        city_with_overlay_draw_building_footprint(x, y, grid_offset, image_offset);
    } else {
        int image_id = image_group(GROUP_TERRAIN_OVERLAY);
        switch (map_terrain_get(grid_offset) & (TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE)) {
            case TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE:
                image_id += 27;
                break;
            case TERRAIN_RESERVOIR_RANGE:
                image_id += 11;
                break;
            case TERRAIN_FOUNTAIN_RANGE:
                image_id += 19;
                break;
            default:
                image_id = map_image_at(grid_offset);
                break;
        }
        image_draw_isometric_footprint_from_draw_tile(image_id, x, y, 0);
    }
}

static void draw_top_water(int x, int y, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }
    if (map_terrain_is(grid_offset, terrain_on_water_overlay())) {
        if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
            image_draw_isometric_top_from_draw_tile(map_image_at(grid_offset), x, y, 0);
        }
    } else if (map_building_at(grid_offset)) {
        city_with_overlay_draw_building_top(x, y, grid_offset);
    }
}

const city_overlay *city_overlay_for_water()
{
    static city_overlay overlay = {
        OVERLAY_WATER,
        COLUMN_TYPE_ACCESS,
        show_building_water,
        show_figure_none,
        get_column_height_none,
        get_tooltip_water,
        0,
        draw_footprint_water,
        draw_top_water
    };
    return &overlay;
}

static int terrain_on_desirability_overlay()
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER |
        TERRAIN_SCRUB | TERRAIN_GARDEN | TERRAIN_ROAD |
        TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE;
}

static int get_desirability_image_offset(int desirability)
{
    if (desirability < -10) {
        return 0;
    } else if (desirability < -5) {
        return 1;
    } else if (desirability < 0) {
        return 2;
    } else if (desirability == 1) {
        return 3;
    } else if (desirability < 5) {
        return 4;
    } else if (desirability < 10) {
        return 5;
    } else if (desirability < 15) {
        return 6;
    } else if (desirability < 20) {
        return 7;
    } else if (desirability < 25) {
        return 8;
    } else {
        return 9;
    }
}

static void draw_footprint_desirability(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, terrain_on_desirability_overlay()) && !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        // display normal tile
        if (map_property_is_draw_tile(grid_offset)) {
            image_draw_isometric_footprint_from_draw_tile(map_image_at(grid_offset), x, y, 0);
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
        // display empty land/grass
        int image_id = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7);
        image_draw_isometric_footprint_from_draw_tile(image_id, x, y, 0);
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING) || map_desirability_get(grid_offset)) {
        int offset = get_desirability_image_offset(map_desirability_get(grid_offset));
        image_draw_isometric_footprint_from_draw_tile(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y, 0);
    } else {
        image_draw_isometric_footprint_from_draw_tile(map_image_at(grid_offset), x, y, 0);
    }
}

static void draw_top_desirability(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, terrain_on_desirability_overlay()) && !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        // display normal tile
        if (map_property_is_draw_tile(grid_offset)) {
            image_draw_isometric_top_from_draw_tile(map_image_at(grid_offset), x, y, 0);
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
        // grass, no top needed
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING) || map_desirability_get(grid_offset)) {
        int offset = get_desirability_image_offset(map_desirability_get(grid_offset));
        image_draw_isometric_top_from_draw_tile(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y, 0);
    } else {
        image_draw_isometric_top_from_draw_tile(map_image_at(grid_offset), x, y, 0);
    }
}

const city_overlay *city_overlay_for_desirability()
{
    static city_overlay overlay = {
        OVERLAY_DESIRABILITY,
        COLUMN_TYPE_ACCESS,
        show_building_desirability,
        show_figure_none,
        get_column_height_none,
        get_tooltip_desirability,
        0,
        draw_footprint_desirability,
        draw_top_desirability
    };
    return &overlay;
}
