#ifndef ASSETS_IMAGE_H
#define ASSETS_IMAGE_H

#include "assets/layer.h"
#include "assets/xml.h"

typedef struct {
    int index;
    int active;
    int loaded;
    char id[XML_STRING_MAX_LENGTH];
    layer first_layer;
    layer *last_layer;
    image img;
    color_t *data;
    int is_clone;
#ifdef BUILDING_ASSET_PACKER
    int has_frame_elements;
    int has_defined_size;
#endif
} asset_image;

int asset_image_load(asset_image *img);
int asset_image_add_layer(asset_image *img,
    const char *path, const char *group_id, const char *image_id,
    int src_x, int src_y, int offset_x, int offset_y, int width, int height,
    layer_invert_type invert, layer_rotate_type rotate, layer_isometric_part part);
void asset_image_unload_layers(asset_image *img);
void asset_image_unload(asset_image *img);

asset_image *asset_image_get_from_id(int image_id);

int asset_image_init_array(void);
asset_image *asset_image_create(void);

#endif // ASSETS_IMAGE_H
