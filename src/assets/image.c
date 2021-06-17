#include "image.h"

#include "assets/group.h"
#include "core/array.h"
#include "core/image.h"
#include "core/log.h"
#include "graphics/color.h"
#include "graphics/graphics.h"
#include "graphics/image.h"

#include <stdlib.h>
#include <string.h>

#define ASSET_ARRAY_SIZE 2000

array(asset_image) asset_images;

static void load_image_layers(asset_image *img)
{
    for (layer *l = img->last_layer; l; l = l->prev) {
        layer_load(l);
    }
}

void asset_image_unload_layers(asset_image *img)
{
    layer *l = img->last_layer;
    while (l) {
        layer *current = l;
        l = l->prev;
        layer_unload(current);
    };
    img->last_layer = &img->first_layer;
}

int asset_image_load(asset_image *img)
{
    if (img->loaded) {
        return img->data != 0;
    }
    img->loaded = 1;

    load_image_layers(img);

    // Special cases for images which are a single layer
    if (&img->first_layer == img->last_layer) {
        layer *l = img->last_layer;
        if (img->img.width == l->width && img->img.height == l->height &&
            l->x_offset == 0 && l->y_offset == 0 &&
            l->invert == INVERT_NONE && l->rotate == ROTATE_NONE) {
            img->data = l->data;
            img->is_clone = l->is_asset_image_reference;
            l->is_asset_image_reference = 1;
            layer_unload(l);
            return 1;
        }
    }

    img->data = malloc(img->img.draw.data_length);
    if (!img->data) {
        log_error("Not enough memory to load image", img->id, 0);
        asset_image_unload_layers(img);
        return 0;
    }
    memset(img->data, 0, img->img.draw.data_length);
    for (int y = 0; y < img->img.height; ++y) {
        color_t *pixel = &img->data[y * img->img.width];
        for (int x = 0; x < img->img.width; ++x) {
            for (const layer *l = img->last_layer; l; l = l->prev) {
                color_t image_pixel_alpha = *pixel & COLOR_CHANNEL_ALPHA;
                if (image_pixel_alpha == ALPHA_OPAQUE) {
                    break;
                }
                color_t layer_pixel = layer_get_color_for_image_position(l, x, y);
                color_t layer_pixel_alpha = layer_pixel & COLOR_CHANNEL_ALPHA;
                if (layer_pixel_alpha == ALPHA_TRANSPARENT) {
                    continue;
                }
                if (image_pixel_alpha == ALPHA_TRANSPARENT) {
                    *pixel = layer_pixel;
                } else if (layer_pixel_alpha == ALPHA_OPAQUE) {
                    color_t alpha = image_pixel_alpha >> COLOR_BITSHIFT_ALPHA;
                    *pixel = COLOR_BLEND_ALPHA_TO_OPAQUE(*pixel, layer_pixel, alpha);
                } else {
                    color_t alpha_src = image_pixel_alpha >> COLOR_BITSHIFT_ALPHA;
                    color_t alpha_dst = layer_pixel_alpha >> COLOR_BITSHIFT_ALPHA;
                    color_t alpha_mix = COLOR_MIX_ALPHA(alpha_src, alpha_dst);
                    *pixel = COLOR_BLEND_ALPHAS(*pixel, layer_pixel, alpha_src, alpha_dst, alpha_mix);
                }
            }
            ++pixel;
        }
    }
    asset_image_unload_layers(img);
    return 1;
}

static layer *create_layer_for_image(asset_image *img)
{
    if (!img->last_layer->width || !img->last_layer->height) {
        return img->last_layer;
    }
    layer *l = malloc(sizeof(layer));
    if (!l) {
        log_error("Out of memory to create layer", 0, 0);
        return 0;
    }
    memset(l, 0, sizeof(layer));
    l->prev = img->last_layer;
    return l;
}

int asset_image_add_layer(asset_image *img,
    const char *path, const char *group_id, const char *image_id,
    int offset_x, int offset_y,
    layer_invert_type invert, layer_rotate_type rotate, layer_isometric_part part)
{
    layer *current_layer = create_layer_for_image(img);

    if (path) {
        current_layer = layer_add_from_image_path(current_layer, path, offset_x, offset_y);
    } else if (group_id) {
        current_layer = layer_add_from_image_id(current_layer, group_id, image_id, offset_x, offset_y);
    } else {
        layer_unload(current_layer);
        return 0;
    }
    if (!current_layer) {
        return 0;
    }
    if (!img->img.width) {
        img->img.width = current_layer->width;
    }
    if (!img->img.height) {
        img->img.height = current_layer->height;
    }
    current_layer->invert = invert;
    current_layer->rotate = rotate;
    current_layer->part = part;
    img->last_layer = current_layer;
    return 1;
}

asset_image *asset_image_get_from_id(int image_id)
{
    asset_image *last = array_last(asset_images);
    if (image_id < 0 || !last || image_id > last->index) {
        return 0;
    }
    return array_item(asset_images, image_id);
}

void asset_image_unload(asset_image *img)
{
    if (!img->loaded) {
        asset_image_unload_layers(img);
    }
    if (!img->is_clone) {
        free(img->data);
    }
    img->active = 0;
}

static void new_image(asset_image *img, int index)
{
    img->index = index;
    img->active = 1;
}

static int is_image_active(const asset_image *img)
{
    return img->active;
}

int asset_image_init_array(void)
{
    return array_init(asset_images, ASSET_ARRAY_SIZE, new_image, is_image_active);
}

asset_image *asset_image_create(void)
{
    asset_image *result;
    array_new_item(asset_images, 0, result);
    return result;
}
