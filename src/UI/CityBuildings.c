#include "CityBuildings_private.h"

#include "building/animation.h"
#include "building/construction.h"
#include "building/dock.h"
#include "city/finance.h"
#include "city/view.h"
#include "city/warning.h"
#include "figure/formation_legion.h"
#include "game/resource.h"
#include "game/settings.h"
#include "game/state.h"
#include "graphics/graphics.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/scroll.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/sprite.h"
#include "map/terrain.h"
#include "sound/city.h"
#include "sound/speech.h"
#include "sound/effect.h"
#include "widget/city_figure.h"
#include "widget/minimap.h"
#include "window/building_info.h"
#include "window/city.h"

static void drawBuildingFootprints();
static void drawBuildingTopsFiguresAnimation(void);
static void drawHippodromeAndElevatedFigures(void);

static time_millis lastWaterAnimationTime = 0;
static int advanceWaterAnimation;

static struct {
    int image_id_water_first;
    int image_id_water_last;
    int selected_figure_id;
    struct UI_CityPixelCoordinate *selected_figure_coord;
} draw_context;

static void init_draw_context(int selected_figure_id, struct UI_CityPixelCoordinate *figure_coord)
{
    draw_context.image_id_water_first = image_group(GROUP_TERRAIN_WATER);
    draw_context.image_id_water_last = 5 + draw_context.image_id_water_first;
    draw_context.selected_figure_id = selected_figure_id;
    draw_context.selected_figure_coord = figure_coord;
}

void UI_CityBuildings_drawForeground(int x, int y)
{
	Data_CityView.xInTiles = x;
	Data_CityView.yInTiles = y;
	graphics_set_clip_rectangle(
		Data_CityView.xOffsetInPixels, Data_CityView.yOffsetInPixels,
		Data_CityView.widthInPixels, Data_CityView.heightInPixels);

	advanceWaterAnimation = 0;
	time_millis now = time_get_millis();
	if (now - lastWaterAnimationTime > 60 || now < lastWaterAnimationTime) {
		lastWaterAnimationTime = now;
		advanceWaterAnimation = 1;
	}

	if (game_state_overlay()) {
		UI_CityBuildings_drawOverlayFootprints();
		UI_CityBuildings_drawOverlayTopsFiguresAnimation();
		UI_CityBuildings_drawSelectedBuildingGhost();
		UI_CityBuildings_drawOverlayElevatedFigures();
	} else {
        init_draw_context(0, 0);
		drawBuildingFootprints();
		drawBuildingTopsFiguresAnimation();
		UI_CityBuildings_drawSelectedBuildingGhost();
		drawHippodromeAndElevatedFigures();
	}

	graphics_reset_clip_rectangle();
}

void UI_CityBuildings_drawBuildingCost()
{
	if (!building_construction_in_progress()) {
		return;
	}
	if (scroll_in_progress()) {
		return;
	}
	int cost = building_construction_cost();
	if (!cost) {
		return;
	}
	graphics_set_clip_rectangle(
		Data_CityView.xOffsetInPixels, Data_CityView.yOffsetInPixels,
		Data_CityView.widthInPixels, Data_CityView.heightInPixels);
	color_t color;
	if (cost <= city_finance_treasury()) {
		color = COLOR_ORANGE;
	} else {
		color = COLOR_RED;
	}
	text_draw_number_colored(cost, '@', " ",
		Data_CityView.selectedTile.xOffsetInPixels + 58 + 1,
		Data_CityView.selectedTile.yOffsetInPixels + 1, FONT_NORMAL_PLAIN, COLOR_BLACK);
	text_draw_number_colored(cost, '@', " ",
		Data_CityView.selectedTile.xOffsetInPixels + 58,
		Data_CityView.selectedTile.yOffsetInPixels, FONT_NORMAL_PLAIN, color);
	graphics_reset_clip_rectangle();
}

void UI_CityBuildings_drawForegroundForFigure(int x, int y, int figureId, struct UI_CityPixelCoordinate *coord)
{
	Data_CityView.xInTiles = x;
	Data_CityView.yInTiles = y;
	graphics_set_clip_rectangle(
		Data_CityView.xOffsetInPixels, Data_CityView.yOffsetInPixels,
		Data_CityView.widthInPixels, Data_CityView.heightInPixels);

    init_draw_context(figureId, coord);
	drawBuildingFootprints();
	drawBuildingTopsFiguresAnimation();
	drawHippodromeAndElevatedFigures();

	graphics_reset_clip_rectangle();
}

static void draw_footprint(int x, int y, int grid_offset)
{
    if (grid_offset == Data_State.selectedBuilding.gridOffsetStart) {
        Data_State.selectedBuilding.reservoirOffsetX = x;
        Data_State.selectedBuilding.reservoirOffsetY = y;
    }
    if (grid_offset < 0) {
        // Outside map: draw black tile
        image_draw_isometric_footprint(image_group(GROUP_TERRAIN_BLACK), x, y, 0);
    } else if (map_property_is_draw_tile(grid_offset)) {
        // Valid gridOffset and leftmost tile -> draw
        int buildingId = map_building_at(grid_offset);
        color_t colorMask = 0;
        if (buildingId) {
            building *b = building_get(buildingId);
            if (b->isDeleted) {
                colorMask = COLOR_MASK_RED;
            }
            if (x < 4) {
                sound_city_mark_building_view(b, SOUND_DIRECTION_LEFT);
            } else if (x > Data_CityView.widthInTiles + 2) {
                sound_city_mark_building_view(b, SOUND_DIRECTION_RIGHT);
            } else {
                sound_city_mark_building_view(b, SOUND_DIRECTION_CENTER);
            }
        }
        if (map_terrain_is(grid_offset, TERRAIN_GARDEN)) {
            building *b = building_get(0); // abuse empty building
            b->type = BUILDING_GARDENS;
            sound_city_mark_building_view(b, SOUND_DIRECTION_CENTER);
        }
        int image_id = map_image_at(grid_offset);
        if (map_property_is_constructing(grid_offset)) {
            image_id = image_group(GROUP_TERRAIN_OVERLAY);
        }
        switch (map_property_multi_tile_size(grid_offset)) {
            case 1:
                if (advanceWaterAnimation &&
                    image_id >= draw_context.image_id_water_first &&
                    image_id <= draw_context.image_id_water_last) {
                    image_id++;
                    if (image_id > draw_context.image_id_water_last) {
                        image_id = draw_context.image_id_water_first;
                    }
                    map_image_set(grid_offset, image_id);
                }
                image_draw_isometric_footprint(image_id, x, y, colorMask);
                break;
            case 2:
                image_draw_isometric_footprint(image_id, x + 30, y - 15, colorMask);
                break;
            case 3:
                image_draw_isometric_footprint(image_id, x + 60, y - 30, colorMask);
                break;
            case 4:
                image_draw_isometric_footprint(image_id, x + 90, y - 45, colorMask);
                break;
            case 5:
                image_draw_isometric_footprint(image_id, x + 120, y - 60, colorMask);
                break;
        }
    }
}
static void drawBuildingFootprints()
{
    city_view_foreach_map_tile(draw_footprint);
}

static void draw_hippodrome_spectators(const building *b, int x, int y, color_t color_mask)
{
    int subtype = b->subtype.orientation;
    int orientation = city_view_orientation();
    if ((subtype == 0 || subtype == 3) && Data_CityInfo.population > 2000) {
        // first building part
        switch (orientation) {
            case DIR_0_TOP:
                image_draw_masked(image_group(GROUP_BUILDING_HIPPODROME_2) + 6, x + 147, y - 72, color_mask);
                break;
            case DIR_2_RIGHT:
                image_draw_masked(image_group(GROUP_BUILDING_HIPPODROME_1) + 8, x + 58, y - 79, color_mask);
                break;
            case DIR_4_BOTTOM:
                image_draw_masked(image_group(GROUP_BUILDING_HIPPODROME_2) + 8, x + 119, y - 80, color_mask);
                break;
            case DIR_6_LEFT:
                image_draw_masked(image_group(GROUP_BUILDING_HIPPODROME_1) + 6, x, y - 72, color_mask);
        }
    } else if ((subtype == 1 || subtype == 4) && Data_CityInfo.population > 100) {
        // middle building part
        switch (orientation) {
            case DIR_0_TOP:
            case DIR_4_BOTTOM:
                image_draw_masked(image_group(GROUP_BUILDING_HIPPODROME_2) + 7, x + 122, y - 79, color_mask);
                break;
            case DIR_2_RIGHT:
            case DIR_6_LEFT:
                image_draw_masked(image_group(GROUP_BUILDING_HIPPODROME_1) + 7, x, y - 80, color_mask);
        }
    } else if ((subtype == 2 || subtype == 5) && Data_CityInfo.population > 1000) {
        // last building part
        switch (orientation) {
            case DIR_0_TOP:
                image_draw_masked(image_group(GROUP_BUILDING_HIPPODROME_2) + 8, x + 119, y - 80, color_mask);
                break;
            case DIR_2_RIGHT:
                image_draw_masked(image_group(GROUP_BUILDING_HIPPODROME_1) + 6, x, y - 72, color_mask);
                break;
            case DIR_4_BOTTOM:
                image_draw_masked(image_group(GROUP_BUILDING_HIPPODROME_2) + 6, x + 147, y - 72, color_mask);
                break;
            case DIR_6_LEFT:
                image_draw_masked(image_group(GROUP_BUILDING_HIPPODROME_1) + 8, x + 58, y - 79, color_mask);
                break;
        }
    }
}

static void draw_entertainment_spectators(building *b, int x, int y, color_t color_mask)
{
    if (b->type == BUILDING_AMPHITHEATER && b->numWorkers > 0) {
        image_draw_masked(image_group(GROUP_BUILDING_AMPHITHEATER_SHOW), x + 36, y - 47, color_mask);
    }
    if (b->type == BUILDING_THEATER && b->numWorkers > 0) {
        image_draw_masked(image_group(GROUP_BUILDING_THEATER_SHOW), x + 34, y - 22, color_mask);
    }
    if (b->type == BUILDING_COLOSSEUM && b->numWorkers > 0) {
        image_draw_masked(image_group(GROUP_BUILDING_COLOSSEUM_SHOW), x + 70, y - 90, color_mask);
    }
    if (b->type == BUILDING_HIPPODROME && building_main(b)->numWorkers > 0 &&
        Data_CityInfo.entertainmentHippodromeHasShow) {
        draw_hippodrome_spectators(b, x, y, color_mask);
    }
}

static void draw_workshop_raw_material_storage(const building *b, int x, int y, color_t color_mask)
{
    if (b->type == BUILDING_WINE_WORKSHOP) {
        if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
            image_draw_masked(image_group(GROUP_BUILDING_WORKSHOP_RAW_MATERIAL), x + 45, y + 23, color_mask);
        }
    }
    if (b->type == BUILDING_OIL_WORKSHOP) {
        if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
            image_draw_masked(image_group(GROUP_BUILDING_WORKSHOP_RAW_MATERIAL) + 1, x + 35, y + 15, color_mask);
        }
    }
    if (b->type == BUILDING_WEAPONS_WORKSHOP) {
        if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
            image_draw_masked(image_group(GROUP_BUILDING_WORKSHOP_RAW_MATERIAL) + 3, x + 46, y + 24, color_mask);
        }
    }
    if (b->type == BUILDING_FURNITURE_WORKSHOP) {
        if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
            image_draw_masked(image_group(GROUP_BUILDING_WORKSHOP_RAW_MATERIAL) + 2, x + 48, y + 19, color_mask);
        }
    }
    if (b->type == BUILDING_POTTERY_WORKSHOP) {
        if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
            image_draw_masked(image_group(GROUP_BUILDING_WORKSHOP_RAW_MATERIAL) + 4, x + 47, y + 24, color_mask);
        }
    }
}

static void draw_senate_rating_flags(const building *b, int x, int y, color_t color_mask)
{
    if (b->type == BUILDING_SENATE_UPGRADED) {
        // rating flags
        int image_id = image_group(GROUP_BUILDING_SENATE);
        image_draw_masked(image_id + 1, x + 138, y + 44 - Data_CityInfo.ratingCulture / 2, color_mask);
        image_draw_masked(image_id + 2, x + 168, y + 36 - Data_CityInfo.ratingProsperity / 2, color_mask);
        image_draw_masked(image_id + 3, x + 198, y + 27 - Data_CityInfo.ratingPeace / 2, color_mask);
        image_draw_masked(image_id + 4, x + 228, y + 19 - Data_CityInfo.ratingFavor / 2, color_mask);
        // unemployed
        image_id = image_group(GROUP_FIGURE_HOMELESS);
        if (Data_CityInfo.unemploymentPercentageForSenate > 0) {
            image_draw_masked(image_id + 108, x + 80, y, color_mask);
        }
        if (Data_CityInfo.unemploymentPercentageForSenate > 5) {
            image_draw_masked(image_id + 104, x + 230, y - 30, color_mask);
        }
        if (Data_CityInfo.unemploymentPercentageForSenate > 10) {
            image_draw_masked(image_id + 107, x + 100, y + 20, color_mask);
        }
        if (Data_CityInfo.unemploymentPercentageForSenate > 15) {
            image_draw_masked(image_id + 106, x + 235, y - 10, color_mask);
        }
        if (Data_CityInfo.unemploymentPercentageForSenate > 20) {
            image_draw_masked(image_id + 106, x + 66, y + 20, color_mask);
        }
    }
}

static void draw_top(int x, int y, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }
    building *b = building_get(map_building_at(grid_offset));
    int image_id = map_image_at(grid_offset);
    color_t color_mask = 0;
    if (b->id && b->isDeleted) {
        color_mask = COLOR_MASK_RED;
    }
    switch (map_property_multi_tile_size(grid_offset)) {
        case 1: DRAWTOP_SIZE1_C(image_id, x, y, color_mask); break;
        case 2: DRAWTOP_SIZE2_C(image_id, x, y, color_mask); break;
        case 3: DRAWTOP_SIZE3_C(image_id, x, y, color_mask); break;
        case 4: DRAWTOP_SIZE4_C(image_id, x, y, color_mask); break;
        case 5: DRAWTOP_SIZE5_C(image_id, x, y, color_mask); break;
    }
    // specific buildings
    draw_senate_rating_flags(b, x, y, color_mask);
    draw_entertainment_spectators(b, x, y, color_mask);
    draw_workshop_raw_material_storage(b, x, y, color_mask);
}

static void draw_figures(int x, int y, int grid_offset)
{
    int figure_id = map_figure_at(grid_offset);
    while (figure_id) {
        figure *f = figure_get(figure_id);
        if (!f->isGhost) {
            if (!draw_context.selected_figure_id) {
                city_draw_figure(f, x, y);
            } else if (figure_id == draw_context.selected_figure_id) {
                city_draw_selected_figure(f, x, y, draw_context.selected_figure_coord);
            }
        }
        figure_id = f->nextFigureIdOnSameTile;
    }
}

static void draw_dock_workers(const building *b, int x, int y, color_t color_mask)
{
    int num_dockers = building_dock_count_idle_dockers(b);
    if (num_dockers > 0) {
        int image_dock = map_image_at(b->gridOffset);
        int image_dockers = image_group(GROUP_BUILDING_DOCK_DOCKERS);
        if (image_dock == image_group(GROUP_BUILDING_DOCK_1)) {
            image_dockers += 0;
        } else if (image_dock == image_group(GROUP_BUILDING_DOCK_2)) {
            image_dockers += 3;
        } else if (image_dock == image_group(GROUP_BUILDING_DOCK_3)) {
            image_dockers += 6;
        } else {
            image_dockers += 9;
        }
        if (num_dockers == 2) {
            image_dockers += 1;
        } else if (num_dockers == 3) {
            image_dockers += 2;
        }
        const image *img = image_get(image_dockers);
        image_draw_masked(image_dockers, x + img->sprite_offset_x, y + img->sprite_offset_y, color_mask);
    }
}

static void draw_warehouse_ornaments(const building *b, int x, int y, color_t color_mask)
{
    image_draw_masked(image_group(GROUP_BUILDING_WAREHOUSE) + 17, x - 4, y - 42, color_mask);
    if (b->id == Data_CityInfo.buildingTradeCenterBuildingId) {
        image_draw_masked(image_group(GROUP_BUILDING_TRADE_CENTER_FLAG), x + 19, y - 56, color_mask);
    }
}

static void draw_granary_stores(const image *img, const building *b, int x, int y, color_t color_mask)
{
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
}

static void draw_animation(int x, int y, int grid_offset)
{
    int image_id = map_image_at(grid_offset);
    const image *img = image_get(image_id);
    if (img->num_animation_sprites) {
        if (map_property_is_draw_tile(grid_offset)) {
            int buildingId = map_building_at(grid_offset);
            building *b = building_get(buildingId);
            int color_mask = 0;
            if (buildingId && b->isDeleted) {
                color_mask = COLOR_MASK_RED;
            }
            if (b->type == BUILDING_DOCK) {
                draw_dock_workers(b, x, y, color_mask);
            } else if (b->type == BUILDING_WAREHOUSE) {
                draw_warehouse_ornaments(b, x, y, color_mask);
            } else if (b->type == BUILDING_GRANARY) {
                draw_granary_stores(img, b, x, y, color_mask);
            } else if (b->type == BUILDING_BURNING_RUIN && b->ruinHasPlague) {
                image_draw_masked(image_group(GROUP_PLAGUE_SKULL), x + 18, y - 32, color_mask);
            }
            int animation_offset = building_animation_offset(b, image_id, grid_offset);
            if (b->type != BUILDING_HIPPODROME && animation_offset > 0) {
                if (animation_offset > img->num_animation_sprites) {
                    animation_offset = img->num_animation_sprites;
                }
                if (b->type == BUILDING_GRANARY) {
                    image_draw_masked(image_id + animation_offset + 5, x + 77, y - 49, color_mask);
                } else {
                    int ydiff = 0;
                    switch (map_property_multi_tile_size(grid_offset)) {
                        case 1: ydiff = 30; break;
                        case 2: ydiff = 45; break;
                        case 3: ydiff = 60; break;
                        case 4: ydiff = 75; break;
                        case 5: ydiff = 90; break;
                    }
                    image_draw_masked(image_id + animation_offset,
                                      x + img->sprite_offset_x,
                                      y + ydiff + img->sprite_offset_y - img->height,
                                      color_mask);
                }
            }
        }
    } else if (map_sprite_bridge_at(grid_offset)) {
        UI_CityBuildings_drawBridge(grid_offset, x, y);
    } else if (building_get(map_building_at(grid_offset))->type == BUILDING_FORT) {
        if (map_property_is_draw_tile(grid_offset)) {
            building *fort = building_get(map_building_at(grid_offset));
            int offset = 0;
            switch (fort->subtype.fortFigureType) {
                case FIGURE_FORT_LEGIONARY: offset = 4; break;
                case FIGURE_FORT_MOUNTED: offset = 3; break;
                case FIGURE_FORT_JAVELIN: offset = 2; break;
            }
            if (offset) {
                image_draw(image_group(GROUP_BUILDING_FORT) + offset, x + 81, y + 5);
            }
        }
    } else if (building_get(map_building_at(grid_offset))->type == BUILDING_GATEHOUSE) {
        int xy = map_property_multi_tile_xy(grid_offset);
        int orientation = city_view_orientation();
        if ((orientation == DIR_0_TOP && xy == Edge_X1Y1) ||
            (orientation == DIR_2_RIGHT && xy == Edge_X0Y1) ||
            (orientation == DIR_4_BOTTOM && xy == Edge_X0Y0) ||
            (orientation == DIR_6_LEFT && xy == Edge_X1Y0)) {
            building *gate = building_get(map_building_at(grid_offset));
            int image_id = image_group(GROUP_BULIDING_GATEHOUSE);
            if (gate->subtype.orientation == 1) {
                if (orientation == DIR_0_TOP || orientation == DIR_4_BOTTOM) {
                    image_draw(image_id, x - 22, y - 80);
                } else {
                    image_draw(image_id + 1, x - 18, y - 81);
                }
            } else if (gate->subtype.orientation == 2) {
                if (orientation == DIR_0_TOP || orientation == DIR_4_BOTTOM) {
                    image_draw(image_id + 1, x - 18, y - 81);
                } else {
                    image_draw(image_id, x - 22, y - 80);
                }
            }
        }
    }
}

static void drawBuildingTopsFiguresAnimation()
{
    city_view_foreach_valid_map_tile(
        draw_top,
        draw_figures,
        draw_animation
    );
}

void UI_CityBuildings_drawBridge(int grid_offset, int x, int y)
{
	if (!map_terrain_is(grid_offset, TERRAIN_WATER)) {
		map_sprite_clear_tile(grid_offset);
		return;
	}
	if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
		return;
	}
	color_t color_mask = 0;
	if (map_property_is_deleted(grid_offset)) {
		color_mask = COLOR_MASK_RED;
	}
	int image_id = image_group(GROUP_BUILDING_BRIDGE);
	switch (map_sprite_bridge_at(grid_offset)) {
		case 1:
			image_draw_masked(image_id + 5, x, y - 20, color_mask);
			break;
		case 2:
			image_draw_masked(image_id, x - 1, y - 8, color_mask);
			break;
		case 3:
			image_draw_masked(image_id + 3, x, y - 8, color_mask);
			break;
		case 4:
			image_draw_masked(image_id + 2, x + 7, y - 20, color_mask);
			break;
		case 5:
			image_draw_masked(image_id + 4, x , y - 21, color_mask);
			break;
		case 6:
			image_draw_masked(image_id + 1, x + 5, y - 21, color_mask);
			break;
		case 7:
			image_draw_masked(image_id + 11, x - 3, y - 50, color_mask);
			break;
		case 8:
			image_draw_masked(image_id + 6, x - 1, y - 12, color_mask);
			break;
		case 9:
			image_draw_masked(image_id + 9, x - 30, y - 12, color_mask);
			break;
		case 10:
			image_draw_masked(image_id + 8, x - 23, y - 53, color_mask);
			break;
		case 11:
			image_draw_masked(image_id + 10, x, y - 37, color_mask);
			break;
		case 12:
			image_draw_masked(image_id + 7, x + 7, y - 38, color_mask);
			break;
			// Note: no nr 13
		case 14:
			image_draw_masked(image_id + 13, x, y - 38, color_mask);
			break;
		case 15:
			image_draw_masked(image_id + 12, x + 7, y - 38, color_mask);
			break;
	}
}

static void draw_elevated_figures(int x, int y, int grid_offset)
{
    int figure_id = map_figure_at(grid_offset);
    while (figure_id > 0) {
        figure *f = figure_get(figure_id);
        if ((f->useCrossCountry && !f->isGhost) || f->heightAdjustedTicks) {
            city_draw_figure(f, x, y);
        }
        figure_id = f->nextFigureIdOnSameTile;
    }
}

static void draw_hippodrome_ornaments(int x, int y, int grid_offset)
{
    int image_id = map_image_at(grid_offset);
    const image *img = image_get(image_id);
    if (img->num_animation_sprites
        && map_property_is_draw_tile(grid_offset)
        && building_get(map_building_at(grid_offset))->type == BUILDING_HIPPODROME) {
        image_draw(image_id + 1, x + img->sprite_offset_x, y + img->sprite_offset_y - img->height + 90);
    }
}

static void drawHippodromeAndElevatedFigures(void)
{
    city_view_foreach_valid_map_tile(
        draw_elevated_figures,
        draw_hippodrome_ornaments,
        0
    );
}

// MOUSE HANDLING

static void updateCityViewCoords(const mouse *m)
{
	Data_State.map.current.x = Data_State.map.current.y = 0;
	int gridOffset = Data_State.map.current.gridOffset =
		city_view_pixels_to_grid_offset(m->x, m->y);
	if (gridOffset) {
		Data_State.map.current.x = (gridOffset - Data_State.map.gridStartOffset) % 162;
		Data_State.map.current.y = (gridOffset - Data_State.map.gridStartOffset) / 162;
	}
}

static int handleRightClickAllowBuildingInfo()
{
	int allow = 1;
	if (!window_is(WINDOW_CITY)) {
		allow = 0;
	}
	if (building_construction_type()) {
		allow = 0;
	}
	building_construction_reset(BUILDING_NONE);
	window_city_show();

	if (!Data_State.map.current.gridOffset) {
		allow = 0;
	}
	if (allow && city_has_warnings()) {
		city_warning_clear_all();
		allow = 0;
	}
	return allow;
}

static int isLegionClick()
{
	if (Data_State.map.current.gridOffset) {
		int formationId = formation_legion_at_grid_offset(
			map_grid_offset(Data_State.map.current.x, Data_State.map.current.y));
		if (formationId > 0 && !formation_get(formationId)->in_distant_battle) {
			Data_State.selectedLegionFormationId = formationId;
			window_city_military_show();
			return 1;
		}
	}
	return 0;
}

static void buildStart()
{
	if (Data_State.map.current.gridOffset /*&& !Data_State.gamePaused*/) { // TODO FIXME
		Data_State.selectedBuilding.gridOffsetStart = Data_State.map.current.gridOffset;
		building_construction_start(Data_State.map.current.x, Data_State.map.current.y);
	}
}

static void buildMove()
{
	if (!building_construction_in_progress() ||
		!Data_State.map.current.gridOffset) {
		return;
	}
	Data_State.selectedBuilding.gridOffsetEnd = Data_State.map.current.gridOffset;
	building_construction_update(Data_State.map.current.x, Data_State.map.current.y);
}

static void buildEnd()
{
	if (building_construction_in_progress()) {
		if (!Data_State.map.current.gridOffset) {
			Data_State.map.current.gridOffset = Data_State.selectedBuilding.gridOffsetEnd;
		}
		if (building_construction_type() != BUILDING_NONE) {
			sound_effect_play(SOUND_EFFECT_BUILD);
		}
		building_construction_place();
	}
}

static void scroll_map(int direction)
{
    if (city_view_scroll(direction)) {
        sound_city_decay_views();
    }
}

void UI_CityBuildings_handleMouse(const mouse *m)
{
	scroll_map(scroll_get_direction(m));
	updateCityViewCoords(m);
	Data_State.selectedBuilding.drawAsConstructing = 0;
	if (m->left.went_down) {
		if (!isLegionClick()) {
			buildStart();
			buildMove();
		}
	} else if (m->left.is_down) {
		buildMove();
	} else if (m->left.went_up) {
		buildEnd();
	} else if (m->right.went_up) {
		if (handleRightClickAllowBuildingInfo()) {
			window_building_info_show(Data_State.map.current.gridOffset);
		}
	}
}

void UI_CityBuildings_getTooltip(tooltip_context *c)
{
	if (setting_tooltips() == TOOLTIPS_NONE) {
		return;
	}
	if (!window_is(WINDOW_CITY)) {
		return;
	}
	if (Data_State.map.current.gridOffset == 0) {
		return;
	}
	int gridOffset = Data_State.map.current.gridOffset;
	int buildingId = map_building_at(gridOffset);
	int overlay = game_state_overlay();
	// regular tooltips
	if (overlay == OVERLAY_NONE && buildingId && building_get(buildingId)->type == BUILDING_SENATE_UPGRADED) {
		c->type = TOOLTIP_SENATE;
		c->high_priority = 1;
		return;
	}
	// overlay tooltips
	c->text_group = 66;
	c->text_id = UI_CityBuildings_getOverlayTooltipText(c, gridOffset);
    if (c->text_id) {
        c->type = TOOLTIP_OVERLAY;
        c->high_priority = 1;
    }
}

static void militaryMapClick()
{
	if (!Data_State.map.current.gridOffset) {
		window_city_show();
		return;
	}
	int formationId = Data_State.selectedLegionFormationId;
    formation *m = formation_get(formationId);
	if (m->in_distant_battle || m->cursed_by_mars) {
		return;
	}
	int otherFormationId = formation_legion_at_building(
		map_grid_offset(Data_State.map.current.x, Data_State.map.current.y));
	if (otherFormationId && otherFormationId == formationId) {
		formation_legion_return_home(m);
	} else {
		formation_legion_move_to(m, Data_State.map.current.x, Data_State.map.current.y);
		sound_speech_play_file("wavs/cohort5.wav");
	}
	window_city_show();
}

void UI_CityBuildings_handleMouseMilitary(const mouse *m)
{
	updateCityViewCoords(m);
	if (!city_view_is_sidebar_collapsed() && widget_minimap_handle_mouse(m)) {
		return;
	}
	scroll_map(scroll_get_direction(m));
	if (m->right.went_up) {
		city_warning_clear_all();
		window_city_show();
	} else {
		updateCityViewCoords(m);
		if (m->left.went_down) {
			militaryMapClick();
		}
	}
}
