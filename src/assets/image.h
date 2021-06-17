#ifndef ASSETS_IMAGE_H
#define ASSETS_IMAGE_H

#include "assets/layer.h"
#include "assets/xml.h"

#define IMAGE_PRELOAD_MAX_SIZE 65535

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
} asset_image;

int asset_image_load(asset_image *img);
int asset_image_add_layer(asset_image *img,
    const char *path, const char *group_id, const char *image_id,
    int offset_x, int offset_y,
    layer_invert_type invert, layer_rotate_type rotate, layer_isometric_part part);
void asset_image_unload_layers(asset_image *img);
void asset_image_unload(asset_image *img);

asset_image *asset_image_get_from_id(int image_id);

int asset_image_init_array(void);
asset_image *asset_image_create(void);

#endif // ASSETS_IMAGE_H
