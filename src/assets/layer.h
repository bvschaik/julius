#ifndef ASSETS_LAYER_H
#define ASSETS_LAYER_H

#include "core/image.h"
#include "graphics/color.h"
#include "graphics/renderer.h"

typedef enum {
    INVERT_NONE = 0,
    INVERT_HORIZONTAL = 1,
    INVERT_VERTICAL = 2,
    INVERT_BOTH = 3
} layer_invert_type;

typedef enum {
    ROTATE_NONE = 0,
    ROTATE_90_DEGREES = 1,
    ROTATE_180_DEGREES = 2,
    ROTATE_270_DEGREES = 3
} layer_rotate_type;

typedef enum {
    PART_NONE = 0,
    PART_FOOTPRINT = 1,
    PART_TOP = 2,
    PART_BOTH = 3
} layer_isometric_part;

typedef enum {
    LAYER_MASK_NONE = 0,
    LAYER_MASK_GRAYSCALE = 1,
    LAYER_MASK_ALPHA = 2
} layer_mask;

typedef struct layer {
    char *asset_image_path;
    int calculated_image_id;
    int src_x;
    int src_y;
    int x_offset;
    int y_offset;
    int width;
    int height;
    layer_invert_type invert;
    layer_rotate_type rotate;
    layer_isometric_part part;
    layer_mask mask;
    const color_t *data;
    struct layer *prev;
    struct layer *next;
    // Extra layer information specific for the asset packer
#ifdef BUILDING_ASSET_PACKER
    char *original_image_group;
    char *original_image_id;
#endif
} layer;

void layer_load(layer *l, color_t **main_data, int *main_image_widths);
void layer_unload(layer *l);

const color_t *layer_get_color_for_image_position(const layer *l, int x, int y);

int layer_add_from_image_path(layer *l, const char *path, int src_x, int src_y,
    int offset_x, int offset_y, int width, int height);
int layer_add_from_image_id(layer *l, const char *group_id, const char *image_id, int offset_x, int offset_y);

#endif // ASSETS_LAYER_H
