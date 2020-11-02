#include "layer.h"

#include "core/file.h"
#include "core/log.h"
#include "core/png_read.h"
#include "core/string.h"
#include "graphics/image.h"
#include "graphics/graphics.h"
#include "mods/group.h"
#include "mods/image.h"
#include "mods/xml.h"

#include <stdlib.h>
#include <string.h>

static color_t DUMMY_LAYER_DATA = COLOR_BLACK;

static void load_dummy_layer(layer *l)
{
    l->data = &DUMMY_LAYER_DATA;
    l->width = 1;
    l->height = 1;
    l->is_modded_image_reference = 1;
}

void layer_load(layer *l)
{
    const image *layer_image = image_get(l->original_image_id);
    if (layer_image->draw.type == IMAGE_TYPE_MOD) {
        // Ugly const removal. The only other way would be to memcpy the image data.
        // That's a waste of ram, and we're not going to change l->data anyway
        l->data = (color_t *) image_data(l->original_image_id);
        l->is_modded_image_reference = 1;
        if (!l->data) {
            log_error("Problem loading layer from image id", 0, l->original_image_id);
            load_dummy_layer(l);
        }
        return;
    }
    int size = l->width * l->height * sizeof(color_t);
    l->data = malloc(size);
    if (!l->data) {
        log_error("Problem loading layer", l->modded_image_path, 0);
        load_dummy_layer(l);
        return;
    }
    memset(l->data, 0, size);
    if (l->modded_image_path) {
        if (!png_read(l->modded_image_path, (uint8_t *) l->data)) {
            free(l->data);
            log_error("Problem loading layer from file", l->modded_image_path, 0);
            load_dummy_layer(l);
        }
        return;
    }
    graphics_set_custom_canvas(l->data, l->width, l->height);
    if (layer_image->draw.type == IMAGE_TYPE_ISOMETRIC) {
        int tiles = (l->width + 2) / 60;
        int y_offset = l->height - 30 * tiles;
        y_offset += 15 * tiles - 15;
        if (l->part & PART_FOOTPRINT) {
            image_draw_isometric_footprint_from_draw_tile(l->original_image_id, 0, y_offset, 0);
        }
        if (l->part & PART_TOP) {
            image_draw_isometric_top_from_draw_tile(l->original_image_id, 0, y_offset, 0);
        }
    } else {
        image_draw(l->original_image_id, 0, 0);
    }
    graphics_restore_original_canvas();
}

void layer_unload(layer *l)
{
    free(l->modded_image_path);
    if (!l->is_modded_image_reference) {
        free(l->data);
    }
    if (l->prev) {
        free(l);
    } else {
        l->data = 0;
        l->modded_image_path = 0;
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
    if (l->rotate == ROTATE_90_DEGREES || l->rotate == ROTATE_180_DEGREES) {
        x = l->width - x;
        y = l->height - y;
    }
    if (l->invert & INVERT_HORIZONTAL) {
        x = l->width - x - 1;
    }
    if (l->invert & INVERT_VERTICAL) {
        y = l->height - y - 1;
    }
    if (x < 0 || x >= l->width || y < 0 || y >= l->height) {
        return ALPHA_TRANSPARENT;
    }
    return l->data[y * l->width + x];
}

layer *layer_add_from_image_path(layer *l, const char *path, int offset_x, int offset_y)
{
    if (!l) {
        return 0;
    }
    l->modded_image_path = malloc(FILE_NAME_MAX * sizeof(char));
    xml_get_current_full_path_for_image(l->modded_image_path, path);
    if (!png_get_image_size(l->modded_image_path, &l->width, &l->height)) {
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
    l->width = 0;
    l->height = 0;
    const image *original_image = 0;
    if (strcmp(group_id, "this") == 0) {
        const image_groups *group = group_get_current();
        for (const modded_image *image = group->first_image; image; image = image->next) {
            if (strcmp(image->id, image_id) == 0) {
                l->original_image_id = group->id + image->index;
                original_image = &image->img;
                break;
            }
        }
        if (!l->original_image_id) {
            log_error("Unable to find image on current group with id", image_id, 0);
            layer_unload(l);
            return 0;
        }
    } else {
        int group = string_to_int(string_from_ascii(group_id));
        int id = image_id ? string_to_int(string_from_ascii(image_id)) : 0;
        l->original_image_id = image_group(group) + id;
        original_image = image_get(l->original_image_id);
    }
    if (!original_image) {
        log_error("Unable to find image for group id", group_id, 0);
        layer_unload(l);
        return 0;
    }
    l->width = original_image->width;
    l->height = original_image->height;
    l->x_offset = offset_x;
    l->y_offset = offset_y;
    return l;
}
