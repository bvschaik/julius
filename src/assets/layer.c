#include "layer.h"

#include "assets/group.h"
#include "assets/image.h"
#include "assets/xml.h"
#include "core/file.h"
#include "core/log.h"
#include "core/png_read.h"
#include "core/string.h"

#include <stdlib.h>
#include <string.h>

static color_t DUMMY_LAYER_DATA = COLOR_BLACK;

static void load_dummy_layer(layer *l)
{
    l->data = &DUMMY_LAYER_DATA;
    l->width = 1;
    l->height = 1;
    l->calculated_image_id = 0;
}

#ifndef BUILDING_ASSET_PACKER
static void convert_layer_to_grayscale(color_t *pixels, int width, int height)
{
    for (int y = 0; y < height; y++) {
        color_t *color = &pixels[y * width];
        for (int x = 0; x < width; x++) {
            color_t r = (*color & COLOR_CHANNEL_RED) >> COLOR_BITSHIFT_RED;
            color_t g = (*color & COLOR_CHANNEL_GREEN) >> COLOR_BITSHIFT_GREEN;
            color_t b = (*color & COLOR_CHANNEL_BLUE) >> COLOR_BITSHIFT_BLUE;
            color_t gray = (color_t) (r * 0.299f + g * 0.587f + b * 0.114f);
            *color = (*color & COLOR_CHANNEL_ALPHA) | (gray << COLOR_BITSHIFT_RED) |
                (gray << COLOR_BITSHIFT_GREEN) | (gray << COLOR_BITSHIFT_BLUE);
            color++;
        }
    }
}

static void load_layer_from_another_image(layer *l, color_t **main_data, int *main_image_widths)
{
    const image *img = image_get(l->calculated_image_id);
    if (!img) {
        log_error("Problem loading layer from image id", 0, l->calculated_image_id);
        load_dummy_layer(l);
        return;
    }
    
    asset_image *asset_img = 0;

    atlas_type type = img->atlas.id >> IMAGE_ATLAS_BIT_OFFSET;
    if (type == ATLAS_EXTRA_ASSET || type == ATLAS_UNPACKED_EXTRA_ASSET ||
        l->calculated_image_id >= IMAGE_MAIN_ENTRIES) {
        asset_img = asset_image_get_from_id(l->calculated_image_id - IMAGE_MAIN_ENTRIES);
        if (!asset_img) {
            log_error("Problem loading layer from image id", 0, l->calculated_image_id);
            load_dummy_layer(l);
            return;
        }
        while (asset_img->is_reference) {
            if (asset_img->first_layer.calculated_image_id >= IMAGE_MAIN_ENTRIES) {
                asset_img = asset_image_get_from_id(asset_img->first_layer.calculated_image_id - IMAGE_MAIN_ENTRIES);
            } else {
                if (type == ATLAS_MAIN) {
                    asset_img = 0;
                } else if (type == ATLAS_EXTERNAL && !asset_img->data) {
                    layer *asset_img_layer = &asset_img->first_layer;
                    int layer_image_id = asset_img_layer->calculated_image_id;
                    layer_load(asset_img_layer, main_data, main_image_widths);
                    asset_img_layer->calculated_image_id = layer_image_id;
                    asset_img->data = asset_img_layer->data;
                    asset_img_layer->data = 0;
                }
                break;
            }
        }
        if (l->mask == LAYER_MASK_NONE && asset_img && asset_img->img.width == l->width &&
            asset_img->img.height == l->height && l->x_offset == 0 && l->y_offset == 0 && type != ATLAS_EXTERNAL &&
            !asset_img->img.is_isometric) {
            l->data = asset_img->data;
            return;
        }
    }

    int width;
    int height;
    if (type == ATLAS_EXTERNAL && !asset_img) {
        image_get_external_dimensions(img, &width, &height);
    } else {
        width = l->width;
        height = l->height;
    }

    int size = width * height * sizeof(color_t);
    color_t *data = malloc(size);
    if (!data) {
        log_error("Problem loading layer from image id - out of memory", 0, l->calculated_image_id);
        load_dummy_layer(l);
        return;
    }
    memset(data, 0, size);

    if (asset_img) {
        int asset_img_width = asset_img->img.width;
        int asset_img_height = asset_img->img.height;
        if (asset_img->img.top) {
            asset_img_height += asset_img->img.top->height;
        }
        // No difference in image data - keep using the original image
        if (!img->is_isometric || ((l->part & PART_FOOTPRINT) && !img->top)) {
            image_copy_info copy = {
                .src = { 0, img->atlas.y_offset, asset_img_width, asset_img_height, asset_img->data },
                .dst = { 0, 0, l->width, l->height, data },
                .rect = { 0, 0, img->width, img->height }
            };
            image_copy(&copy);
        } else {
            int tiles = (img->width + 2) / (FOOTPRINT_WIDTH + 2);
            if ((l->part & PART_TOP) && img->top) {
                image_copy_info copy = {
                    .src = { 0, 0, asset_img_width, asset_img_height, asset_img->data },
                    .dst = { 0, 0, l->width, l->height, data },
                    .rect = { 0, 0, img->top->width, img->top->height }
                };
                image_copy(&copy);
            }
            if (l->part & PART_FOOTPRINT) {
                image_copy_info copy = {
                    .src = { 0, img->atlas.y_offset, asset_img_width, asset_img_height, asset_img->data },
                    .dst = { 0, l->height - tiles * FOOTPRINT_HEIGHT, l->width, l->height, data },
                    .rect = { 0, 0, img->width, img->height }
                };
                image_copy_isometric_footprint(&copy);
            }
        }
    } else if (type == ATLAS_EXTERNAL) {
        if (!image_load_external_pixels(data, img, width)) {
            free(data);
            log_error("Problem loading layer from image id", 0, l->calculated_image_id);
            load_dummy_layer(l);
            return;
        }
        if (l->x_offset != 0 || l->y_offset != 0 || l->width != width || l->height != height) {
            color_t *new_data = malloc(l->width * l->height * sizeof(color_t));
            if (!new_data) {
                free(data);
                log_error("Problem loading layer from image id", 0, l->calculated_image_id);
                load_dummy_layer(l);
                return;
            }
            int src_x_offset = l->x_offset < 0 ? -l->x_offset : 0;
            int src_y_offset = l->y_offset < 0 ? -l->y_offset : 0;
            int rect_x_offset = l->x_offset > 0 ? l->x_offset : 0;
            int rect_y_offset = l->y_offset > 0 ? l->y_offset : 0;
            
            image_copy_info copy = {
                .src = { src_x_offset, src_y_offset, width, height, data },
                .dst = { 0, 0, l->width, l->height, new_data },
                .rect = { rect_x_offset, rect_y_offset, l->width, l->height }
            };
            image_copy(&copy);
            free(data);
            data = new_data;
        }
    } else if (type == ATLAS_MAIN) {
        int atlas_width = main_image_widths[img->atlas.id & IMAGE_ATLAS_BIT_MASK];
        const color_t *atlas_pixels = main_data[img->atlas.id & IMAGE_ATLAS_BIT_MASK];
        if (!atlas_width || !atlas_pixels) {
            free(data);
            log_error("Problem loading layer from image id", 0, l->calculated_image_id);
            load_dummy_layer(l);
            return;
        }
        // No difference in image data - keep using the original image
        if (!img->is_isometric || ((l->part & PART_FOOTPRINT) && !img->top)) {
            image_copy_info copy = {
                .src = { img->atlas.x_offset, img->atlas.y_offset, atlas_width, atlas_width, atlas_pixels },
                .dst = { 0, 0, l->width, l->height, data },
                .rect = { img->x_offset, img->y_offset, img->width, img->height }
            };
            image_copy(&copy);
        } else {
            int tiles = (img->width + 2) / (FOOTPRINT_WIDTH + 2);
            if ((l->part & PART_TOP) && img->top) {
                int top_width = main_image_widths[img->top->atlas.id & IMAGE_ATLAS_BIT_MASK];
                const color_t *top_pixels = main_data[img->top->atlas.id & IMAGE_ATLAS_BIT_MASK];
                image_copy_info copy = {
                    .src = { img->top->atlas.x_offset, img->top->atlas.y_offset, top_width, top_width, top_pixels },
                    .dst = { 0, 0, l->width, l->height, data },
                    .rect = { img->top->x_offset, img->top->y_offset, img->top->width, img->top->height }
                };
                image_copy(&copy);
            }
            if (l->part & PART_FOOTPRINT) {
                image_copy_info copy = {
                    .src = { img->atlas.x_offset, img->atlas.y_offset, atlas_width, atlas_width, atlas_pixels},
                    .dst = { 0, l->height - tiles * FOOTPRINT_HEIGHT, l->width, l->height, data },
                    .rect = { img->x_offset, img->y_offset, img->width, img->height }
                };
                image_copy_isometric_footprint(&copy);
            }
        }
    }
    l->calculated_image_id = 0;

    if (l->mask == LAYER_MASK_GRAYSCALE) {
        convert_layer_to_grayscale(data, l->width, l->height);
    }

    l->data = data;
}
#endif

void layer_load(layer *l, color_t **main_data, int *main_image_widths)
{
#ifndef BUILDING_ASSET_PACKER
    if (l->calculated_image_id) {
        load_layer_from_another_image(l, main_data, main_image_widths);
        return;
    }
#endif
    if (!l->asset_image_path) {
        log_error("No layer source", l->asset_image_path, 0);
        load_dummy_layer(l);
        return;
    }

    int size = l->width * l->height * sizeof(color_t);
    color_t *data = malloc(size);
    if (!data) {
        log_error("Problem loading layer - out of memory", l->asset_image_path, 0);
        load_dummy_layer(l);
        return;
    }
    memset(data, 0, size);
    if (!png_read(l->asset_image_path, data, l->src_x, l->src_y, l->width, l->height, 0, 0, l->width, 0)) {
        free(data);
        log_error("Problem loading layer from file", l->asset_image_path, 0);
        load_dummy_layer(l);
        return;
    }
#ifndef BUILDING_ASSET_PACKER
    if (l->mask == LAYER_MASK_GRAYSCALE) {
        convert_layer_to_grayscale(data, l->width, l->height);
    }
#endif

    l->data = data;
}

void layer_unload(layer *l)
{
    free(l->asset_image_path);
#ifdef BUILDING_ASSET_PACKER
    free(l->original_image_group);
    free(l->original_image_id);
#endif
    if (!l->calculated_image_id && l->data != &DUMMY_LAYER_DATA) {
        free((color_t *)l->data); // Freeing a const pointer. Ugly but necessary
    }
    if (l->prev) {
        free(l);
    } else {
        memset(l, 0, sizeof(layer));
    }
}

const color_t *layer_get_color_for_image_position(const layer *l, int x, int y)
{
    x -= l->x_offset;
    y -= l->y_offset;

    if (l->rotate == ROTATE_90_DEGREES || l->rotate == ROTATE_270_DEGREES) {
        int temp = x;
        x = y;
        y = temp;
    }
    layer_invert_type invert = l->invert;
    if (l->rotate == ROTATE_90_DEGREES) {
        invert ^= INVERT_VERTICAL;
    } else if (l->rotate == ROTATE_180_DEGREES) {
        invert ^= INVERT_BOTH;
    } else if (l->rotate == ROTATE_270_DEGREES) {
        invert ^= INVERT_HORIZONTAL;
    }
    if (invert & INVERT_HORIZONTAL) {
        x = l->width - x - 1;
    }
    if (invert & INVERT_VERTICAL) {
        y = l->height - y - 1;
    }
    return &l->data[y * l->width + x];
}

int layer_add_from_image_path(layer *l, const char *path,
    int src_x, int src_y, int offset_x, int offset_y, int width, int height)
{
    l->src_x = src_x;
    l->src_y = src_y;
    l->width = width;
    l->height = height;
    l->calculated_image_id = 0;
    l->asset_image_path = malloc(FILE_NAME_MAX * sizeof(char));
    if (path) {
        xml_get_full_image_path(l->asset_image_path, path);
    } else {
        snprintf(l->asset_image_path, FILE_NAME_MAX, "%s.png", group_get_current()->name);
    }
#ifndef BUILDING_ASSET_PACKER
    if (!l->width || !l->height) {
        if (!png_get_image_size(l->asset_image_path, &width, &height)) {
            log_info("Unable to load image", path, 0);
            layer_unload(l);
            return 0;
        }
        if (!l->width) {
            l->width = width;
        }
        if (!l->height) {
            l->height = height;
        }
    }
#endif
    l->x_offset = offset_x;
    l->y_offset = offset_y;
    return 1;
}

#ifdef BUILDING_ASSET_PACKER
static char *copy_attribute(const char *attribute)
{
    if (!attribute) {
        return 0;
    }
    char *dest = malloc((strlen(attribute) + 1) * sizeof(char));
    if (!dest) {
        log_error("There was no memory to copy the attribute", attribute, 0);
        return 0;
    }
    strcpy(dest, attribute);
    return dest;
}
#endif

#ifndef BUILDING_ASSET_PACKER
static int determine_layer_height(const image *img, layer_isometric_part part)
{
    if (!img->is_isometric) {
        return img->height + img->y_offset;
    }
    switch (part) {
        case PART_BOTH:
        case PART_FOOTPRINT:
            // The original top height already includes half the footprint height
            return img->top ? img->top->original.height + img->height / 2 : img->height;
        case PART_TOP:
            return img->top ? img->top->original.height : 0;
        default:
            return 0;
    }
}
#endif

int layer_add_from_image_id(layer *l, const char *group_id, const char *image_id, int offset_x, int offset_y)
{
    l->src_x = 0;
    l->src_y = 0;
    l->width = 0;
    l->height = 0;
    l->x_offset = offset_x;
    l->y_offset = offset_y;
#ifdef BUILDING_ASSET_PACKER
    l->original_image_group = copy_attribute(group_id);
    l->original_image_id = copy_attribute(image_id);
#else
    const image *original_image = 0;
    if (strcmp(group_id, "this") == 0) {
        if (!image_id) {
            log_error("No image ID provided for the current layer", 0, 0);
            layer_unload(l);
            return 0;
        }
        const image_groups *group = group_get_current();
        const asset_image *image = asset_image_get_from_id(group->first_image_index);
        while (image && image->index <= group->last_image_index) {
            if (image->id && strcmp(image->id, image_id) == 0) {
                l->calculated_image_id = image->index + IMAGE_MAIN_ENTRIES;
                original_image = &image->img;
                break;
            }
            image = asset_image_get_from_id(image->index + 1);
        }
        if (!l->calculated_image_id) {
            log_error("Unable to find image on current group with id", image_id, 0);
            layer_unload(l);
            return 0;
        }
    } else {
        int group = atoi(group_id);
        if (group >= 0 && group < IMAGE_MAX_GROUPS) {
            int id = image_id ? atoi(image_id) : 0;
            l->calculated_image_id = image_group(group) + id;
        } else {
            log_info("Image group is out of range", group_id, 0);
        }
        if (l->calculated_image_id >= 0 && l->calculated_image_id < IMAGE_MAIN_ENTRIES) {
            original_image = image_get(l->calculated_image_id);
        } else {
            log_info("Image id is out of range", 0, l->calculated_image_id);
            l->calculated_image_id = 0;
        }
    }
    if (!original_image) {
        log_error("Unable to find image for group id", group_id, 0);
        layer_unload(l);
        return 0;
    }
    l->width = original_image->width + original_image->x_offset;
    l->height = determine_layer_height(original_image, l->part);
#endif
    return 1;
}
