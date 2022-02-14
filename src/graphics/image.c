#include "image.h"

#include "assets/assets.h"
#include "core/image.h"
#include "graphics/renderer.h"
#include "graphics/screen.h"

#include <stdlib.h>
#include <string.h>

void image_draw(int image_id, int x, int y, color_t color, float scale)
{
    const image *img = image_get(image_id);
    if (image_is_external(image_id)) {
        image_load_external_data(image_id);
    } else if ((img->atlas.id >> IMAGE_ATLAS_BIT_OFFSET) == ATLAS_UNPACKED_EXTRA_ASSET) {
        assets_load_unpacked_asset(image_id);
    }
    graphics_renderer()->draw_image(img, x, y, color, scale);
}

void image_draw_enemy(int image_id, int x, int y, float scale)
{
    if (image_id <= 0 || image_id >= 801) {
        return;
    }
    graphics_renderer()->draw_image(image_get_enemy(image_id), x, y, COLOR_MASK_NONE, scale);
}

void image_blend_footprint_color(int x, int y, color_t color, float scale)
{
    graphics_renderer()->draw_custom_image(color == COLOR_MASK_GREEN ?
        CUSTOM_IMAGE_GREEN_FOOTPRINT : CUSTOM_IMAGE_RED_FOOTPRINT, x, y, scale);
}

static color_t base_color_for_font(font_t font)
{
    if (font == FONT_NORMAL_PLAIN || font == FONT_LARGE_PLAIN || font == FONT_SMALL_PLAIN) {
        return COLOR_FONT_PLAIN;
    }
    return COLOR_MASK_NONE;
}

static void draw_multibyte_letter(font_t font, const image *img, int x, int y, color_t color, float scale)
{
    switch (font) {
        case FONT_NORMAL_WHITE:
            graphics_renderer()->draw_image(img, x + 1, y + 1, 0xff311c10, scale);
            graphics_renderer()->draw_image(img, x, y, COLOR_WHITE, scale);
            break;
        case FONT_NORMAL_RED:
            graphics_renderer()->draw_image(img, x + 1, y + 1, 0xffe7cfad, scale);
            graphics_renderer()->draw_image(img, x, y, 0xff731408, scale);
            break;
        case FONT_NORMAL_GREEN:
            graphics_renderer()->draw_image(img, x + 1, y + 1, 0xffe7cfad, scale);
            graphics_renderer()->draw_image(img, x, y, 0xff180800, scale);
            break;
        case FONT_NORMAL_BLACK:
        case FONT_LARGE_BLACK:
            graphics_renderer()->draw_image(img, x + 1, y + 1, 0xffcead9c, scale);
            graphics_renderer()->draw_image(img, x, y, COLOR_BLACK, scale);
            break;
        default: // Plain + brown
            if (!color) {
                color = base_color_for_font(font);
            }
            graphics_renderer()->draw_image(img, x, y, ALPHA_OPAQUE | color, scale);
            break;
    }
}

void image_draw_letter(font_t font, int letter_id, int x, int y, color_t color, float scale)
{

    const image *img = image_letter(letter_id);
    if (letter_id >= IMAGE_FONT_MULTIBYTE_OFFSET) {
        draw_multibyte_letter(font, img, x, y, color, scale);
        return;
    }
    if (!color) {
        color = base_color_for_font(font);
    }
    graphics_renderer()->draw_image(img, x, y, color, scale);
}

static inline void draw_fullscreen_background(int image_id)
{
    int s_width = screen_width();
    int s_height = screen_height();
    const image *img = image_get(image_id);
    float scale_w = img->width / (float) screen_width();
    float scale_h = img->height / (float) screen_height();
    float scale = scale_w < scale_h ? scale_w : scale_h;
    
    if (scale >= SCALE_NONE) {
        image_draw(image_id, (s_width - img->width) / 2, (s_height - img->height) / 2, COLOR_MASK_NONE, SCALE_NONE);
    } else {
        int x = 0;
        int y = 0;
        if (scale == scale_h) {
            x = (int) ((s_width - img->width / scale) / 2 * scale);
        }
        if (scale == scale_w) {
            y = (int) ((s_height - img->height / scale) / 2 * scale);
        }
        const image *img = image_get(image_id);
        if (image_is_external(image_id)) {
            image_load_external_data(image_id);
        } else if ((img->atlas.id >> IMAGE_ATLAS_BIT_OFFSET) == ATLAS_UNPACKED_EXTRA_ASSET) {
            assets_load_unpacked_asset(image_id);
        }
        graphics_renderer()->draw_image(img, x, y, COLOR_MASK_NONE, scale);
    }
}

static inline void draw_fullscreen_borders(void)
{
    int width = screen_width();
    int height = screen_height();
    int image_base = image_group(GROUP_EMPIRE_PANELS);

    // horizontal bar borders
    for (int x = 0; x < width; x += 86) {
        image_draw(image_base + 1, x, 0, COLOR_MASK_NONE, SCALE_NONE);
        image_draw(image_base + 1, x, height - 16, COLOR_MASK_NONE, SCALE_NONE);
    }

    // vertical bar borders
    for (int y = 16; y < height; y += 86) {
        image_draw(image_base, 0, y, COLOR_MASK_NONE, SCALE_NONE);
        image_draw(image_base, width - 16, y, COLOR_MASK_NONE, SCALE_NONE);
    }

    // crossbars
    image_draw(image_base + 2, 0, 0, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_base + 2, 0, height - 16, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_base + 2, width - 16, 0, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_base + 2, width - 16, height - 16, COLOR_MASK_NONE, SCALE_NONE);
}

void image_draw_fullscreen_background(int image_id)
{
    graphics_renderer()->clear_screen();
    draw_fullscreen_background(image_id);
    draw_fullscreen_borders();
}

void image_draw_isometric_footprint(int image_id, int x, int y, color_t color_mask, float scale)
{
    const image *img = image_get(image_id);
    if ((img->atlas.id >> IMAGE_ATLAS_BIT_OFFSET) == ATLAS_UNPACKED_EXTRA_ASSET) {
        assets_load_unpacked_asset(image_id);
    }
    int num_tiles = (img->width + 2) / (FOOTPRINT_WIDTH + 2);
    x -= 30 * (num_tiles - 1);
    graphics_renderer()->draw_image(img, x, y, color_mask, scale);
}

void image_draw_isometric_footprint_from_draw_tile(int image_id, int x, int y, color_t color_mask, float scale)
{
    const image *img = image_get(image_id);
    if ((img->atlas.id >> IMAGE_ATLAS_BIT_OFFSET) == ATLAS_UNPACKED_EXTRA_ASSET) {
        assets_load_unpacked_asset(image_id);
    }
    int num_tiles = (img->width + 2) / (FOOTPRINT_WIDTH + 2);
    y -= FOOTPRINT_HALF_HEIGHT * (num_tiles - 1);
    graphics_renderer()->draw_image(img, x, y, color_mask, scale);
}

void image_draw_isometric_top(int image_id, int x, int y, color_t color_mask, float scale)
{
    const image *img = image_get(image_id);
    if ((img->atlas.id >> IMAGE_ATLAS_BIT_OFFSET) == ATLAS_UNPACKED_EXTRA_ASSET) {
        assets_load_unpacked_asset(image_id);
    }
    int num_tiles = (img->width + 2) / (FOOTPRINT_WIDTH + 2);
    x -= 30 * (num_tiles - 1);
    y -= img->top_height - FOOTPRINT_HALF_HEIGHT * num_tiles;
    graphics_renderer()->draw_isometric_top(image_get(image_id), x, y, color_mask, scale);
}

void image_draw_isometric_top_from_draw_tile(int image_id, int x, int y, color_t color_mask, float scale)
{
    const image *img = image_get(image_id);
    if ((img->atlas.id >> IMAGE_ATLAS_BIT_OFFSET) == ATLAS_UNPACKED_EXTRA_ASSET) {
        assets_load_unpacked_asset(image_id);
    }
    int num_tiles = (img->width + 2) / (FOOTPRINT_WIDTH + 2);
    y -= FOOTPRINT_HALF_HEIGHT * (num_tiles - 1);
    y -= img->top_height - FOOTPRINT_HALF_HEIGHT * num_tiles;
    graphics_renderer()->draw_isometric_top(img, x, y, color_mask, scale);
}
