#include "city_figure.h"

#include "city/view.h"
#include "figure/formation.h"
#include "figure/image.h"
#include "graphics/image.h"

static void draw_figure_with_cart(const figure *f, int x, int y)
{
    if (f->yOffsetCart >= 0) {
        image_draw(f->graphicId, x, y);
        image_draw(f->cartGraphicId, x + f->xOffsetCart, y + f->yOffsetCart);
    } else {
        image_draw(f->cartGraphicId, x + f->xOffsetCart, y + f->yOffsetCart);
        image_draw(f->graphicId, x, y);
    }
}

static void draw_hippodrome_horse(const figure *f, int x, int y)
{
    int val = f->waitTicksMissile;
    switch (city_view_orientation()) {
        case DIR_0_TOP:
            x += 10;
            if (val <= 10) {
                y -= 2;
            } else if (val <= 11) {
                y -= 10;
            } else if (val <= 12) {
                y -= 18;
            } else if (val <= 13) {
                y -= 16;
            } else if (val <= 20) {
                y -= 14;
            } else if (val <= 21) {
                y -= 10;
            } else {
                y -= 2;
            }
            break;
        case DIR_2_RIGHT:
            x -= 10;
            if (val <= 9) {
                y -= 12;
            } else if (val <= 10) {
                y += 4;
            } else if (val <= 11) {
                x -= 5;
                y += 2;
            } else if (val <= 13) {
                x -= 5;
            } else if (val <= 20) {
                y -= 2;
            } else if (val <= 21) {
                y -= 6;
            } else {
                y -= 12;
            }
        case DIR_4_BOTTOM:
            x += 20;
            if (val <= 9) {
                y += 4;
            } else if (val <= 10) {
                x += 10;
                y += 4;
            } else if (val <= 11) {
                x += 10;
                y -= 4;
            } else if (val <= 13) {
                y -= 6;
            } else if (val <= 20) {
                y -= 12;
            } else if (val <= 21) {
                y -= 10;
            } else {
                y -= 2;
            }
            break;
        case DIR_6_LEFT:
            x -= 10;
            if (val <= 9) {
                y -= 12;
            } else if (val <= 10) {
                y += 4;
            } else if (val <= 11) {
                y += 2;
            } else if (val <= 13) {
                // no change
            } else if (val <= 20) {
                y -= 2;
            } else if (val <= 21) {
                y -= 6;
            } else {
                y -= 12;
            }
            break;
    }
    draw_figure_with_cart(f, x, y);
}

static void draw_fort_standard(const figure *f, int x, int y)
{
    if (!formation_get(f->formationId)->in_distant_battle) {
        // base
        image_draw(f->graphicId, x, y);
        // flag
        int flag_height = image_get(f->cartGraphicId)->height;
        image_draw(f->cartGraphicId, x, y - flag_height);
        // top icon
        int icon_image_id = image_group(GROUP_FIGURE_FORT_STANDARD_ICONS) + f->formationId - 1;
        image_draw(icon_image_id, x, y - image_get(icon_image_id)->height - flag_height);
    }
}

static void tile_cross_country_offset_to_pixel_offset(int cross_country_x, int cross_country_y,
                                                      int *pixel_x, int *pixel_y)
{
    int dir = city_view_orientation();
    if (dir == DIR_0_TOP || dir == DIR_4_BOTTOM) {
        int base_pixel_x = 2 * cross_country_x - 2 * cross_country_y;
        int base_pixel_y = cross_country_x + cross_country_y;
        *pixel_x = dir == DIR_0_TOP ? base_pixel_x : -base_pixel_x;
        *pixel_y = dir == DIR_0_TOP ? base_pixel_y : -base_pixel_y;
    } else {
        int base_pixel_x = 2 * cross_country_x + 2 * cross_country_y;
        int base_pixel_y = cross_country_x - cross_country_y;
        *pixel_x = dir == DIR_2_RIGHT ? base_pixel_x : -base_pixel_x;
        *pixel_y = dir == DIR_6_LEFT ? base_pixel_y : -base_pixel_y;
    }
}

static int tile_progress_to_pixel_offset_x(int direction, int progress)
{
    switch (direction) {
        case DIR_0_TOP:
        case DIR_2_RIGHT:
            return 2 * progress - 28;
        case DIR_1_TOP_RIGHT:
            return 4 * progress - 56;
        case DIR_4_BOTTOM:
        case DIR_6_LEFT:
            return 28 - 2 * progress;
        case DIR_5_BOTTOM_LEFT:
            return 56 - 4 * progress;
        default:
            return 0;
    }
}

static int tile_progress_to_pixel_offset_y(int direction, int progress)
{
    switch (direction) {
        case DIR_0_TOP:
        case DIR_6_LEFT:
            return 14 - progress;
        case DIR_2_RIGHT:
        case DIR_4_BOTTOM:
            return progress - 14;
        case DIR_3_BOTTOM_RIGHT:
            return 2 * progress - 28;
        case DIR_7_TOP_LEFT:
            return 28 - 2 * progress;
        default:
            return 0;
    }
}

static void tile_progress_to_pixel_offset(int direction, int progress, int *pixel_x, int *pixel_y)
{
    *pixel_x = tile_progress_to_pixel_offset_x(direction, progress);
    *pixel_y = tile_progress_to_pixel_offset_y(direction, progress);
}

static void adjust_pixel_offset(const figure *f, int *pixel_x, int *pixel_y)
{
    // determining x/y offset on tile
    int x_offset = 0;
    int y_offset = 0;
    if (f->useCrossCountry) {
        tile_cross_country_offset_to_pixel_offset(f->crossCountryX % 15, f->crossCountryY % 15, &x_offset, &y_offset);
        y_offset -= f->missileDamage;
    } else {
        int direction = figure_image_normalize_direction(f->direction);
        tile_progress_to_pixel_offset(direction, f->progressOnTile, &x_offset, &y_offset);
        y_offset -= f->currentHeight;
        if (f->numPreviousFiguresOnSameTile && f->type != FIGURE_BALLISTA) {
            // an attempt to not let people walk through each other
            static const int BUSY_ROAD_X_OFFSETS[] = {
                0, 8, 8, -8, -8, 0, 16, 0, -16, 8, -8, 16, -16, 16, -16, 8, -8, 0, 24, 0, -24, 0, 0, 0
            };
            static const int BUSY_ROAD_Y_OFFSETS[] = {
                0, 0, 8, 8, -8, -16, 0, 16, 0, -16, 16, 8, -8, -8, 8, 16, -16, -24, 0, 24, 0, 0, 0, 0
            };
            x_offset += BUSY_ROAD_X_OFFSETS[f->numPreviousFiguresOnSameTile];
            y_offset += BUSY_ROAD_Y_OFFSETS[f->numPreviousFiguresOnSameTile];
        }
    }

    x_offset += 29;
    y_offset += 15;

    const image *img = f->isEnemyGraphic ? image_get_enemy(f->graphicId) : image_get(f->graphicId);
    *pixel_x += x_offset - img->sprite_offset_x;
    *pixel_y += y_offset - img->sprite_offset_y;
}

static void draw_figure(const figure *f, int x, int y)
{
    if (f->cartGraphicId) {
        switch (f->type) {
            case FIGURE_CART_PUSHER:
            case FIGURE_WAREHOUSEMAN:
            case FIGURE_LION_TAMER:
            case FIGURE_DOCKER:
            case FIGURE_NATIVE_TRADER:
            case FIGURE_IMMIGRANT:
            case FIGURE_EMIGRANT:
                draw_figure_with_cart(f, x, y);
                break;
            case FIGURE_HIPPODROME_HORSES:
                draw_hippodrome_horse(f, x, y);
                break;
            case FIGURE_FORT_STANDARD:
                draw_fort_standard(f, x, y);
                break;
            default:
                image_draw(f->graphicId, x, y);
                break;
        }
    } else {
        if (f->isEnemyGraphic) {
            image_draw_enemy(f->graphicId, x, y);
        } else {
            image_draw(f->graphicId, x, y);
        }
    }
}

void city_draw_figure(const figure *f, int x, int y)
{
    adjust_pixel_offset(f, &x, &y);
    draw_figure(f, x, y);
}

void city_draw_selected_figure(const figure *f, int x, int y, struct UI_CityPixelCoordinate *coord)
{
    adjust_pixel_offset(f, &x, &y);
    draw_figure(f, x, y);
    coord->x = x;
    coord->y = y;
}
