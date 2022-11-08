#include "city_overlay_other.h"

#include "building/model.h"
#include "building/monument.h"
#include "building/roadblock.h"
#include "city/constants.h"
#include "city/finance.h"
#include "core/calc.h"
#include "core/config.h"
#include "game/resource.h"
#include "game/state.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/desirability.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"
#include "translation/translation.h"
#include "widget/city_draw_highway.h"

static int show_building_religion(const building *b)
{
    return
        b->type == BUILDING_ORACLE || b->type == BUILDING_LARARIUM || b->type == BUILDING_SMALL_TEMPLE_CERES ||
        b->type == BUILDING_SMALL_TEMPLE_NEPTUNE || b->type == BUILDING_SMALL_TEMPLE_MERCURY ||
        b->type == BUILDING_SMALL_TEMPLE_MARS || b->type == BUILDING_SMALL_TEMPLE_VENUS ||
        b->type == BUILDING_LARGE_TEMPLE_CERES || b->type == BUILDING_LARGE_TEMPLE_NEPTUNE ||
        b->type == BUILDING_LARGE_TEMPLE_MERCURY || b->type == BUILDING_LARGE_TEMPLE_MARS ||
        b->type == BUILDING_SMALL_MAUSOLEUM || b->type == BUILDING_LARGE_MAUSOLEUM ||
        b->type == BUILDING_LARGE_TEMPLE_VENUS || b->type == BUILDING_GRAND_TEMPLE_CERES ||
        b->type == BUILDING_GRAND_TEMPLE_NEPTUNE || b->type == BUILDING_GRAND_TEMPLE_MERCURY ||
        b->type == BUILDING_GRAND_TEMPLE_MARS || b->type == BUILDING_GRAND_TEMPLE_VENUS ||
        b->type == BUILDING_PANTHEON || b->type == BUILDING_NYMPHAEUM;
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
    return b->type == BUILDING_WELL || b->type == BUILDING_FOUNTAIN || b->type == BUILDING_RESERVOIR || 
        (b->type == BUILDING_GRAND_TEMPLE_NEPTUNE && building_monument_gt_module_is_active(NEPTUNE_MODULE_2_CAPACITY_AND_WATER));
}

static int show_building_sentiment(const building *b)
{
    return b->house_size > 0;
}

static int show_building_desirability(const building *b)
{
    return 0;
}

static int show_building_roads(const building *b)
{
    return building_type_is_roadblock(b->type) || b->type == BUILDING_GATEHOUSE;
}

static int show_building_mothball(const building *b)
{
    return b->state == BUILDING_STATE_MOTHBALLED;
}

static int show_building_warehouses(const building *b)
{
    return b->type == BUILDING_WAREHOUSE || b->type == BUILDING_WAREHOUSE_SPACE;
}


static int show_building_none(const building *b)
{
    return 0;
}


static int show_figure_religion(const figure *f)
{
    return f->type == FIGURE_PRIEST || f->type == FIGURE_PRIEST_SUPPLIER;
}

static int show_figure_food_stocks(const figure *f)
{
    if (f->type == FIGURE_MARKET_SUPPLIER || f->type == FIGURE_MARKET_TRADER ||
        f->type == FIGURE_DELIVERY_BOY || f->type == FIGURE_FISHING_BOAT) {
        return 1;
    } else if (f->type == FIGURE_CART_PUSHER) {
        return resource_is_food(f->resource_id);
    }
    return 0;
}

static int show_figure_tax_income(const figure *f)
{
    return f->type == FIGURE_TAX_COLLECTOR;
}

static int show_figure_warehouses(const figure *f)
{
    if (f->type != FIGURE_WAREHOUSEMAN) {
        return 0;
    }
    building *b = building_get(f->building_id);
    return b->type == BUILDING_WAREHOUSE;
}

static int show_figure_none(const figure *f)
{
    return 0;
}

static int get_column_height_religion(const building *b)
{
    return b->house_size && b->data.house.num_gods ? b->data.house.num_gods * 18 / 10 : NO_COLUMN;
}

static int get_column_height_food_stocks(const building *b)
{
    if (b->house_size && model_get_house(b->subtype.house_level)->food_types) {
        int pop = b->house_population;
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

static int get_column_height_levy(const building *b)
{
    int levy = building_get_levy(b);
    int height = calc_percentage(levy, PANTHEON_LEVY_MONTHLY) / 10;
    height = calc_bound(height, 1, 10);
    return levy ? height : NO_COLUMN;
}

static int get_column_height_tax_income(const building *b)
{
    if (b->house_size) {
        int pct = calc_adjust_with_percentage(b->tax_income_or_storage / 2, city_finance_tax_percentage());
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

static void add_god(tooltip_context *c, int god_id)
{
    int index = c->num_extra_texts;
    c->extra_text_groups[index] = 59;
    c->extra_text_ids[index] = 11 + god_id;
    c->num_extra_texts++;
}

static int get_tooltip_religion(tooltip_context *c, const building *b)
{
    if (b->house_pantheon_access) {
        c->translation_key = TR_TOOLTIP_OVERLAY_PANTHEON_ACCESS;
        return 0;
    }

    if (b->data.house.num_gods < 5) {
        if (b->data.house.temple_ceres) {
            add_god(c, GOD_CERES);
        }
        if (b->data.house.temple_neptune) {
            add_god(c, GOD_NEPTUNE);
        }
        if (b->data.house.temple_mercury) {
            add_god(c, GOD_MERCURY);
        }
        if (b->data.house.temple_mars) {
            add_god(c, GOD_MARS);
        }
        if (b->data.house.temple_venus) {
            add_god(c, GOD_VENUS);
        }
    }
    if (b->data.house.num_gods <= 0) {
        return 12;
    } else if (b->data.house.num_gods == 1) {
        return 13;
    } else if (b->data.house.num_gods == 2) {
        return 14;
    } else if (b->data.house.num_gods == 3) {
        return 15;
    } else if (b->data.house.num_gods == 4) {
        return 16;
    } else if (b->data.house.num_gods == 5) {
        return 17;
    } else {
        return 18; // >5 gods, shouldn't happen...
    }
}

static int get_tooltip_food_stocks(tooltip_context *c, const building *b)
{
    if (b->house_population <= 0) {
        return 0;
    }
    if (!model_get_house(b->subtype.house_level)->food_types) {
        return 104;
    } else {
        int stocks_present = 0;
        for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
            stocks_present += b->data.house.inventory[i];
        }
        int stocks_per_pop = calc_percentage(stocks_present, b->house_population);
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
    int denarii = calc_adjust_with_percentage(b->tax_income_or_storage / 2, city_finance_tax_percentage());
    if (denarii > 0) {
        c->has_numeric_prefix = 1;
        c->numeric_prefix = denarii;
        return 45;
    } else if (b->house_tax_coverage > 0) {
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
    int desirability;
    if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        int building_id = map_building_at(grid_offset);
        building *b = building_get(building_id);
        desirability = b->desirability;
    } else {
        desirability = map_desirability_get(grid_offset);
    }
    if (desirability < 0) {
        return 91;
    } else if (desirability == 0) {
        return 92;
    } else {
        return 93;
    }
}

static int get_tooltip_none(tooltip_context *c, int grid_offset)
{
    return 0;
}

static int get_tooltip_levy(tooltip_context *c, const building *b)
{
    int levy = building_get_levy(b);
    if (levy > 0) {
        c->has_numeric_prefix = 1;
        c->numeric_prefix = levy;
        c->translation_key = TR_TOOLTIP_OVERLAY_LEVY;
        return 1;
    }
    return 0;
}

static int get_tooltip_sentiment(tooltip_context *c, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        return 0;
    }
    int building_id = map_building_at(grid_offset);
    building *b = building_get(building_id);
    if (!b || !b->house_population) {
        return 0;
    }
    int happiness = b->sentiment.house_happiness;
    int sentiment_text_id = TR_BUILDING_WINDOW_HOUSE_SENTIMENT_1;
    if (happiness > 0) {
        sentiment_text_id = happiness / 10 + TR_BUILDING_WINDOW_HOUSE_SENTIMENT_2;
    }
    c->translation_key = sentiment_text_id;
    return 1;
}


const city_overlay *city_overlay_for_religion(void)
{
    static city_overlay overlay = {
        OVERLAY_RELIGION,
        COLUMN_COLOR_GREEN_TO_RED,
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

const city_overlay *city_overlay_for_food_stocks(void)
{
    static city_overlay overlay = {
        OVERLAY_FOOD_STOCKS,
        COLUMN_COLOR_RED,
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

const city_overlay *city_overlay_for_tax_income(void)
{
    static city_overlay overlay = {
        OVERLAY_TAX_INCOME,
        COLUMN_COLOR_GREEN_TO_RED,
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

static int has_deleted_building(int grid_offset)
{
    building *b = building_get(map_building_at(grid_offset));
    b = building_main(b);
    return b->id && (b->is_deleted || map_property_is_deleted(b->grid_offset));
}

static int terrain_on_water_overlay(void)
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SHRUB | TERRAIN_MEADOW |
        TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_AQUEDUCT | TERRAIN_ELEVATION |
        TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE | TERRAIN_HIGHWAY;
}

static void draw_footprint_water(int x, int y, float scale, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }
    int is_building = map_terrain_is(grid_offset, TERRAIN_BUILDING);
    if (map_terrain_is(grid_offset, TERRAIN_HIGHWAY)) {
        city_draw_highway_footprint(x, y, scale, grid_offset);
    } else if (map_terrain_is(grid_offset, terrain_on_water_overlay()) && !is_building) {
        image_draw_isometric_footprint_from_draw_tile(map_image_at(grid_offset), x, y, 0, scale);
    } else if (map_terrain_is(grid_offset, TERRAIN_WALL)) {
        // display grass
        int image_id = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7);
        image_draw_isometric_footprint_from_draw_tile(image_id, x, y, 0, scale);
    } else if (is_building) {
        building *b = building_get(map_building_at(grid_offset));
        int terrain = map_terrain_get(grid_offset);
        if (b->id && (b->has_well_access || (b->house_size && b->has_water_access))) {
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
        image_draw_isometric_footprint_from_draw_tile(image_id, x, y, 0, scale);
    }
}

static void draw_top_water(int x, int y, float scale, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }
    if (map_terrain_is(grid_offset, terrain_on_water_overlay())) {
        if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
            color_t color_mask = 0;
            if (map_property_is_deleted(grid_offset) && map_property_multi_tile_size(grid_offset) == 1) {
                color_mask = COLOR_MASK_RED;
            }
            image_draw_isometric_top_from_draw_tile(map_image_at(grid_offset), x, y, color_mask, scale);
        }
    } else if (map_building_at(grid_offset)) {
        city_with_overlay_draw_building_top(x, y, grid_offset);
    }
}

const city_overlay *city_overlay_for_water(void)
{
    static city_overlay overlay = {
        OVERLAY_WATER,
        COLUMN_COLOR_GREEN,
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

static color_t get_sentiment_color(int sentiment)
{
    sentiment = calc_bound(sentiment - 50, -40, 40);
    color_t color = COLOR_OVERLAY_NEUTRAL;
    if (sentiment < 0) {
        color += COLOR_OVERLAY_NEGATIVE_STEP * (sentiment - 10);
    } else {
        color -= COLOR_OVERLAY_POSITIVE_STEP * (sentiment + 10);
    }
    return color;
}

static void draw_sentiment_values(int x, int y, float scale, int grid_offset)
{
    if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        int building_id = map_building_at(grid_offset);
        building *b = building_get(building_id);
        if (!b || !b->house_population) {
            return;
        }
        color_t color = get_sentiment_color(b->sentiment.house_happiness);
        image_draw(image_group(GROUP_TERRAIN_FLAT_TILE), x, y, color, scale);
    }
}

const city_overlay *city_overlay_for_sentiment(void)
{
    static city_overlay overlay = {
        OVERLAY_SENTIMENT,
        COLUMN_COLOR_GREEN,
        show_building_sentiment,
        show_figure_none,
        get_column_height_none,
        get_tooltip_sentiment,
        0,
        0,
        0,
        draw_sentiment_values
    };
    return &overlay;
}

static int terrain_on_desirability_overlay(void)
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER |
        TERRAIN_SHRUB | TERRAIN_GARDEN | TERRAIN_ROAD |
        TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE |
        TERRAIN_HIGHWAY;
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

static void draw_footprint_desirability(int x, int y, float scale, int grid_offset)
{
    color_t color_mask = map_property_is_deleted(grid_offset) ? COLOR_MASK_RED : 0;
    if (map_terrain_is(grid_offset, TERRAIN_HIGHWAY)) {
        city_draw_highway_footprint(x, y, scale, grid_offset);
    } else if (map_terrain_is(grid_offset, terrain_on_desirability_overlay())
        && !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        // display normal tile
        if (map_property_is_draw_tile(grid_offset)) {
            image_draw_isometric_footprint_from_draw_tile(map_image_at(grid_offset), x, y, color_mask, scale);
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
        // display empty land/grass
        int image_id = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7);
        image_draw_isometric_footprint_from_draw_tile(image_id, x, y, color_mask, scale);
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        if (has_deleted_building(grid_offset)) {
            color_mask = COLOR_MASK_RED;
        }
        int building_id = map_building_at(grid_offset);
        building *b = building_get(building_id);
        int offset = get_desirability_image_offset(b->desirability);
        image_draw_isometric_footprint_from_draw_tile(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y,
            color_mask, scale);
    } else if (map_desirability_get(grid_offset)) {
        int offset = get_desirability_image_offset(map_desirability_get(grid_offset));
        image_draw_isometric_footprint_from_draw_tile(
            image_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y, color_mask, scale);
    } else {
        image_draw_isometric_footprint_from_draw_tile(map_image_at(grid_offset), x, y, color_mask, scale);
    }
}

static void draw_top_desirability(int x, int y, float scale, int grid_offset)
{
    color_t color_mask = map_property_is_deleted(grid_offset) ? COLOR_MASK_RED : 0;
    if (map_terrain_is(grid_offset, terrain_on_desirability_overlay())
        && !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        // display normal tile
        if (map_property_is_draw_tile(grid_offset)) {
            image_draw_isometric_top_from_draw_tile(map_image_at(grid_offset), x, y, color_mask, scale);
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
        // grass, no top needed
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        if (has_deleted_building(grid_offset)) {
            color_mask = COLOR_MASK_RED;
        }
        int building_id = map_building_at(grid_offset);
        building *b = building_get(building_id);
        int offset = get_desirability_image_offset(b->desirability);
        image_draw_isometric_top_from_draw_tile(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y,
            color_mask, scale);
    } else if (map_desirability_get(grid_offset)) {
        int offset = get_desirability_image_offset(map_desirability_get(grid_offset));
        image_draw_isometric_top_from_draw_tile(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y,
            color_mask, scale);
    } else {
        image_draw_isometric_top_from_draw_tile(map_image_at(grid_offset), x, y, color_mask, scale);
    }
}

const city_overlay *city_overlay_for_desirability(void)
{
    static city_overlay overlay = {
        OVERLAY_DESIRABILITY,
        COLUMN_COLOR_GREEN,
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

const city_overlay *city_overlay_for_roads(void)
{
    static city_overlay overlay = {
        OVERLAY_ROADS,
        COLUMN_COLOR_GREEN,
        show_building_roads,
        show_figure_none,
        get_column_height_none,
        get_tooltip_none,
        0,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_levy(void)
{
    static city_overlay overlay = {
        OVERLAY_LEVY,
        COLUMN_COLOR_GREEN,
        show_building_none,
        show_figure_none,
        get_column_height_levy,
        0,
        get_tooltip_levy,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_mothball(void)
{
    static city_overlay overlay = {
        OVERLAY_MOTHBALL,
        COLUMN_COLOR_GREEN,
        show_building_mothball,
        show_figure_none,
        get_column_height_none,
        get_tooltip_none,
        0,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_warehouses(void)
{
    static city_overlay overlay = {
        OVERLAY_WAREHOUSE,
        COLUMN_COLOR_GREEN,
        show_building_warehouses,
        show_figure_warehouses,
        get_column_height_none,
        get_tooltip_none,
        0,
        0,
        0
    };
    return &overlay;
}

