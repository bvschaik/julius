#include "CityBuildings_private.h"

#include "building/animation.h"
#include "building/industry.h"
#include "building/model.h"
#include "city/view.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "game/resource.h"
#include "game/state.h"
#include "graphics/image.h"
#include "map/desirability.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"
#include "widget/city_figure.h"
#include "widget/city_without_overlay.h"
#include "widget/overlay.h"
#include "widget/overlay_education.h"
#include "widget/overlay_entertainment.h"
#include "widget/overlay_health.h"
#include "widget/overlay_risks.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#define NO_COLUMN -1

static const city_overlay *overlay = 0;

static const city_overlay *get_city_overlay()
{
    switch (game_state_overlay()) {
        case OVERLAY_FIRE:
            return overlay_for_fire();
        case OVERLAY_CRIME:
            return overlay_for_crime();
        case OVERLAY_DAMAGE:
            return overlay_for_damage();
        case OVERLAY_PROBLEMS:
            return overlay_for_problems();
        case OVERLAY_NATIVE:
            return overlay_for_native();
        case OVERLAY_ENTERTAINMENT:
            return overlay_for_entertainment();
        case OVERLAY_THEATER:
            return overlay_for_theater();
        case OVERLAY_AMPHITHEATER:
            return overlay_for_amphitheater();
        case OVERLAY_COLOSSEUM:
            return overlay_for_colosseum();
        case OVERLAY_HIPPODROME:
            return overlay_for_hippodrome();
        case OVERLAY_EDUCATION:
            return overlay_for_education();
        case OVERLAY_SCHOOL:
            return overlay_for_school();
        case OVERLAY_LIBRARY:
            return overlay_for_library();
        case OVERLAY_ACADEMY:
            return overlay_for_academy();
        case OVERLAY_BARBER:
            return overlay_for_barber();
        case OVERLAY_BATHHOUSE:
            return overlay_for_bathhouse();
        case OVERLAY_CLINIC:
            return overlay_for_clinic();
        case OVERLAY_HOSPITAL:
            return overlay_for_hospital();
        case OVERLAY_RELIGION:
            return 0;
        case OVERLAY_TAX_INCOME:
            return 0;
        case OVERLAY_FOOD_STOCKS:
            return 0;
        case OVERLAY_WATER:
            return 0;
        default:
            return 0;
    }
}

static void select_city_overlay()
{
    if (!overlay || overlay->type != game_state_overlay()) {
        overlay = get_city_overlay();
    }
}

static void drawFootprintForWaterOverlay(int x, int y, int grid_offset);
static void drawBuildingFootprintForOverlay(int gridOffset, int xOffset, int yOffset, int graphicOffset);
static void drawBuildingTopForOverlay(int grid_offset, int x, int y);
static void drawFootprintForDesirabilityOverlay(int x, int y, int grid_offset);

void draw_foot_with_size(int grid_offset, int image_x, int image_y)
{
    int image_id = map_image_at(grid_offset);
    switch (map_property_multi_tile_size(grid_offset)) {
        case 1:
            DRAWFOOT_SIZE1(image_id, image_x, image_y);
            break;
        case 2:
            DRAWFOOT_SIZE2(image_id, image_x, image_y);
            break;
        case 3:
            DRAWFOOT_SIZE3(image_id, image_x, image_y);
            break;
        case 4:
            DRAWFOOT_SIZE4(image_id, image_x, image_y);
            break;
        case 5:
            DRAWFOOT_SIZE5(image_id, image_x, image_y);
            break;
    }
}

void draw_top_with_size(int grid_offset, int image_x, int image_y)
{
    int image_id = map_image_at(grid_offset);
    switch (map_property_multi_tile_size(grid_offset)) {
        case 1:
            DRAWTOP_SIZE1(image_id, image_x, image_y);
            break;
        case 2:
            DRAWTOP_SIZE2(image_id, image_x, image_y);
            break;
        case 3:
            DRAWTOP_SIZE3(image_id, image_x, image_y);
            break;
        case 4:
            DRAWTOP_SIZE4(image_id, image_x, image_y);
            break;
        case 5:
            DRAWTOP_SIZE5(image_id, image_x, image_y);
            break;
    }
}

static void draw_footprint(int x, int y, int grid_offset)
{
    if (grid_offset == Data_State.selectedBuilding.gridOffsetStart) {
        Data_State.selectedBuilding.reservoirOffsetX = x;
        Data_State.selectedBuilding.reservoirOffsetY = y;
    }
    int overlay_type = game_state_overlay();
    if (grid_offset < 0) {
        // Outside map: draw black tile
        DRAWFOOT_SIZE1(image_group(GROUP_TERRAIN_BLACK), x, y);
    } else if (overlay->draw_custom_footprint) {
        overlay->draw_custom_footprint(x, y, grid_offset);
    } else if (overlay_type == OVERLAY_DESIRABILITY) {
        drawFootprintForDesirabilityOverlay(x, y, grid_offset);
    } else if (overlay_type == OVERLAY_WATER) {
        drawFootprintForWaterOverlay(x, y, grid_offset);
    } else if (map_property_is_draw_tile(grid_offset)) {
        int terrain = map_terrain_get(grid_offset);
        if (terrain & (TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
            // display grass
            int image_id = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7);
            DRAWFOOT_SIZE1(image_id, x, y);
        } else if ((terrain & TERRAIN_ROAD) && !(terrain & TERRAIN_BUILDING)) {
            draw_foot_with_size(grid_offset, x, y);
        } else if (terrain & TERRAIN_BUILDING) {
            drawBuildingFootprintForOverlay(grid_offset, x, y, 0);
        } else {
            draw_foot_with_size(grid_offset, x, y);
        }
    }
}

static int showOnOverlay(const figure *f)
{
    if (overlay) {
        return overlay->show_figure(f);
    }
    switch (game_state_overlay()) {
        case OVERLAY_WATER:
        case OVERLAY_DESIRABILITY:
            return 0;
        case OVERLAY_TAX_INCOME:
            return f->type == FIGURE_TAX_COLLECTOR;
        case OVERLAY_RELIGION:
            return f->type == FIGURE_PRIEST;
        case OVERLAY_FOOD_STOCKS:
            if (f->type == FIGURE_MARKET_BUYER || f->type == FIGURE_MARKET_TRADER ||
                f->type == FIGURE_DELIVERY_BOY || f->type == FIGURE_FISHING_BOAT) {
                return 1;
            } else if (f->type == FIGURE_CART_PUSHER) {
                return resource_is_food(f->resourceId);
            }
            return 0;
    }
    return 1;
}

static void draw_figures(int x, int y, int grid_offset)
{
    int figure_id = map_figure_at(grid_offset);
    while (figure_id) {
        figure *fig = figure_get(figure_id);
        if (!fig->isGhost && showOnOverlay(fig)) {
            city_draw_figure(fig, x, y);
        }
        figure_id = fig->nextFigureIdOnSameTile;
    }
}

static void draw_animation(int x, int y, int grid_offset)
{
    int overlay = game_state_overlay();
    int draw = 0;
    if (map_building_at(grid_offset)) {
        int btype = building_get(map_building_at(grid_offset))->type;
        switch (overlay) {
            case OVERLAY_FIRE:
            case OVERLAY_CRIME:
                if (btype == BUILDING_PREFECTURE || btype == BUILDING_BURNING_RUIN) {
                    draw = 1;
                }
                break;
            case OVERLAY_DAMAGE:
                if (btype == BUILDING_ENGINEERS_POST) {
                    draw = 1;
                }
                break;
            case OVERLAY_WATER:
                if (btype == BUILDING_RESERVOIR || btype == BUILDING_FOUNTAIN) {
                    draw = 1;
                }
                break;
            case OVERLAY_FOOD_STOCKS:
                if (btype == BUILDING_MARKET || btype == BUILDING_GRANARY) {
                    draw = 1;
                }
                break;
        }
    }

    int graphicId = map_image_at(grid_offset);
    const image *img = image_get(graphicId);
    if (img->num_animation_sprites && draw) {
        if (map_property_is_draw_tile(grid_offset)) {
            building *b = building_get(map_building_at(grid_offset));
            int color_mask = 0;
            if (b->type == BUILDING_GRANARY) {
                image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 1,
                                  x + img->sprite_offset_x,
                                  y + 60 + img->sprite_offset_y - img->height,
                                  color_mask);
                if (b->data.storage.resourceStored[RESOURCE_NONE] < 2400) {
                    image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 2, x + 33, y - 60, color_mask);
                }
                if (b->data.storage.resourceStored[RESOURCE_NONE] < 1800) {
                    image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 3, x + 56, y - 50, color_mask);
                }
                if (b->data.storage.resourceStored[RESOURCE_NONE] < 1200) {
                    image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 4, x + 91, y - 50, color_mask);
                }
                if (b->data.storage.resourceStored[RESOURCE_NONE] < 600) {
                    image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 5, x + 117, y - 62, color_mask);
                }
            } else {
                int animationOffset = building_animation_offset(b, graphicId, grid_offset);
                if (animationOffset > 0) {
                    if (animationOffset > img->num_animation_sprites) {
                        animationOffset = img->num_animation_sprites;
                    }
                    int ydiff = 0;
                    switch (map_property_multi_tile_size(grid_offset)) {
                        case 1: ydiff = 30; break;
                        case 2: ydiff = 45; break;
                        case 3: ydiff = 60; break;
                        case 4: ydiff = 75; break;
                        case 5: ydiff = 90; break;
                    }
                    image_draw_masked(graphicId + animationOffset,
                                      x + img->sprite_offset_x,
                                      y + ydiff + img->sprite_offset_y - img->height,
                                      color_mask);
                }
            }
        }
    } else if (map_is_bridge(grid_offset)) {
        city_draw_bridge(x, y, grid_offset);
    }
}

static void draw_elevated_figures(int x, int y, int grid_offset)
{
    int figure_id = map_figure_at(grid_offset);
    while (figure_id > 0) {
        figure *f = figure_get(figure_id);
        if (((f->useCrossCountry && !f->isGhost) || f->heightAdjustedTicks) && showOnOverlay(f)) {
            city_draw_figure(f, x, y);
        }
        figure_id = f->nextFigureIdOnSameTile;
    }
}

static int terrain_on_water_overlay()
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SCRUB |
        TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_AQUEDUCT | TERRAIN_ELEVATION |
        TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE;
}

static void drawFootprintForWaterOverlay(int x, int y, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }
	if (map_terrain_is(grid_offset, terrain_on_water_overlay())) {
		if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
			drawBuildingFootprintForOverlay(grid_offset, x, y, 0);
		} else {
			draw_foot_with_size(grid_offset, x, y);
		}
	} else if (map_terrain_is(grid_offset, TERRAIN_WALL)) {
		// display grass
		int graphicId = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7);
		DRAWFOOT_SIZE1(graphicId, x, y);
	} else if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        building *b = building_get(map_building_at(grid_offset));
		int terrain = map_terrain_get(grid_offset);
		if (b->id && b->hasWellAccess == 1) {
			terrain |= TERRAIN_FOUNTAIN_RANGE;
		}
		int graphicOffset;
		switch (terrain & (TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE)) {
			case TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE:
				graphicOffset = 24;
				break;
			case TERRAIN_RESERVOIR_RANGE:
				graphicOffset = 8;
				break;
			case TERRAIN_FOUNTAIN_RANGE:
				graphicOffset = 16;
				break;
			default:
				graphicOffset = 0;
				break;
		}
		drawBuildingFootprintForOverlay(grid_offset, x, y, graphicOffset);
	} else {
		int graphicId = image_group(GROUP_TERRAIN_OVERLAY);
		switch (map_terrain_get(grid_offset) & (TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE)) {
			case TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE:
				graphicId += 27;
				break;
			case TERRAIN_RESERVOIR_RANGE:
				graphicId += 11;
				break;
			case TERRAIN_FOUNTAIN_RANGE:
				graphicId += 19;
				break;
			default:
				graphicId = map_image_at(grid_offset);
				break;
		}
		DRAWFOOT_SIZE1(graphicId, x, y);
	}
}

static void drawTopForWaterOverlay(int x, int y, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }
	if (map_terrain_is(grid_offset, terrain_on_water_overlay())) {
		if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
			draw_top_with_size(grid_offset, x, y);
		}
	} else if (map_building_at(grid_offset)) {
		drawBuildingTopForOverlay(grid_offset, x, y);
	}
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

static void drawFootprintForDesirabilityOverlay(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, terrain_on_desirability_overlay()) && !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        // display normal tile
        if (map_property_is_draw_tile(grid_offset)) {
            draw_foot_with_size(grid_offset, x, y);
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
        // display empty land/grass
        int image_id = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7);
        DRAWFOOT_SIZE1(image_id, x, y);
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING) || map_desirability_get(grid_offset)) {
        int offset = get_desirability_image_offset(map_desirability_get(grid_offset));
        DRAWFOOT_SIZE1(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y);
    } else {
        DRAWFOOT_SIZE1(map_image_at(grid_offset), x, y);
    }
}

static void drawTopForDesirabilityOverlay(int x, int y, int grid_offset)
{
    if (map_terrain_is(grid_offset, terrain_on_desirability_overlay()) && !map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        // display normal tile
        if (map_property_is_draw_tile(grid_offset)) {
            draw_top_with_size(grid_offset, x, y);
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
        // grass, no top needed
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING) || map_desirability_get(grid_offset)) {
        int offset = get_desirability_image_offset(map_desirability_get(grid_offset));
        DRAWTOP_SIZE1(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, x, y);
    } else {
        DRAWTOP_SIZE1(map_image_at(grid_offset), x, y);
    }
}

static int is_drawable_farmhouse(int grid_offset, int map_orientation)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return 0;
    }
    int xy = map_property_multi_tile_xy(grid_offset);
    if (map_orientation == DIR_0_TOP && xy == Edge_X0Y1) {
        return 1;
    }
    if (map_orientation == DIR_2_RIGHT && xy == Edge_X0Y0) {
        return 1;
    }
    if (map_orientation == DIR_4_BOTTOM && xy == Edge_X1Y0) {
        return 1;
    }
    if (map_orientation == DIR_2_RIGHT && xy == Edge_X1Y1) {
        return 1;
    }
    return 0;
}

static int is_drawable_farm_corner(int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return 0;
    }

    int map_orientation = city_view_orientation();
    int xy = map_property_multi_tile_xy(grid_offset);
    if (map_orientation == DIR_0_TOP && xy == Edge_X0Y2) {
        return 1;
    }
    if (map_orientation == DIR_2_RIGHT && xy == Edge_X0Y0) {
        return 1;
    }
    if (map_orientation == DIR_4_BOTTOM && xy == Edge_X2Y0) {
        return 1;
    }
    if (map_orientation == DIR_2_RIGHT && xy == Edge_X2Y2) {
        return 1;
    }
    return 0;
}

static void draw_flattened_overlay_building(const building *b, int x, int y, int image_offset)
{
    int image_base = image_group(GROUP_TERRAIN_OVERLAY) + image_offset;
    if (b->houseSize) {
        image_base += 4;
    }
    if (b->size == 1) {
        image_draw_isometric_footprint(image_base, x, y, 0);
    } else if (b->size == 2) {
        int xTileOffset[] = {30, 0, 60, 30};
        int yTileOffset[] = {-15, 0, 0, 15};
        for (int i = 0; i < 4; i++) {
            image_draw_isometric_footprint(image_base + i, x + xTileOffset[i], y + yTileOffset[i], 0);
        }
    } else if (b->size == 3) {
        int graphicTileOffset[] = {0, 1, 2, 1, 3, 2, 3, 3, 3};
        int xTileOffset[] = {60, 30, 90, 0, 60, 120, 30, 90, 60};
        int yTileOffset[] = {-30, -15, -15, 0, 0, 0, 15, 15, 30};
        for (int i = 0; i < 9; i++) {
            image_draw_isometric_footprint(image_base + graphicTileOffset[i], x + xTileOffset[i], y + yTileOffset[i], 0);
        }
    } else if (b->size == 4) {
        int graphicTileOffset[] = {0, 1, 2, 1, 3, 2, 1, 3, 3, 2, 3, 3, 3, 3, 3, 3};
        int xTileOffset[] = {
            90,
            60, 120,
            30, 90, 150,
            0, 60, 120, 180,
            30, 90, 150,
            60, 120,
            90
        };
        int yTileOffset[] = {
            -45,
            -30, -30,
            -15, -15, -15,
            0, 0, 0, 0,
            15, 15, 15,
            30, 30,
            45
        };
        for (int i = 0; i < 16; i++) {
            image_draw_isometric_footprint(image_base + graphicTileOffset[i], x + xTileOffset[i], y + yTileOffset[i], 0);
        }
    } else if (b->size == 5) {
        int graphicTileOffset[] = {0, 1, 2, 1, 3, 2, 1, 3, 3, 2, 1, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
        int xTileOffset[] = {
            120,
            90, 150,
            60, 120, 180,
            30, 90, 150, 210,
            0, 60, 120, 180, 240,
            30, 90, 150, 210,
            60, 120, 180,
            90, 150,
            120
        };
        int yTileOffset[] = {
            -60,
            -45, -45,
            -30, -30, -30,
            -15, -15, -15, -15,
            0, 0, 0, 0, 0,
            15, 15, 15, 15,
            30, 30, 30,
            45, 45,
            60
        };
        for (int i = 0; i < 25; i++) {
            image_draw_isometric_footprint(image_base + graphicTileOffset[i], x + xTileOffset[i], y + yTileOffset[i], 0);
        }
    }
}


static int show_building_religion(building *b)
{
    return
        b->type == BUILDING_ORACLE || b->type == BUILDING_SMALL_TEMPLE_CERES ||
        b->type == BUILDING_SMALL_TEMPLE_NEPTUNE || b->type == BUILDING_SMALL_TEMPLE_MERCURY ||
        b->type == BUILDING_SMALL_TEMPLE_MARS || b->type == BUILDING_SMALL_TEMPLE_VENUS ||
        b->type == BUILDING_LARGE_TEMPLE_CERES || b->type == BUILDING_LARGE_TEMPLE_NEPTUNE ||
        b->type == BUILDING_LARGE_TEMPLE_MERCURY || b->type == BUILDING_LARGE_TEMPLE_MARS ||
        b->type == BUILDING_LARGE_TEMPLE_VENUS;
}

static int show_building_food_stocks(building *b)
{
    return b->type == BUILDING_MARKET || b->type == BUILDING_WHARF || b->type == BUILDING_GRANARY;
}

static int show_building_tax_income(building *b)
{
    return b->type == BUILDING_FORUM || b->type == BUILDING_SENATE_UPGRADED;
}

static int show_building_water(building *b)
{
    return b->type == BUILDING_WELL || b->type == BUILDING_FOUNTAIN || b->type == BUILDING_RESERVOIR;
}

static int should_show_building_on_overlay(building *b)
{
    if (overlay) {
        return overlay->show_building(b);
    }
    switch (game_state_overlay()) {
        case OVERLAY_RELIGION:
            return show_building_religion(b);
        case OVERLAY_TAX_INCOME:
            return show_building_tax_income(b);
        case OVERLAY_FOOD_STOCKS:
            return show_building_food_stocks(b);
        case OVERLAY_WATER:
            return show_building_water(b);
        default:
            return 0;
    }
}

static void drawBuildingFootprintForOverlay(int gridOffset, int xOffset, int yOffset, int graphicOffset)
{
    int buildingId = map_building_at(gridOffset);
    if (!buildingId) {
        return;
    }
    building *b = building_get(buildingId);
    int show_on_overlay = should_show_building_on_overlay(b);
    if (show_on_overlay) {
        switch (b->size) {
            case 1:
                DRAWFOOT_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
                break;
            case 2:
                DRAWFOOT_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
                break;
            case 3:
                if (building_is_farm(b->type)) {
                    if (is_drawable_farmhouse(gridOffset, city_view_orientation())) {
                        DRAWFOOT_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
                    } else if (map_property_is_draw_tile(gridOffset)) {
                        DRAWFOOT_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
                    }
                } else {
                    DRAWFOOT_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
                }
                break;
            case 4:
                DRAWFOOT_SIZE4(map_image_at(gridOffset), xOffset, yOffset);
                break;
            case 5:
                DRAWFOOT_SIZE5(map_image_at(gridOffset), xOffset, yOffset);
                break;
        }
    } else {
        int draw = 1;
        if (b->size == 3 && building_is_farm(b->type)) {
            draw = is_drawable_farm_corner(gridOffset);
        }
        if (draw) {
            draw_flattened_overlay_building(b, xOffset, yOffset, graphicOffset);
        }
    }
}

static int get_column_height_religion(building *b)
{
    return b->houseSize && b->data.house.numGods ? b->data.house.numGods * 17 / 10 : NO_COLUMN;
}

static int get_column_height_tax_income(building *b)
{
    if (b->houseSize) {
        int pct = calc_adjust_with_percentage(b->taxIncomeOrStorage / 2, Data_CityInfo.taxPercentage);
        if (pct > 0) {
            return pct / 25;
        }
    }
    return NO_COLUMN;
}

static int get_column_height_food_stocks(building *b)
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

static int get_building_column_height(building *b)
{
    if (overlay) {
        return overlay->get_column_height(b);
    }
    switch (game_state_overlay()) {
        case OVERLAY_RELIGION:
            return get_column_height_religion(b);
        case OVERLAY_TAX_INCOME:
            return get_column_height_tax_income(b);
        case OVERLAY_FOOD_STOCKS:
            return get_column_height_food_stocks(b);
        default:
            return NO_COLUMN;
    }
}

static void draw_building_top(int grid_offset, building *b, int x, int y)
{
    if (building_is_farm(b->type)) {
        if (is_drawable_farmhouse(grid_offset, city_view_orientation())) {
            DRAWTOP_SIZE2(map_image_at(grid_offset), x, y);
        } else if (map_property_is_draw_tile(grid_offset)) {
            DRAWTOP_SIZE1(map_image_at(grid_offset), x, y);
        }
        return;
    }
    if (b->type == BUILDING_GRANARY) {
        const image *img = image_get(map_image_at(grid_offset));
        image_draw(image_group(GROUP_BUILDING_GRANARY) + 1, x + img->sprite_offset_x, y + img->sprite_offset_y - 30 - (img->height - 90));
        if (b->data.storage.resourceStored[RESOURCE_NONE] < 2400) {
            image_draw(image_group(GROUP_BUILDING_GRANARY) + 2, x + 33, y - 60);
            if (b->data.storage.resourceStored[RESOURCE_NONE] < 1800) {
                image_draw(image_group(GROUP_BUILDING_GRANARY) + 3, x + 56, y - 50);
            }
            if (b->data.storage.resourceStored[RESOURCE_NONE] < 1200) {
                image_draw(image_group(GROUP_BUILDING_GRANARY) + 4, x + 91, y - 50);
            }
            if (b->data.storage.resourceStored[RESOURCE_NONE] < 600) {
                image_draw(image_group(GROUP_BUILDING_GRANARY) + 5, x + 117, y - 62);
            }
        }
    }
    if (b->type == BUILDING_WAREHOUSE) {
        image_draw(image_group(GROUP_BUILDING_WAREHOUSE) + 17, x - 4, y - 42);
    }

    draw_top_with_size(grid_offset, x, y);
}

static void draw_overlay_column(int x, int y, int height, int is_red)
{
    int image_id = image_group(GROUP_OVERLAY_COLUMN);
    if (is_red) {
        image_id += 9;
    }
    if (height > 10) {
        height = 10;
    }
    int capital_height = image_get(image_id)->height;
    // base
    image_draw(image_id + 2, x + 9, y - 8);
    if (height) {
        // column
        for (int i = 1; i < height; i++) {
            image_draw(image_id + 1, x + 17, y - 8 - 10 * i + 13);
        }
        // capital
        image_draw(image_id, x + 5, y - 8 - capital_height - 10 * (height - 1) + 13);
    }
}

static void drawBuildingTopForOverlay(int grid_offset, int x, int y)
{
    int overlay = game_state_overlay();
    building *b = building_get(map_building_at(grid_offset));
    if (overlay == OVERLAY_PROBLEMS) {
        overlay_problems_prepare_building(b);
    }
    if (should_show_building_on_overlay(b)) {
        draw_building_top(grid_offset, b, x, y);
    } else {
        int column_height = get_building_column_height(b);
        if (column_height != NO_COLUMN) {
            int is_red = 0;
            switch (overlay) {
                case OVERLAY_FIRE:
                case OVERLAY_DAMAGE:
                case OVERLAY_CRIME:
                case OVERLAY_FOOD_STOCKS:
                    is_red = 1;
            }
            int draw = 1;
            if (building_is_farm(b->type)) {
                draw = is_drawable_farm_corner(grid_offset);
            }
            if (draw) {
                draw_overlay_column(x, y, column_height, is_red);
            }
        }
    }
}

static void draw_top(int x, int y, int grid_offset)
{
    if (overlay->draw_custom_top) {
        overlay->draw_custom_top(x, y, grid_offset);
        return;
    }
    int overlay_type = game_state_overlay();
    if (overlay_type == OVERLAY_DESIRABILITY) {
        drawTopForDesirabilityOverlay(x, y, grid_offset);
    } else if (overlay_type == OVERLAY_WATER) {
        drawTopForWaterOverlay(x, y, grid_offset);
    } else if (map_property_is_draw_tile(grid_offset)) {
        if (!map_terrain_is(grid_offset, TERRAIN_WALL | TERRAIN_AQUEDUCT | TERRAIN_ROAD)) {
            if (map_terrain_is(grid_offset, TERRAIN_BUILDING) && map_building_at(grid_offset)) {
                drawBuildingTopForOverlay(grid_offset, x, y);
            } else if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
                // terrain
                draw_top_with_size(grid_offset, x, y);
            }
        }
    }
}

void city_with_overlay_draw()
{
    select_city_overlay();

    city_view_foreach_map_tile(draw_footprint);
    city_view_foreach_valid_map_tile(
        draw_figures,
        draw_top,
        draw_animation
    );
    UI_CityBuildings_drawSelectedBuildingGhost();
    city_view_foreach_valid_map_tile(draw_elevated_figures, 0, 0);
}

void city_with_overlay_draw_building_footprint(int x, int y, int grid_offset, int image_offset)
{
    drawBuildingFootprintForOverlay(grid_offset, x, y, image_offset);
}

void city_with_overlay_draw_building_top(int x, int y, int grid_offset)
{
    drawBuildingTopForOverlay(grid_offset, x, y);
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

static int get_tooltip_food_stocks(tooltip_context *c, const building *b)
{
    if (b->housePopulation <= 0) {
        return 0;
    }
    if (!model_get_house(b->subtype.houseLevel)->food_types) {
        return 104;
    } else {
        int stocksPresent = 0;
        for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
            stocksPresent += b->data.house.inventory[i];
        }
        int stocksPerPop = calc_percentage(stocksPresent, b->housePopulation);
        if (stocksPerPop <= 0) {
            return 4;
        } else if (stocksPerPop < 100) {
            return 5;
        } else if (stocksPerPop <= 200) {
            return 6;
        } else {
            return 7;
        }
    }
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

int UI_CityBuildings_getOverlayTooltipText(tooltip_context *c, int grid_offset)
{
    int overlay_type = game_state_overlay();
    int buildingId = map_building_at(grid_offset);
    if (overlay_type != OVERLAY_WATER && overlay_type != OVERLAY_DESIRABILITY && !buildingId) {
        return 0;
    }
    int overlayRequiresHouse =
        overlay_type != OVERLAY_WATER && overlay_type != OVERLAY_FIRE &&
        overlay_type != OVERLAY_DAMAGE && overlay_type != OVERLAY_NATIVE;
    int overlayForbidsHouse = overlay_type == OVERLAY_NATIVE;
    building *b = building_get(buildingId);
    if (overlayRequiresHouse && !b->houseSize) {
        return 0;
    }
    if (overlayForbidsHouse && b->houseSize) {
        return 0;
    }
    if (overlay) {
        if (overlay->get_tooltip_for_building) {
            return overlay->get_tooltip_for_building(c, b);
        } else if (overlay->get_tooltip_for_grid_offset) {
            return overlay->get_tooltip_for_grid_offset(c, grid_offset);
        }
    }
    switch (overlay_type) {
        case OVERLAY_WATER:
            return get_tooltip_water(c, grid_offset);
        case OVERLAY_DESIRABILITY:
            return get_tooltip_desirability(c, grid_offset);
        case OVERLAY_RELIGION:
            return get_tooltip_religion(c, b);
        case OVERLAY_TAX_INCOME:
            return get_tooltip_tax_income(c, b);
        case OVERLAY_FOOD_STOCKS:
            return get_tooltip_food_stocks(c, b);
    }
    return 0;
}
