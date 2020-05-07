#include "city_without_overlay.h"

#include "building/animation.h"
#include "building/construction.h"
#include "building/dock.h"
#include "building/type.h"
#include "city/buildings.h"
#include "city/entertainment.h"
#include "city/labor.h"
#include "city/population.h"
#include "city/ratings.h"
#include "city/view.h"
#include "core/time.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/sprite.h"
#include "map/terrain.h"
#include "sound/city.h"
#include "widget/city_bridge.h"
#include "widget/city_building_ghost.h"
#include "widget/city_figure.h"

static struct {
    time_millis last_water_animation_time;
    int advance_water_animation;

    int image_id_water_first;
    int image_id_water_last;
    int selected_figure_id;
    pixel_coordinate *selected_figure_coord;
} draw_context = {0, 0, 0, 0, 0, 0};

static void init_draw_context(int selected_figure_id, pixel_coordinate *figure_coord)
{
    draw_context.advance_water_animation = 0;
    if (!selected_figure_id) {
        time_millis now = time_get_millis();
        if (now - draw_context.last_water_animation_time > 60) {
            draw_context.last_water_animation_time = now;
            draw_context.advance_water_animation = 1;
        }
    }
    draw_context.image_id_water_first = image_group(GROUP_TERRAIN_WATER);
    draw_context.image_id_water_last = 5 + draw_context.image_id_water_first;
    draw_context.selected_figure_id = selected_figure_id;
    draw_context.selected_figure_coord = figure_coord;
}

static int draw_building_as_deleted(building *b)
{
    b = building_main(b);
    return (b->id && (b->is_deleted || map_property_is_deleted(b->grid_offset)));
}

static int is_multi_tile_terrain(int grid_offset)
{
    return (!map_building_at(grid_offset) && map_property_multi_tile_size(grid_offset) > 1);
}

static void draw_footprint(int x, int y, int grid_offset)
{
    building_construction_record_view_position(x, y, grid_offset);
    if (grid_offset < 0) {
        // Outside map: draw black tile
        image_draw_isometric_footprint_from_draw_tile(image_group(GROUP_TERRAIN_BLACK), x, y, 0);
    } else if (map_property_is_draw_tile(grid_offset)) {
        // Valid grid_offset and leftmost tile -> draw
        int building_id = map_building_at(grid_offset);
        color_t color_mask = 0;
        if (building_id) {
            building *b = building_get(building_id);
            if (draw_building_as_deleted(b)) {
                color_mask = COLOR_MASK_RED;
            }
            int view_x, view_y, view_width, view_height;
            city_view_get_scaled_viewport(&view_x, &view_y, &view_width, &view_height);
            if (x < view_x + 100) {
                sound_city_mark_building_view(b, SOUND_DIRECTION_LEFT);
            } else if (x > view_x + view_width - 100) {
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
        if (draw_context.advance_water_animation &&
            image_id >= draw_context.image_id_water_first &&
            image_id <= draw_context.image_id_water_last) {
            image_id++;
            if (image_id > draw_context.image_id_water_last) {
                image_id = draw_context.image_id_water_first;
            }
            map_image_set(grid_offset, image_id);
        }
        image_draw_isometric_footprint_from_draw_tile(image_id, x, y, color_mask);
    }
}

static void draw_hippodrome_spectators(const building *b, int x, int y, color_t color_mask)
{
    int subtype = b->subtype.orientation;
    int orientation = city_view_orientation();
    int population = city_population();
    if ((subtype == 0 || subtype == 3) && population > 2000) {
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
    } else if ((subtype == 1 || subtype == 4) && population > 100) {
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
    } else if ((subtype == 2 || subtype == 5) && population > 1000) {
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
    if (b->type == BUILDING_AMPHITHEATER && b->num_workers > 0) {
        image_draw_masked(image_group(GROUP_BUILDING_AMPHITHEATER_SHOW), x + 36, y - 47, color_mask);
    }
    if (b->type == BUILDING_THEATER && b->num_workers > 0) {
        image_draw_masked(image_group(GROUP_BUILDING_THEATER_SHOW), x + 34, y - 22, color_mask);
    }
    if (b->type == BUILDING_COLOSSEUM && b->num_workers > 0) {
        image_draw_masked(image_group(GROUP_BUILDING_COLOSSEUM_SHOW), x + 70, y - 90, color_mask);
    }
    if (b->type == BUILDING_HIPPODROME && building_main(b)->num_workers > 0 && city_entertainment_hippodrome_has_race()) {
        draw_hippodrome_spectators(b, x, y, color_mask);
    }
}

static void draw_workshop_raw_material_storage(const building *b, int x, int y, color_t color_mask)
{
    if (b->type == BUILDING_WINE_WORKSHOP) {
        if (b->loads_stored >= 2 || b->data.industry.has_raw_materials) {
            image_draw_masked(image_group(GROUP_BUILDING_WORKSHOP_RAW_MATERIAL), x + 45, y + 23, color_mask);
        }
    }
    if (b->type == BUILDING_OIL_WORKSHOP) {
        if (b->loads_stored >= 2 || b->data.industry.has_raw_materials) {
            image_draw_masked(image_group(GROUP_BUILDING_WORKSHOP_RAW_MATERIAL) + 1, x + 35, y + 15, color_mask);
        }
    }
    if (b->type == BUILDING_WEAPONS_WORKSHOP) {
        if (b->loads_stored >= 2 || b->data.industry.has_raw_materials) {
            image_draw_masked(image_group(GROUP_BUILDING_WORKSHOP_RAW_MATERIAL) + 3, x + 46, y + 24, color_mask);
        }
    }
    if (b->type == BUILDING_FURNITURE_WORKSHOP) {
        if (b->loads_stored >= 2 || b->data.industry.has_raw_materials) {
            image_draw_masked(image_group(GROUP_BUILDING_WORKSHOP_RAW_MATERIAL) + 2, x + 48, y + 19, color_mask);
        }
    }
    if (b->type == BUILDING_POTTERY_WORKSHOP) {
        if (b->loads_stored >= 2 || b->data.industry.has_raw_materials) {
            image_draw_masked(image_group(GROUP_BUILDING_WORKSHOP_RAW_MATERIAL) + 4, x + 47, y + 24, color_mask);
        }
    }
}

static void draw_senate_rating_flags(const building *b, int x, int y, color_t color_mask)
{
    if (b->type == BUILDING_SENATE_UPGRADED) {
        // rating flags
        int image_id = image_group(GROUP_BUILDING_SENATE);
        image_draw_masked(image_id + 1, x + 138, y + 44 - city_rating_culture() / 2, color_mask);
        image_draw_masked(image_id + 2, x + 168, y + 36 - city_rating_prosperity() / 2, color_mask);
        image_draw_masked(image_id + 3, x + 198, y + 27 - city_rating_peace() / 2, color_mask);
        image_draw_masked(image_id + 4, x + 228, y + 19 - city_rating_favor() / 2, color_mask);
        // unemployed
        image_id = image_group(GROUP_FIGURE_HOMELESS);
        int unemployment_pct = city_labor_unemployment_percentage_for_senate();
        if (unemployment_pct > 0) {
            image_draw_masked(image_id + 108, x + 80, y, color_mask);
        }
        if (unemployment_pct > 5) {
            image_draw_masked(image_id + 104, x + 230, y - 30, color_mask);
        }
        if (unemployment_pct > 10) {
            image_draw_masked(image_id + 107, x + 100, y + 20, color_mask);
        }
        if (unemployment_pct > 15) {
            image_draw_masked(image_id + 106, x + 235, y - 10, color_mask);
        }
        if (unemployment_pct > 20) {
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
    if (draw_building_as_deleted(b) || (map_property_is_deleted(grid_offset) && !is_multi_tile_terrain(grid_offset))) {
        color_mask = COLOR_MASK_RED;
    }
    image_draw_isometric_top_from_draw_tile(image_id, x, y, color_mask);
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
        if (!f->is_ghost) {
            if (!draw_context.selected_figure_id) {
                city_draw_figure(f, x, y);
            } else if (figure_id == draw_context.selected_figure_id) {
                city_draw_selected_figure(f, x, y, draw_context.selected_figure_coord);
            }
        }
        figure_id = f->next_figure_id_on_same_tile;
    }
}

static void draw_dock_workers(const building *b, int x, int y, color_t color_mask)
{
    int num_dockers = building_dock_count_idle_dockers(b);
    if (num_dockers > 0) {
        int image_dock = map_image_at(b->grid_offset);
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
    if (b->id == city_buildings_get_trade_center()) {
        image_draw_masked(image_group(GROUP_BUILDING_TRADE_CENTER_FLAG), x + 19, y - 56, color_mask);
    }
}

static void draw_granary_stores(const image *img, const building *b, int x, int y, color_t color_mask)
{
    image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 1,
                      x + img->sprite_offset_x,
                      y + 60 + img->sprite_offset_y - img->height,
                      color_mask);
    if (b->data.granary.resource_stored[RESOURCE_NONE] < 2400) {
        image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 2, x + 33, y - 60, color_mask);
    }
    if (b->data.granary.resource_stored[RESOURCE_NONE] < 1800) {
        image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 3, x + 56, y - 50, color_mask);
    }
    if (b->data.granary.resource_stored[RESOURCE_NONE] < 1200) {
        image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 4, x + 91, y - 50, color_mask);
    }
    if (b->data.granary.resource_stored[RESOURCE_NONE] < 600) {
        image_draw_masked(image_group(GROUP_BUILDING_GRANARY) + 5, x + 117, y - 62, color_mask);
    }
}

static void draw_animation(int x, int y, int grid_offset)
{
    int image_id = map_image_at(grid_offset);
    const image *img = image_get(image_id);
    if (img->num_animation_sprites) {
        if (map_property_is_draw_tile(grid_offset)) {
            int building_id = map_building_at(grid_offset);
            building *b = building_get(building_id);
            int color_mask = 0;
            if (draw_building_as_deleted(b) || map_property_is_deleted(grid_offset)) {
                color_mask = COLOR_MASK_RED;
            }
            if (b->type == BUILDING_DOCK) {
                draw_dock_workers(b, x, y, color_mask);
            } else if (b->type == BUILDING_WAREHOUSE) {
                draw_warehouse_ornaments(b, x, y, color_mask);
            } else if (b->type == BUILDING_GRANARY) {
                draw_granary_stores(img, b, x, y, color_mask);
            } else if (b->type == BUILDING_BURNING_RUIN && b->ruin_has_plague) {
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
        city_draw_bridge(x, y, grid_offset);
    } else if (building_get(map_building_at(grid_offset))->type == BUILDING_FORT) {
        if (map_property_is_draw_tile(grid_offset)) {
            building *fort = building_get(map_building_at(grid_offset));
            int offset = 0;
            switch (fort->subtype.fort_figure_type) {
                case FIGURE_FORT_LEGIONARY: offset = 4; break;
                case FIGURE_FORT_MOUNTED: offset = 3; break;
                case FIGURE_FORT_JAVELIN: offset = 2; break;
            }
            if (offset) {
                image_draw_masked(image_group(GROUP_BUILDING_FORT) + offset, x + 81, y + 5, draw_building_as_deleted(fort) ? COLOR_MASK_RED : 0);
            }
        }
    } else if (building_get(map_building_at(grid_offset))->type == BUILDING_GATEHOUSE) {
        int xy = map_property_multi_tile_xy(grid_offset);
        int orientation = city_view_orientation();
        if ((orientation == DIR_0_TOP && xy == EDGE_X1Y1) ||
            (orientation == DIR_2_RIGHT && xy == EDGE_X0Y1) ||
            (orientation == DIR_4_BOTTOM && xy == EDGE_X0Y0) ||
            (orientation == DIR_6_LEFT && xy == EDGE_X1Y0)) {
            building *gate = building_get(map_building_at(grid_offset));
            int image_id = image_group(GROUP_BULIDING_GATEHOUSE);
            int color_mask = draw_building_as_deleted(gate) ? COLOR_MASK_RED : 0;
            if (gate->subtype.orientation == 1) {
                if (orientation == DIR_0_TOP || orientation == DIR_4_BOTTOM) {
                    image_draw_masked(image_id, x - 22, y - 80, color_mask);
                } else {
                    image_draw_masked(image_id + 1, x - 18, y - 81, color_mask);
                }
            } else if (gate->subtype.orientation == 2) {
                if (orientation == DIR_0_TOP || orientation == DIR_4_BOTTOM) {
                    image_draw_masked(image_id + 1, x - 18, y - 81, color_mask);
                } else {
                    image_draw_masked(image_id, x - 22, y - 80, color_mask);
                }
            }
        }
    }
}

static void draw_elevated_figures(int x, int y, int grid_offset)
{
    int figure_id = map_figure_at(grid_offset);
    while (figure_id > 0) {
        figure *f = figure_get(figure_id);
        if ((f->use_cross_country && !f->is_ghost) || f->height_adjusted_ticks) {
            city_draw_figure(f, x, y);
        }
        figure_id = f->next_figure_id_on_same_tile;
    }
}

static void draw_hippodrome_ornaments(int x, int y, int grid_offset)
{
    int image_id = map_image_at(grid_offset);
    const image *img = image_get(image_id);
    building* b = building_get(map_building_at(grid_offset));
    if (img->num_animation_sprites
        && map_property_is_draw_tile(grid_offset)
        && b->type == BUILDING_HIPPODROME) {
        image_draw_masked(image_id + 1,
            x + img->sprite_offset_x, y + img->sprite_offset_y - img->height + 90,
            draw_building_as_deleted(b) ? COLOR_MASK_RED : 0
        );
    }
}

static void deletion_draw_terrain_top(int x, int y, int grid_offset)
{
    if (is_multi_tile_terrain(grid_offset)) {
        draw_top(x, y, grid_offset);
    }
}

static void deletion_draw_figures_animations(int x, int y, int grid_offset)
{
    if (map_property_is_deleted(grid_offset) && !map_building_at(grid_offset)) {
        image_draw_blend(image_group(GROUP_TERRAIN_FLAT_TILE), x, y, COLOR_MASK_RED);
    }
    if (!is_multi_tile_terrain(grid_offset)) {
        draw_top(x, y, grid_offset);
    }
    draw_figures(x, y, grid_offset);
    draw_animation(x, y, grid_offset);
}

static void deletion_draw_remaining(int x, int y, int grid_offset)
{
    draw_elevated_figures(x, y, grid_offset);
    draw_hippodrome_ornaments(x, y, grid_offset);
}

void city_without_overlay_draw(int selected_figure_id, pixel_coordinate *figure_coord, const map_tile *tile)
{
    init_draw_context(selected_figure_id, figure_coord);
    int should_mark_deleting = city_building_ghost_mark_deleting(tile);
    city_view_foreach_map_tile(draw_footprint);
    if (!should_mark_deleting) {
        city_view_foreach_valid_map_tile(
            draw_top,
            draw_figures,
            draw_animation
        );
        if (!selected_figure_id) {
            city_building_ghost_draw(tile);
        }
        city_view_foreach_valid_map_tile(
            draw_elevated_figures,
            draw_hippodrome_ornaments,
            0
        );
    } else {
        city_view_foreach_map_tile(deletion_draw_terrain_top);
        city_view_foreach_map_tile(deletion_draw_figures_animations);
        city_view_foreach_map_tile(deletion_draw_remaining);
    }
}
