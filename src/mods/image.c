#include "image.h"

#include "core/image.h"
#include "core/log.h"
#include "graphics/color.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "mods/group.h"

#include <stdlib.h>
#include <string.h>

static void load_image_layers(modded_image *img)
{
    for (layer *l = img->last_layer; l; l = l->prev) {
        layer_load(l);
    }
}

void modded_image_unload_layers(modded_image *img)
{
    layer *l = img->last_layer;
    while (l) {
        layer *current = l;
        l = l->prev;
        layer_unload(current);
    };
    img->last_layer = &img->first_layer;
}

int modded_image_load(modded_image *img)
{
    if (img->loaded) {
        return 1;
    }

    load_image_layers(img);

    // Special cases for images which are a single layer
    if (&img->first_layer == img->last_layer) {
        layer *l = img->last_layer;
        if (img->img.width == l->width && img->img.height == l->height &&
            l->x_offset == 0 && l->y_offset == 0 &&
            l->invert == INVERT_NONE && l->rotate == ROTATE_NONE) {
            img->data = l->data;
            img->is_clone = l->is_modded_image_reference;
            l->is_modded_image_reference = 1;
            layer_unload(l);
            img->loaded = 1;
            return 1;
        }
    }

    img->data = malloc(img->img.draw.data_length);
    if (!img->data) {
        log_error("Not enough memory to load image", img->id, 0);
        modded_image_unload_layers(img);
        img->active = 0;
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
    modded_image_unload_layers(img);
    img->loaded = 1;
    return 1;
}

static layer *create_layer_for_image(modded_image *img)
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

int modded_image_add_layer(modded_image *img,
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

modded_image *modded_image_get_from_id(int image_id)
{
    image_groups *group = group_get_from_hash(image_id);
    if (!group) {
        return 0;
    }
    int image_index = image_id & 0xff;
    for (modded_image *img = group->first_image; img; img = img->next) {
        if (img->index == image_index) {
            return img;
        }
    }
    return 0;
}
