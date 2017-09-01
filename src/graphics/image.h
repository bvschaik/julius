#ifndef GRAPHICS_IMAGE_H
#define GRAPHICS_IMAGE_H

#include <stdint.h>

/**
 * @file
 * Image functions
 */

/**
 * Image metadata
 */
typedef struct {
    int offset;
    int data_length;
    int uncompressed_length;
    int inverted_image_offset;
    int width;
    int height;
    int num_animation_sprites;
    int sprite_offset_x;
    int sprite_offset_y;
    int animation_can_reverse;
    int type;
    int is_fully_compressed;
    int is_external;
    int has_compressed_part;
    int building_size;
    int bitmap_id;
    int animation_speed_id;
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
 * Loads data from an external image
 * @param image_id ID of the image
 * @return Pointer to data or null. Short-term use only
 */
const uint8_t *image_load_external_data(int image_id);

/**
 * Gets the image id of the first image in the group
 * @param group Image group
 * @return Image id of first image
 */
int image_group(int group);

/**
 * Gets an image by id
 */
const image *image_get(int id);

#endif // GRAPHICS_IMAGE_H
