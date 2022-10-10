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

static struct {
    array(asset_image) asset_images;
    int total_isometric_images;
} data;

typedef enum {
    IMAGE_ORIGINAL = 0,
    IMAGE_TRANSLATED_REFERENCE = 1,
    IMAGE_FULL_REFERENCE = 2
} image_reference_type;

static int load_image_layers(asset_image *img, color_t **main_images, int *main_image_widths)
{
    int has_alpha_mask = 0;
    for (layer *l = img->last_layer; l; l = l->prev) {
        if (l->mask == LAYER_MASK_ALPHA) {
            has_alpha_mask = 1;
        }
        layer_load(l, main_images, main_image_widths);
    }
    return has_alpha_mask;
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

#ifndef BUILDING_ASSET_PACKER
static image_reference_type get_image_reference_type(const asset_image *img)
{
    if (!img->active || &img->first_layer != img->last_layer) {
        return IMAGE_ORIGINAL;
    }
    const layer *l = img->last_layer;
    if (l->invert != INVERT_NONE || l->rotate != ROTATE_NONE || l->part != PART_BOTH || l->mask != LAYER_MASK_NONE) {
        return IMAGE_ORIGINAL;
    }
    int reference = img->img.width == l->width && img->img.height == l->height && l->x_offset == 0 && l->y_offset == 0 ?
        IMAGE_FULL_REFERENCE : IMAGE_TRANSLATED_REFERENCE;
    return reference == IMAGE_TRANSLATED_REFERENCE && image_get(l->calculated_image_id)->is_isometric ?
        IMAGE_ORIGINAL : reference;
}

static void translate_reference_position(asset_image *img)
{
    layer *l = &img->first_layer;
    int width;
    int height;
    if (l->calculated_image_id >= IMAGE_MAIN_ENTRIES) {
        const asset_image *referenced = asset_image_get_from_id(l->calculated_image_id - IMAGE_MAIN_ENTRIES);
        img->img.atlas.id = referenced->img.atlas.id;
        img->img.x_offset = referenced->img.x_offset;
        img->img.y_offset = referenced->img.y_offset;
        img->img.atlas.x_offset = referenced->img.atlas.x_offset;
        img->img.atlas.y_offset = referenced->img.atlas.y_offset;
        img->img.top = referenced->img.top;
        width = referenced->img.width;
        height = referenced->img.height;
    } else {
        const image *referenced = image_get(l->calculated_image_id);
        img->img.atlas.id = referenced->atlas.id;
        img->img.x_offset = referenced->x_offset;
        img->img.y_offset = referenced->y_offset;
        img->img.atlas.x_offset = referenced->atlas.x_offset;
        img->img.atlas.y_offset = referenced->atlas.y_offset;
        img->img.top = referenced->top;
        width = referenced->width;
        height = referenced->height;
    }
    if (img->img.width > width) {
        img->img.width = width;
    }
    if (img->img.height > height) {
        img->img.height = height;
    }
    // Isometric images don't need translation
    if (img->img.is_isometric) {
        img->img.original.width = img->img.width;
        img->img.original.height = img->img.height;
        return;
    }
    if (l->x_offset >= 0) {
        img->img.x_offset += l->x_offset;
        img->img.width -= l->x_offset;
    } else {
        img->img.atlas.x_offset -= l->x_offset;
        int remaining_width = width - img->img.width;
        if (remaining_width < 0) {
            img->img.width += remaining_width;
        }
    }
    if (l->y_offset >= 0) {
        img->img.y_offset += l->y_offset;
        img->img.height -= l->y_offset;
    } else {
        img->img.atlas.y_offset -= l->y_offset;
        int remaining_height = height - img->img.height;
        if (remaining_height < 0) {
            img->img.height += remaining_height;
        }
    }
    if (img->img.width <= 0 || img->img.height <= 0) {
        img->img.width = 0;
        img->img.height = 0;
    }
    l->width = img->img.width;
    l->height = img->img.height;
    img->img.original.width = img->img.width;
    img->img.original.height = img->img.height;
}

static void make_similar_images_references(const asset_image *img)
{
    const image_groups *group = group_get_from_image_index(img->index);
    for (int i = img->index + 1; i <= group->last_image_index; i++) {
        asset_image *reference = asset_image_get_from_id(i);
        if (get_image_reference_type(reference) != IMAGE_ORIGINAL && reference->last_layer->asset_image_path &&
            strcmp(reference->last_layer->asset_image_path, img->last_layer->asset_image_path) == 0 &&
            reference->last_layer->src_x == img->last_layer->src_x &&
            reference->last_layer->src_y == img->last_layer->src_y) {
            reference->last_layer->calculated_image_id = img->index + IMAGE_MAIN_ENTRIES;
            reference->is_reference = 1;
            translate_reference_position(reference);
        }
    }
}

static int has_top_part(const image *img, const color_t *pixels)
{
    int tiles = (img->width + 2) / (FOOTPRINT_WIDTH + 2);
    int top_height = img->height - tiles * FOOTPRINT_HEIGHT;
    for (int y = 0; y < top_height; y++) {
        const color_t *row = &pixels[y * img->width];
        int footprint_row = y - img->height - 1 - tiles * FOOTPRINT_HALF_HEIGHT;
        int half_top_pixels_in_row = (footprint_row < 0 ? img->width : img->width - 2 + 4 * footprint_row) / 2;
        for (int x = 0; x < half_top_pixels_in_row; x++) {
            if ((row[x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                return 1;
            }
        }
        row += img->width - half_top_pixels_in_row;
        for (int x = 0; x < half_top_pixels_in_row; x++) {
            if ((row[x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                return 1;
            }
        }
    }
    return 0;
}

void split_top_and_footprint(const image *img, color_t *dst, const color_t *src, int src_height)
{
    int tiles = (img->width + 2) / (FOOTPRINT_WIDTH + 2);

    // Copy the top
    for (int y = 0; y < img->top->height; y++) {
        const color_t *src_row = &src[y * img->width];
        color_t *dst_row = &dst[y * img->width];
        int footprint_row = y - img->top->height - 1 + tiles * FOOTPRINT_HALF_HEIGHT;
        int half_top_pixels_in_row = (footprint_row < 0 ? img->width : img->width - 2 - 4 * footprint_row) / 2;
        memcpy(dst_row, src_row, half_top_pixels_in_row * sizeof(color_t));
        src_row += img->width - half_top_pixels_in_row;
        dst_row += img->width - half_top_pixels_in_row;
        memcpy(dst_row, src_row, half_top_pixels_in_row * sizeof(color_t));
    }

    image_copy_info copy = {
        .src = { 0, src_height - tiles * FOOTPRINT_HEIGHT, img->width, src_height, src },
        .dst = { 0, img->atlas.y_offset, img->width, img->height + img->top->height, dst },
        .rect = { 0, 0, img->width, img->height }
    };

    image_copy_isometric_footprint(&copy);
}

static int load_image(asset_image *img, color_t **main_images, int *main_image_widths)
{
    img->img.original.width = img->img.width;
    img->img.original.height = img->img.height;

    image_reference_type reference_type = get_image_reference_type(img);
    if (reference_type == IMAGE_FULL_REFERENCE) {
        layer *l = img->last_layer;
        if (!l->calculated_image_id && !img->img.is_isometric) {
            layer_load(l, main_images, main_image_widths);
            img->data = l->data;
            l->data = 0;
            make_similar_images_references(img);
            layer_unload(l);
            return 1;
        }
    }
    int has_alpha_mask = load_image_layers(img, main_images, main_image_widths);

    color_t *data = malloc(img->img.width * img->img.height * sizeof(color_t));
    if (!data) {
        log_error("Error creating image - out of memory", 0, 0);
        unload_image_layers(img);
        return 0;
    }
    memset(data, 0, img->img.width * img->img.height * sizeof(color_t));

    // Images with an alpha mask layer need to be loaded from first to last, which is slower
    const layer *l = has_alpha_mask ? &img->first_layer : img->last_layer;

    while (l) {
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
            image_valid_width = image_start_x + l->height - (l->x_offset < 0 ? -l->x_offset : 0);
            image_valid_height = image_start_y + l->width - (l->y_offset < 0 ? -l->y_offset : 0);
            layer_step_x = l->width;
        }
        if (image_valid_width > img->img.width) {
            image_valid_width = img->img.width;
        }
        if (image_valid_height > img->img.height) {
            image_valid_height = img->img.height;
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
                if (l->mask == LAYER_MASK_ALPHA) {
                    // Since the alpha mask should be grayscale, we use the blue color as the mask value
                    color_t alpha_mask = (*layer_pixel & COLOR_CHANNEL_BLUE) << COLOR_BITSHIFT_ALPHA;
                    if (alpha_mask == ALPHA_TRANSPARENT || image_pixel_alpha == ALPHA_TRANSPARENT) {
                        *pixel = ALPHA_TRANSPARENT;
                    } else if (alpha_mask != ALPHA_OPAQUE) {
                        if (image_pixel_alpha != ALPHA_OPAQUE) {
                            color_t alpha_src = image_pixel_alpha >> COLOR_BITSHIFT_ALPHA;
                            color_t alpha_dst = alpha_mask >> COLOR_BITSHIFT_ALPHA;
                            alpha_mask = COLOR_MIX_ALPHA(alpha_src, alpha_dst) << COLOR_BITSHIFT_ALPHA;
                        }
                        color_t result = *pixel | ALPHA_OPAQUE;
                        result &= alpha_mask & COLOR_WHITE;
                        *pixel = result;
                    }
                    pixel++;
                    layer_pixel += layer_step_x;
                    continue;
                }
                color_t layer_pixel_alpha = *layer_pixel & COLOR_CHANNEL_ALPHA;
                if ((image_pixel_alpha == ALPHA_OPAQUE && !has_alpha_mask) || layer_pixel_alpha == ALPHA_TRANSPARENT) {
                    pixel++;
                    layer_pixel += layer_step_x;
                    continue;
                }
                if (image_pixel_alpha == ALPHA_TRANSPARENT || (layer_pixel_alpha == ALPHA_OPAQUE && has_alpha_mask)) {
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
        l = has_alpha_mask ? l->next : l->prev;
    }

    // The top and footprint parts of the image need to be split
    if (img->img.is_isometric) {
        int tiles = (img->img.width + 2) / (FOOTPRINT_WIDTH + 2);
        int footprint_height = tiles * FOOTPRINT_HEIGHT;

        if (has_top_part(&img->img, data)) {
            img->img.top = malloc(sizeof(image));
            if (!img->img.top) {
                log_error("Error creating image - out of memory", 0, 0);
                unload_image_layers(img);
                return 0;
            }
            memset(img->img.top, 0, sizeof(image));
            img->img.top->width = img->img.width;
            img->img.top->height = img->img.height - footprint_height / 2;
            img->img.top->original.width = img->img.top->width;
            img->img.top->original.height = img->img.top->height;
            img->img.atlas.y_offset = img->img.top->height;
            color_t *new_data = malloc(sizeof(color_t) * (img->img.height + img->img.top->height) * img->img.width);
            if (!new_data) {
                log_error("Error creating image - out of memory", 0, 0);
                unload_image_layers(img);
                return 0;
            }
            memset(new_data, 0, sizeof(color_t) * (footprint_height + img->img.top->height) *img->img.width);
            split_top_and_footprint(&img->img, new_data, data, img->img.height);

            img->img.height = footprint_height;
            free(data);
            data = new_data;
        }
        if (reference_type == IMAGE_FULL_REFERENCE) {
            make_similar_images_references(img);
        }
    }

    unload_image_layers(img);

    img->data = data;

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

void asset_image_count_isometric(void)
{
    data.total_isometric_images++;
}

int asset_image_add_layer(asset_image *img,
    const char *path, const char *group_id, const char *image_id,
    int src_x, int src_y, int offset_x, int offset_y, int width, int height,
    layer_invert_type invert, layer_rotate_type rotate, layer_isometric_part part, layer_mask mask)
{
    layer *current_layer = create_layer_for_image(img);

    if (!current_layer) {
        return 0;
    }
    current_layer->invert = invert;
    current_layer->rotate = rotate;
    current_layer->part = part;
    current_layer->mask = mask;

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
    if (img->last_layer != current_layer) {
        img->last_layer->next = current_layer;
    }
    img->last_layer = current_layer;
    return 1;
}

asset_image *asset_image_get_from_id(int image_id)
{
    asset_image *last = array_last(data.asset_images);
    if (image_id < 0 || !last || image_id > last->index) {
        return 0;
    }
    return array_item(data.asset_images, image_id);
}

void asset_image_unload(asset_image *img)
{
    unload_image_layers(img);
    free((char *) img->id);
    free((color_t *) img->data); // Freeing a const pointer - ugly but necessary
    if (!img->is_reference) {
        free(img->img.top);
    }
    free(img->img.animation);
    img->id = 0;
    img->data = 0;
    img->active = 0;
    memset(&img->img, 0, sizeof(image));
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
    data.total_isometric_images = 0;
    asset_image *image;
    array_foreach(data.asset_images, image) {
        asset_image_unload(image);
    }
    return array_init(data.asset_images, ASSET_ARRAY_SIZE, new_image, is_image_active);
}

asset_image *asset_image_create(void)
{
    asset_image *result;
    array_new_item(data.asset_images, 1, result);
    return result;
}

int asset_image_load_all(color_t **main_images, int *main_image_widths)
{
#ifndef BUILDING_ASSET_PACKER
    image_packer packer;
    int max_width, max_height;
    graphics_renderer()->get_max_image_size(&max_width, &max_height);
    if (image_packer_init(&packer, data.asset_images.size + data.total_isometric_images,
            max_width, max_height) != IMAGE_PACKER_OK) {
        log_error("Failed to init image packer", 0, 0);
        return 0;
    }
    packer.options.fail_policy = IMAGE_PACKER_NEW_IMAGE;
    packer.options.reduce_image_size = 1;
    packer.options.sort_by = IMAGE_PACKER_SORT_BY_AREA;

    asset_image *current_image;
    int rect = 0;
    array_foreach(data.asset_images, current_image) {
        if (current_image->is_reference) {
            continue;
        }
        load_image(current_image, main_images, main_image_widths);
        int top_height = current_image->img.top ? current_image->img.top->height : 0;

        if (graphics_renderer()->should_pack_image(current_image->img.width, current_image->img.height + top_height)) {
            image *img_to_crop = 0;
            if (current_image->img.is_isometric) {
                if (current_image->img.top) {
                    img_to_crop = current_image->img.top;
                }
            } else {
                img_to_crop = &current_image->img;
            }
            if (img_to_crop) {
                image_crop(img_to_crop, current_image->data);
            }
            current_image->img.atlas.id = ATLAS_EXTRA_ASSET << IMAGE_ATLAS_BIT_OFFSET;
            packer.rects[rect].input.width = current_image->img.width;
            packer.rects[rect].input.height = current_image->img.height;

            if (current_image->img.is_isometric && img_to_crop) {
                rect++;
                img_to_crop->atlas.id = ATLAS_EXTRA_ASSET << IMAGE_ATLAS_BIT_OFFSET;
                packer.rects[rect].input.width = img_to_crop->width;
                packer.rects[rect].input.height = img_to_crop->height;
            }

            // Uncrop image for now, crop it later again
            if (img_to_crop) {
                img_to_crop->x_offset = 0;
                img_to_crop->y_offset = 0;
                img_to_crop->width = img_to_crop->original.width;
                img_to_crop->height = img_to_crop->original.height;
            }
            rect++;
        } else {
            current_image->img.atlas.id = ATLAS_UNPACKED_EXTRA_ASSET << IMAGE_ATLAS_BIT_OFFSET;
            if (current_image->img.top) {
                current_image->img.top->atlas.id = ATLAS_UNPACKED_EXTRA_ASSET << IMAGE_ATLAS_BIT_OFFSET;
            }
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
    rect = 0;

    array_foreach(data.asset_images, current_image) {
        int top_height = current_image->img.top ? current_image->img.top->height : 0;

        if (current_image->is_reference) {
            if (current_image->first_layer.calculated_image_id >= IMAGE_MAIN_ENTRIES) {
                translate_reference_position(current_image);
            } else if (image_is_external(image_get(current_image->first_layer.calculated_image_id))) {
                free((color_t *) current_image->data); // Freeing a const pointer - ugly but necessary
                current_image->data = 0;
            }
        } else if (graphics_renderer()->should_pack_image(current_image->img.width, current_image->img.height + top_height)) {
            int original_width = current_image->img.width;
            int original_height = current_image->img.height;
            if (current_image->img.top) {
                original_height += current_image->img.top->original.height;
            }
            int original_y_offset = current_image->img.atlas.y_offset;
            if (!current_image->img.is_isometric) {
                image_crop(&current_image->img, current_image->data);
            }
            current_image->img.atlas.x_offset = packer.rects[rect].output.x;
            current_image->img.atlas.y_offset = packer.rects[rect].output.y;
            current_image->img.atlas.id += packer.rects[rect].output.image_index;
            int dst_side = atlas_data->image_widths[packer.rects[rect].output.image_index];
            image_copy_info copy = {
                .src = { current_image->img.x_offset, current_image->img.y_offset + original_y_offset,
                    original_width, original_height, current_image->data },
                .dst = { current_image->img.atlas.x_offset, current_image->img.atlas.y_offset,
                    dst_side, dst_side, atlas_data->buffers[packer.rects[rect].output.image_index] },
                .rect = { 0, 0, current_image->img.width, current_image->img.height }
            };
            image_copy(&copy);
            if (current_image->img.top) {
                rect++;
                image *top = current_image->img.top;
                int top_width = top->width;
                int top_height = top->original.height;
                top->atlas.x_offset = packer.rects[rect].output.x;
                top->atlas.y_offset = packer.rects[rect].output.y;
                top->atlas.id += packer.rects[rect].output.image_index;
                dst_side = atlas_data->image_widths[packer.rects[rect].output.image_index];
                image_crop(top, current_image->data);
                image_copy_info copy = {
                    .src = { top->x_offset, top->y_offset, top_width, top_height, current_image->data },
                    .dst = { top->atlas.x_offset, top->atlas.y_offset,
                        dst_side, dst_side, atlas_data->buffers[packer.rects[rect].output.image_index] },
                    .rect = { 0, 0, top->width, top->height }
                };
                image_copy(&copy);
            }

            free((color_t *) current_image->data); // Freeing a const pointer - ugly but necessary

            current_image->data = 0;
            rect++;
        } else {
            current_image->img.atlas.id += total_unpacked_assets;
            if (current_image->img.top) {
                current_image->img.top->atlas.id += total_unpacked_assets;
            }
            total_unpacked_assets++;
        }
    }
    image_packer_free(&packer);
    graphics_renderer()->create_image_atlas(atlas_data, 1);
#endif
    return 1;
}

void asset_image_reload_climate(void)
{
#ifndef BUILDING_ASSET_PACKER
    asset_image *current_image;
    array_foreach(data.asset_images, current_image) {
        if (current_image->is_reference && (current_image->img.atlas.id >> IMAGE_ATLAS_BIT_OFFSET) == ATLAS_MAIN) {
            translate_reference_position(current_image);
        }
    }
#endif
}

void asset_image_check_and_handle_reference(asset_image *img)
{
#ifndef BUILDING_ASSET_PACKER
    if (get_image_reference_type(img) != IMAGE_ORIGINAL && img->first_layer.calculated_image_id) {
        img->is_reference = 1;
        if (img->first_layer.calculated_image_id < IMAGE_MAIN_ENTRIES) {
            translate_reference_position(img);
        }
    }
#endif
}
