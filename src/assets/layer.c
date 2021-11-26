#include "layer.h"

#include "assets/group.h"
#include "assets/image.h"
#include "assets/xml.h"
#include "core/file.h"
#include "core/log.h"
#include "core/png_read.h"
#include "core/string.h"
#include "graphics/image.h"
#include "graphics/graphics.h"

#include <stdlib.h>
#include <string.h>

static color_t DUMMY_LAYER_DATA = COLOR_BLACK;

static void load_dummy_layer(layer *l)
{
    l->data = &DUMMY_LAYER_DATA;
    l->width = 1;
    l->height = 1;
    l->is_asset_image_reference = 1;
}

void layer_load(layer *l)
{
#ifndef BUILDING_ASSET_PACKER
    const image *layer_image = image_get(l->calculated_image_id);
    if (layer_image->draw.type == IMAGE_TYPE_EXTRA_ASSET) {
        // Ugly const removal. The only other way would be to memcpy the image data.
        // That's a waste of ram, and we're not going to change l->data anyway
        l->data = (color_t *) image_data(l->calculated_image_id);
        l->is_asset_image_reference = 1;
        if (!l->data) {
            log_error("Problem loading layer from image id", 0, l->calculated_image_id);
            load_dummy_layer(l);
        }
        return;
    }
#endif
    int size = l->width * l->height * sizeof(color_t);
    l->data = malloc(size);
    if (!l->data) {
        log_error("Problem loading layer", l->asset_image_path, 0);
        load_dummy_layer(l);
        return;
    }
    memset(l->data, 0, size);
    if (l->asset_image_path) {
        if (!png_read(l->asset_image_path, l->data, l->src_x, l->src_y, l->width, l->height, 0, 0, l->width, 0)) {
            free(l->data);
            log_error("Problem loading layer from file", l->asset_image_path, 0);
            load_dummy_layer(l);
        }
        return;
    }
#ifndef BUILDING_ASSET_PACKER
    graphics_set_custom_canvas(l->data, l->width, l->height);
    if (layer_image->draw.type == IMAGE_TYPE_ISOMETRIC) {
        int tiles = (l->width + 2) / 60;
        int y_offset = l->height - 30 * tiles;
        y_offset += 15 * tiles - 15;
        if (l->part & PART_FOOTPRINT) {
            image_draw_isometric_footprint_from_draw_tile(l->calculated_image_id, 0, y_offset, 0);
        }
        if (l->part & PART_TOP) {
            image_draw_isometric_top_from_draw_tile(l->calculated_image_id, 0, y_offset, 0);
        }
    } else {
        image_draw(l->calculated_image_id, 0, 0);
    }
    graphics_restore_original_canvas();
#endif
}

void layer_unload(layer *l)
{
    free(l->asset_image_path);
#ifdef BUILDING_ASSET_PACKER
    free(l->original_image_group);
    free(l->original_image_id);
#endif
    if (!l->is_asset_image_reference) {
        free(l->data);
    }
    if (l->prev) {
        free(l);
    } else {
        l->data = 0;
        l->asset_image_path = 0;
#ifdef BUILDING_ASSET_PACKER
        l->original_image_group = 0;
        l->original_image_id = 0;
#endif
    }
}

color_t layer_get_color_for_image_position(const layer *l, int x, int y)
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
    }
    if (invert & INVERT_HORIZONTAL) {
        x = l->width - x - 1;
    }
    if (invert & INVERT_VERTICAL) {
        y = l->height - y - 1;
    }
    if (x < 0 || x >= l->width || y < 0 || y >= l->height) {
        return ALPHA_TRANSPARENT;
    }
    return l->data[y * l->width + x];
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

layer *layer_add_from_image_path(layer *l, const char *path,
    int src_x, int src_y, int offset_x, int offset_y, int width, int height)
{
    if (!l) {
        return 0;
    }
    l->src_x = src_x;
    l->src_y = src_y;
    l->width = width;
    l->height = height;
    l->asset_image_path = malloc(FILE_NAME_MAX * sizeof(char));
    if (path) {
        xml_get_full_image_path(l->asset_image_path, path);
    } else {
        snprintf(l->asset_image_path, FILE_NAME_MAX, "%s.png", group_get_current()->name);
    }

    if ((!l->width || !l->height) && !png_get_image_size(l->asset_image_path, &l->width, &l->height)) {
        log_info("Unable to load image", path, 0);
        layer_unload(l);
        return 0;
    }
    l->x_offset = offset_x;
    l->y_offset = offset_y;
    return l;
}

layer *layer_add_from_image_id(layer *l, const char *group_id, const char *image_id, int offset_x, int offset_y)
{
    if (!l) {
        return 0;
    }
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
        const image_groups *group = group_get_current();
        const asset_image *image = asset_image_get_from_id(group->first_image_index);
        while (image && image->index <= group->last_image_index) {
            if (strcmp(image->id, image_id) == 0) {
                l->calculated_image_id = image->index + MAIN_ENTRIES;
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
        int group = string_to_int(string_from_ascii(group_id));
        int id = image_id ? string_to_int(string_from_ascii(image_id)) : 0;
        l->calculated_image_id = image_group(group) + id;
        original_image = image_get(l->calculated_image_id);
    }
    if (!original_image) {
        log_error("Unable to find image for group id", group_id, 0);
        layer_unload(l);
        return 0;
    }
    l->width = original_image->width;
    l->height = original_image->height;
#endif
    return l;
}
