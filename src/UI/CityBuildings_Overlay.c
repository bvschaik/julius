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

#include "Data/CityInfo.h"
#include "Data/State.h"

static void drawFootprintForWaterOverlay(int gridOffset, int xOffset, int yOffset);
static void drawTopForWaterOverlay(int gridOffset, int xOffset, int yOffset);
static void drawFootprintForNativeOverlay(int gridOffset, int xOffset, int yOffset);
static void drawTopForNativeOverlay(int gridOffset, int xOffset, int yOffset);
static void drawBuildingFootprintForOverlay(int buildingId, int gridOffset, int xOffset, int yOffset, int graphicOffset);
static void drawBuildingFootprintForDesirabilityOverlay(int gridOffset, int xOffset, int yOffset);
static void drawBuildingTopForDesirabilityOverlay(int gridOffset, int xOffset, int yOffset);
static void drawBuildingTopForFireOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForDamageOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForCrimeOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForEntertainmentOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForEducationOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForTheaterOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForAmphitheaterOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForColosseumOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForHippodromeOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForFoodStocksOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForBathhouseOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForReligionOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForSchoolOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForLibraryOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForAcademyOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForBarberOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForClinicsOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForHospitalOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForTaxIncomeOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawBuildingTopForProblemsOverlay(int gridOffset, building *b, int xOffset, int yOffset);
static void drawOverlayColumn(int height, int xOffset, int yOffset, int isRed);

static void draw_foot_with_size(int grid_offset, int image_x, int image_y)
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

static void draw_top_with_size(int grid_offset, int image_x, int image_y)
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
    int overlay = game_state_overlay();
    if (grid_offset == Data_State.selectedBuilding.gridOffsetStart) {
        Data_State.selectedBuilding.reservoirOffsetX = x;
        Data_State.selectedBuilding.reservoirOffsetY = y;
    }
    if (grid_offset < 0) {
        // Outside map: draw black tile
        DRAWFOOT_SIZE1(image_group(GROUP_TERRAIN_BLACK), x, y);
    } else if (overlay == OVERLAY_DESIRABILITY) {
        drawBuildingFootprintForDesirabilityOverlay(grid_offset, x, y);
    } else if (map_property_is_draw_tile(grid_offset)) {
        int terrain = map_terrain_get(grid_offset);
        if (overlay == OVERLAY_WATER) {
            drawFootprintForWaterOverlay(grid_offset, x, y);
        } else if (overlay == OVERLAY_NATIVE) {
            drawFootprintForNativeOverlay(grid_offset, x, y);
        } else if (terrain & (TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
            // display grass
            int image_id = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7);
            DRAWFOOT_SIZE1(image_id, x, y);
        } else if ((terrain & TERRAIN_ROAD) && !(terrain & TERRAIN_BUILDING)) {
            draw_foot_with_size(grid_offset, x, y);
        } else if (terrain & TERRAIN_BUILDING) {
            drawBuildingFootprintForOverlay(map_building_at(grid_offset), grid_offset, x, y, 0);
        } else {
            draw_foot_with_size(grid_offset, x, y);
        }
    }
}

void UI_CityBuildings_drawOverlayFootprints()
{
    city_view_foreach_map_tile(draw_footprint);
}

static building *get_entertainment_building(const figure *f)
{
    if (f->actionState == FIGURE_ACTION_94_ENTERTAINER_ROAMING ||
        f->actionState == FIGURE_ACTION_95_ENTERTAINER_RETURNING) {
        return building_get(f->buildingId);
    } else {
        return building_get(f->destinationBuildingId);
    }
}

static int showOnOverlay(const figure *f)
{
    switch (game_state_overlay()) {
        case OVERLAY_WATER:
        case OVERLAY_DESIRABILITY:
            return 0;
        case OVERLAY_NATIVE:
            return f->type == FIGURE_INDIGENOUS_NATIVE || f->type == FIGURE_MISSIONARY;
        case OVERLAY_FIRE:
            return f->type == FIGURE_PREFECT;
        case OVERLAY_DAMAGE:
            return f->type == FIGURE_ENGINEER;
        case OVERLAY_TAX_INCOME:
            return f->type == FIGURE_TAX_COLLECTOR;
        case OVERLAY_CRIME:
            return f->type == FIGURE_PREFECT || f->type == FIGURE_PROTESTER ||
                f->type == FIGURE_CRIMINAL || f->type == FIGURE_RIOTER;
        case OVERLAY_ENTERTAINMENT:
            return f->type == FIGURE_ACTOR || f->type == FIGURE_GLADIATOR ||
                f->type == FIGURE_LION_TAMER || f->type == FIGURE_CHARIOTEER;
        case OVERLAY_EDUCATION:
            return f->type == FIGURE_SCHOOL_CHILD || f->type == FIGURE_LIBRARIAN ||
                f->type == FIGURE_TEACHER;
        case OVERLAY_THEATER:
            if (f->type == FIGURE_ACTOR) {
                return get_entertainment_building(f)->type == BUILDING_THEATER;
            }
            return 0;
        case OVERLAY_AMPHITHEATER:
            if (f->type == FIGURE_ACTOR || f->type == FIGURE_GLADIATOR) {
                return get_entertainment_building(f)->type == BUILDING_AMPHITHEATER;
            }
            return 0;
        case OVERLAY_COLOSSEUM:
            if (f->type == FIGURE_GLADIATOR) {
                return get_entertainment_building(f)->type == BUILDING_COLOSSEUM;
            } else if (f->type == FIGURE_LION_TAMER) {
                return 1;
            }
            return 0;
        case OVERLAY_HIPPODROME:
            return f->type == FIGURE_CHARIOTEER;
        case OVERLAY_RELIGION:
            return f->type == FIGURE_PRIEST;
        case OVERLAY_SCHOOL:
            return f->type == FIGURE_SCHOOL_CHILD;
        case OVERLAY_LIBRARY:
            return f->type == FIGURE_LIBRARIAN;
        case OVERLAY_ACADEMY:
            return f->type == FIGURE_TEACHER;
        case OVERLAY_BARBER:
            return f->type == FIGURE_BARBER;
        case OVERLAY_BATHHOUSE:
            return f->type == FIGURE_BATHHOUSE_WORKER;
        case OVERLAY_CLINIC:
            return f->type == FIGURE_DOCTOR;
        case OVERLAY_HOSPITAL:
            return f->type == FIGURE_SURGEON;
        case OVERLAY_FOOD_STOCKS:
            if (f->type == FIGURE_MARKET_BUYER || f->type == FIGURE_MARKET_TRADER ||
                f->type == FIGURE_DELIVERY_BOY || f->type == FIGURE_FISHING_BOAT) {
                return 1;
            } else if (f->type == FIGURE_CART_PUSHER) {
                return resource_is_food(f->resourceId);
            }
            return 0;
        case OVERLAY_PROBLEMS:
            if (f->type == FIGURE_LABOR_SEEKER) {
                return building_get(f->buildingId)->showOnProblemOverlay;
            } else if (f->type == FIGURE_CART_PUSHER) {
                return f->actionState == FIGURE_ACTION_20_CARTPUSHER_INITIAL || f->minMaxSeen;
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

static void draw_top(int x, int y, int grid_offset)
{
    int overlay = game_state_overlay();
    if (overlay == OVERLAY_DESIRABILITY) {
        drawBuildingTopForDesirabilityOverlay(grid_offset, x, y);
    } else if (map_property_is_draw_tile(grid_offset)) {
        if (overlay == OVERLAY_WATER) {
            drawTopForWaterOverlay(grid_offset, x, y);
        } else if (overlay == OVERLAY_NATIVE) {
            drawTopForNativeOverlay(grid_offset, x, y);
        } else if (!map_terrain_is(grid_offset, TERRAIN_WALL | TERRAIN_AQUEDUCT | TERRAIN_ROAD)) {
            if (map_terrain_is(grid_offset, TERRAIN_BUILDING) && map_building_at(grid_offset)) {
                building *b = building_get(map_building_at(grid_offset));
                switch (overlay) {
                    case OVERLAY_FIRE:
                        drawBuildingTopForFireOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_DAMAGE:
                        drawBuildingTopForDamageOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_CRIME:
                        drawBuildingTopForCrimeOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_ENTERTAINMENT:
                        drawBuildingTopForEntertainmentOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_THEATER:
                        drawBuildingTopForTheaterOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_AMPHITHEATER:
                        drawBuildingTopForAmphitheaterOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_COLOSSEUM:
                        drawBuildingTopForColosseumOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_HIPPODROME:
                        drawBuildingTopForHippodromeOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_RELIGION:
                        drawBuildingTopForReligionOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_EDUCATION:
                        drawBuildingTopForEducationOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_SCHOOL:
                        drawBuildingTopForSchoolOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_LIBRARY:
                        drawBuildingTopForLibraryOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_ACADEMY:
                        drawBuildingTopForAcademyOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_BARBER:
                        drawBuildingTopForBarberOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_BATHHOUSE:
                        drawBuildingTopForBathhouseOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_CLINIC:
                        drawBuildingTopForClinicsOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_HOSPITAL:
                        drawBuildingTopForHospitalOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_FOOD_STOCKS:
                        drawBuildingTopForFoodStocksOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_TAX_INCOME:
                        drawBuildingTopForTaxIncomeOverlay(grid_offset, b, x, y);
                        break;
                    case OVERLAY_PROBLEMS:
                        drawBuildingTopForProblemsOverlay(grid_offset, b, x, y);
                        break;
                }
            } else if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
                // terrain
                draw_top_with_size(grid_offset, x, y);
            }
        }
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
        widget_city_draw_bridge(x, y, grid_offset);
    }
}
void UI_CityBuildings_drawOverlayTopsFiguresAnimation()
{
    city_view_foreach_valid_map_tile(
        draw_figures,
        draw_top,
        draw_animation
    );
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

void UI_CityBuildings_drawOverlayElevatedFigures(void)
{
    city_view_foreach_valid_map_tile(draw_elevated_figures, 0, 0);
}

static int terrain_on_water_overlay()
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SCRUB |
        TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_AQUEDUCT | TERRAIN_ELEVATION |
        TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE;
}

static void drawFootprintForWaterOverlay(int gridOffset, int xOffset, int yOffset)
{
	if (map_terrain_is(gridOffset, terrain_on_water_overlay())) {
		if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
			drawBuildingFootprintForOverlay(map_building_at(gridOffset),
				gridOffset, xOffset, yOffset, 0);
		} else {
			draw_foot_with_size(gridOffset, xOffset, yOffset);
		}
	} else if (map_terrain_is(gridOffset, TERRAIN_WALL)) {
		// display grass
		int graphicId = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(gridOffset) & 7);
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	} else if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
        building *b = building_get(map_building_at(gridOffset));
		int terrain = map_terrain_get(gridOffset);
		if (b->id && b->hasWellAccess == 1) {
			terrain |= TERRAIN_FOUNTAIN_RANGE;
		}
		if (b->type == BUILDING_WELL || b->type == BUILDING_FOUNTAIN) {
			DRAWFOOT_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
		} else if (b->type == BUILDING_RESERVOIR) {
			DRAWFOOT_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
		} else {
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
			drawBuildingFootprintForOverlay(b->id, gridOffset, xOffset, yOffset, graphicOffset);
		}
	} else {
		int graphicId = image_group(GROUP_TERRAIN_OVERLAY);
		switch (map_terrain_get(gridOffset) & (TERRAIN_RESERVOIR_RANGE | TERRAIN_FOUNTAIN_RANGE)) {
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
				graphicId = map_image_at(gridOffset);
				break;
		}
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	}
}

static void drawTopForWaterOverlay(int gridOffset, int xOffset, int yOffset)
{
	if (map_terrain_is(gridOffset, terrain_on_water_overlay())) {
		if (!map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
			draw_top_with_size(gridOffset, xOffset, yOffset);
		}
	} else if (map_building_at(gridOffset)) {
		building *b = building_get(map_building_at(gridOffset));
		if (b->type == BUILDING_WELL || b->type == BUILDING_FOUNTAIN) {
			DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
		} else if (b->type == BUILDING_RESERVOIR) {
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
		}
	}
}

static int terrain_on_native_overlay()
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SCRUB |
        TERRAIN_GARDEN | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE;
}

static void drawFootprintForNativeOverlay(int gridOffset, int xOffset, int yOffset)
{
	if (map_terrain_is(gridOffset, terrain_on_native_overlay())) {
		if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
			drawBuildingFootprintForOverlay(map_building_at(gridOffset),
				gridOffset, xOffset, yOffset, 0);
		} else {
			draw_foot_with_size(gridOffset, xOffset, yOffset);
		}
	} else if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
		// display grass
		int graphicId = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(gridOffset) & 7);
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	} else if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
		drawBuildingFootprintForOverlay(map_building_at(gridOffset),
		gridOffset, xOffset, yOffset, 0);
	} else {
		if (map_property_is_native_land(gridOffset)) {
			DRAWFOOT_SIZE1(image_group(GROUP_TERRAIN_DESIRABILITY) + 1, xOffset, yOffset);
		} else {
			draw_foot_with_size(gridOffset, xOffset, yOffset);
		}
	}
}

static void drawTopForNativeOverlay(int gridOffset, int xOffset, int yOffset)
{
	if (map_terrain_is(gridOffset, terrain_on_native_overlay())) {
		if (!map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
			draw_top_with_size(gridOffset, xOffset, yOffset);
		}
	} else if (map_building_at(gridOffset)) {
		int graphicId = map_image_at(gridOffset);
		switch (building_get(map_building_at(gridOffset))->type) {
			case BUILDING_NATIVE_HUT:
				DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
				break;
			case BUILDING_NATIVE_MEETING:
			case BUILDING_MISSION_POST:
				DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
				break;
		}
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

static int is_drawable_farm_corner(int grid_offset, int map_orientation)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return 0;
    }

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

static void drawBuildingFootprintForOverlay(int buildingId, int gridOffset, int xOffset, int yOffset, int graphicOffset)
{
	if (!buildingId) {
		return;
	}
	building *b = building_get(buildingId);
	int origGraphicId = map_image_at(gridOffset);
	if (b->size == 1) {
		int graphicId = image_group(GROUP_TERRAIN_OVERLAY);
		if (b->houseSize) {
			graphicId += 4;
		}
		switch (game_state_overlay()) {
			case OVERLAY_DAMAGE:
				if (b->type == BUILDING_ENGINEERS_POST) {
					graphicId = origGraphicId;
				}
				break;
			case OVERLAY_BARBER:
				if (b->type == BUILDING_BARBER) {
					graphicId = origGraphicId;
				}
				break;
			case OVERLAY_CLINIC:
				if (b->type == BUILDING_DOCTOR) {
					graphicId = origGraphicId;
				}
				break;
			case OVERLAY_NATIVE:
				if (b->type == BUILDING_NATIVE_HUT) {
					graphicId = origGraphicId;
					graphicOffset = 0;
				}
				break;
			case OVERLAY_PROBLEMS:
				if (b->showOnProblemOverlay) {
					graphicId = origGraphicId;
				}
				break;
			case OVERLAY_FIRE:
			case OVERLAY_CRIME:
				if (b->type == BUILDING_PREFECTURE || b->type == BUILDING_BURNING_RUIN) {
					graphicId = origGraphicId;
				}
				break;
		}
		graphicId += graphicOffset;
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	} else if (b->size == 2) {
		int drawOrig = 0;
		switch (game_state_overlay()) {
			case OVERLAY_ENTERTAINMENT:
			case OVERLAY_THEATER:
				if (b->type == BUILDING_THEATER) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_EDUCATION:
				if (b->type == BUILDING_SCHOOL || b->type == BUILDING_LIBRARY) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_SCHOOL:
				if (b->type == BUILDING_SCHOOL) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_LIBRARY:
				if (b->type == BUILDING_LIBRARY) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_BATHHOUSE:
				if (b->type == BUILDING_BATHHOUSE) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_RELIGION:
				if (b->type == BUILDING_ORACLE || b->type == BUILDING_SMALL_TEMPLE_CERES ||
					b->type == BUILDING_SMALL_TEMPLE_NEPTUNE || b->type == BUILDING_SMALL_TEMPLE_MERCURY ||
					b->type == BUILDING_SMALL_TEMPLE_MARS || b->type == BUILDING_SMALL_TEMPLE_VENUS) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_FOOD_STOCKS:
				if (b->type == BUILDING_MARKET || b->type == BUILDING_WHARF) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_TAX_INCOME:
				if (b->type == BUILDING_FORUM) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_NATIVE:
				if (b->type == BUILDING_NATIVE_MEETING || b->type == BUILDING_MISSION_POST) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_PROBLEMS:
				if (b->showOnProblemOverlay) {
					drawOrig = 1;
				}
				break;
		}
		if (drawOrig) {
			DRAWFOOT_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
		} else {
			int graphicBase = image_group(GROUP_TERRAIN_OVERLAY) + graphicOffset;
			if (b->houseSize) {
				graphicBase += 4;
			}
			int xTileOffset[] = {30, 0, 60, 30};
			int yTileOffset[] = {-15, 0, 0, 15};
			for (int i = 0; i < 4; i++) {
				image_draw_isometric_footprint(graphicBase + i,
					xOffset + xTileOffset[i], yOffset + yTileOffset[i], 0);
			}
		}
	} else if (b->size == 3) {
		int drawOrig = 0;
		switch (game_state_overlay()) {
			case OVERLAY_ENTERTAINMENT:
				if (b->type == BUILDING_AMPHITHEATER || b->type == BUILDING_GLADIATOR_SCHOOL ||
					b->type == BUILDING_LION_HOUSE || b->type == BUILDING_ACTOR_COLONY ||
					b->type == BUILDING_CHARIOT_MAKER) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_THEATER:
				if (b->type == BUILDING_ACTOR_COLONY) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_AMPHITHEATER:
				if (b->type == BUILDING_ACTOR_COLONY || b->type == BUILDING_GLADIATOR_SCHOOL ||
					b->type == BUILDING_AMPHITHEATER) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_COLOSSEUM:
				if (b->type == BUILDING_GLADIATOR_SCHOOL || b->type == BUILDING_LION_HOUSE) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_HIPPODROME:
				if (b->type == BUILDING_CHARIOT_MAKER) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_EDUCATION:
			case OVERLAY_ACADEMY:
				if (b->type == BUILDING_ACADEMY) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_HOSPITAL:
				if (b->type == BUILDING_HOSPITAL) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_RELIGION:
				if (b->type == BUILDING_LARGE_TEMPLE_CERES || b->type == BUILDING_LARGE_TEMPLE_NEPTUNE ||
					b->type == BUILDING_LARGE_TEMPLE_MERCURY || b->type == BUILDING_LARGE_TEMPLE_MARS ||
					b->type == BUILDING_LARGE_TEMPLE_VENUS) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_FOOD_STOCKS:
				if (b->type == BUILDING_GRANARY) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_PROBLEMS:
				if (b->showOnProblemOverlay) {
					drawOrig = 1;
				}
				break;
		}
		// farms have multiple drawable tiles: the farmhouse and 5 fields
		if (drawOrig) {
			if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_PIG_FARM) {
				if (is_drawable_farmhouse(gridOffset, city_view_orientation())) {
					DRAWFOOT_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
				} else if (map_property_is_draw_tile(gridOffset)) {
					DRAWFOOT_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
				}
			} else {
				DRAWFOOT_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			}
		} else {
			int draw = 1;
			if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_PIG_FARM) {
			    draw = is_drawable_farm_corner(gridOffset, city_view_orientation());
			}
			if (draw) {
				int graphicBase = image_group(GROUP_TERRAIN_OVERLAY) + graphicOffset;
				if (b->houseSize) {
					graphicBase += 4;
				}
				int graphicTileOffset[] = {0, 1, 2, 1, 3, 2, 3, 3, 3};
				int xTileOffset[] = {60, 30, 90, 0, 60, 120, 30, 90, 60};
				int yTileOffset[] = {-30, -15, -15, 0, 0, 0, 15, 15, 30};
				for (int i = 0; i < 9; i++) {
					image_draw_isometric_footprint(graphicBase + graphicTileOffset[i],
						xOffset + xTileOffset[i], yOffset + yTileOffset[i], 0);
				}
			}
		}
	} else if (b->size == 4) {
		int graphicBase = image_group(GROUP_TERRAIN_OVERLAY) + graphicOffset;
		if (b->houseSize) {
			graphicBase += 4;
		}
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
			image_draw_isometric_footprint(graphicBase + graphicTileOffset[i],
				xOffset + xTileOffset[i], yOffset + yTileOffset[i], 0);
		}
	} else if (b->size == 5) {
		int drawOrig = 0;
		switch (game_state_overlay()) {
			case OVERLAY_ENTERTAINMENT:
				if (b->type == BUILDING_HIPPODROME || b->type == BUILDING_COLOSSEUM) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_COLOSSEUM:
				if (b->type == BUILDING_COLOSSEUM) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_HIPPODROME:
				if (b->type == BUILDING_HIPPODROME) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_TAX_INCOME:
				if (b->type == BUILDING_SENATE_UPGRADED) {
					drawOrig = 1;
				}
				break;
			case OVERLAY_PROBLEMS:
				if (b->showOnProblemOverlay) {
					drawOrig = 1;
				}
				break;
		}
		if (drawOrig) {
			DRAWFOOT_SIZE5(map_image_at(gridOffset), xOffset, yOffset);
		} else {
			int graphicBase = image_group(GROUP_TERRAIN_OVERLAY) + graphicOffset;
			if (b->houseSize) {
				graphicBase += 4;
			}
			int graphicTileOffset[] = {0, 1, 2, 1, 3, 2, 1, 3, 3, 2,
				1, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
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
				image_draw_isometric_footprint(graphicBase + graphicTileOffset[i],
					xOffset + xTileOffset[i], yOffset + yTileOffset[i], 0);
			}
		}
	}
}

static int terrain_on_desirability_overlay()
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER |
        TERRAIN_SCRUB | TERRAIN_GARDEN | TERRAIN_ROAD |
        TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE;
}

static void drawBuildingFootprintForDesirabilityOverlay(int gridOffset, int xOffset, int yOffset)
{
	if (map_terrain_is(gridOffset, terrain_on_desirability_overlay()) && !map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
		// display normal tile
		if (map_property_is_draw_tile(gridOffset)) {
			draw_foot_with_size(gridOffset, xOffset, yOffset);
		}
	} else if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
		// display empty land/grass
		int graphicId = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(gridOffset) & 7);
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	} else if (map_terrain_is(gridOffset, TERRAIN_BUILDING) || map_desirability_get(gridOffset)) {
		int des = map_desirability_get(gridOffset);
		int offset = 0;
		if (des < -10) {
			offset = 0;
		} else if (des < -5) {
			offset = 1;
		} else if (des < 0) {
			offset = 2;
		} else if (des == 1) {
			offset = 3;
		} else if (des < 5) {
			offset = 4;
		} else if (des < 10) {
			offset = 5;
		} else if (des < 15) {
			offset = 6;
		} else if (des < 20) {
			offset = 7;
		} else if (des < 25) {
			offset = 8;
		} else {
			offset = 9;
		}
		DRAWFOOT_SIZE1(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, xOffset, yOffset);
	} else {
		DRAWFOOT_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	}
}

static void drawBuildingTopForDesirabilityOverlay(int gridOffset, int xOffset, int yOffset)
{
	if (map_terrain_is(gridOffset, terrain_on_desirability_overlay()) && !map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
		// display normal tile
		if (map_property_is_draw_tile(gridOffset)) {
			draw_top_with_size(gridOffset, xOffset, yOffset);
		}
	} else if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
		// grass, no top needed
	} else if (map_terrain_is(gridOffset, TERRAIN_BUILDING) || map_desirability_get(gridOffset)) {
		int des = map_desirability_get(gridOffset);
		int offset;
		if (des < -10) {
			offset = 0;
		} else if (des < -5) {
			offset = 1;
		} else if (des < 0) {
			offset = 2;
		} else if (des == 1) {
			offset = 3;
		} else if (des < 5) {
			offset = 4;
		} else if (des < 10) {
			offset = 5;
		} else if (des < 15) {
			offset = 6;
		} else if (des < 20) {
			offset = 7;
		} else if (des < 25) {
			offset = 8;
		} else {
			offset = 9;
		}
		DRAWTOP_SIZE1(image_group(GROUP_TERRAIN_DESIRABILITY) + offset, xOffset, yOffset);
	} else {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	}
}

static void drawBuildingTopForFireOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_PREFECTURE) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->type == BUILDING_BURNING_RUIN) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->fireRisk > 0) {
		int draw = 1;
		if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_PIG_FARM) {
			draw = is_drawable_farm_corner(gridOffset, city_view_orientation());
		}
		if (draw) {
			drawOverlayColumn(b->fireRisk / 10, xOffset, yOffset, 1);
		}
	}
}

static void drawBuildingTopForDamageOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_ENGINEERS_POST) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->damageRisk > 0) {
		int draw = 1;
		if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_PIG_FARM) {
			draw = is_drawable_farm_corner(gridOffset, city_view_orientation());
		}
		if (draw) {
			drawOverlayColumn(b->damageRisk / 10, xOffset, yOffset, 1);
		}
	}
}

static void drawBuildingTopForCrimeOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_PREFECTURE) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->type == BUILDING_BURNING_RUIN) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize) {
		int happiness = b->sentiment.houseHappiness;
		if (happiness < 50) {
			int colVal;
			if (happiness <= 0) {
				colVal = 10;
			} else if (happiness <= 10) {
				colVal = 8;
			} else if (happiness <= 20) {
				colVal = 6;
			} else if (happiness <= 30) {
				colVal = 4;
			} else if (happiness <= 40) {
				colVal = 2;
			} else {
				colVal = 1;
			}
			drawOverlayColumn(colVal, xOffset, yOffset, 1);
		}
	}
}

static void drawBuildingTopForEntertainmentOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_THEATER:
			DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_ACTOR_COLONY:
		case BUILDING_GLADIATOR_SCHOOL:
		case BUILDING_LION_HOUSE:
		case BUILDING_CHARIOT_MAKER:
		case BUILDING_AMPHITHEATER:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_COLOSSEUM:
		case BUILDING_HIPPODROME:
			DRAWTOP_SIZE5(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize && b->data.house.entertainment) {
				drawOverlayColumn(b->data.house.entertainment / 10, xOffset, yOffset, 0);
			}
			break;
	}
}

static void drawBuildingTopForEducationOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_ACADEMY:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_LIBRARY:
		case BUILDING_SCHOOL:
			DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize && b->data.house.education) {
				drawOverlayColumn(b->data.house.education * 3 - 1,
					xOffset, yOffset, 0);
			}
			break;
	}
}

static void drawBuildingTopForTheaterOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_ACTOR_COLONY:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_THEATER:
			DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize && b->data.house.theater) {
				drawOverlayColumn(b->data.house.theater / 10, xOffset, yOffset, 0);
			}
			break;
	}
}

static void drawBuildingTopForAmphitheaterOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_ACTOR_COLONY:
		case BUILDING_GLADIATOR_SCHOOL:
		case BUILDING_AMPHITHEATER:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize && b->data.house.amphitheaterActor) {
				drawOverlayColumn(b->data.house.amphitheaterActor / 10, xOffset, yOffset, 0);
			}
			break;
	}
}

static void drawBuildingTopForColosseumOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_GLADIATOR_SCHOOL:
		case BUILDING_LION_HOUSE:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_COLOSSEUM:
			DRAWTOP_SIZE5(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize && b->data.house.colosseumGladiator) {
				drawOverlayColumn(b->data.house.colosseumGladiator / 10, xOffset, yOffset, 0);
			}
			break;
	}
}

static void drawBuildingTopForHippodromeOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_HIPPODROME) {
		DRAWTOP_SIZE5(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->type == BUILDING_CHARIOT_MAKER) {
		DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.hippodrome) {
		drawOverlayColumn(b->data.house.hippodrome / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForFoodStocksOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_MARKET:
		case BUILDING_WHARF:
			DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_GRANARY:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize) {
				if (model_get_house(b->subtype.houseLevel)->food_types) {
					int pop = b->housePopulation;
					int stocks = 0;
					for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
						stocks += b->data.house.inventory[i];
					}
					int pctStocks = calc_percentage(stocks, pop);
					int colVal = 0;
					if (pctStocks <= 0) {
						colVal = 10;
					} else if (pctStocks < 100) {
						colVal = 5;
					} else if (pctStocks <= 200) {
						colVal = 1;
					}
					if (colVal) {
						drawOverlayColumn(colVal, xOffset, yOffset, 1);
					}
				}
			}
			break;
	}
}

static void drawBuildingTopForBathhouseOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_BATHHOUSE) {
		DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.bathhouse) {
		drawOverlayColumn(b->data.house.bathhouse / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForReligionOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	switch (b->type) {
		case BUILDING_ORACLE:
		case BUILDING_SMALL_TEMPLE_CERES:
		case BUILDING_SMALL_TEMPLE_NEPTUNE:
		case BUILDING_SMALL_TEMPLE_MERCURY:
		case BUILDING_SMALL_TEMPLE_MARS:
		case BUILDING_SMALL_TEMPLE_VENUS:
			DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
			break;
		case BUILDING_LARGE_TEMPLE_CERES:
		case BUILDING_LARGE_TEMPLE_NEPTUNE:
		case BUILDING_LARGE_TEMPLE_MERCURY:
		case BUILDING_LARGE_TEMPLE_MARS:
		case BUILDING_LARGE_TEMPLE_VENUS:
			DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
			break;
		default:
			if (b->houseSize && b->data.house.numGods) {
				drawOverlayColumn(b->data.house.numGods * 17 / 10, xOffset, yOffset, 0);
			}
			break;
	}
}

static void drawBuildingTopForSchoolOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_SCHOOL) {
		DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.school) {
		drawOverlayColumn(b->data.house.school / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForLibraryOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_LIBRARY) {
		DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.library) {
		drawOverlayColumn(b->data.house.library / 10,xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForAcademyOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_ACADEMY) {
		DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.academy) {
		drawOverlayColumn(b->data.house.academy / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForBarberOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_BARBER) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.barber) {
		drawOverlayColumn(b->data.house.barber / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForClinicsOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_DOCTOR) {
		DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.clinic) {
		drawOverlayColumn(b->data.house.clinic / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForHospitalOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_HOSPITAL) {
		DRAWTOP_SIZE3(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize && b->data.house.hospital) {
		drawOverlayColumn(b->data.house.hospital / 10, xOffset, yOffset, 0);
	}
}

static void drawBuildingTopForTaxIncomeOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->type == BUILDING_SENATE_UPGRADED) {
		DRAWTOP_SIZE5(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->type == BUILDING_FORUM) {
		DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
	} else if (b->houseSize) {
		int pct = calc_adjust_with_percentage(b->taxIncomeOrStorage / 2, Data_CityInfo.taxPercentage);
		if (pct > 0) {
			drawOverlayColumn(pct / 25, xOffset, yOffset, 0);
		}
	}
}

static int is_problem_cartpusher(int figure_id)
{
    if (figure_id) {
        figure *fig = figure_get(figure_id);
        return fig->actionState == FIGURE_ACTION_20_CARTPUSHER_INITIAL && fig->minMaxSeen;
    } else {
        return 0;
    }
}

static void drawBuildingTopForProblemsOverlay(int gridOffset, building *b, int xOffset, int yOffset)
{
	if (b->houseSize) {
		return;
	}
	int type = b->type;
	if (type == BUILDING_FOUNTAIN || type == BUILDING_BATHHOUSE) {
		if (!b->hasWaterAccess) {
			b->showOnProblemOverlay = 1;
		}
	} else if (type >= BUILDING_WHEAT_FARM && type <= BUILDING_CLAY_PIT) {
		if (is_problem_cartpusher(b->figureId)) {
			b->showOnProblemOverlay = 1;
		}
	} else if (building_is_workshop(type)) {
		if (is_problem_cartpusher(b->figureId)) {
			b->showOnProblemOverlay = 1;
		} else if (b->loadsStored <= 0) {
			b->showOnProblemOverlay = 1;
		}
	}

	if (b->showOnProblemOverlay <= 0) {
		return;
	}

	if (type >= BUILDING_WHEAT_FARM && type <= BUILDING_PIG_FARM) {
		if (is_drawable_farmhouse(gridOffset, city_view_orientation())) {
			DRAWTOP_SIZE2(map_image_at(gridOffset), xOffset, yOffset);
		} else if (map_property_is_draw_tile(gridOffset)) {
            DRAWTOP_SIZE1(map_image_at(gridOffset), xOffset, yOffset);
		}
		return;
	}
	if (type == BUILDING_GRANARY) {
		const image *img = image_get(map_image_at(gridOffset));
		image_draw(image_group(GROUP_BUILDING_GRANARY) + 1,
			xOffset + img->sprite_offset_x,
			yOffset + img->sprite_offset_y - 30 -
			(img->height - 90));
		if (b->data.storage.resourceStored[RESOURCE_NONE] < 2400) {
			image_draw(image_group(GROUP_BUILDING_GRANARY) + 2,
				xOffset + 32, yOffset - 61);
			if (b->data.storage.resourceStored[RESOURCE_NONE] < 1800) {
				image_draw(image_group(GROUP_BUILDING_GRANARY) + 3,
					xOffset + 56, yOffset - 51);
			}
			if (b->data.storage.resourceStored[RESOURCE_NONE] < 1200) {
				image_draw(image_group(GROUP_BUILDING_GRANARY) + 4,
					xOffset + 91, yOffset - 51);
			}
			if (b->data.storage.resourceStored[RESOURCE_NONE] < 600) {
				image_draw(image_group(GROUP_BUILDING_GRANARY) + 5,
					xOffset + 118, yOffset - 61);
			}
		}
	}
	if (type == BUILDING_WAREHOUSE) {
		image_draw(image_group(GROUP_BUILDING_WAREHOUSE) + 17, xOffset - 4, yOffset - 42);
	}

	draw_top_with_size(gridOffset, xOffset, yOffset);
}

static void drawOverlayColumn(int height, int xOffset, int yOffset, int isRed)
{
	int graphicId = image_group(GROUP_OVERLAY_COLUMN);
	if (isRed) {
		graphicId += 9;
	}
	if (height > 10) {
		height = 10;
	}
	int capitalHeight = image_get(graphicId)->height;
	// draw base
	image_draw(graphicId + 2, xOffset + 9, yOffset - 8);
	if (height) {
		for (int i = 1; i < height; i++) {
			image_draw(graphicId + 1, xOffset + 17, yOffset - 8 - 10 * i + 13);
		}
		// top
		image_draw(graphicId,
			xOffset + 5, yOffset - 8 - capitalHeight - 10 * (height - 1) + 13);
	}
}

int UI_CityBuildings_getOverlayTooltipText(tooltip_context *c, int gridOffset)
{
    int overlay = game_state_overlay();
    int buildingId = map_building_at(gridOffset);
    if (overlay != OVERLAY_WATER && overlay != OVERLAY_DESIRABILITY && !buildingId) {
        return 0;
    }
    int overlayRequiresHouse =
        overlay != OVERLAY_WATER && overlay != OVERLAY_FIRE &&
        overlay != OVERLAY_DAMAGE && overlay != OVERLAY_NATIVE;
    int overlayForbidsHouse = overlay == OVERLAY_NATIVE;
    building *b = building_get(buildingId);
    if (overlayRequiresHouse && !b->houseSize) {
        return 0;
    }
    if (overlayForbidsHouse && b->houseSize) {
        return 0;
    }
    switch (overlay) {
        case OVERLAY_WATER:
            if (map_terrain_is(gridOffset, TERRAIN_RESERVOIR_RANGE)) {
                if (map_terrain_is(gridOffset, TERRAIN_FOUNTAIN_RANGE)) {
                    return 2;
                } else {
                    return 1;
                }
            } else if (map_terrain_is(gridOffset, TERRAIN_FOUNTAIN_RANGE)) {
                return 3;
            }
            break;
        case OVERLAY_RELIGION:
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
            break;
        case OVERLAY_FIRE:
            if (b->fireRisk <= 0) {
                return 46;
            } else if (b->fireRisk <= 20) {
                return 47;
            } else if (b->fireRisk <= 40) {
                return 48;
            } else if (b->fireRisk <= 60) {
                return 49;
            } else if (b->fireRisk <= 80) {
                return 50;
            } else {
                return 51;
            }
            break;
        case OVERLAY_DAMAGE:
            if (b->damageRisk <= 0) {
                return 52;
            } else if (b->damageRisk <= 40) {
                return 53;
            } else if (b->damageRisk <= 80) {
                return 54;
            } else if (b->damageRisk <= 120) {
                return 55;
            } else if (b->damageRisk <= 160) {
                return 56;
            } else {
                return 57;
            }
            break;
        case OVERLAY_CRIME:
            if (b->sentiment.houseHappiness <= 0) {
                return 63;
            } else if (b->sentiment.houseHappiness <= 10) {
                return 62;
            } else if (b->sentiment.houseHappiness <= 20) {
                return 61;
            } else if (b->sentiment.houseHappiness <= 30) {
                return 60;
            } else if (b->sentiment.houseHappiness < 50) {
                return 59;
            } else {
                return 58;
            }
            break;
        case OVERLAY_ENTERTAINMENT:
            if (b->data.house.entertainment <= 0) {
                return 64;
            } else if (b->data.house.entertainment < 10) {
                return 65;
            } else if (b->data.house.entertainment < 20) {
                return 66;
            } else if (b->data.house.entertainment < 30) {
                return 67;
            } else if (b->data.house.entertainment < 40) {
                return 68;
            } else if (b->data.house.entertainment < 50) {
                return 69;
            } else if (b->data.house.entertainment < 60) {
                return 70;
            } else if (b->data.house.entertainment < 70) {
                return 71;
            } else if (b->data.house.entertainment < 80) {
                return 72;
            } else if (b->data.house.entertainment < 90) {
                return 73;
            } else {
                return 74;
            }
            break;
        case OVERLAY_THEATER:
            if (b->data.house.theater <= 0) {
                return 75;
            } else if (b->data.house.theater >= 80) {
                return 76;
            } else if (b->data.house.theater >= 20) {
                return 77;
            } else {
                return 78;
            }
            break;
        case OVERLAY_AMPHITHEATER:
            if (b->data.house.amphitheaterActor <= 0) {
                return 79;
            } else if (b->data.house.amphitheaterActor >= 80) {
                return 80;
            } else if (b->data.house.amphitheaterActor >= 20) {
                return 81;
            } else {
                return 82;
            }
            break;
        case OVERLAY_COLOSSEUM:
            if (b->data.house.colosseumGladiator <= 0) {
                return 83;
            } else if (b->data.house.colosseumGladiator >= 80) {
                return 84;
            } else if (b->data.house.colosseumGladiator >= 20) {
                return 85;
            } else {
                return 86;
            }
            break;
        case OVERLAY_HIPPODROME:
            if (b->data.house.hippodrome <= 0) {
                return 87;
            } else if (b->data.house.hippodrome >= 80) {
                return 88;
            } else if (b->data.house.hippodrome >= 20) {
                return 89;
            } else {
                return 90;
            }
            break;
        case OVERLAY_EDUCATION:
            switch (b->data.house.education) {
                case 0: return 100;
                case 1: return 101;
                case 2: return 102;
                case 3: return 103;
            }
            break;
        case OVERLAY_SCHOOL:
            if (b->data.house.school <= 0) {
                return 19;
            } else if (b->data.house.school >= 80) {
                return 20;
            } else if (b->data.house.school >= 20) {
                return 21;
            } else {
                return 22;
            }
            break;
        case OVERLAY_LIBRARY:
            if (b->data.house.library <= 0) {
                return 23;
            } else if (b->data.house.library >= 80) {
                return 24;
            } else if (b->data.house.library >= 20) {
                return 25;
            } else {
                return 26;
            }
            break;
        case OVERLAY_ACADEMY:
            if (b->data.house.academy <= 0) {
                return 27;
            } else if (b->data.house.academy >= 80) {
                return 28;
            } else if (b->data.house.academy >= 20) {
                return 29;
            } else {
                return 30;
            }
            break;
        case OVERLAY_BARBER:
            if (b->data.house.barber <= 0) {
                return 31;
            } else if (b->data.house.barber >= 80) {
                return 32;
            } else if (b->data.house.barber < 20) {
                return 33;
            } else {
                return 34;
            }
            break;
        case OVERLAY_BATHHOUSE:
            if (b->data.house.bathhouse <= 0) {
                return 8;
            } else if (b->data.house.bathhouse >= 80) {
                return 9;
            } else if (b->data.house.bathhouse >= 20) {
                return 10;
            } else {
                return 11;
            }
            break;
        case OVERLAY_CLINIC:
            if (b->data.house.clinic <= 0) {
                return 35;
            } else if (b->data.house.clinic >= 80) {
                return 36;
            } else if (b->data.house.clinic >= 20) {
                return 37;
            } else {
                return 38;
            }
            break;
        case OVERLAY_HOSPITAL:
            if (b->data.house.hospital <= 0) {
                return 39;
            } else if (b->data.house.hospital >= 80) {
                return 40;
            } else if (b->data.house.hospital >= 20) {
                return 41;
            } else {
                return 42;
            }
            break;
        case OVERLAY_TAX_INCOME: {
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
            break;
        }
        case OVERLAY_FOOD_STOCKS:
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
            break;
        case OVERLAY_DESIRABILITY: {
            int desirability = map_desirability_get(gridOffset);
            if (desirability < 0) {
                return 91;
            } else if (desirability == 0) {
                return 92;
            } else {
                return 93;
            }
            break;
        }
    }
    return 0;
}
