#include "image.h"

#include "assets/group.h"
#include "core/array.h"
#include "core/image.h"
#include "core/image_packer.h"
#include "core/log.h"
#include "core/png_read.h"
#include "graphics/color.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/renderer.h"

#include <stdlib.h>
#include <string.h>

#define ASSET_ARRAY_SIZE 2000

static array(asset_image) asset_images;

static void load_image_layers(asset_image *img, color_t **main_images, int *main_image_widths)
{
    for (layer *l = img->last_layer; l; l = l->prev) {
        layer_load(l, main_images, main_image_widths);
    }
}

static void unload_image_layers(asset_image *img)
{
    layer *l = img->last_layer;
    while (l) {
        layer *current = l;
        l = l->prev;
        layer_unload(current);
    };
    img->last_layer = &img->first_layer;
}

static int has_top_part(const asset_image *img)
{
    int tiles = (img->img.width + 2) / (FOOTPRINT_WIDTH + 2);
    for (int y = 0; y < img->img.top_height; y++) {
        const color_t *row = &img->data[y * img->img.width];
        int footprint_row = y - img->img.height - 1 - tiles * FOOTPRINT_HALF_HEIGHT;
        int half_top_pixels_in_row = (footprint_row < 0 ? img->img.width : img->img.width - 2 + 4 * footprint_row) / 2;
        for (int x = 0; x < half_top_pixels_in_row; x++) {
            if ((row[x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                return 1;
            }
        }
        row += img->img.width - half_top_pixels_in_row;
        for (int x = 0; x < half_top_pixels_in_row; x++) {
            if ((row[x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                return 1;
            }
        }
    }
    return 0;
}

static int is_single_layer_unchanged_image(const asset_image *img)
{
    if (!img->active || &img->first_layer != img->last_layer) {
        return 0;
    }
    const layer *l = img->last_layer;
    return img->img.width == l->width && img->img.height == l->height &&
        l->x_offset == 0 && l->y_offset == 0 &&
        l->invert == INVERT_NONE && l->rotate == ROTATE_NONE && !l->grayscale;
}

static void make_similar_images_references(const asset_image *img)
{
    const image_groups *group = group_get_from_image_index(img->index);
    for (int i = img->index + 1; i <= group->last_image_index; i++) {
        asset_image *reference = asset_image_get_from_id(i);
        if (is_single_layer_unchanged_image(reference) && reference->last_layer->asset_image_path &&
            strcmp(reference->last_layer->asset_image_path, img->last_layer->asset_image_path) == 0 &&
            reference->last_layer->src_x == img->last_layer->src_x &&
            reference->last_layer->src_y == img->last_layer->src_y) {
            reference->last_layer->calculated_image_id = img->index + IMAGE_MAIN_ENTRIES;
            reference->is_reference = 1;
        }
    }
}

#ifndef BUILDING_ASSET_PACKER
static int load_image(asset_image *img, color_t **main_images, int *main_image_widths)
{
    if (img->is_reference) {
        return 1;
    }
    if (is_single_layer_unchanged_image(img)) {
        layer *l = img->last_layer;
        if (!l->calculated_image_id) {
            layer_load(l, main_images, main_image_widths);
            img->data = l->data;
            l->data = 0;
            make_similar_images_references(img);
            layer_unload(l);
        } else {
            img->is_reference = 1;
        }
        return 1;
    }
    load_image_layers(img, main_images, main_image_widths);

    int image_height;

    if (!img->img.is_isometric || graphics_renderer()->isometric_images_are_joined()) {
        image_height = img->img.height;
    } else {
        int tiles = (img->img.width + 2) / (FOOTPRINT_WIDTH + 2);
        image_height = img->img.height - tiles * FOOTPRINT_HALF_HEIGHT;
    }

    color_t *data = malloc(img->img.width * image_height * sizeof(color_t));
    if (!data) {
        log_error("Error creating image - out of memory", 0, 0);
        return 0;
    }
    memset(data, 0, img->img.width * image_height * sizeof(color_t));

    for (const layer *l = img->last_layer; l; l = l->prev) {
        int image_start_x, image_start_y, image_valid_width, image_valid_height, layer_step_x;

        if (l->rotate == ROTATE_NONE || l->rotate == ROTATE_180_DEGREES) {
            image_start_x = l->x_offset < 0 ? 0 : l->x_offset;
            image_start_y = l->y_offset < 0 ? 0 : l->y_offset;
            image_valid_width = image_start_x + l->width - (l->x_offset < 0 ? -l->x_offset : 0);
            image_valid_height = image_start_y + l->height - (l->y_offset < 0 ? -l->y_offset : 0);
            layer_step_x = 1;
        } else {
            image_start_x = l->x_offset < 0 ? 0 : l->x_offset;
            image_start_y = l->y_offset < 0 ? 0 : l->y_offset;
            image_valid_width = image_start_y + l->height - (l->y_offset < 0 ? -l->y_offset : 0);
            image_valid_height = image_start_x + l->width - (l->x_offset < 0 ? -l->x_offset : 0);
            layer_step_x = l->width;
        }
        if (image_valid_width > img->img.width) {
            image_valid_width = img->img.width;
        }
        if (image_valid_height > image_height) {
            image_valid_height = image_height;
        }

        // The above code is innacurate when a layer is rotated either by 90 or 270 degrees and inverted.
        // In those cases, we use layer_get_color_for_image_position for every pixel, which is much slower but accurate.
        int inverts_and_rotates = l->invert != INVERT_NONE &&
            (l->rotate == ROTATE_90_DEGREES || l->rotate == ROTATE_270_DEGREES);

        if (!inverts_and_rotates) {
            layer_invert_type invert = l->invert;
            if (l->rotate == ROTATE_90_DEGREES || l->rotate == ROTATE_180_DEGREES) {
                invert ^= INVERT_HORIZONTAL;
            }
            if (invert & INVERT_HORIZONTAL) {
                layer_step_x = -layer_step_x;
            }
        }
       
        for (int y = image_start_y; y < image_valid_height; y++) {
            color_t *pixel = &data[y * img->img.width + image_start_x];
            const color_t *layer_pixel = 0;
            if (!inverts_and_rotates) {
                layer_pixel = layer_get_color_for_image_position(l, image_start_x, y);
            }
            for (int x = image_start_x; x < image_valid_width; x++) {
                if (inverts_and_rotates) {
                    layer_pixel = layer_get_color_for_image_position(l, x, y);
                }
                color_t image_pixel_alpha = *pixel & COLOR_CHANNEL_ALPHA;
                color_t layer_pixel_alpha = *layer_pixel & COLOR_CHANNEL_ALPHA;
                if (image_pixel_alpha == ALPHA_OPAQUE || layer_pixel_alpha == ALPHA_TRANSPARENT) {
                    pixel++;
                    layer_pixel += layer_step_x;
                    continue;
                }
                if (image_pixel_alpha == ALPHA_TRANSPARENT) {
                    *pixel = *layer_pixel;
                } else if (layer_pixel_alpha == ALPHA_OPAQUE) {
                    color_t alpha = image_pixel_alpha >> COLOR_BITSHIFT_ALPHA;
                    *pixel = COLOR_BLEND_ALPHA_TO_OPAQUE(*pixel, *layer_pixel, alpha);
                } else {
                    color_t alpha_src = image_pixel_alpha >> COLOR_BITSHIFT_ALPHA;
                    color_t alpha_dst = layer_pixel_alpha >> COLOR_BITSHIFT_ALPHA;
                    color_t alpha_mix = COLOR_MIX_ALPHA(alpha_src, alpha_dst);
                    *pixel = COLOR_BLEND_ALPHAS(*pixel, *layer_pixel, alpha_src, alpha_dst, alpha_mix);
                }
                pixel++;
                layer_pixel += layer_step_x;
            }
        }
    }

    img->data = data;

    unload_image_layers(img);

    return 1;
}
#endif

static inline int layer_is_empty(const layer *l)
{
#ifndef BUILDING_ASSET_PACKER
    return !l->width && !l->height;
#else
    return !l->original_image_group && !l->original_image_id && !l->asset_image_path;
#endif
}

static layer *create_layer_for_image(asset_image *img)
{
    if (layer_is_empty(img->last_layer)) {
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
    int src_x, int src_y, int offset_x, int offset_y, int width, int height,
    layer_invert_type invert, layer_rotate_type rotate, layer_isometric_part part, int grayscale)
{
    layer *current_layer = create_layer_for_image(img);

    if (!current_layer) {
        return 0;
    }
    current_layer->invert = invert;
    current_layer->rotate = rotate;
    current_layer->part = part;
    current_layer->grayscale = grayscale;

    int result;

    if (group_id) {
        result = layer_add_from_image_id(current_layer, group_id, image_id, offset_x, offset_y);
    } else {
        result = layer_add_from_image_path(current_layer, path, src_x, src_y, offset_x, offset_y, width, height);
    }
    if (!result) {
        return 0;
    }
    if (rotate == ROTATE_NONE || rotate == ROTATE_180_DEGREES) {
        if (!img->img.width) {
            img->img.width = current_layer->width;
        }
        if (!img->img.height) {
            img->img.height = current_layer->height;
        }
    } else {
        if (!img->img.width) {
            img->img.width = current_layer->height;
        }
        if (!img->img.height) {
            img->img.height = current_layer->width;
        }
    }
#ifdef BUILDING_ASSET_PACKER
    if (img->last_layer != current_layer) {
        img->last_layer->next = current_layer;
    }
#endif
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
    unload_image_layers(img);
    free((color_t *) img->data); // Freeing a const pointer - ugly but necessary
    img->data = 0;
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
    asset_image *image;
    array_foreach(asset_images, image)
    {
        asset_image_unload(image);
    }
    return array_init(asset_images, ASSET_ARRAY_SIZE, new_image, is_image_active);
}

asset_image *asset_image_create(void)
{
    asset_image *result;
    array_new_item(asset_images, 1, result);
    return result;
}

#ifndef BUILDING_ASSET_PACKER
static void copy_asset_to_final_image(color_t *dst, const asset_image *img, int src_width, int dst_width)
{
    if (!graphics_renderer()->isometric_images_are_joined() && img->img.is_isometric) {
        int tiles = (img->img.width + 2) / (FOOTPRINT_WIDTH + 2);
        asset_image_copy_isometric_top(dst, img->data, img->img.width, img->img.top_height,
            img->img.atlas.x_offset, img->img.atlas.y_offset, dst_width, 0, 0, src_width);
        asset_image_copy_isometric_footprint(dst, img->data, img->img.width, img->img.height - img->img.top_height,
            img->img.atlas.x_offset, img->img.atlas.y_offset + img->img.top_height, dst_width,
            0, img->img.top_height - tiles * FOOTPRINT_HALF_HEIGHT, src_width);
    } else {
        for (int y = 0; y < img->img.height; y++) {
            memcpy(&dst[(y + img->img.atlas.y_offset) * dst_width + img->img.atlas.x_offset],
                &img->data[(y + img->img.y_offset) * src_width + img->img.x_offset], img->img.width * sizeof(color_t));
        }
    }
}

static void trim_image(asset_image *img)
{
    // Don't draw the top part if it's empty
    if (!has_top_part(img)) {
        if (!graphics_renderer()->isometric_images_are_joined()) {
            img->img.height -= img->img.top_height;
            img->img.y_offset = img->img.top_height;
        }
        img->img.top_height = 0;
    }
}
#endif

int asset_image_load_all(color_t **main_images, int *main_image_widths)
{
#ifndef BUILDING_ASSET_PACKER
    image_packer packer;
    int max_width, max_height;
    graphics_renderer()->get_max_image_size(&max_width, &max_height);
    if (image_packer_init(&packer, asset_images.size, max_width, max_height) != IMAGE_PACKER_OK) {
        log_error("Failed to init image packer", 0, 0);
        return 0;
    }
    packer.options.fail_policy = IMAGE_PACKER_NEW_IMAGE;
    packer.options.reduce_image_size = 1;
    packer.options.sort_by = IMAGE_PACKER_SORT_BY_AREA;

    asset_image *current_image;
    array_foreach(asset_images, current_image) {
        load_image(current_image, main_images, main_image_widths);

        if (current_image->is_reference) {
            const image *referenced = image_get(current_image->first_layer.calculated_image_id);
            current_image->img.atlas.id = referenced->atlas.id;
            current_image->img.atlas.x_offset = referenced->atlas.x_offset;
            current_image->img.atlas.y_offset = referenced->atlas.y_offset;
            continue;
        }

        int width = current_image->img.width;
        int height = current_image->img.height;

        if (graphics_renderer()->should_pack_image(current_image->img.width, current_image->img.height)) {
            if (current_image->img.is_isometric) {
                trim_image(current_image);
            } else {
                image_crop(&current_image->img, current_image->data, 1);
            }
            current_image->img.atlas.id = ATLAS_EXTRA_ASSET << IMAGE_ATLAS_BIT_OFFSET;
            packer.rects[i].input.width = current_image->img.width;
            packer.rects[i].input.height = current_image->img.height;

            // Uncrop image for now, crop it later again
            if (!current_image->img.is_isometric) {
                current_image->img.x_offset = 0;
                current_image->img.y_offset = 0;
                current_image->img.width = width;
                current_image->img.height = height;
            }
        } else {
            current_image->img.atlas.id = ATLAS_UNPACKED_EXTRA_ASSET << IMAGE_ATLAS_BIT_OFFSET;
        }
    }

    png_unload();
    image_packer_pack(&packer);

    const image_atlas_data *atlas_data = graphics_renderer()->prepare_image_atlas(ATLAS_EXTRA_ASSET,
        packer.result.images_needed, packer.result.last_image_width, packer.result.last_image_height);
    if (!atlas_data) {
        log_error("Failed to create packed images atlas - out of memory", 0, 0);
        return 0;
    }

    int total_unpacked_assets = 0;

    array_foreach(asset_images, current_image) {
        if (current_image->is_reference) {
            const image *referenced = image_get(current_image->first_layer.calculated_image_id);
            current_image->img.atlas.id = referenced->atlas.id;
            current_image->img.atlas.x_offset = referenced->atlas.x_offset;
            current_image->img.atlas.y_offset = referenced->atlas.y_offset;
            current_image->img.width = referenced->width;
            current_image->img.height = referenced->height;
        } else if (graphics_renderer()->should_pack_image(current_image->img.width, current_image->img.height)) {
            image_packer_rect *rect = &packer.rects[i];
            current_image->img.atlas.x_offset = rect->output.x;
            current_image->img.atlas.y_offset = rect->output.y;
            current_image->img.atlas.id += rect->output.image_index;
            int original_width = current_image->img.width;
            if (!current_image->img.is_isometric) {
                image_crop(&current_image->img, current_image->data, 1);
            }
            copy_asset_to_final_image(atlas_data->buffers[rect->output.image_index], current_image, original_width,
                atlas_data->image_widths[rect->output.image_index]);
            free((color_t *) current_image->data); // Freeing a const pointer - ugly but necessary

            current_image->data = 0;
        } else {
            if (current_image->img.is_isometric && !graphics_renderer()->isometric_images_are_joined()) {
                size_t size = current_image->img.width * current_image->img.height * sizeof(color_t);
                color_t *data = malloc(size);
                if (!data) {
                    log_error("Unable to create data for asset. The game may crash. Asset index:",
                        0, current_image->index);
                } else {
                    memset(data, 0, size);
                    copy_asset_to_final_image(data, current_image, current_image->img.width, current_image->img.width);
                    free((color_t *) current_image->data); // Freeing a const pointer - ugly but necessary
                    current_image->data = data;
                }
            }
            current_image->img.atlas.id += total_unpacked_assets++;
        }
    }
    image_packer_free(&packer);
    graphics_renderer()->create_image_atlas(atlas_data);
#endif
    return 1;
}

void asset_image_reload_climate(void)
{
#ifndef BUILDING_ASSET_PACKER
    asset_image *current_image;
    array_foreach(asset_images, current_image) {
        if (current_image->is_reference) {
            const image *referenced = image_get(current_image->first_layer.calculated_image_id);
            current_image->img.x_offset = referenced->x_offset;
            current_image->img.y_offset = referenced->y_offset;
            current_image->img.atlas.id = referenced->atlas.id;
            current_image->img.atlas.x_offset = referenced->atlas.x_offset;
            current_image->img.atlas.y_offset = referenced->atlas.y_offset;
            current_image->img.width = referenced->width;
            current_image->img.height = referenced->height;
        }
    }
#endif
}

void asset_image_copy_isometric_top(color_t *dst, const color_t *src, int width, int height,
    int dst_x_offset, int dst_y_offset, int dst_width, int src_x_offset, int src_y_offset, int src_width)
{
    int tiles = (width + 2) / (FOOTPRINT_WIDTH + 2);
    for (int y = 0; y < height; y++) {
        const color_t *src_row = &src[(src_y_offset + y) * src_width + src_x_offset];
        color_t *dst_row = &dst[(dst_y_offset + y) * dst_width + dst_x_offset];
        int footprint_row = y - height - 1 - tiles * FOOTPRINT_HALF_HEIGHT;
        int half_top_pixels_in_row = (footprint_row < 0 ? width : width - 2 + 4 * footprint_row) / 2;
        memcpy(dst_row, src_row, half_top_pixels_in_row * sizeof(color_t));
        src_row += width - half_top_pixels_in_row;
        dst_row += width - half_top_pixels_in_row;
        memcpy(dst_row, src_row, half_top_pixels_in_row * sizeof(color_t));
    }
}

void asset_image_copy_isometric_footprint(color_t *dst, const color_t *src, int width, int height,
    int dst_x_offset, int dst_y_offset, int dst_width, int src_x_offset, int src_y_offset, int src_width)
{
    if (src_y_offset < 0) {
        src_y_offset = 0;
    }
    int half_height = height / 2;
    for (int y = 0; y < height; y++) {
        int x_read = 2 + 4 * (y < half_height ? y : height - 1 - y);
        int x_skip = (width - x_read) / 2;
        memcpy(&dst[(dst_y_offset + y) * dst_width + dst_x_offset + x_skip],
            &src[(src_y_offset + y) * src_width + src_x_offset + x_skip], x_read * sizeof(color_t));
    }
}
