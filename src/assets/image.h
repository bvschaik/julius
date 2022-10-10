#ifndef ASSETS_IMAGE_H
#define ASSETS_IMAGE_H

#include "assets/layer.h"
#include "assets/xml.h"

typedef struct {
    int index;
    int active;
    const char *id;
    layer first_layer;
    layer *last_layer;
    image img;
    const color_t *data;
    int is_reference;
#ifdef BUILDING_ASSET_PACKER
    int has_frame_elements;
    int has_defined_size;
#endif
} asset_image;

void asset_image_check_and_handle_reference(asset_image *img);

int asset_image_add_layer(asset_image *img,
    const char *path, const char *group_id, const char *image_id,
    int src_x, int src_y, int offset_x, int offset_y, int width, int height,
    layer_invert_type invert, layer_rotate_type rotate, layer_isometric_part part, layer_mask mask);
void asset_image_unload(asset_image *img);

asset_image *asset_image_get_from_id(int image_id);

int asset_image_init_array(void);
asset_image *asset_image_create(void);
int asset_image_load_all(color_t **main_images, int *main_image_widths);
void asset_image_reload_climate(void);
void asset_image_count_isometric(void);

void asset_image_copy_isometric_footprint(color_t *dst, const color_t *src, int width, int height,
    int dst_x_offset, int dst_y_offset, int dst_width, int src_x_offset, int src_y_offset, int src_width);
void asset_image_copy_isometric_top(color_t *dst, const color_t *src, int width, int height,
    int dst_x_offset, int dst_y_offset, int dst_width, int src_x_offset, int src_y_offset, int src_width);

#endif // ASSETS_IMAGE_H
