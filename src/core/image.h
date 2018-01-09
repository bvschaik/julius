#ifndef CORE_IMAGE_H
#define CORE_IMAGE_H

#include <stdint.h>

#include "core/image_group.h"
#include "graphics/color.h"

/**
 * @file
 * Image functions
 */

/**
 * Image metadata
 */
typedef struct {
    int width;
    int height;
    int num_animation_sprites;
    int sprite_offset_x;
    int sprite_offset_y;
    int animation_can_reverse;
    int animation_speed_id;
    struct {
        int type;
        int is_fully_compressed;
        int is_external;
        int has_compressed_part;
        int bitmap_id;
        int offset;
        int data_length;
        int uncompressed_length;
    } draw;
} image;

/**
 * Initializes the image system
 */
int image_init();

/**
 * Loads the image collection for the specified climate
 * @param climate_id Climate to load
 * @return boolean true on success, false on failure
 */
int image_load_climate(int climate_id);

/**
 * Loads the image collection for the specified enemy
 * @param enemy_id Enemy to load
 * @return boolean true on success, false on failure
 */
int image_load_enemy(int enemy_id);

/**
 * Gets the image id of the first image in the group
 * @param group Image group
 * @return Image id of first image
 */
int image_group(int group);

/**
 * Gets an image by id
 * @param id Image ID
 * @return Image
 */
const image *image_get(int id);

/**
 * Gets an enemy image by id
 * @param id Enemy image ID
 * @return Enemy image
 */
const image *image_get_enemy(int id);

/**
 * Gets image pixel data by id
 * @param id Image ID
 * @return Pointer to data or null, short term use only.
 */
const color_t *image_data(int id);

/**
 * Gets enemy image pixel data by id
 * @param id Enemy image ID
 * @return Pointer to data or null, short term use only.
 */
const color_t *image_data_enemy(int id);

#endif // CORE_IMAGE_H
