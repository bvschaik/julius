#ifndef MODS_IMAGE_H
#define MODS_IMAGE_H

#include "mods/layer.h"
#include "mods/xml.h"

#define IMAGE_PRELOAD_MAX_SIZE 65535

typedef struct modded_image {
    int active;
    int loaded;
    char id[XML_STRING_MAX_LENGTH];
    layer first_layer;
    layer *last_layer;
    image img;
    color_t *data;
    int is_clone;
    int index;
    struct modded_image *next;
} modded_image;

int modded_image_load(modded_image *img);
int modded_image_add_layer(modded_image *img,
    const char *path, const char *group_id, const char *image_id,
    int offset_x, int offset_y,
    layer_invert_type invert, layer_rotate_type rotate, layer_isometric_part part);
void modded_image_unload_layers(modded_image *img);

modded_image *modded_image_get_from_id(int image_id);

#endif // MODS_IMAGE_H
